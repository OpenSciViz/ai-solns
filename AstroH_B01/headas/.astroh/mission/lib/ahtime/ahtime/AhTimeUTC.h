/** 
 * \file AhTimeUTC.h
 * \brief Container for UTC date and time.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/20 14:47:39 $
 *
 */
 
#ifndef AHTIME_AHTIMEUTC_H
#define AHTIME_AHTIMEUTC_H

#include "ahtime/ahtime.h"
#include "ahtime/ahleapsec.h"

#include <string>
#include <ctime>

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  // forward declaration
  class AhTimeUTC;

  /// \brief alias for date/time container type
  typedef struct tm typ_datetime;

  /// \brief return number of leap seconds between two dates (inclusive)
  /// \param[in] t1 smaller UTC time
  /// \param[in] t2 larger UTC time
  /// \return number of leap seconds
  int numLeapSecInInterval(AhTimeUTC & t1, AhTimeUTC & t2);

  /// \brief return number of leap seconds occurring before given time
  /// \param[in] time UTC time
  /// \return number of leap seconds
  int numLeapSecBefore(AhTimeUTC & time);

  /// \brief calculate number of seconds in interval of two UTC times.
  /// \param[in] t1 lower limit
  /// \param[in] t2 upper limit
  /// \param[out] subseconds if not NULL, fill in subseconds
  /// \return whole number of seconds (not rounded)
  int numSecInInterval(AhTimeUTC & t1, AhTimeUTC & t2, double* subseconds);

  /// \brief unit test for AhTimeUTC
  bool ut_AhTimeUTC();

  /// \brief container storing date and time
  ///
  /// This class uses the ctime struct tm as a container for the following: 
  /// years, months, days, hours, minutes, and seconds.  A separate variable
  /// exists to hold a fractional second in the range [0,1).  All operations
  /// acting on AhTimeUTC instances automatically account for leap seconds.
  /// Therefore, leap second data must be loaded before instantiating an
  /// AhTimeUTC variable; see ahleapsec.  
  class AhTimeUTC {
    private:
      typ_datetime m_datetime;    ///< struct storing all date/time components
      double m_subsecond;         ///< fraction of second not in m_datetime

    public:


      /// \brief initialize the GPS epoch
      AhTimeUTC();

      /// \brief initialize with separate year, month, day, etc
      /// \param[in] year 4-digit year
      /// \param[in] month index (1 = January)
      /// \param[in] day
      /// \param[in] hour (0-23)
      /// \param[in] minute (0-59)
      /// \param[in] second (0-60; 60 if leap second)
      /// \param[in] subsecond fraction of second [0.0:1.0)
      AhTimeUTC(const int & year, const int & month, const int & day, 
                const int & hour, const int & minute, const int & second,
                const double & subsecond);

      /// \brief initialize with date and time as separate strings
      /// \param[in] date date string: "YYYY-MM-DD"
      /// \param[in] time time string: "HH:MM:SS"
      /// \param[in] subsecond fraction of second [0.0:1.0)
      AhTimeUTC(const std::string & date, const std::string & time, 
                const double & subsecond);

      /// \brief initialize with date and time as a single string
      /// \param[in] datetime date/time string: "YYYY-MM-DD HH:MM:SS"
      /// \param[in] subsecond fraction of second [0.0:1.0)
      AhTimeUTC(const std::string & datetime, const double & subsecond);

      /// \brief initialize with date and time as separate integers
      /// \param[in] date integer date: 10000*year+100*month+day
      /// \param[in] time integer time: 10000*hour+100*minute+second
      /// \param[in] subsecond fraction of second [0.0:1.0)
      AhTimeUTC(const int & date, const int & time, const double & subsecond);

      /// \brief set to given date and time strings
      /// \param[in] date date string: "YYYY-MM-DD"
      /// \param[in] time time string: "HH:MM:SS"
      /// \param[in] subsecond fraction of second [0.0:1.0)
      void set(const std::string & date, const std::string & time,
               const double & subsecond);

      /// \brief set date and time using separate integers
      /// \param[in] date integer date: 10000*year+100*month+day
      /// \param[in] time integer time: 10000*hour+100*minute+second
      /// \param[in] subsecond fraction of second [0.0:1.0)
      void set(const int & date, const int & time, const double & subsecond);

      /// \brief set to given date/time string
      /// \param[in] datetime date/time string: "YYYY-MM-DD HH:MM:SS"
      /// \param[in] subsecond fraction of second [0.0:1.0)
      void set(const std::string & datetime, const double & subsecond);

      /// \brief initialize with separate year, month, day, etc
      /// \param[in] year 4-digit year
      /// \param[in] month index (1 = January)
      /// \param[in] day
      /// \param[in] hour (0-23)
      /// \param[in] minute (0-59)
      /// \param[in] second (0-60; 60 if leap second)
      /// \param[in] subsecond fraction of second [0.0:1.0)
      void set(const int & year, const int & month, const int & day,
               const int & hour, const int & minute, const int & second,
               const double & subsecond);

      /// \brief return year
      /// \return year
      int year() const;

      /// \brief return month
      /// \return month
      int month() const;

      /// \brief return day
      /// \return day
      int day() const;

      /// \brief return hour
      /// \return hour
      int hour() const;

      /// \brief return minute
      /// \return minute
      int minute() const;

      /// \brief return second
      /// \return second
      int second() const;

      /// \brief return fractional part of second
      /// \return fractional part of second
      double subsecond() const;

      /// \brief compare with another instance; +1 means this > dt
      /// \param[in] dt AhTimeUTC instance to compare to
      /// \return +1,0,-1 based on order
      ///
      /// Compare stored date and time with those in another AhTimeUTC
      /// instance. Return values are +1, 0, or -1 depending on the 
      /// current time being later than, concurrent to, or earlier than the 
      /// given time.
      int compare(const AhTimeUTC & dt) const;

      /// \brief return date/time as a string: "YYYY-MM-DDTHH:MM:SS"
      /// \return formatted string
      std::string string_datetime();

      /// \brief return date as a string: "YYYY-MM-DD"
      /// \return formatted string
      std::string string_date();

      /// \brief return time as a string: "HH:MM:SS"
      /// \return formatted string
      std::string string_time();

      /// \brief return date as an integer: 10000*year+100*month+day
      /// \return date as integer
      int integer_date();

      /// \brief return time as an integer: 10000*hour+100*minute+second
      /// \return time as integer
      int integer_time();

      /// \brief return subsecond count to given precision (will round values
      /// of finer precision)
      /// \param[in] ndigits number of digits to return
      /// \return 10^n * subsecond
      int integer_subsecond(const int & ndigits);

      /// \brief return number of seconds elapsed since start of current year
      /// (leap second is discounted, if applicable)
      /// \param[out] subseconds fill with number of subseconds if not NULL
      /// \return number of seconds
      int _secIntoYear(double* subseconds);

      /// \brief return number of seconds left in current year (leap second is
      /// discounted, if applicable; _secLeftInYear()+_secIntoYear() = 86400)
      /// \param[out] subseconds fill with number of subseconds if not NULL
      /// \return number of seconds
      int _secLeftInYear(double* subseconds);

      /// \brief add the given number of seconds (and subseconds) to time
      ///   (does not account for leap seconds)
      /// \param[in] seconds integer number of seconds
      /// \param[in] subseconds fractional number of seconds
      void _addSeconds(const int & seconds, const double & subseconds);

      /// \brief substract the given number of seconds (and subseconds) from
      ///   time (does not account for leap seconds)
      /// \param[in] seconds integer number of seconds
      /// \param[in] subseconds fractional number of seconds
      void _subtractSeconds(const int & seconds, const double & subseconds);

      /// \brief add one second to time without checking if minutes (and so on)
      ///   need to be incremented; this is used to allow for the number of 
      ///   seconds to be 60 when time is at a leap second time.
      void _addOneSecondForce();

  };


  /** @} */

}

#endif /* AHTIME_AHTIMEUTC_H */

/* Revision Log
 $Log: AhTimeUTC.h,v $
 Revision 1.6  2012/07/20 14:47:39  mwitthoe
 add const to ahtime year(), month(), etc functions

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
