/** 
 * \file ahtime.h
 * \brief General time constants and functions.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:53 $
 * 
 */
 
#ifndef AHTIME_AHTIME_H
#define AHTIME_AHTIME_H

#include <string>

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief number of seconds in a minute
  const int SECS_IN_MINUTE=60;

  /// \brief number of seconds in an hour
  const int SECS_IN_HOUR=SECS_IN_MINUTE*60;

  /// \brief number of seconds in a day
  const int SECS_IN_DAY=SECS_IN_HOUR*24;

  /// \brief number of seconds in a (non-leap) year
  const int SECS_IN_YEAR=SECS_IN_DAY*365;

  // if a subsecond is within this threshold of 0/1, then it will be
  // rounded down/up (to take care of some numerical precision issues)
  const double SUBSECOND_THRESHOLD=1.e-10;

  /// \brief GPS epoch as a single string
  const std::string GPS_EPOCH="1980-01-06T00:00:00";

  /// \brief year of minimum UTC date
  const int MINDATE_YEAR=1972;

  /// \brief month of minimum UTC date
  const int MINDATE_MON=1;

  /// \brief day of minimum UTC date
  const int MINDATE_DAY=1;

  /// \brief Return true if given year is a leap year.
  /// \param[in] year integer year, YYYY
  /// \return true if given a leap year, false otherwise
  bool isLeapYear(const int & year); 

  /// \brief Return number of days in given month index (1 = January); does not
  /// account for leap years.
  /// \param[in] imon index of month (1=January)
  /// \return number of days (28, 30, or 31)
  int daysInMonth(const int & imon); 

  /// \brief Return number of days in given month index (1 = January); does
  /// account for leap years.
  /// \param[in] imon index of month (1=January)
  /// \param[in] year year
  /// \return number of days
  int daysInMonth(const int & imon, const int & year); 

  /// \brief combine time string with fractional part of seconds
  /// \param[in] time time string
  /// \param[in] subseconds fractional part of seconds: [0.:1.)
  /// \param[in] digits number of decimals to retain
  /// \return time string with fractional part of seconds appended
  ///
  /// This function will combine a time string (e.g. 12:34:10) with a fractional
  /// number of seconds (e.g. 0.234) to get a new string with both pieces of
  /// information (e.g. 12:34:10.234).  The fractional part is rounded to 
  /// conform to the value of the digits parameter.
  std::string catTimeSubseconds(const std::string & time, 
                                const double & subseconds, const int & digits);

  /// \brief split from given time string the fractional part of the seconds
  ///   count; this is the reverse of catTimeSubseconds()
  /// \param[in] instr input time string with fractional number of seconds
  /// \param[out] time time string with whole number of seconds
  /// \param[out] subseconds fractional number of seconds
  /// \return false if no fractional part is found in input string
  bool uncatTimeSubseconds(const std::string & instr, std::string & time,
                           double & subseconds);

  /// \brief write results of a test section using logger
  /// \param[in] name of test section
  /// \param[in] okay true if test was successful
  bool ut_report(std::string section, bool okay);

  /// \brief unit test for ahtime.h
  bool ut_ahtime();

  /** @} */

}

#endif /* AHTIME_AHTIME_H */

/* Revision Log
 $Log: ahtime.h,v $
 Revision 1.6  2012/07/16 21:02:53  mwitthoe
 update ahtime library/tool header comments

 Revision 1.5  2012/07/13 19:17:22  mwitthoe
 set 1972-01-01 as the minimum date supported by AhTimeUTC

 Revision 1.4  2012/07/13 18:32:07  mwitthoe
 clean up ahtime code

 Revision 1.3  2012/07/12 12:12:50  mwitthoe
 standardize and consolidate ahtime unit tests

 Revision 1.2  2012/07/11 16:19:05  mwitthoe
 remove obsolete libraries from ahtime

 Revision 1.1  2012/06/19 21:31:34  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.6  2012/06/07 11:11:39  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

 Revision 1.5  2012/05/29 14:47:47  mwitthoe
 created class AhDateTime to replace functions in ahdatetime.h; moved leap-second functions into ahleapsec.h

 Revision 1.4  2012/05/23 23:27:46  mwitthoe
 add ahdatetime defining a structure to hold a date and time (and functions operating on it); add functions to ahtime to determine number of leap seconds in a date/time interval; remove AhTimeElem from ahtime (but class still present)

 Revision 1.3  2012/04/24 19:16:18  mwitthoe
 documentation for ahtime

*/
