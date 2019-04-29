#ifndef __mergesort__
#define __mergesort__

#ifdef _OPENMP

#include <algorithm>

template < typename T, typename Ord >
void mergeargsort_serial(int * index, T * arr, const int & start, const int & end, Ord ord);

template < typename T, typename Ord >
void mergeargsort_parallel_omp (int * index, T * arr, const int & start, const int & end, const int & threads, Ord ord);

#endif // _OPENMP

#endif // __mergesort__
