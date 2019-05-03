#include <iostream>         // std::cout
#include <fstream>          // fopen
#include <actions/actions.h>
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

  if ( !exist )
  {
    std :: cerr << "Python script not found! Given " << filename << std :: endl;
    std :: exit(1);
  }
}

int main()
{
  const int rng_seed = 123;
  std :: string python_script = "./utility/super_mario_rewards.py";

  file_exist(python_script);

#ifdef _OPENMP

  int nth = omp_get_max_threads();

#else

  int nth = 1;

#endif

  std :: srand(rng_seed);
  const int genome_size = 1000;

  auto best_move = ga_omp<_multi, actions>(genome_size,
                                           128,       // number of dna
                                           50,       // max number of iterations
                                           fitness,  // fitness function as anonymous struct operator
                                           10,        // number of mutations
                                           .3f,      // percentage of population to conserve
                                           .3f,      // probability of mutation
                                           rng_seed, // random seed
                                           nth       // number of threads
                                           );

  std :: cout << "Best sequence movements found:" << std :: endl;
  std :: cout << best_move << std :: endl;

  return 0;
}

