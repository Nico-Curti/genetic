#ifndef __ga_mpi__
#define __ga_mpi__

#ifdef _MPI

#include <iostream>    // std::cout
#include <memory>      // std::unique_ptr
#include <algorithm>   // std::generate_n
#include <random>      // std::uniform_distribution

#ifdef _OPENMP
#include <omp.h>       // omp_get_max_threads
#include <sorting/merge_sort.h>
#endif

#include <boost/mpi.hpp>
#include <boost/mpi/timer.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#ifdef VERBOSE
#include <progress/progress.h>
#endif

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
genome ga_mpi(int argc, char * argv[], const genome & target, const std :: size_t & n_population, const std :: size_t & max_iter, Func fit,
              std :: size_t num_mutation = 1,
              float elite_rate = .1f,
              float mutation_rate = .3f,
              std :: size_t sub_iter = 100,
              std :: size_t seed = 0,
              __unused int nth = 0);

template < typename genome >
genome master(boost :: mpi :: communicator & world, const int & n_process,
              const int * block_pop,
              const std :: size_t & n_population, const std :: size_t & max_iters,
              const std :: size_t & seed);

template < int cross_t, typename genome, typename Func >
void slave(boost :: mpi :: communicator & world, const int & pID,
           const genome & target, const std :: size_t & n_population,
           const std :: size_t & max_iters, const std :: size_t & num_mutation, const float & elite_rate,
           const float & mutation_rate, Func fit, const std :: size_t & seed,
           __unused int nth
           );

#endif // _MPI

#endif // __ga_mpi__
