#include <actions/actions.h>

actions :: actions ()
{
}

actions & actions :: operator= ( actions a)
{
  _size = a.size();
  _data = std :: move(a._data);
  return *this;
}

actions & actions :: operator= ( Genome < int > a)
{
  _size = a.size();
  _data = std :: move(a._data);
  return *this;
}

int actions :: gen_random ()
{
  return moveset[ std :: rand() % max_index_move_set ];
}

std :: string actions :: capture_output (const char * cmd)
{
  std :: string result;

#if !defined (__clang__)

  std :: array <char, FILENAME_MAX> buffer;

#ifdef _MSC_VER
  std :: shared_ptr < FILE > pipe (Popen(cmd, "r"), Pclose);
#else
  std :: shared_ptr < FILE > pipe (Popen(cmd, "r"), Pclose);
#endif

  if ( !pipe ) throw std :: runtime_error("popen() failed!");

  while ( !feof(pipe.get()) )
    if ( fgets(buffer.data(), FILENAME_MAX, pipe.get()) != nullptr )
      result += buffer.data();

#else

  std :: cerr << "Unsupported architecture!" << std :: endl;
  std :: exit(1);

#endif

  return result;
}

std :: vector < std :: string > actions :: split (std :: string original, char delimiter)
{
  std :: vector < std :: string > results;

  std :: string :: const_iterator start = original.begin();
  std :: string :: const_iterator end   = original.end();
  std :: string :: const_iterator next  = std :: find(start, end, delimiter);

  while ( next != end )
  {
    results.emplace_back(std :: string(start, next));
    start = next + 1;
    next = std :: find(start, end, delimiter);
  }

  results.emplace_back(std :: string(start, next));
  return results;
}


std :: ostream & operator << (std :: ostream & os, actions s)
{
  for ( std :: size_t i = 0; i < s.size(); ++i)
    os << s[i] << " ";
  return os;
}

