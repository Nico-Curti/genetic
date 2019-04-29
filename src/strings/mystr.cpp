#include <strings/mystr.h>

mystring :: mystring ()
{
}

mystring :: mystring (std :: string str)
{
  _size = str.size();
#ifdef _MPI
  _data.resize(_size);
  std :: move(str.begin(), str.end(), _data.begin());
#else
  _data = std :: make_unique <char []>(_size);
  std :: move(str.begin(), str.end(), _data.get());
#endif
}

mystring & mystring :: operator= ( mystring a)
{
  _size = a.size();
  _data = std :: move(a._data);
  return *this;
}

mystring & mystring :: operator= ( Genome < char > a)
{
  _size = a.size();
  _data = std :: move(a._data);
  return *this;
}

char mystring :: gen_random ()
{
  return charset[ std :: rand() % max_index_char_set ];
}

std :: ostream & operator << (std :: ostream & os, mystring s)
{
  std :: string tmp(s.get());
  os << tmp;
  return os;
}

