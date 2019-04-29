#ifndef __genome__
#define __genome__

#include <algorithm>   // std::copy_n
#include <memory>      // std::unique_ptr
#include <type_traits> // std::enable_if

#ifdef _MPI
#include <vector>
#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
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

enum { _single = 1,
       _multi  = 2,
};

template < class type > class Genome
{

protected:

  std :: size_t _size;

public:

#ifdef _MPI
  std :: vector < type > _data;
#else
  std :: unique_ptr < type[] > _data;
#endif

  // Constructors

  Genome ();
  Genome (std :: size_t size);
  Genome (const Genome & g);

  // Destructors

  ~Genome () = default;

  // Operators

  type & operator [] (std :: size_t i);

  Genome & operator = (const Genome & g);
  Genome & operator = (Genome && g);

  // random generator Genome

  template < typename Func, typename std :: enable_if < std :: is_same < typename std :: result_of < Func() > :: type, type > :: value > :: type * = nullptr >
  static Genome randomize ( Func func, std :: size_t size, std :: size_t seed);

  // crossover Genome

  template < int c_type, typename std :: enable_if < (c_type == _single) > :: type * = nullptr > Genome crossover (Genome g);
  template < int c_type, typename std :: enable_if < (c_type == _multi ) > :: type * = nullptr > Genome crossover (Genome g);

  // mutation Genome

  template < typename Func, typename std :: enable_if < std :: is_same < typename std :: result_of < Func() > :: type, type > :: value > :: type * = nullptr >
  void mutate ( Func func, std :: size_t npos = 1);

  std :: size_t size () const;
  type * get ();

protected:

  std :: size_t randpos ();

#ifdef _MPI
  friend class boost::serialization::access;

  template < class Archive >
  void serialize (Archive & ar, __unused const std :: size_t version)
  {
    ar & _size;
    ar & _data;
  }
#endif

};

template < class type > std :: ostream & operator << (std :: ostream & os, const Genome <type> & s);

#endif // __genome__
