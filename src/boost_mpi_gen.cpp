// mpic++ -O3 -lboost_mpi -lboost_serialization -I. boost_mpi_gen.cpp -std=c++14 -o boost_gen
// mpirun -np 4 ./boost_gen
#include <iostream>
#include <memory>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>
#include <climits>

#include <boost/mpi.hpp>
#include <boost/mpi/timer.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#ifdef _OPENMP
#include <omp.h>       // omp_get_max_threads
#endif

//const std::string TARGET = "I'm a genetic algorithm";
const std::string TARGET = "Nel mezzo del cammin di nostra vita "
                            "mi ritrovai per una selva oscura, "
                            "che la diritta via era smarrita. "
                            "Ahi quanto a dir qual era e cosa dura, "
                            "esta selva selvaggia e aspra e forte, "
                            "che nel pensier rinova la paura! "
                            ;/*
                            "Tant'e amara che poco e piu morte; "
                            "ma per trattar del ben ch'i' vi trovai, "
                            "diro de l'altre cose ch'i' v'ho scorte. "
                            "Io non so ben ridir com'i' v'intrai, "
                            "tant'era pien di sonno a quel punto "
                            "che la verace via abbandonai.";
                            */
const std::size_t LENGTH = TARGET.size();
static constexpr int root        = 0;
static constexpr int rng_seed    = 123;
static constexpr float inf       = std::numeric_limits<float>::infinity();
static constexpr float fit_limit = 0.f;

#include <mystr.h>
#ifdef _OPENMP
#include <merge_sort.h>
#endif


template<typename genome>
void master(boost::mpi::communicator &world, const int &n_process,
            const int* block_pop, const genome &target,
            const int &n_population, const int &max_iters,
            const bool &verbose)
{
  const int n_slave = n_process - 1;
  const int tag_pop  = 1;
  const int tag_fit  = 2;
  const int tag_stop = 3;

  boost::mpi::timer start_time;
  //auto start_time = std::chrono::high_resolution_clock::now();

  // genetic variables for message passing
  std::unique_ptr<genome[]> pop    (new genome[n_population]);
  std::unique_ptr<float[]>  fitness(new float [n_slave]);

  bool stop = false;
  int iter = 0,
      best = 0,
      start, m_size;
  float best_fit;
  boost::mpi::request requests_pop[n_slave],
                      requests_fit[n_slave];
  std::mt19937 engine(rng_seed);
  std::uniform_real_distribution<float> rng;

  while(true)
  {
    best_fit = inf;
    // receive the executed job from slaves
    for (int i = 0, slave = 1; i < n_slave; ++i, ++slave)
    {
      start = i * block_pop[i];
      requests_pop[i]  = world.irecv(slave, tag_pop,  pop.get()     + start, block_pop[i]);
      requests_fit[i]  = world.irecv(slave, tag_fit,  fitness[i]);
    }
    boost::mpi::wait_all(requests_fit,  requests_fit  + n_slave);
    // compute the minimum of fitness
    for(int i = 0; i < n_slave; ++i)
    {
      best     = (fitness[i] < best_fit) ? i          : best;
      best_fit = (fitness[i] < best_fit) ? fitness[i] : best_fit;
    }
    best *= block_pop[best]; // position in the population array

    boost::mpi::wait_all(requests_pop,  requests_pop  + n_slave);
    if (verbose) printProgress(match(pop[best], target),
                               LENGTH, start_time);

    ++iter;
    stop = (iter >= max_iters) || (best_fit <= fit_limit);
    // send arrest to the slaves
    for (int i = 0, slave = 1; i < n_slave; ++i, ++slave)
    {
      start = i * block_pop[i];
      world.isend(slave, tag_stop, stop);
    }
    if (stop) break;

    // send the population to the slaves
    for (int i = 0, slave = 1; i < n_slave; ++i, ++slave)
    {
      start = i * block_pop[i];
      // select the next population block for "shuffle"
      int pop_ = slave % n_slave;
      // random select the position to switch
      int pos_ = static_cast<int>(rng(engine) * (m_size = std::min(block_pop[i], block_pop[pop_])));
      // swap ranges for population shuffle
      std::swap_ranges(pop.get() + start + pos_,
                       pop.get() + start + m_size,
                       pop.get() + pop_ * block_pop[pop_] + pos_);
      // send the population block to the slaves
      world.isend(slave, tag_pop,  pop.get() + start, block_pop[i]);
    }
  } // end while iters

  if (verbose) std::cout << std::endl;
  // write output here
  std::cout << "Best found: "
              << std::endl
              << pop[best]
              << std::endl;
  return;
}


template<typename genome, typename Func>
void slave(boost::mpi::communicator &world, const int &pID,
           const genome &target, const int &n_population,
           const int &max_iters, const float &elit_rate,
           const float &mut_rate, Func fit
           )
{
#ifdef _OPENMP
  int nth = omp_get_max_threads();
  nth -= nth % 2;
  const int diff_size = n_population % nth,
            size      = diff_size                ?
                        n_population - diff_size :
                        n_population;
#endif

  //using res_t = typename std::result_of<Func(const mystring &, const mystring &)>::type; // since c++17
  const int seed = rng_seed + pID;
  const int elite = n_population * elit_rate;
  const int tag_pop  = 1;
  const int tag_fit  = 2;
  const int tag_stop = 3;
  int best     = 0,
      sub_iter = 0,
      idx; // tmp variable for the first iteration after recv new population
  bool stop = false;

  // genetic variables for message passing
  std::unique_ptr<mystring[]> population(new genome[n_population]),
                              new_gen   (new genome[n_population]);
  std::unique_ptr<int[]>      rank      (new int   [n_population]);
  std::unique_ptr<float[]>    fitness   (new float [n_population]);

  std::mt19937 engine(seed);
  std::uniform_real_distribution<float> rng;

#ifdef _OPENMP
#pragma omp parallel
  {
#endif

#ifdef _OPENMP
#pragma omp for
  for(int i = 0; i < n_population; ++i)
    population[i] = genome(rng(engine));
#else
  std::generate_n(population.get(), n_population,
                  [&](){return genome(rng(engine));});
#endif

  // compute sub-genetic algorithm
  while(sub_iter <= max_iters)
  {
#ifdef _OPENMP
#pragma omp for
#endif
    for(int i = 0; i < n_population; ++i)
    {
      // argsort variables
      fitness[i] = fit(population[i], target);
      rank[i]    = i;
    }

#ifdef _OPENMP
#pragma omp single
    {
      mergeargsort_parallel_omp(rank.get(), fitness.get(), 0, size, nth, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
      if (diff_size)
      {
        std::sort(rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
        std::inplace_merge(rank.get(), rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
      }
    }
#else
    std::sort(rank.get(), rank.get() + n_population,
              [&](const int &a1, const int &a2)
              {
                return fitness[a1] < fitness[a2];
              });
#endif

    best = rank[0];
    if (fitness[best] <= fit_limit) break;

#ifdef _OPENMP
#pragma omp for
#endif
    for(int i = 0; i < n_population; ++i)
    {
      // crossover
      new_gen[i] = ( rank[i] < elite ) ? population[rank[i]]
                                       : population[rank[static_cast<int>(rng(engine) * elite)]] + population[rank[static_cast<int>(rng(engine) * elite)]];
      // random mutation
      if (rng(engine) < mut_rate) !new_gen[i];
    }

#ifdef _OPENMP
#pragma omp for
    for(int i = 0; i < n_population; ++i) population[i] = new_gen[i];
#else
    std::copy_n(new_gen.get(), n_population, population.get());
#endif

#ifdef _OPENMP
#pragma omp single
#endif
    ++sub_iter;
  }

#ifdef _OPENMP
#pragma omp for
#endif
  for(int i = 0; i < n_population; ++i)
    new_gen[i] = population[rank[i]];

  // send update population
#ifdef _OPENMP
#pragma omp single
  {
#endif
  world.send(root, tag_pop,  new_gen.get(), n_population);
  world.send(root, tag_fit,  fitness[best]);
#ifdef _OPENMP
  }
#endif

  while(true)
  {
    // receive iter value
#ifdef _OPENMP
#pragma omp single
    {
#endif
    world.recv(root, tag_stop, stop);
    if (stop) break;
    world.recv(root, tag_pop, population.get(), n_population);
#ifdef _OPENMP
    }
#endif

    sub_iter = 0;
    // compute sub-genetic algorithm
    while(sub_iter <= max_iters)
    {
#ifdef _OPENMP
#pragma omp for private(idx)
#endif
      for(int i = 0; i < n_population; ++i)
      {
        idx = (sub_iter == 0 ) ? i : rank[i];
        // crossover
/*ERR*/ new_gen[i] = ( idx < elite ) ? population[idx]
                                     : population[rank[static_cast<int>(rng(engine) * elite)]] + population[rank[static_cast<int>(rng(engine) * elite)]];
        // random mutation
        if (rng(engine) < mut_rate) !new_gen[i];
      }

#ifdef _OPENMP
#pragma omp for
      for(int i = 0; i < n_population; ++i) population[i] = new_gen[i];
#else
      std::copy_n(new_gen.get(), n_population, population.get());
#endif

#ifdef _OPENMP
#pragma omp for
#endif
      for(int i = 0; i < n_population; ++i)
      {
        // argsort variables
        fitness[i] = fit(population[i], target);
        rank[i]    = i;
      }

#ifdef _OPENMP
#pragma omp single
      {
        mergeargsort_parallel_omp(rank.get(), fitness.get(), 0, size, nth, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
        if (diff_size)
        {
          std::sort(rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
          std::inplace_merge(rank.get(), rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
        }
      }
#else
      std::sort(rank.get(), rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
#endif

      best = rank[0];
      if (fitness[best] <= fit_limit) break;
#ifdef _OPENMP
#pragma omp single
#endif
      ++sub_iter;
    }

#ifdef _OPENMP
#pragma omp for
#endif
    for(int i = 0; i < n_population; ++i)
      new_gen[i] = population[rank[i]];
    // send update population
#ifdef _OPENMP
#pragma omp single
    {
#endif
    world.send(root, tag_pop,  new_gen.get(), n_population);
    world.send(root, tag_fit,  fitness[best]);
#ifdef _OPENMP
    }
#endif
  }

#ifdef _OPENMP
  } // end parallel section
#endif

  return;
}


int main(int argc, char **argv)
{
  const bool verbose     = true;
  const int n_population = 50, // 85 ???
            max_iters    = 1000,
            rng_seed     = 123;
  const float mut_rate   = .3f,
              elit_rate  = .1f;

  mystring target(TARGET);
  std::srand(rng_seed);

  // Initialize the MPI environment
  boost::mpi::environment env(argc, argv);
  boost::mpi::communicator world;
  // Get the number of processes
  const int n_process = world.size();
  // Get the rank of the process
  const int pID       = world.rank();
  const int n_slave   = n_process - 1;
  const int sub_iter  = 100;

  // divide population size in "equal" parts
  std::unique_ptr<int[]> block_pop(new int[n_slave]);
  std::div_t dv {};
  dv = std::div(n_population, n_slave);
  std::fill_n(block_pop.get(), n_slave, dv.quot);
  std::fill_n(block_pop.get(), dv.rem,  dv.quot + 1);

  switch(pID)
  {
    case root: master(world, n_process, block_pop.get(), target, n_population, max_iters, verbose);
    break;
    default:   slave(world, pID, target, block_pop[pID - 1], sub_iter, elit_rate, mut_rate, fitness);
    break;
  }

  return 0;
}
