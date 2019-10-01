#ifndef __feature_selection_h__
#define __feature_selection_h__

#include <numeric>
#include <fstream>    // std :: ifstream
#include <sstream>    // std :: stringstream
#include <vector>     // std :: vector
#include <string>     // std :: string
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


class feature_sel : public Genome < float[] >
{

public:

  // Members

  Eigen :: VectorXf labels;

  // Constructors

  feature_sel ();
  feature_sel (std :: string & filename);
  feature_sel & operator = ( feature_sel a );
  feature_sel & operator = ( Genome < float[] > g );

  // Destructors

  feature_sel () = default;

  // Member functions


  // Static members

  // Source : https://github.com/iroot900/A-Machine-Learning-Library
  static float ridge_train (Eigen :: MatrixXf data, Eigen :: VectorXf label, double lambda);
  //static float ridge_predict (Eigen :: MatrixXf X);

private:

  // Private members

  std :: vector < std :: string > split (const std :: string & text, const std :: string & delimiter);
};

std :: ostream & operator << (std :: ostream & os, feature_sel f);

__unused struct
{
  float operator () (feature_sel weights, feature_sel full_set_of_features)
  {
    // take the data inside weights
    // weight each feature by the genome
    // compute the ridge regression against the label
  }

} fitness;


#endif // __feature_selection_h__
