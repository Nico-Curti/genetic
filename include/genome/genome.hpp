#ifndef __genome_hpp__
#define __genome_hpp__

#include <genome/genome.h>

template < class type > Genome < type > :: Genome ()
{
}

template < class type > type & Genome < type > :: operator [] (std :: size_t i)
{
  return _data[i];
}

template < class type > Genome <type> :: Genome (std :: size_t size) : _size(size)
{
#ifdef _MPI
  _data.resize(_size);
#else
  _data.reset( new type [_size]);
#endif
}

template < class type > Genome <type> :: Genome (const Genome & g)
{
  _size = g.size();
#ifdef _MPI
  _data.resize(_size);
  std :: copy_n (g._data.begin(), _size, _data.begin());
#else
  _data.reset( new type [_size]);
  std :: copy_n (g._data.get(), _size, _data.get());
#endif
}

template < class type > Genome <type> & Genome <type> :: operator = (const Genome & g)
{
  _size = g.size();
#ifdef _MPI
  _data.resize(_size);
  std :: copy_n (g._data.begin(), _size, _data.begin());
#else
  _data.reset( new type [_size]);
  std :: copy_n (g.get(), _size, _data.get());
#endif
  return *this;
}

template < class type > Genome <type> & Genome <type> :: operator = (Genome && g)
{
  _size = g.size();
  _data = std :: move(g._data);
  return *this;
}

template < class type >  template < typename Func, typename std :: enable_if < std :: is_same < typename std :: result_of < Func() > :: type, type > :: value > :: type * > Genome <type> Genome <type> :: randomize (Func func, std :: size_t size, std :: size_t seed)
{
  Genome < type > new_genome (size);
  std :: srand(seed);
  std :: generate_n(new_genome.get(), new_genome.size(), func);
  return new_genome;
}


template < class type > template <int c_type, typename std :: enable_if < (c_type == _single) > :: type * > Genome <type> Genome <type> :: crossover ( Genome <type> g)
{
  Genome < type > new_genome = g;

  int pos = randpos();
#ifdef _MPI
  std :: copy_n (_data.begin() + pos, _size - pos, new_genome.get() + pos);
#else
  std :: copy_n (_data.get() + pos, _size - pos, new_genome.get() + pos);
#endif
  return new_genome;
}

template < class type > template < int c_type, typename std :: enable_if < (c_type == _multi) > :: type * > Genome <type> Genome <type> :: crossover ( Genome <type> g)
{
  Genome < type > new_genome (g);

  std :: size_t half_pos = _size >> 1;

  for (std :: size_t i = 0; i < _size; ++i)
  {
    std :: size_t pos = randpos();
    new_genome[i] = pos > half_pos ? new_genome[i] : _data[i];
  }
  return new_genome;
}

template < class type > template < typename Func, typename std :: enable_if < std :: is_same < typename std :: result_of < Func() > :: type, type > :: value > :: type * > void Genome < type > :: mutate ( Func func, std :: size_t npos)
{
  for (std :: size_t i = 0; i < npos; ++i)
  {
    std :: size_t pos = randpos();
    _data[pos] = func();
  }
}

template < class type > std :: size_t Genome < type > :: size () const
{
  return _size;
}

template < class type > type * Genome < type > :: get ()
{
#ifdef _MPI
  return _data.data();
#else
  return _data.get();
#endif
}

template < class type > std :: size_t Genome < type > :: randpos ()
{
  return std :: rand() % _size;
}

template < class type > std :: ostream & operator << (std :: ostream & os, const Genome <type> & s)
{
  for ( std :: size_t i = 0; i < s.size(); ++i)
    os << s[i];
  return os;
}

#endif // __genome_hpp__
