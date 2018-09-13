#ifdef _OPENMP
#include <algorithm>

static constexpr int min_dim_sort = 100;


template<typename T, typename Ord> void inline mergeargsort_serial(int *index, T *arr, const int &start, const int &end, Ord ord)
{
  if((end - start) == 2)
  {
      if(ord(arr[start], arr[end-1])) return;
      else
      {
          std::swap(index[start], index[end-1]);
          return;
      }
  }
  int pivot = start + ((end - start) >> 1);
  if((end-start) < min_dim_sort) std::sort(index + start, index + end, ord);
  else
  {
      mergeargsort_serial(index, arr, start, pivot, ord);
      mergeargsort_serial(index, arr, pivot, end, ord);
  }

  std::inplace_merge(index + start, index + pivot, index + end, ord);
  return;
}

template<typename T, typename Ord> void inline mergeargsort_parallel_omp (int *index, T *arr, const int &start, const int &end, const int &threads, Ord ord)
{
  int pivot = start + ((end-start) >> 1);
  if(threads <= 1) {mergeargsort_serial(index, arr, start, end, ord); return;}
  else
  {
#pragma omp task shared(start, end, threads)
    {
        mergeargsort_parallel_omp(index, arr, start, pivot, threads >> 1, ord);
    }
#pragma omp task shared(start, end, threads)
    {
        mergeargsort_parallel_omp(index, arr, pivot, end, threads - (threads >> 1), ord);
    }
#pragma omp taskwait
  }
  std::inplace_merge(index + start, index + pivot, index + end, ord);
  return;
}

#endif