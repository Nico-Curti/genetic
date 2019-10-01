#include <feature_selection/feature_selection.h>

#ifdef __eigen__

feature_sel :: feature_sel ()
{
}

feature_sel :: feature_sel (std :: string filename)
{
  // Input file must be a matrix (Nsamples, Nfeatures)
  // with header as first row and delimiter == " "

  std :: ifstream is (filename);

  std :: string row;
  std :: vector < std :: string > token;

  std :: getline(is, row);

  token = this->split(row, " ");

  // the genome size must be equal to the number of features
  _size = token.size();

  is.unsetf(std :: ios_base :: skipws);
  int n_sample = std :: count(std :: istream_iterator < char >(is),
                              std :: istream_iterator < char >(),
                              '\n');
  is.clear();
  is.seekg(0, std :: ios :: beg);
  is.setf(std :: ios :: skipws);

  // re-skip the header
  std :: getline(is, row);

#ifdef _MPI
  _data.resize(n_sample * _size);
#else
  _data = std :: make_unique < std :: vector < float >[] >(n_sample * _size);
#endif

  int i = 0;
  float value;

  while ( is >> value )
  {
    _data[i] = std :: stof(tk);
    ++i;
  }

  is.close();
}

feature_sel & feature_sel :: operator = (feature_sel f)
{
  _size = f.size();
  _data = std :: move(s._data);
  return *this;
}

feature_sel & feature_sel :: operator = (Genome < float[] > g)
{
  _size = a.size();
  _data = std :: move(g._data);
  return *this;
}

float feature_sel :: gen_random ()
{
  // weights between 0 and 1
  return static_cast < float >( ((double) std :: rand() / (RAND_MAX)) + 1 );
}

float feature_sel :: ridge_train (Eigen :: VectorXf data, Eigen :: VectorXf label, double lambda)
{
  // center data
  float mean_data = std :: accumulate(data.get(), data.get() + _size, 0.f);

  std :: transform(data.get(), data.get() + _size,
                   data.get(), [&] (float & a)
                   {
                     return a - mean_data;
                   });

  float mean_label = label.mean();
  label = label.array() - mean_label;

  // Parameter estiamtion
  Eigen :: VectorXf beta = (data.transpose() * data + row * lamda * MatrixXf :: Identity(1, 1)).inverse() * data.transpose() * label;
  const float offset = mean_label - (beta * mean_data)(0, 0);

  // value of objective function
  const float obj = ((1 / row)* (label - data * beta).transpose() * (label - data * beta) + lamda * beta.transpose() * beta)(0, 0);

  return obj;
}

//float feature_sel :: ridge_predict (Eigen :: MatrixXf data)
//{
//  return (X * beta).array() + offset;
//}


std :: vector < std :: string > feature_sel :: split (const std :: string & text, const std :: string & delimiter)
{
  std :: vector < std :: string > token;

  std :: size_t pos = text.find_first_of(delimiter);
  std :: size_t start = 0;
  std :: size_t end = text.size();

  while (pos != std :: string :: npos)
  {
    if ( pos )
      token.emplace_back(text.substr(start, pos));

    start += pos + 1;

    pos = text.substr(start, end).find_first_of(delimiter);
  }

  if ( start != end )
    token.emplace_back(text.substr(start, pos));

  return token;
}

std :: ostream & operator << (std :: ostream & os, feature_sel & f)
{
  for ( std :: size_t i = 0; i < f.size(); ++i)
    os << f[i] << " ";
}


#endif // __eigen__
