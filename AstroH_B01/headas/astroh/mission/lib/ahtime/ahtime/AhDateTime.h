/** 
 * \file AhDateTime.h
 * \brief Container storing date and time.
 * \author Mike Witthoeft
 * \date $Date: 2012/06/21 15:10:35 $
 *
 */
 
#ifndef AHTIME_AHDATETIME_H
#define AHTIME_AHDATETIME_H

#include <string>
#include <ctime>

#include "ahtime/ahtime.h"

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief alias for date/time container type
  typedef struct tm typ_datetime;


  /// \brief container storing date and time
  ///
  /// This class uses the ctime struct tm as a container for the following: 
  /// years, months, days, hours, minutes, and seconds.  Many methods are 
  /// defined to assist in populating this structure and comparing it with 
  /// other instances.
  class AhDateTime {
    private:
      typ_datetime m_datetime;    ///< struct storing all date/time components
      double m_subsecond;         ///< fraction of second not in m_datetime

    public:

      /// \brief initialize with date and time separately
      /// \param[in] date date string: "YYYY-MM-DD"
      /// \param[in] time time string: "HH:MM:SS"
      AhDateTime(std::string date, std::string time);

      /// \brief initialize with date and time together
      /// \param[in] datetime date/time string: "YYYY-MM-DD HH:MM:SS"
      AhDateTime(std::string datetime);

      /// \brief initialize with MJD
      /// \param[in] mjdi integral part of MJD
      /// \param[in] mjdf fractional part of MJD
      AhDateTime(int mjdi, double mjdf);

      /// \brief set to given date and time strings
      /// \param[in] date date string: "YYYY-MM-DD"
      /// \param[in] time time string: "HH:MM:SS"
      void set(std::string date, std::string time);

      /// \brief set to given date/time string
      /// \param[in] datetime date/time string: "YYYY-MM-DD HH:MM:SS"
      void set(std::string datetime);

      /// \brief set to given MJD
      /// \param[in] mjdi integral part of MJD
      /// \param[in] mjdf fractional part of MJD
      void setMJD(int mjdi, double mjdf);

      /// \brief return year
      /// \return year
      int year();

      /// \brief return month
      /// \return month
      int month();

      /// \brief return day
      /// \return day
      int day();

      /// \brief return hour
      /// \return hour
      int hour();

      /// \brief return minute
      /// \return minute
      int minute();

      /// \brief return second
      /// \return second
      int second();

      /// \brief return fractional part of second
      /// \return fractional part of second
      double subsecond();

      /// \brief return date/time as a string: "YYYY-MM-DD HH:MM:SS"
      /// \return formatted string
      std::string format();

      /// \brief compare with another instance; +1 means this > dt
      /// \param[in] dt AhDateTime instance to compare to
      /// \return +1,0,-1 based on order
      ///
      /// Compare stored date and time with those in another AhDateTime
      /// instance. Return values are +1, 0, or -1 depending on the 
      /// current time being later than, concurrent to, or earlier than the 
      /// given time.
      int compare(AhDateTime dt);

      /// \brief return result of this < dt
      /// \param[in] dt AhDateTime instance to compare to
      /// \return result of this < dt
      bool isBefore(AhDateTime dt);

      /// \brief return result of this <= dt
      /// \param[in] dt AhDateTime instance to compare to
      /// \return result of this <= dt
      bool isBeforeOrSame(AhDateTime dt);

      /// \brief return result of this > dt
      /// \param[in] dt AhDateTime instance to compare to
      /// \return result of this > dt
      bool isAfter(AhDateTime dt);

      /// \brief return result of this >= dt
      /// \param[in] dt AhDateTime instance to compare to
      /// \return result of this >= dt
      bool isAfterOrSame(AhDateTime dt);

      /// \brief return number of seconds elapsed since start of current year
      /// \return number of seconds
      int secIntoYear();

      /// \brief return number of seconds left in current year
      /// \return number of seconds
      int secLeftInYear();

      /// \brief return number of seconds since given AhDateTime instance
      /// \param[in] dt AhDateTime instance to compare to
      /// \return number of seconds
      int secSince(AhDateTime dt);

      /// \brief return number of seconds since given date/time string
      /// \param[in] datetime date/time string
      /// \return number of seconds
      int secSince(std::string datetime);

      /// \brief calculate integral and fractional part of MJD
      /// \param[out] mjdi integral part of MJD
      /// \param[out] mjdf fractional part of MJD
      void getMJD(int & mjdi, double & mjdf);

  };


  // ---- Unit test functions ----

  /// \brief Comprehensive tests of all comparison functions given four times:
  /// dt2 < (dt1=dt3) < dt4.
  /// \param[in] dt1 reference time
  /// \param[in] dt2 time earlier than dt1
  /// \param[in] dt3 time same as dt1
  /// \param[in] dt4 time later than dt1
  /// \return true/false based on success of tests
  bool ut_comparison_tests(AhDateTime dt1, AhDateTime dt2, 
                           AhDateTime dt3, AhDateTime dt4);

  /// \brief unit test for AhDateTime
  /// \return true/false based on success of tests
  bool ut_AhDateTime();


  /** @} */

}

#endif /* AHTIME_AHDATETIME_H */

/* Revision Log
 $Log: AhDateTime.h,v $
 Revision 1.2  2012/06/21 15:10:35  mwitthoe
 added support for Modified Julian Dates (MJD) in AhDateTime

 Revision 1.1  2012/06/19 21:31:33  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/05/29 14:47:46  mwitthoe
 created class AhDateTime to replace functions in ahdatetime.h; moved leap-second functions into ahleapsec.h

*/





