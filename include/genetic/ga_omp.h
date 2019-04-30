#ifndef __ga_omp__
#define __ga_omp__

#include <iostream>    // std::cout
#include <memory>      // std::unique_ptr
#include <algorithm>   // std::generate_n
#include <numeric>     // std::inner_product
#include <random>      // std::uniform_distribution
#include <type_traits> // std::result_of, std::invoke_result

#ifdef _OPENMP
#include <omp.h>       // omp_get_max_threads
#include <sorting/merge_sort.h>
#endif

#ifdef VERBOSE
#include <progress/progress.h>
#endif

#include <cassert>

#ifdef _MSC_VER
  #ifndef __unused
  #define __unused
  #endif
#else
  #ifndef __unused
  #define __unused __attribute__((__unused__))
  #endif
#endif

template < int cross_t, typename genome, typename Func >
genome ga_omp(const std :: size_t & lenght, const std :: size_t & n_population, const std :: size_t & max_iter, Func fit,
              std :: size_t num_mutation = 1,
              float elite_rate = .1f,
              float mutation_rate = .3f,
              std :: size_t seed = 0,
              __unused int nth = 4,
              const genome * target = nullptr
              );

template < int cross_t, typename genome, typename Func >
genome brkga_omp(const std :: size_t & lenght, const int & n_population, const std :: size_t & max_iter, Func fit,
                 float elit_percentage = .3f,
                 float mutant_percentage = .1f,
                 std :: size_t seed = 0,
                 __unused int nth = 4,
                 const genome * target = nullptr
                 );

#endif // __ga_omp__
