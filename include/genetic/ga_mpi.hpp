#include <genetic/ga_mpi.h>
#include <sorting/merge_sort.hpp>

#ifdef _MPI

static constexpr int root        = 0;
static constexpr float inf       = std :: numeric_limits <float> :: infinity();
static constexpr float fit_limit = 0.f;

template < int cross_t, typename genome, typename Func >
genome ga_mpi(int argc, char * argv[], const genome & target, const std :: size_t & n_population, const std :: size_t & max_iters, Func fit,
              std :: size_t num_mutation,
              float elite_rate,
              float mutation_rate,
              std :: size_t sub_iter,
              std :: size_t seed,
              __unused int nth)
{
  std :: srand(seed);

  // Initialize the MPI environment
  boost :: mpi :: environment env(argc, argv);
  boost :: mpi :: communicator world;
  // Get the number of processes
  const int n_process = world.size();
  // Get the rank of the process
  const int pID       = world.rank();
  const int n_slave   = n_process - 1;

  // divide population size in "equal" parts
  std :: unique_ptr < int[] > block_pop(new int[n_slave]);
  std :: div_t dv {};
  dv = std :: div(n_population, n_slave);
  std :: fill_n(block_pop.get(), n_slave, dv.quot);
  std :: fill_n(block_pop.get(), dv.rem,  dv.quot + 1);

  genome best;

  switch (pID)
  {
    case root:
    {
      best = master < genome >(world, n_process, block_pop.get(), n_population, max_iters, seed);
    } break;
    default:
    {
             slave < cross_t >(world, pID, target, block_pop[pID - 1], sub_iter, num_mutation, elite_rate, mutation_rate, fit, seed, nth);
    } break;
  }

  return best;
}


template < typename genome >
genome master(boost :: mpi :: communicator & world, const int & n_process,
              const int * block_pop,
              const std :: size_t & n_population, const std :: size_t & max_iters,
              const std :: size_t & seed)
{
  const std :: size_t n_slave = n_process - 1;
  const std :: size_t tag_pop  = 1;
  const std :: size_t tag_fit  = 2;
  const std :: size_t tag_stop = 3;

  boost :: mpi :: timer start_time;
  //auto start_time = std :: chrono :: high_resolution_clock :: now();

  // genetic variables for message passing
  std :: unique_ptr < genome[] > pop    (new genome[n_population]);
  std :: unique_ptr < float[] >  fitness(new float [n_slave]);

  bool stop = false;
  std :: size_t iter = 0;
  int best = 0,
      start, m_size;
  float best_fit;
  boost :: mpi :: request *requests_pop = new boost :: mpi :: request[n_slave],
                          *requests_fit = new boost :: mpi :: request[n_slave];
  std :: mt19937 engine(seed);
  std :: uniform_real_distribution < float > rng;

#ifdef VERBOSE
  ProgressBar progress_ = 0;
#endif

#ifdef VERBOSE
  for ( ; progress_ != max_iters; )
#else
  while (iter <= max_iters)
#endif
  {
    best_fit = inf;
    // receive the executed job from slaves
    for (std :: size_t i = 0, slave = 1; i < n_slave; ++i, ++slave)
    {
      start = i * block_pop[i];
      requests_pop[i]  = world.irecv(slave, tag_pop,  pop.get()     + start, block_pop[i]);
      requests_fit[i]  = world.irecv(slave, tag_fit,  fitness[i]);
    }
    boost :: mpi :: wait_all(requests_fit,  requests_fit  + n_slave);
    // compute the minimum of fitness
    for (std :: size_t i = 0; i < n_slave; ++i)
    {
      best     = (fitness[i] < best_fit) ? i          : best;
      best_fit = (fitness[i] < best_fit) ? fitness[i] : best_fit;
    }
    best *= block_pop[best]; // position in the population array

    boost :: mpi :: wait_all(requests_pop,  requests_pop  + n_slave);

#ifdef VERBOSE

#ifdef _OPENMP
#pragma omp single nowait
      {
#endif
        progress_.update(best_fit);
        progress_.print();
#ifdef _OPENMP
      }
#endif

#endif

    ++ iter;
    stop = (iter >= max_iters) || (best_fit <= fit_limit);
    // send arrest to the slaves
    for (std :: size_t i = 0, slave = 1; i < n_slave; ++i, ++slave)
    {
      start = i * block_pop[i];
      world.isend(slave, tag_stop, stop);
    }
    if (stop) break;

    // send the population to the slaves
    for (std :: size_t i = 0, slave = 1; i < n_slave; ++i, ++slave)
    {
      start = i * block_pop[i];
      // select the next population block for "shuffle"
      int pop_ = slave % n_slave;
      // random select the position to switch
      int pos_ = static_cast < int >(rng(engine) * (m_size = std :: min(block_pop[i], block_pop[pop_])));
      // swap ranges for population shuffle
      std :: swap_ranges(pop.get() + start + pos_,
                         pop.get() + start + m_size,
                         pop.get() + pop_ * block_pop[pop_] + pos_);
      // send the population block to the slaves
      world.isend(slave, tag_pop,  pop.get() + start, block_pop[i]);
    }
  } // end while iters

#ifdef VERBOSE
  progress_.breaks();
#endif


  delete[] requests_pop;
  delete[] requests_fit;

  return pop[best];
}


template < int cross_t, typename genome, typename Func >
void slave(boost :: mpi :: communicator & world, const int & pID,
           const genome & target, const std :: size_t & n_population,
           const std :: size_t & max_iters, const std :: size_t & num_mutation, const float & elite_rate,
           const float & mutation_rate, Func fit, const std :: size_t & seed,
           __unused int nth
           )
{
#ifdef _OPENMP
  nth -= nth % 2;
  const int diff_size = n_population % nth,
            size      = diff_size                ?
                        n_population - diff_size :
                        n_population;
#endif

  const std :: size_t LENGHT = target.size();

  const int s_seed   = seed + pID;
  const int elite    = n_population * elite_rate;
  const int tag_pop  = 1;
  const int tag_fit  = 2;
  const int tag_stop = 3;
  std :: size_t sub_iter = 0;
  int best     = 0,
      idx; // tmp variable for the first iteration after recv new population
  bool stop = false;

  // genetic variables for message passing
  std :: unique_ptr < mystring[] > population(new genome[n_population]),
                                   new_gen   (new genome[n_population]);
  std :: unique_ptr < int[] >      rank      (new int   [n_population]);
  std :: unique_ptr < float[] >    fitness   (new float [n_population]);


#ifdef _OPENMP
#pragma omp parallel num_threads(nth)
  {
#endif

#ifdef _OPENMP
    int th_id = omp_get_thread_num();
#else
    int th_id = 0;
#endif

    std :: mt19937 engine(s_seed + th_id);
    std :: uniform_real_distribution < float > rng;

#ifdef _OPENMP
#pragma omp for
    for (std :: size_t i = 0; i < n_population; ++i) population[i] = genome :: randomize(genome :: gen_random, LENGHT, std :: rand());
#else
    std :: generate_n(population.get(), n_population, [&](){return genome :: randomize(genome :: gen_random, LENGHT, std :: rand());});
#endif

    // compute sub-genetic algorithm
    while (sub_iter <= max_iters)
    {

#ifdef _OPENMP

#pragma omp for
      for (std :: size_t i = 0; i < n_population; ++i)
      {
        // argsort variables
        fitness[i] = fit(population[i], target);
        rank[i]    = i;
      }

#else
      std :: transform(population.get(), population.get() + n_population,
                       fitness.get(),
                       [&](const genome & pop)
                       {
                        return fit(pop, target);
                       });
      std :: iota(rank.get(), rank.get() + n_population, 0);

#endif

#ifdef _OPENMP
#pragma omp single
      {
        mergeargsort_parallel_omp(rank.get(), fitness.get(), 0, size, nth, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
        if (diff_size)
        {
          std :: sort(rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
          std :: inplace_merge(rank.get(), rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
        }
      }
#else
      std :: sort(rank.get(), rank.get() + n_population,
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
      for(std :: size_t i = 0; i < n_population; ++i)
      {
        const int r1 = static_cast < int >(rng(engine) * elite);
        const int r2 = static_cast < int >(rng(engine) * elite);

        const int rank_i = rank[i];
        // crossover
        new_gen[i] = ( rank_i < elite ) ? population[ rank_i ]
                                        : population[ rank[r1] ].template crossover < cross_t > ( population[ rank[r2] ] );

        // random mutation
        if ( rng(engine) < mutation_rate ) new_gen[i].mutate(genome :: gen_random, num_mutation);
      }

#ifdef _OPENMP
#pragma omp for
      for (std :: size_t i = 0; i < n_population; ++i) population[i] = std :: move(new_gen[i]);
#else
      std :: move(new_gen.get(), new_gen.get() + n_population, population.get());
#endif

#ifdef _OPENMP
#pragma omp single
#endif
      ++ sub_iter;
    }

#ifdef _OPENMP
#pragma omp for
#endif
    for (std :: size_t i = 0; i < n_population; ++i)
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

    while (true)
    {
      // receive iter value
#ifdef _OPENMP
#pragma omp single
      {
#endif
        world.recv(root, tag_stop, stop);
#ifdef _OPENMP
      }
#endif

      if (stop) break;

#ifdef _OPENMP
      {
#endif
        world.recv(root, tag_pop, population.get(), n_population);
#ifdef _OPENMP
      }
#endif

      sub_iter = 0;
      // compute sub-genetic algorithm
      while (sub_iter <= max_iters)
      {
#ifdef _OPENMP
#pragma omp for private(idx)
#endif
        for (std :: size_t i = 0; i < n_population; ++i)
        {
          idx = (sub_iter == 0 ) ? i : rank[i];
          // crossover
          const int r1 = static_cast < int >(rng(engine) * elite);
          const int r2 = static_cast < int >(rng(engine) * elite);

          // crossover
          new_gen[i] = ( idx < elite ) ? population[ idx ]
                                       : population[ rank[r1] ].template crossover < cross_t > ( population[ rank[r2] ] );

          // random mutation
          if ( rng(engine) < mutation_rate ) new_gen[i].mutate(genome :: gen_random, num_mutation);
        }

#ifdef _OPENMP
#pragma omp for
        for (std :: size_t i = 0; i < n_population; ++i) population[i] = std :: move(new_gen[i]);
#else
        std :: move(new_gen.get(), new_gen.get() + n_population, population.get());
#endif

#ifdef _OPENMP
#pragma omp for
#endif
        for (std :: size_t i = 0; i < n_population; ++i)
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
            std :: sort(rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
            std :: inplace_merge(rank.get(), rank.get() + size, rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
          }
        }
#else
        std :: sort(rank.get(), rank.get() + n_population, [&](const int &a1, const int &a2){return fitness[a1] < fitness[a2];});
#endif

        best = rank[0];
        if (fitness[best] <= fit_limit) break;
#ifdef _OPENMP
#pragma omp single
#endif
        ++ sub_iter;
      }

#ifdef _OPENMP
#pragma omp for
#endif
      for (std :: size_t i = 0; i < n_population; ++i) new_gen[i] = population[rank[i]];
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

#endif // _MPI
