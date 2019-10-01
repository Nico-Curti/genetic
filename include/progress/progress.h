#ifndef __progress__
#define __progress__

#ifdef VERBOSE

#include <iostream>
#include <iomanip>
#include <thread>
#include <sstream>
#include <climits>
#include <cmath>
#ifdef _MPI
#include <mpi.h>
#include <boost/mpi/timer.hpp>
#elif defined _OPENMP
#include <omp.h>
#else
#include <chrono>
#endif

static const int INF = std :: numeric_limits < int > :: infinity();

template<typename Char, typename Traits, typename Allocator>
std :: basic_string < Char, Traits, Allocator > operator * (const std :: basic_string < Char, Traits, Allocator > s, std :: size_t n)
{
  std :: basic_string<Char, Traits, Allocator> tmp = s;
  for (std :: size_t i = 0; i < n; ++i) tmp += s;
  return tmp;
}

template<typename Char, typename Traits, typename Allocator>
std :: basic_string < Char, Traits, Allocator > operator * (std :: size_t n, const std :: basic_string < Char, Traits, Allocator > & s)
{
  return s * n;
}

class ProgressBar
{
  std::thread _th;

#ifdef _MPI

  boost :: mpi :: timer _start_time;
  boost :: mpi :: timer _estimated;

#elif defined _OPENMP

  double _start_time;
  double _estimated;

#else

  std :: chrono :: time_point < std :: chrono :: high_resolution_clock > _start_time;
  std :: chrono :: time_point < std :: chrono :: high_resolution_clock >  _estimated;

#endif

  bool _go;

  int _max_iters = INF;
  int _current_iter;

  double _it;

  std :: string _c;
  std :: string _e;
  std :: string _value;

  static const int PBWIDTH = 20;

public:

  ProgressBar ();
  ProgressBar (const int & start_iter, std :: string c = "█", std :: string e = ".");
  ProgressBar& operator = (const int & start);

  void operator ++ ();
  bool operator != (const int & max_iters);

  template < class T > void update(T val)
  {
    ++_current_iter;
    std :: stringstream ss;
    ss << std :: fixed << std :: setprecision(2) << val;
    _value = ss.str();
    _it    = iteration();
#ifdef _MPI
    _estimated  = boost :: mpi :: timer();
#elif defined _OPENMP
    _estimated  = omp_get_wtime();
#else
    _estimated  = std :: chrono :: high_resolution_clock :: now();
#endif
  }

  void start();
  void breaks();

  void print();

private:

  void stop();
  void run();

  double iteration();
  double duration();

};

#endif // VERBOSE

#endif // __progress__
