#include <iostream>         // std::cout
#include <fstream>          // fopen
#include <feature_selection/feature_selection.h>
#include <genetic/ga_omp.hpp>
#ifdef _OPENMP
#include <omp.h>            // omp_get_max_threads
#endif

void file_exist (const std :: string & filename)
{
  bool exist = false;
  if (FILE *file = fopen(filename.c_str(), "r"))
  {
    fclose(file);
    exist = true;
  }

  if (!exist)
  {
    std :: cerr << "File not found! Given " << filename << std :: endl;
    std :: exit(1);
  }
}

int main()
{
  const int rng_seed = 123;
  std :: string filename = "./features.dat";

  file_exist(filename);

#ifdef _OPENMP

  int nth = omp_get_max_threads();

#else

  int nth = 1;

#endif

  std :: srand(rng_seed);
  feature_sel full_set(filename);

  auto best_weights = ga_omp <_single>(,
                                       50,       // number of dna
                                       15000,    // max number of iterations
                                       fitness,  // fitness function as anonymous struct operator
                                       1,        // number of mutations
                                       .1f,      // percentage of population to conserve
                                       .3f,      // probability of mutation
                                       rng_seed, // random seed
                                       nth,      // number of threads);
                                       &full_set);

  std :: cout << "Best weights found:" << std :: endl;
  std :: cout << best_weights << std :: endl;

  return 0;
}
