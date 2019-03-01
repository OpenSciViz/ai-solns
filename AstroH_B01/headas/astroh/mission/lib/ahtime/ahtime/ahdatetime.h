/** 
 * \file ahdatetime.h
 * \brief data type and functions dealing with date/time quantities decomposed
 * as years, months, days, hours, minutes, and seconds.
 * \author Mike Witthoeft
 * \date $Date: 2012/06/19 21:31:34 $
 *
 * The library defines the type, datetime, which is an alias for "struct tm"
 * from the ctime library.  Several functions are defined which read/write
 * dates and times into the formats "YYYY-MM-DD" and "HH:MM:SS".  Also, 
 * conversion functions exist to get seconds elapsed between two quantities.
 * 
 */
 
#ifndef AHTIME_AHDATETIME_H_
#define AHTIME_AHDATETIME_H_

#include <ctime>

/// \ingroup mod_ahtime
/// \brief number of seconds in a day
#define SECS_IN_DAY 86400

/// \ingroup mod_ahtime
/// \brief number of seconds in a (non-leap) year
#define SECS_IN_YEAR 31536000

/// \ingroup mod_ahtime
/// \brief reference date for Astro-H
#define ASTROH_EPOCH "2012-01-01"

/// \ingroup mod_ahtime
/// \brief time at beginning of a new day
#define ZERO_TIME "00:00:00"


/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief Structure to hold year, month, day, hour, minute, second.
  typedef struct tm datetime;


  /// \brief Return number of days in given month index (1 = January); does not
  /// account for leap years.
  /// \param[in] imon index of month (1=January)
  /// \return number of days (28, 30, or 31)
  int daysInMonth(const int & imon); 


  /// \brief Return datetime object containing information from date string,
  /// YYYY-MM-DD, and time string, HH:MM:SS.
  /// \param[in] date string representation of date: YYYY-MM-DD
  /// \param[in] time string representation of time: HH:MM:SS
  /// \return datetime object with given date and time
  datetime initDateTime(std::string date, std::string time=""); 


  /// \brief Take string, YYYY-MM-DD, and populate datetime structure.  If
  /// given, take the time from old.
  /// \param[in] date string representation of date: YYYY-MM-DD
  /// \param[in] optional pointer to datetime object providing time
  /// \return datetime object with given date
  datetime parseDate(const std::string & date, const datetime* old=NULL); 


  /// \brief Take string, HH:MM:SS, and populate datetime structure.  If given,
  /// take the date from old.
  /// \param[in] date string representation of time: HH:MM:SS (need leading 
  /// zeros)
  /// \param[in] optional pointer to datetime object providing date
  /// \return datetime object with given time
  datetime parseTime(const std::string & time, const datetime* old=NULL); 


  /// \brief Take datetime and return formatted string for date.
  /// \param[in] dt datetime pointer providing date
  /// \return string of format, "YYYY-MM-DD"
  std::string fmtDate(const datetime* dt);


  /// \brief Take datetime and return formatted string for time.
  /// \param[in] dt datetime pointer providing time
  /// \return string of format, "HH:MM:SS"
  std::string fmtTime(const datetime* dt);


  /// \brief Take datetime pointer and return formatted string with date and
  /// time.
  /// \param[in] dt datetime pointer providing date and time
  /// \return string of format, "YYYY-MM-DD HH:MM:SS"
  std::string fmtDateTime(const datetime* dt);


  /// \brief Change date to ASTROH_EPOCH.
  /// \param[in] dt datetime pointer 
  /// \return datetime with AstroH epoch date and time from dt
  datetime clearDate(const datetime* dt);


  /// \brief Clear time in datetime object.
  /// \param[in] dt datetime pointer 
  /// \return datetime with midnight as the time and the date from dt
  datetime clearTime(const datetime* dt);


  /// \brief Set the datetime to the beginning of the day on ASTROH_EPOCH.
  /// \param[in] dt datetime pointer 
  /// \return datetime with date and time reset
  datetime clearDateTime(const datetime* dt);


  /// \brief Return -1,0,1 based on order of two given times: dt1 and dt2:
  ///  - dt1 < dt2:   -1
  ///  - dt1 = dt2:    0
  ///  - dt1 > dt2:   +1
  /// \param[in] dt1 datetime pointer (reference)
  /// \param[in] dt2 datetime pointer (other)
  /// \return -1, 0, or +1 based on sort order of arguments
  int sortDateTime(const datetime* dt1, const datetime* dt2);


  /// \brief Return number of seconds from beginning of year to given date; 
  /// will include a leap day, if necessary.
  /// \param[in] dt datetime pointer
  /// \return number of seconds
  int secIntoYear(const datetime* dt);


  /// \brief Return number of seconds until the end of the year of the given 
  /// date; will include a leap day, if necessary.
  /// \param[in] dt datetime pointer
  /// \return number of seconds
  int secLeftInYear(const datetime* dt);


  /// \brief Return number of seconds between the times: dt1-dt2; leap days are
  /// included, but not leap seconds.
  /// \param[in] dt1 datetime pointer of larger date/time
  /// \param[in] dt2 datetime pointer of smaller date/time
  /// \return number of seconds in interval (negative means dt1 < dt2)
  long secDiff(datetime* dt1, datetime* dt2);


  /// \brief Return number of seconds between Astro-H epoch and given time, dt;
  /// leap days are included, but not leap seconds.
  /// \param[in] dt datetime pointer
  /// \return number of seconds (negative means dt before Astro-H epoch)
  long secSinceEpoch(datetime* dt);



  /** @} */

}

#endif /* AHTIME_AHDATETIME_H_ */

/* Revision Log
 $Log: ahdatetime.h,v $
 Revision 1.1  2012/06/19 21:31:34  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/05/23 23:27:46  mwitthoe
 add ahdatetime defining a structure to hold a date and time (and functions operating on it); add functions to ahtime to determine number of leap seconds in a date/time interval; remove AhTimeElem from ahtime (but class still present)

*/
