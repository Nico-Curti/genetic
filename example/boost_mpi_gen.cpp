// mpic++ -O3 -lboost_mpi -lboost_serialization -I. boost_mpi_gen.cpp -std=c++14 -o boost_gen
// mpirun -np 4 ./boost_gen
#include <iostream>             // std::cout
#include <strings/mystr.h>
#include <genetic/ga_mpi.hpp>
#ifdef _OPENMP
#include <omp.h>                // omp_get_max_threads
#endif

//const std :: string TARGET = "I'm a genetic algorithm";
const std :: string TARGET = "Nel mezzo del cammin di nostra vita "
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

int main(int argc, char **argv)
{
  const int rng_seed = 123;

#ifdef _OPENMP

  int nth = omp_get_max_threads();

#else

  int nth = 1;

#endif

  const int n_population = 50, // 85 ???
            max_iters    = 1000,
            num_mutation = 1;
  const float mut_rate   = .3f,
              elite_rate = .1f;

  std :: srand(rng_seed);
  mystring target(TARGET);

  auto best_str = ga_mpi<_single>(argc, argv,
                                  target, n_population, max_iters, fitness,
                                  num_mutation, elite_rate, mut_rate,
                                  100, rng_seed, nth);

  std :: cout << "Naive GA Best found: " << std :: endl;
  std :: cout << best_str << std :: endl;

  return 0;
}
