#include <iostream>    // std::cout
#include <string>      // std::string
#include <memory>      // std::unique_ptr
#include <algorithm>   // std::generate_n
#include <numeric>     // std::inner_product
#include <chrono>      // std::chrono
#include <iomanip>     // std::setw
#include <random>      // std::uniform_distribution
#include <type_traits> // std::result_of, std::invoke_result
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
                            "Tant'e amara che poco e piu morte; "
                            "ma per trattar del ben ch'i' vi trovai, "
                            "diro de l'altre cose ch'i' v'ho scorte. "
                            "Io non so ben ridir com'i' v'intrai, "
                            "tant'era pien di sonno a quel punto "
                            "che la verace via abbandonai.";
const std::size_t LENGTH = TARGET.size();

#include <mystr.h>
#ifdef _OPENMP
#include <merge_sort.h>
#endif

static constexpr int rng_seed    = 123;
static constexpr float inf       = std::numeric_limits<float>::infinity();
static constexpr float fit_limit = 0.f;


template<typename genome, typename Func> auto par_gen(const genome &target,
                                                      const int &n_population,
                                                      const int &max_iter,
                                                      Func fit,
                                                      float elit_rate = .1f,
                                                      float mutation_rate = .3f,
                                                      std::size_t seed = 0,
                                                      int verbose = 1, // logfile
                                                      int nth = 4)
{
  using res_t = typename std::result_of<Func(const genome &, const genome &)>::type; // since c++17
#ifdef _OPENMP
  nth -= nth % 2;
  const int diff_size = n_population % nth,
            size      = diff_size ? n_population - diff_size : n_population;
#endif
  const int elite = n_population * elit_rate;
  int iteration = 0,
      best = 0;
  std::unique_ptr<genome[]> population(new genome[n_population]),
                            new_gen(   new genome[n_population]);
  std::unique_ptr<int[]>    rank(      new int[n_population]);
  std::unique_ptr<res_t[]> fitness (   new res_t[n_population]);

  std::mt19937 engine(seed);
  std::uniform_real_distribution<float> rng;
  auto start_time = std::chrono::high_resolution_clock::now();


#ifdef _OPENMP
#pragma omp parallel num_threads(nth)
  {
#endif

#ifdef _OPENMP
#pragma omp for
  for(int i = 0; i < n_population; ++i) population[i] = genome(rng(engine));
#else
  std::generate_n(population.get(), n_population, [&](){return genome(rng(engine));});
#endif

  while(iteration < max_iter)
  {
#ifdef _OPENMP
#pragma omp for
    for(int i = 0; i < n_population; ++i)
    {
      // argsort variables
      fitness[i] = fit(population[i], target);
      rank[i]    = i;
    }
#else
    std::transform(population.get(), population.get() + n_population,
                   fitness.get(),
                   [&](const genome &pop)
                   {
                    return fit(pop, target);
                  });
    std::iota(rank.get(), rank.get() + n_population, 0);
#endif

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

#ifdef _OPENMP
#pragma omp single nowait
    {
#endif
      switch(verbose)
      {
        case 1: printProgress(match(population[best], target), LENGTH, start_time);
        break;
        case 2:
        {
          std::cout << "iter: "
                    << std::setw(5)          << iteration        << "   "
                    << std::setw(LENGTH + 3) << population[best] << "   "
                    << std::setw(5)          << fitness[best]    << "   "
                    << std::setw(LENGTH + 3) << target           << "   "
                    << std::setw(3)          << match(population[best], target)
                    << std::endl;
        } break;
        default: break;
      }
#ifdef _OPENMP
    } // close single section
#endif
    if (fitness[best] == 0) break;

#ifdef _OPENMP
#pragma omp for
#endif
    for(int i = 0; i < n_population; ++i)
    {
      // crossover
      new_gen[i] = ( rank[i] < elite ) ? population[rank[i]]
                                       : population[rank[int(rng(engine) * elite)]] + population[rank[int(rng(engine) * elite)]];
      // random mutation
      if (rng(engine) < mutation_rate) !new_gen[i];
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
    ++iteration;

  } // end while

#ifdef _OPENMP
  } // end parallel section
#endif

  if (verbose == 1) std::cout << std::endl;

  return population[best];
}

int main()
{
#ifdef _OPENMP
  int nth = omp_get_max_threads();
#else
  int nth = 1;
#endif
  std::srand(rng_seed);
  mystring target(TARGET);
  auto best_str = par_gen(target,
                          2048,    // number of dna
                          16384,   // max number of iterations
                          fitness, // fitness function as anonymous struct operator
                          .1f,     // percentage of population to conserve
                          .3f,     // probability of mutation
                          123,     // random seed
                          true,    // output file ON
                          nth);

  std::cout << "Best found: " << std::endl;
  std::cout << best_str << std::endl;
  return 0;
}

