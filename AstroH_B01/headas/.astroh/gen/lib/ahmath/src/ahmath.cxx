#include "ahmath/ahmath.h"
#include "ahlog/ahlog.h"

#include <algorithm>

namespace ahmath {

// ****************************************************************************

  /// \callgraph
  int interpolation_type(const std::string & interp) {
    std::string chk=(std::string)interp;
    std::transform(chk.begin(),chk.end(),chk.begin(),::tolower);
    if (chk == "nearest")
      return NEAREST;
    else if (chk == "twopoint")
      return TWOPOINT;
    else if (chk == "fourpoint")
      return FOURPOINT;
    else
      AH_THROW_RUNTIME("invalid interpolation type");

    // should never reach here
    return NEAREST;
  }

// ****************************************************************************

  /// \callgraph
  /// \internal
  /// \note not checking that xarr is sorted
  /// \note may need to check for jumps in array values; with TI, for example,
  /// the time values reset after a fixed period.
  double interpolate_single(const double & x, vecdbl & xarr, vecdbl & yarr,
                            const int & itype, const bool & extrap) {
    // check for valid point arrays
    int narr=xarr.size();
    if (0 == narr) AH_THROW_RUNTIME("value arrays are empty");
    if (yarr.size() != narr) AH_THROW_RUNTIME("inconsistent array dimensions");

    // find position of x in point array
    int ipos;
    if (x < xarr[0]) {
      if (!extrap) AH_THROW_RUNTIME("x-value out of range (too small)");
      ipos=1;
    } else if (x > xarr[narr-1]) {
      if (!extrap) AH_THROW_RUNTIME("x-value out of range (too large)");
      ipos=narr-1;
    } else {
      ipos=1;
      while (x > xarr[ipos]) ipos++;
    }

    // distances to adjacent points
    double dx1=x-xarr[ipos-1];
    double dx2=xarr[ipos]-x;

    switch (itype) {

      case NEAREST:
        if (dx1 < dx2) return yarr[ipos-1];
        return yarr[ipos];

      case TWOPOINT:
        return (dx1*yarr[ipos]+dx2*yarr[ipos-1])/(dx1+dx2);

      case FOURPOINT:
        AH_THROW_RUNTIME("4-point interpolation not implemented");

      default:
        AH_THROW_RUNTIME("invalid interpolation type; see enum interptypes");
    }

    // just for the compiler's sake
    AH_THROW_RUNTIME("sanity check: this should never be reached");
    return 0.0;
  }

// ****************************************************************************

  /// \callgraph
  /// \internal
  /// \note not checking that xarr is sorted
  /// \note may need to check for jumps in array values; with TI, for example,
  /// the time values reset after a fixed period.
  double interpolate_single(const long long & x, vecllong & xarr, vecdbl & yarr,
                            const int & itype, const bool & extrap) {
    // check for valid point arrays
    int narr=xarr.size();
    if (0 == narr) AH_THROW_RUNTIME("value arrays are empty");
    if (yarr.size() != narr) AH_THROW_RUNTIME("inconsistent array dimensions");

    // find position of x in point array
    int ipos;
    if (x < xarr[0]) {
      if (!extrap) AH_THROW_RUNTIME("x-value out of range (too small)");
      ipos=1;
    } else if (x > xarr[narr-1]) {
      if (!extrap) AH_THROW_RUNTIME("x-value out of range (too large)");
      ipos=narr-1;
    } else {
      ipos=1;
      while (x > xarr[ipos]) ipos++;
    }

    // distances to adjacent points
    long long dx1=(double)(x-xarr[ipos-1]);
    long long dx2=(double)(xarr[ipos]-x);

    switch (itype) {

      case NEAREST:
        if (dx1 < dx2) return yarr[ipos-1];
        return yarr[ipos];

      case TWOPOINT:
        return (dx1*yarr[ipos]+dx2*yarr[ipos-1])/(dx1+dx2);

      case FOURPOINT:
        AH_THROW_RUNTIME("4-point interpolation not implemented");

      default:
        AH_THROW_RUNTIME("invalid interpolation type; see enum interptypes");
    }

    // just for the compiler's sake
    AH_THROW_RUNTIME("sanity check: this should never be reached");
    return 0.0;
  }

// ****************************************************************************

}



