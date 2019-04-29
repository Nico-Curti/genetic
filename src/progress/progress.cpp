#ifdef VERBOSE

#include <progress/progress.h>

ProgressBar :: ProgressBar ()
{
}

ProgressBar :: ProgressBar (const int & start_iter, std::string c, std::string e) : _go(true), _current_iter(start_iter), _c(c), _e(e), _value("")
{
  _it = 0.;
#ifdef _MPI
  _start_time = boost :: mpi :: timer();
  _estimated  = boost :: mpi :: timer();
#elif defined _OPENMP
  _start_time = omp_get_wtime();
  _estimated  = omp_get_wtime();
#else
  _start_time = std :: chrono :: high_resolution_clock :: now();
  _estimated  = std :: chrono :: high_resolution_clock :: now();
#endif
}

ProgressBar& ProgressBar :: operator= (const int & start)
{
  _current_iter = start;
  return *this;
}

void ProgressBar :: operator++ ()
{
  update("");
}

void ProgressBar :: breaks ()
{
  _go = false;
  if (_th.joinable()) _th.join();
  std :: cout << std :: endl;
  std :: cout << std :: flush;
}

bool ProgressBar :: operator!= (const int & max_iters)
{
  _max_iters = max_iters;
  const bool check = _current_iter != _max_iters;
  if ( ! check ) stop();
  return check ? true : false;
}

void ProgressBar :: start ()
{
  _th = std :: thread(&ProgressBar :: run, this);
  _go = true;
}

void ProgressBar :: stop()
{
  _go = false;
  if (_th.joinable()) _th.join();
  double c_it = _current_iter / _it;
  double m_it = (_max_iters - _current_iter) / _it;
  c_it = std :: isnan(c_it) ? 0. : std :: isinf(c_it) ? 0. : c_it;
  m_it = std :: isnan(m_it) ? 0. : std :: isinf(m_it) ? 0. : m_it;
#ifdef _OPENMP
#pragma omp single
  {
#endif
#ifdef _WIN32
    std :: cout << '\r';
#else
    std :: cout << "\r\x1B[K";
#endif
    std :: cout << "It: "
                << std :: right << _max_iters << " / "
                << std :: left  << _max_iters
                << " |"
                << std :: setw(PBWIDTH)
                << _c * PBWIDTH//std :: string(PBWIDTH, _c)
                << "| " << _value
                << " ["
                << std :: right << c_it << ":00"
                << "<"
                << std :: right << m_it << ":00"
                << " sec]"
                ;
    std :: cout << std :: flush;
    std :: cout << std :: endl;
#ifdef _OPENMP
  }
#endif
}

void ProgressBar :: print()
{

  const float perc = static_cast < float >(_current_iter) / _max_iters;
        int lpad   = static_cast < int >(std :: floor(perc * PBWIDTH));
  lpad       = lpad > PBWIDTH ? PBWIDTH : lpad;
  double c_it = _current_iter / _it;
  double m_it = (_max_iters - _current_iter) / _it;
  c_it = std :: isnan(c_it) ? 0. : std :: isinf(c_it) ? 0. : c_it;
  m_it = std :: isnan(m_it) ? 0. : std :: isinf(m_it) ? 0. : m_it;
#ifdef _WIN32
  std :: cout << '\r';
#else
  std :: cout << "\r\x1B[K";
#endif
  std :: cout << "It: "
              << std :: right << _current_iter << " / "
              << std :: left  << _max_iters
              << " |"
              << std :: setw(PBWIDTH)
              << _c * lpad + _e * (PBWIDTH - lpad) // std :: string(lpad, _c) + std::string(PBWIDTH - lpad, _e)
              << "| "  << _value
              << " ["
              << std :: right << c_it << ":00"
              << "<"
              << std :: right << m_it << ":00"
              << " sec, "
              << std :: right << 1.f / _it << " it/sec]"
              ;
  std :: cout << std :: flush;
}

void ProgressBar :: run()
{
  _it = 0.;
#ifdef _MPI
  _start_time = boost :: mpi :: timer();
  _estimated  = boost :: mpi :: timer();
#elif defined _OPENMP
  _start_time = omp_get_wtime();
  _estimated  = omp_get_wtime();
#else
  _start_time = std :: chrono :: high_resolution_clock :: now();
  _estimated  = std :: chrono :: high_resolution_clock :: now();
#endif

  while (_go)
  {
    const float perc = static_cast < float >(_current_iter) / _max_iters;
    int lpad   = static_cast < int >(std :: floor(perc * PBWIDTH));
    lpad       = lpad > PBWIDTH ? PBWIDTH : lpad;
    double c_it = _current_iter / _it;
    double m_it = (_max_iters - _current_iter) / _it;
    c_it = std :: isnan(c_it) ? 0. : std :: isinf(c_it) ? 0. : c_it;
    m_it = std :: isnan(m_it) ? 0. : std :: isinf(m_it) ? 0. : m_it;
#ifdef _OPENMP
#pragma omp single
    {
#endif
#ifdef _WIN32
      std :: cout << '\r';
#else
      std :: cout << "\r\x1B[K";
#endif
      std :: cout << "It: "
                  << std :: right << _current_iter << " / "
                  << std :: left  << _max_iters
                  << " |"
                  << std :: setw(PBWIDTH)
                  << _c * lpad + _e * (PBWIDTH - lpad) // std :: string(lpad, _c) + std::string(PBWIDTH - lpad, _e)
                  << "| "  << _value
                  << " ["
                  << std :: right << c_it << ":00"
                  << "<"
                  << std :: right << m_it << ":00"
                  << " sec, "
                  << std :: right << 1.f / _it << " it/sec]"
                  ;
      std :: cout << std :: flush;
#ifdef _OPENMP
    }
#endif
  }
}

double ProgressBar :: iteration()
{
#ifdef _MPI
  return _estimated.elapsed();
#elif defined _OPENMP
  return omp_get_wtime() - _estimated;
#else
  return std :: chrono :: duration_cast < std :: chrono :: seconds >(std :: chrono :: high_resolution_clock :: now() - _estimated).count();
#endif
}

double ProgressBar :: duration()
{
#ifdef _MPI
  return _start_time.elapsed();
#elif defined _OPENMP
  return omp_get_wtime() - _start_time;
#else
  return std :: chrono :: duration_cast < std :: chrono :: seconds >(std :: chrono :: high_resolution_clock :: now() - _start_time).count();
#endif
}

#endif // VERBOSE
