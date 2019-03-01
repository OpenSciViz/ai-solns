/** 
 * \file AhTimeTT.h
 * \brief Container of Terrestrial Time (TT) using the Modified Julian Date 
 * (MJD).
 * \author Mike Witthoeft
 * \date $Date: 2012/07/13 18:32:07 $
 *
 */
 
#ifndef AHTIME_AHTIMETT_H
#define AHTIME_AHTIMETT_H

#include "ahtime/ahtime.h"

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  // forward declaration
  class AhTimeTT;

  /// \brief calculate number of seconds in interval of two TT times.
  /// \param[in] t1 lower limit
  /// \param[in] t2 upper limit
  /// \param[out] subseconds if not NULL, fill in subseconds
  /// \return whole number of seconds (not rounded)
  int numSecInInterval(AhTimeTT & t1, AhTimeTT & t2, double* subseconds);

  /// \brief unit test for AhTimeUTC
  bool ut_AhTimeTT();

  /// \brief container storing date and time
  ///
  /// This class stores the Modified Julian Date.  The integral and fractional
  /// portion of the date are stored separately.
  class AhTimeTT {
    private:
      int m_mjdi;               ///< integral part of MJD date
      double m_mjdf;            ///< fractional part of MJD [0.0:1.0)

    public:

      /// \brief initialize with zero
      AhTimeTT();

      /// \brief initialize with integral and fractional part separately
      /// \param[in] mjdi integral part of MJD
      /// \param[in] mjdf fractional part of MJD
      AhTimeTT(const int & mjdi, const double & mjdf);

      /// \brief initialize using single MJD value
      /// \param[in] mjd MJD value
      AhTimeTT(const double & mjd);

      /// \brief set date using integral and fractional part separately
      /// \param[in] mjdi integral part of MJD
      /// \param[in] mjdf fractional part of MJD
      void set(const int & mjdi, const double & mjdf);

      /// \brief set date using single MJD value
      /// \param[in] mjd MJD value
      void set(const double & mjd);

      /// \brief return MJD value
      /// \return MJD
      double mjd();

      /// \brief return integral part of MJD
      /// \return whole part of MJD
      int mjdi();

      /// \brief return fractional part of MJD
      /// \return fractional part of MJD
      double mjdf();

      /// \brief compare with another instance; +1 means this > dt
      /// \param[in] dt AhTimeTT instance to compare to
      /// \return +1,0,-1 based on order
      int compare(AhTimeTT & dt);

  };


  /** @} */

}

#endif /* AHTIME_AHTIMETT_H */

/* Revision Log
 $Log: AhTimeTT.h,v $
 Revision 1.4  2012/07/13 18:32:07  mwitthoe
 clean up ahtime code

 Revision 1.3  2012/07/12 17:51:19  mwitthoe
 small changes to AhTime{UTC|TT|TAI} class descriptions

 Revision 1.2  2012/07/12 12:12:50  mwitthoe
 standardize and consolidate ahtime unit tests

 Revision 1.1  2012/07/11 16:26:19  mwitthoe
 new ahtime libraries and test codes: ahleapsec - new library to read leap second data from CALDB; AhTimeTAI - library to store number of seconds relative to an epoch; AhTimeTT - library to store modified Julian date (MJD); AhTimeUTC - library to store UTC times; ahtimeconv - conversion functions between 3 AhTime* classes


*/
