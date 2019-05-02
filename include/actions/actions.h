#ifndef __actions__
#define __actions__

#include <numeric>     // std::inner_product
#include <memory>      // std::make_unique
#include <array>       // std::array
#include <fstream>     // FILE
#include <vector>      // std::vector
#include <string>      // std::string
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

#if defined _WIN32
  #define Popen _popen
  #define Pclose _pclose
  #include <Windows.h>
  #include <direct.h>
#else
  #define Popen popen
  #define Pclose pclose
  #include <dirent.h>
  #include <sys/types.h>
  #include <errno.h>
  #include <unistd.h>
  #include <stdio.h>
#endif

enum { noop = 0,
       right,
       rightA,
       rightB,
       rightAB,
       A,
       left,
       leftA,
       leftB,
       leftAB,
       down,
       up
};// complex_movements

const int moveset[] = {noop, right, rightA, rightB, rightAB, A, left, leftA, leftB, leftAB, down, up };
const std :: size_t max_index_move_set = sizeof ( moveset ) / sizeof ( int );

class actions : public Genome <int>
{

public:

  // Constructors

  actions ();
  actions & operator = (actions s);
  actions & operator = (Genome < int > s);

  // Destructors

  ~actions () = default;

  // Member functions

  // Static Members

  static int gen_random ();

  std :: string capture_output (const char * cmd);
  static std :: vector < std :: string > split (std :: string original, char delimiter = ' ');
};

std :: ostream & operator << (std :: ostream & os, actions s);

__unused struct
{
  int operator () (actions a, __unused actions b)
  {
    std :: string movements = "python ./utility/super_mario_rewards.py --movements ";
    for ( std :: size_t i = 0; i < a.size(); ++i) movements += std :: to_string(a[i]) + " ";
    movements += "--dimension " + std :: to_string(a.size());

    std :: string rewards = a.capture_output(movements.c_str());
    std :: vector < std :: string > reward_token = actions :: split(rewards, ' ');

    return std :: accumulate(reward_token.begin(), reward_token.end(), 0,
                             [](int res, std :: string token)
                             {
                               return res - std :: stoi(token);
                             });
  }
} fitness;

#endif // __actions__
