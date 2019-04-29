#include <genetic/ga_omp.h>
#include <sorting/merge_sort.hpp>

template < int cross_t, typename genome, typename Func >
genome ga_omp(const genome & target, const std :: size_t & n_population, const std :: size_t & max_iter, Func fit,
            std :: size_t num_mutation,
            float elite_rate,
            float mutation_rate,
            std :: size_t seed,
            __unused int nth)
{
  using res_t = typename std :: result_of < Func(const genome &, const genome &) > :: type;
  const std :: size_t LENGHT = target.size();

#ifdef _OPENMP
  nth -= nth % 2;
  const int diff_size = n_population % nth,
            size      = diff_size ? n_population - diff_size : n_population;
#endif

  const int elite = static_cast < int >(n_population * elite_rate);
  int best = 0;
  std :: unique_ptr < genome[] > population(new genome[n_population]),
                                 new_gen(   new genome[n_population]);
  std :: unique_ptr < int[] >    rank(      new int[n_population]);
  std :: unique_ptr < res_t[] > fitness (   new res_t[n_population]);

#ifdef VERBOSE
  ProgressBar progress_ = 0;
#else
  std :: size_t iteration = 0;
#endif

#ifdef _OPENMP
#pragma omp parallel num_threads(nth)
  {
#endif

#ifdef _OPENMP
    int th_id = omp_get_thread_num();
#else
    int th_id = 0;
#endif

    std :: srand(seed + th_id);
    std :: mt19937 engine(seed + th_id);
    std :: uniform_real_distribution < float > rng;

#ifdef _OPENMP
#pragma omp for
    for (std :: size_t i = 0; i < n_population; ++i) population[i] = genome :: randomize(genome :: gen_random, LENGHT, std :: rand());
#else
    std :: generate_n(population.get(), n_population, [&](){return genome :: randomize(genome :: gen_random, LENGHT, std :: rand());});
#endif

#ifdef VERBOSE
    for ( ; progress_ != max_iter; )
#else
    while (iteration <= max_iter)
#endif
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
        mergeargsort_parallel_omp(rank.get(), fitness.get(), 0, size, nth, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});
        if (diff_size)
        {
          std :: sort(rank.get() + size, rank.get() + n_population, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});
          std :: inplace_merge(rank.get(), rank.get() + size, rank.get() + n_population, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});
        }
      }

#else

      std :: sort(rank.get(), rank.get() + n_population, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});

#endif

      best = rank[0];

#ifdef VERBOSE

#ifdef _OPENMP
#pragma omp single nowait
      {
#endif
        progress_.update(fitness[best]);
        progress_.print();
#ifdef _OPENMP
      }
#endif

#endif

      if ( fitness[best] == 0 ) break;

// standard genetic algorithm
#ifdef _OPENMP
#pragma omp for
#endif
      for (std :: size_t i = 0; i < n_population; ++i)
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

#ifndef VERBOSE

#ifdef _OPENMP
#pragma omp single
#endif
      ++ iteration;

#endif // VERBOSE

    } // end while


#ifdef _OPENMP
  } // end parallel section
#endif

#ifdef VERBOSE
  progress_.breaks();
#endif

  return population[best];
}





template < int cross_t, typename genome, typename Func >
genome brkga_omp(const genome & target, const int & n_population, const std :: size_t & max_iter, Func fit,
               float elit_percentage,
               float mutant_percentage,
               std :: size_t seed,
               __unused int nth)
{
  using res_t = typename std :: result_of < Func(const genome &, const genome &) > :: type;
  const std :: size_t LENGHT = target.size();

#ifdef _OPENMP
  nth -= nth % 2;
  const int diff_size = n_population % nth,
            size      = diff_size ? n_population - diff_size : n_population;
#endif

  const int elite  = static_cast < int >(n_population * elit_percentage);
  const int mutant = static_cast < int >(n_population * mutant_percentage);
  int best = 0;
  std :: unique_ptr < genome[] > population(new genome[n_population]),
                                 new_gen(   new genome[n_population]);
  std :: unique_ptr < int[] >    rank(      new int[n_population]);
  std :: unique_ptr < res_t[] > fitness (   new res_t[n_population]);

#ifdef VERBOSE
  ProgressBar progress_ = 0;
#else
  std :: size_t iteration = 0;
#endif

#ifdef _OPENMP
#pragma omp parallel num_threads(nth)
  {
#endif

#ifdef _OPENMP
    int th_id = omp_get_thread_num();
#else
    int th_id = 0;
#endif

    std :: srand(seed + th_id);
    std :: mt19937 engine(seed + th_id);
    std :: uniform_real_distribution < float > rng;

#ifdef _OPENMP
#pragma omp for
    for (int i = 0; i < n_population; ++i) population[i] = genome :: randomize(genome :: gen_random, LENGHT, std :: rand());
#else
    std :: generate_n(population.get(), n_population, [&](){return genome :: randomize(genome :: gen_random, LENGHT, std :: rand());});
#endif

#ifdef VERBOSE
    for ( ; progress_ != max_iter; )
#else
    while (iteration <= max_iter)
#endif
    {

#ifdef _OPENMP

#pragma omp for
      for (int i = 0; i < n_population; ++i)
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
        mergeargsort_parallel_omp(rank.get(), fitness.get(), 0, size, nth, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});
        if (diff_size)
        {
          std :: sort(rank.get() + size, rank.get() + n_population, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});
          std :: inplace_merge(rank.get(), rank.get() + size, rank.get() + n_population, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});
        }
      }

#else

      std :: sort(rank.get(), rank.get() + n_population, [&](const int & a1, const int & a2){return fitness[a1] < fitness[a2];});

#endif

      best = rank[0];

#ifdef VERBOSE

#ifdef _OPENMP
#pragma omp single nowait
      {
#endif
        progress_.update(fitness[best]);
        progress_.print();
#ifdef _OPENMP
      }
#endif

#endif

      if ( fitness[best] == 0 ) break;

// bias random key genetic algorithm
#ifdef _OPENMP
#pragma omp for
#endif
      for (int i = 0; i < elite; ++i) new_gen[i] = population[rank[i]];

#ifdef _OPENMP
#pragma omp for
#endif
        for (int i = elite; i < mutant; ++i)
        {
          const int r1 = static_cast < int >(rng(engine) * elite);
          const int r2 = static_cast < int >(rng(engine) * (n_population - elite) + elite);
          new_gen[i] = population[ rank[r1] ].template crossover < cross_t > ( population[ rank[r2] ]);
        }
#ifdef _OPENMP
#pragma omp for
#endif
        for (int i = mutant; i < n_population; ++i) new_gen[i] = genome :: randomize(genome :: gen_random, LENGHT, std :: rand());



#ifdef _OPENMP

#pragma omp for
      for (int i = 0; i < n_population; ++i) population[i] = std :: move(new_gen[i]);

#else

      std :: move(new_gen.get(), new_gen.get() + n_population, population.get());

#endif

#ifndef VERBOSE

#ifdef _OPENMP
#pragma omp single
#endif
      ++ iteration;

#endif // VERBOSE

    } // end while


#ifdef _OPENMP
  } // end parallel section
#endif

#ifdef VERBOSE
  progress_.breaks();
#endif

  return population[best];
}

