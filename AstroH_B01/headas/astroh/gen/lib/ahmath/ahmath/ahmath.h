/** 
 * \file ahmath.h
 * \brief general mathematical routines
 * \author Mike Witthoeft
 * \date $Date: 2012/06/28 23:14:48 $
 * 
 */
 
#ifndef AHMATH_AHMATH_H
#define AHMATH_AHMATH_H

#include <vector>
#include <string>

/// \ingroup mod_ahmath
namespace ahmath {

  /** \addtogroup mod_ahmath
   *  @{
   */

  /// \brief define vector of doubles
  typedef std::vector<double> vecdbl;
  typedef std::vector<long long> vecllong;

  /// \brief aliases interpolation types
  enum interptypes {
    NEAREST,             ///< return nearest point
    TWOPOINT,            ///< linear interpolation
    FOURPOINT            ///< 4-point Aitken method
  };

  /// \brief convert interpolation type string into enumeration type
  /// \param[in] interpolation type string
  /// \return interpolation enumeration
  int interpolation_type(const std::string & interp);

  /// \brief Perform interpolation of a single value.
  /// \param[in] x value where to interpolate
  /// \param[in] xarr array of x-values
  /// \param[in] yarr array of y-values
  /// \param[in] itype interpolation type (see enum interptypes)
  /// \param[in] extrap set to true to allow for extrapolation
  /// \return interpolated value, y
  double interpolate_single(const double & x, vecdbl & xarr, vecdbl & yarr, 
                            const int & itype, const bool & extrap=false); 

  /// \brief Perform interpolation of a single value.
  /// \param[in] x value where to interpolate
  /// \param[in] xarr array of x-values
  /// \param[in] yarr array of y-values
  /// \param[in] itype interpolation type (see enum interptypes)
  /// \param[in] extrap set to true to allow for extrapolation
  /// \return interpolated value, y
  double interpolate_single(const long long & x, vecllong & xarr, vecdbl & yarr, 
                            const int & itype, const bool & extrap=false); 

  /// \brief Perform interpolation of several (sorted) values.
  /// \param[in] xvec vector of x-position where to interpolate
  /// \param[in] xarr array of x-values
  /// \param[in] yarr array of y-values
  /// \param[in] itype interpolation type (see enum interptypes)
  /// \param[out] yvec output array of y interpolated values
  void interpolate_many(vecdbl & x, vecdbl & xarr, vecdbl & yarr, 
                        int & itype, vecdbl & yvec);

  /// \brief Perform interpolation of several (sorted) values.
  /// \param[in] xvec vector of x-position where to interpolate
  /// \param[in] xarr array of x-values
  /// \param[in] yarr array of y-values
  /// \param[in] itype interpolation type (see enum interptypes)
  /// \param[out] yvec output array of y interpolated values
  void interpolate_many(vecllong & x, vecllong & xarr, vecdbl & yarr, 
                        int & itype, vecdbl & yvec);

  /** @} */

}

#endif /* AHMATH_AHMATH_H */

/* Revision Log
 $Log: ahmath.h,v $
 Revision 1.4  2012/06/28 23:14:48  mwitthoe
 ahmath: add function to convert string into interpolation type enumerated index; ahfits: small fix to error message

 Revision 1.3  2012/06/28 02:05:33  mwitthoe
 ahmath: add option in interpolation functions to allow for extrapolation without throwing an error

 Revision 1.2  2012/06/25 14:06:31  mwitthoe
 add interpolation functions to ahmath which work on integer X-values and double Y-values

 Revision 1.1  2012/06/21 19:03:02  mwitthoe
 add ahmath with an interpolation function


*/
