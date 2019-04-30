#include <iostream>    // std::cout
#include <strings/mystr.h>
#include <genetic/ga_omp.hpp>
#ifdef _OPENMP
#include <omp.h>       // omp_get_max_threads
#endif

//const std :: string TARGET = "I'm a genetic algorithm";
const std :: string TARGET = "Nel mezzo del cammin di nostra vita "
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


int main()
{
  const int rng_seed = 123;

#ifdef _OPENMP

  int nth = omp_get_max_threads();

#else

  int nth = 1;

#endif

  std :: srand(rng_seed);
  mystring target(TARGET);

  auto best_str = ga_omp<_single>(target.size(),
                                  1024,     // number of dna
                                  2000,     // max number of iterations
                                  fitness,  // fitness function as anonymous struct operator
                                  1,        // number of mutations
                                  .1f,      // percentage of population to conserve
                                  .4f,      // probability of mutation
                                  rng_seed, // random seed
                                  nth,      // number of threads
                                  &target
                                  );

  std :: cout << "Naive GA Best found: " << std :: endl;
  std :: cout << best_str << std :: endl;

  auto best_str2 = brkga_omp<_single>(target.size(),
                                      1024,     // number of dna
                                      2000,     // max number of iterations
                                      fitness,  // fitness function as anonymous struct operator
                                      .1f,      // percentage of population to conserve
                                      .1f,      // percentage of mutants in each generation
                                      rng_seed, // random seed
                                      nth,      // number of threads
                                      &target
                                      );

  std :: cout << "BRKGA Best found: " << std :: endl;
  std :: cout << best_str2 << std :: endl;

  return 0;
}

