#include <iostream>    // std::cout
#include <string>      // std::string
#include <algorithm>   // std::generate_n
#include <numeric>     // std::inner_product
#include <chrono>      // std::chrono
#include <iomanip>     // std::setw
static constexpr int PBWIDTH = 50;

auto randchar = []() -> char {
            const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "|\\\"£$%&/()=?'^ìéè[]+*ò@#°àù,;.:-_"
            "\t ";
            const std::size_t max_index = (sizeof(charset) - 1);
            return charset[ std::rand() % max_index ];
        };

#ifdef _MPI
auto printProgress = [](const float &now, const int &total, boost::mpi::timer time)
                      {
                        float perc = now / total;
                        int lpad = static_cast<int>(perc * PBWIDTH);
                        std::cout << "\rOptimization progress:"
                                  << std::right << std::setw(5) << std::setprecision(3) << perc * 100.f << "%  ["
                                  << std::left  << std::setw(PBWIDTH - 1) << std::string(lpad, '|') << "] "
                                  << std::right << std::setw(5) << int(now) << "/" << std::setprecision(5) << total
                                  << "   [" << std::setw(8) << std::setprecision(3) << time.elapsed()
                                  << " sec]";
                        std::cout << std::flush;
                      };
#else
auto printProgress = [](const float &now, const int &total, const std::chrono::high_resolution_clock::time_point &start_time)
                      {
                        float perc = now / total;
                        int lpad = static_cast<int>(perc * PBWIDTH);
                        std::cout << "\rOptimization progress:"
                                  << std::right << std::setw(5) << std::setprecision(3) << perc * 100.f << "%  ["
                                  << std::left  << std::setw(PBWIDTH - 1) << std::string(lpad, '|') << "] "
                                  << std::right << std::setw(5) << int(now) << "/" << std::setprecision(5) << total
                                  << " [" << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count() << " sec]";
                        std::cout << std::flush;
                      };
#endif

struct mystring
{
  std::string str;
  mystring(){};
  mystring(const std::string &str) {this->str = str;}
  mystring(const mystring &a) {this->str = a.str;}
  ~mystring(){this->str.clear();}
  mystring& operator=(const mystring &a){this->str = a.str; return *this;}
  // random generation
  mystring(const unsigned int &seed)
  {
    std::srand(seed);
    this->str.resize(LENGTH);
    std::generate_n( this->str.begin(), LENGTH, randchar );
  }
  // crossover function
  mystring operator+(const mystring &s)
  {
    int pos = std::rand() % LENGTH;
    return this->str.substr(0, pos) + s.str.substr(pos, LENGTH);
  }
  // mutation function
  void operator!(void)
  {
    int ipos  = std::rand() % LENGTH;
    this->str[ipos] = randchar();
  }

#ifdef _MPI

private:
  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int version)
  {
    ar & str;
  }
#endif

};

std::ostream& operator<<(std::ostream &os, const mystring &s)
{
  os << s.str;
  return os;
}

struct
{
  int operator()(const mystring &a, const mystring &b)
  {
    int cnt = 0;
    for(std::size_t i = 0; i < a.str.size(); ++i)
      cnt += (a.str[i] == b.str[i]) ? 1 : 0;
    return cnt;
  }
} match;

struct
{
  float operator()(const mystring &a, const mystring &b)
  {
      return std::inner_product(a.str.begin(), a.str.end(), b.str.begin(), 0.f,
                                std::plus<float>(), [](const char &i, const char &j)
                                {
                                  return (static_cast<int>(i) - static_cast<int>(j)) *
                                         (static_cast<int>(i) - static_cast<int>(j));
                                });
  }
} fitness;

