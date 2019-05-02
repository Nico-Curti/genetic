#ifndef __mystring__
#define __mystring__

#include <numeric>     // std::inner_product
#include <memory>      // std::make_unique
#include <genome/genome.hpp>

#ifdef _MSC_VER
  #ifndef __unused
  #define __unused
  #endif
#else
  #ifndef __unused
  #define __unused __attribute__((__unused__))
  #endif
#endif

const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "|\\\"£$%&/()=?'^ìéè[]+*ò@#°àù,;.:-_"
            "\t ";
const std :: size_t max_index_char_set = sizeof ( charset ) / sizeof ( char );


class mystring : public Genome <char>
{

public:

  // Constructors

  mystring ();
  mystring (std :: string str);
  mystring & operator = (mystring s);
  mystring & operator = (Genome < char > s);

  // Destructors

  ~mystring () = default;

  // Member functions

  // Static Members

  static char gen_random();
};

std :: ostream & operator << (std :: ostream & os, mystring s);

__unused struct
{
  float operator () (mystring a, mystring b)
  {
    float cnt = 0.f;
    for(std :: size_t i = 0; i < a.size(); ++i)
      cnt += a[i] == b[i] ? 1.f : 0.f;
    return (cnt / a.size()) * 100;
  }
} match;

__unused struct
{
  int operator () (mystring a, mystring b)
  {
    return std :: inner_product(a.get(), a.get() + a.size(), b.get(), 0,
                                std :: plus < int >(), [](const auto & i, const auto & j)
                                {
                                  return (static_cast < int >(i) - static_cast < int >(j)) *
                                         (static_cast < int >(i) - static_cast < int >(j));
                                });
  }
} fitness;

#endif // __mystring__
