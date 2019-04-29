#ifndef __myimage__
#define __myimage__

#ifdef __images__

#include <opencv2/opencv.hpp>  // cv::imread
#include <numeric>             // std::inner_product
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

class image : public Genome <unsigned char>
{

public:

  // Members

  std :: size_t _w;
  std :: size_t _h;

  // Constructors

  image ();
  image (std :: string filename);
  image & operator = ( image a);
  image & operator = (Genome < unsigned char > g);

  // Destructors

  ~image () = default;

  // Member functions

  void show ( void );
  void save ( std :: string filename );

  // Private members

  static unsigned char gen_random();

};

std :: ostream & operator << (std :: ostream & os, image im);

__unused struct
{
  float operator()(image a, image b)
  {
    float cnt = 0.f;
    for (std :: size_t i = 0; i < a.size(); ++i)
      cnt += a[i] == b[i] ? 1.f : 0.f;
    return cnt / a.size();
  }
} match;


__unused struct
{
  float operator () (image a, image b)
  {
    return std::inner_product(a.get(), a.get() + a.size(), b.get(), 0.f,
                              std :: plus < float >(), [](const auto & i, const auto & j)
                              {
                                return (static_cast < float >(i) - static_cast < float >(j)) *
                                       (static_cast < float >(i) - static_cast < float >(j));
                              });
  }
} fitness;

#endif // __images__

#endif // __myimage__
