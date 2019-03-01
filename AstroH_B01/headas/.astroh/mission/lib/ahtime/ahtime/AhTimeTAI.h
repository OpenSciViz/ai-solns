/** 
 * \file AhTimeTAI.h
 * \brief Container of International Atomic Time (TAI) relative to a UTC epoch.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:52 $
 *
 */
 
#ifndef AHTIME_AHTIMETAI_H
#define AHTIME_AHTIMETAI_H

#include "ahtime/AhTimeUTC.h"
#include "ahtime/ahtime.h"

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  // forward declaration
  class AhTimeTAI;

  /// \brief calculate number of seconds in interval of two TAI times.
  /// \param[in] t1 lower limit
  /// \param[in] t2 upper limit
  /// \param[out] subseconds if not NULL, fill in subseconds
  /// \return whole number of seconds (not rounded)
  int numSecInInterval(AhTimeTAI & t1, AhTimeTAI & t2, double* subseconds);

  /// \brief unit test for AhTimeTAI
  bool ut_AhTimeTAI();

  /// \brief container storing date and time
  ///
  /// This class stores a number of seconds since a UTC epoch.  The number of
  /// seconds is stored in two parts: whole part (integral) and a fraction.
  /// Since this class requires AhTimeUTC, leap second data must be loaded
  /// before declaring AhTimeTAI variables; see ahleapsec.
  class AhTimeTAI {
    private:
      AhTimeUTC m_epoch;        ///< UTC epoch
      int m_seci;               ///< integral part
      double m_secf;            ///< fractional part [0.0:1.0)

    public:

      /// \brief initialize with default UTC epoch and zero seconds
      AhTimeTAI();

      /// \brief initialize with integral and fractional part separately
      /// \param[in] seci whole number of seconds
      /// \param[in] secf fractional part of seconds
      /// \param[in] epoch UTC epoch
      AhTimeTAI(int seci, double secf, AhTimeUTC & epoch);

      /// \brief initialize using single value for seconds
      /// \param[in] sec number of seconds
      /// \param[in] epoch UTC epoch
      AhTimeTAI(double sec, AhTimeUTC & epoch);

      /// \brief set time with integral and fractional part separately
      /// \param[in] seci whole number of seconds
      /// \param[in] secf fractional part of seconds
      /// \param[in] epoch UTC epoch
      void set(int seci, double secf, AhTimeUTC & epoch);

      /// \brief set time using single value for seconds
      /// \param[in] sec number of seconds
      /// \param[in] epoch UTC epoch
      void set(double sec, AhTimeUTC & epoch);

      /// \brief change epcoh to given value and change time accordingly; epoch
      ///   cannot cause the time to be negative
      /// \param[in] epoch UTC epoch
      void set_epoch(AhTimeUTC & epoch);

      /// \brief return the epoch
      /// \return epoch
      AhTimeUTC epoch();

      /// \brief return number of seconds
      /// \return seconds
      double seconds();

      /// \brief return rounded number of seconds
      /// \return seconds
      int round();

      /// \brief return truncated number of seconds
      /// \return seconds
      int truncate();

      /// \brief return subseconds
      /// \return fraction of second [0.0:1.0)
      double subsecond();

      /// \brief compare with another instance; +1 means this > dt
      /// \param[in] dt AhTimeTAI instance to compare to
      /// \return +1,0,-1 based on order
      int compare(AhTimeTAI dt);

  };


  /** @} */

}

#endif /* AHTIME_AHTIMETAI_H */

/* Revision Log
 $Log: AhTimeTAI.h,v $
 Revision 1.5  2012/07/16 21:02:52  mwitthoe
 update ahtime library/tool header comments

 Revision 1.4  2012/07/13 18:32:07  mwitthoe
 clean up ahtime code

 Revision 1.3  2012/07/12 17:51:19  mwitthoe
 small changes to AhTime{UTC|TT|TAI} class descriptions

 Revision 1.2  2012/07/12 12:12:50  mwitthoe
 standardize and consolidate ahtime unit tests

 Revision 1.1  2012/07/11 16:26:19  mwitthoe
 new ahtime libraries and test codes: ahleapsec - new library to read leap second data from CALDB; AhTimeTAI - library to store number of seconds relative to an epoch; AhTimeTT - library to store modified Julian date (MJD); AhTimeUTC - library to store UTC times; ahtimeconv - conversion functions between 3 AhTime* classes


*/
