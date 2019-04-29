#include <images/myimage.h>

#ifdef __images__

image :: image ()
{
}

image :: image (std :: string filename)
{
  cv :: Mat im = cv :: imread(filename);
  _w = im.cols;
  _h = im.rows;
  const int c = im.channels();
  _size = _w * _h * c;
#ifdef _MPI
  _data.resize(_size);
#else
  _data = std :: make_unique < unsigned char[] >(_size);
#endif
  unsigned char *im_data = (unsigned char*)(im.data);

#ifdef _MPI
  std :: copy_n(im_data, _size, _data.begin());
#else
  std :: copy_n(im_data, _size, _data.get());
#endif
}

image & image :: operator = ( image a)
{
  _w = a._w;
  _h = a._h;
  _size = a.size();
  _data = std :: move(a._data);
  return *this;
}

image & image :: operator = (Genome < unsigned char > g)
{
  _size = g.size();
  _data = std :: move(g._data);
  return *this;
}

void image :: show ( void )
{
  cv :: Mat disp (_h, _w, CV_8UC3);
  std :: copy_n(_data.get(), _size, disp.data);
  cv :: imshow("myimage", disp);
  cv :: waitKey(0);
}

void image :: save ( std :: string filename )
{
  cv :: Mat disp (_w, _h, CV_8UC3);
  std :: copy_n(_data.get(), _size, disp.data);
  cv :: imwrite(filename, disp);
}

unsigned char image :: gen_random ()
{
  return static_cast <unsigned char> (std :: rand() % 255);
}

std :: ostream & operator << (std :: ostream & os, image im)
{
  os << '(' << im._w << ',' << im._h << ") " << im.size();
  return os;
}

#endif // __images__
