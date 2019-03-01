#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>

#include "ahlog/ahlog.h"
#include "ahtime/ahdatetime.h"


namespace ahtime {

  // **************************************************************************

  /// \callgraph
  int daysInMonth(const int & imon){
    switch (imon) {
      case 2:
        return 28;
      case 4:
      case 6:
      case 9:
      case 11:
        return 30;
    }
    return 31;
  }

  // **************************************************************************

  datetime initDateTime(std::string date, std::string time){
    // check if time is empty string, meaning that date and time are together
    if (time.size() == 0) {
      if (date.size() != 19) {
        std::string errmsg="expecting date and time in single string which has the wrong length";
        throw std::runtime_error(AH_PREP(errmsg));
      }
      time=date.substr(11,8);
      date=date.substr(0,10);
    }

    datetime out;
    out=parseDate(date);
    out=parseTime(time,&out);
    return out;
  }


  // **************************************************************************

  /// \callgraph
  datetime parseDate(const std::string & date, const datetime* old) {

    // error for inproperly formatted date
    std::stringstream serr;
    serr << "invalid date format: " << date << "; must be YYYY-MM-DD";
    const char* errmsg=serr.str().c_str();

    // check for valid input
    if (date.size() != 10) throw std::runtime_error(AH_PREP(errmsg));
    if (date[4] != '-' || date[7] != '-') 
      throw std::runtime_error(AH_PREP(errmsg));

    // get year, month, day
    int year,mon,mday;
    try {
      year=atoi(date.substr(0,4).c_str());
      mon=atoi(date.substr(5,2).c_str());
      mday=atoi(date.substr(8,2).c_str());
    } catch(const std::exception &x) {
      throw std::runtime_error(AH_PREP(errmsg));
    }

    // prepare output
    datetime out;
    if (old != NULL) out=*old;

    // fill output
    out.tm_year=year-1900;      // counted from 1900
    out.tm_mon=mon-1;           // month range 0-11
    out.tm_mday=mday;           // day range 1-31

    return out;
  }

  // **************************************************************************

  /// \callgraph
  datetime parseTime(const std::string & time, const datetime* old) {

    // error for inproperly formatted time
    std::stringstream serr;
    serr << "invalid time format: " << time << "; must be HH:MM:SS";
    const char* errmsg=serr.str().c_str();

    // check for valid input
    if (time.size() != 8) throw std::runtime_error(AH_PREP(errmsg));
    if (time[2] != ':' || time[5] != ':') 
      throw std::runtime_error(AH_PREP(errmsg));

    // get hour, min, sec
    int hour,min,sec;
    try {
      hour=atoi(time.substr(0,2).c_str());
      min=atoi(time.substr(3,2).c_str());
      sec=atoi(time.substr(6,2).c_str());
    } catch(const std::exception &x) {
      throw std::runtime_error(AH_PREP(errmsg));
    }

    // prepare output
    datetime out;
    if (old != NULL) out=*old;

    // fill output
    out.tm_hour=hour;     // range: 0-23
    out.tm_min=min;       // range: 0-59
    out.tm_sec=sec;       // range: 0-61  (account for leap seconds)

    return out;
  }

  // **************************************************************************

  /// \callgraph
  std::string fmtDate(const datetime* dt) {
    std::stringstream out;
    out << std::setw(4) << std::setfill('0') << (dt->tm_year+1900);
    out << "-";
    out << std::setw(2) << std::setfill('0') << (dt->tm_mon+1);
    out << "-";
    out << std::setw(2) << std::setfill('0') << dt->tm_mday;
    return out.str();
  }

  // **************************************************************************

  /// \callgraph
  std::string fmtTime(const datetime* dt) {
    std::stringstream out;
    out << std::setw(2) << std::setfill('0') << (dt->tm_hour);
    out << ":";
    out << std::setw(2) << std::setfill('0') << (dt->tm_min);
    out << ":";
    out << std::setw(2) << std::setfill('0') << (dt->tm_sec);
    return out.str();
  }

  // **************************************************************************

  /// \callgraph
  std::string fmtDateTime(const datetime* dt) {
    std::stringstream out;
    out << fmtDate(dt) << " " << fmtTime(dt);
    return out.str();
  }

  // **************************************************************************

  /// \callgraph
  datetime clearDate(const datetime* dt) {
    return parseDate(ASTROH_EPOCH,dt);
  }

  // **************************************************************************

  /// \callgraph
  datetime clearTime(const datetime* dt) {
    return parseTime(ZERO_TIME,dt);
  }

  // **************************************************************************

  /// \callgraph
  datetime clearDateTime(const datetime* dt) {
    datetime out;
    out=clearDate(dt);
    out=clearTime(&out);
    return out;
  }

  // **************************************************************************

  /// \callgraph
  int sortDateTime(const datetime* dt1, const datetime* dt2) {

    if (dt1->tm_year < dt2->tm_year) return -1;
    if (dt1->tm_year > dt2->tm_year) return +1;

    if (dt1->tm_mon < dt2->tm_mon) return -1;
    if (dt1->tm_mon > dt2->tm_mon) return +1;

    if (dt1->tm_mday < dt2->tm_mday) return -1;
    if (dt1->tm_mday > dt2->tm_mday) return +1;

    if (dt1->tm_hour < dt2->tm_hour) return -1;
    if (dt1->tm_hour > dt2->tm_hour) return +1;

    if (dt1->tm_min < dt2->tm_min) return -1;
    if (dt1->tm_min > dt2->tm_min) return +1;

    if (dt1->tm_sec < dt2->tm_sec) return -1;
    if (dt1->tm_sec > dt2->tm_sec) return +1;

    return 0;
  }

  // **************************************************************************

  /// \callgraph
  int secIntoYear(const datetime* dt) {

    // include leap day?   (don't need to readd 1900 here since 1900%4==0)
    bool leap=false;
    if (dt->tm_year%4 == 0 && dt->tm_year%100 != 0) {
      if (dt->tm_mon > 1) leap=true;
    }

    // number of days before current
    // note: jan=0 in datetime object, but jan=1 in daysInMonth()
    int ndays=dt->tm_mday-1;
    for (int imon=0; imon < dt->tm_mon; imon++) ndays+=daysInMonth(imon+1);
    if (leap) ndays++;

    // prepare output
    int out=dt->tm_hour*3600+dt->tm_min*60+dt->tm_sec;  // current day
    out+=ndays*SECS_IN_DAY;
    return out;
  }


  // **************************************************************************

  /// \callgraph
  int secLeftInYear(const datetime* dt) {

    // include leap day?
    bool leap=false;
    if (dt->tm_year%4 == 0 && dt->tm_year%100 != 0) leap=true;

    // use secIntoYear to get result
    long out=SECS_IN_YEAR;
    if (leap) out+=SECS_IN_DAY;
    out-=secIntoYear(dt);
    return out;
  }


  // **************************************************************************

  /// \callgraph
  long secDiff(datetime* dt1, datetime* dt2){

    // switch variables if dt1 < dt2;
    bool flip=false;
    if (sortDateTime(dt1,dt2) < 0) {
      flip=true;
      datetime* tmp=dt2;
      dt2=dt1;
      dt1=tmp;
    }

    // get number of leap days ( x%4 == 0 and x%100 > 0) not including
    // first and last years
    int nleapday=0;
    int year=dt2->tm_year+1;
    while (year < dt1->tm_year-1) {    /// \bug should the -1 be here?
      if (year%4 != 0) {
        year++;
        continue;
      }
      if (year%100 != 0) nleapday++;
      year+=4;
    }

    // time in intervening years
    long nyear=dt1->tm_year-dt2->tm_year-1;
    long out=nyear*SECS_IN_YEAR;
    out+=(long)nleapday*SECS_IN_DAY;

    // include time from first and last years
    out+=(long)secLeftInYear(dt2);
    out+=(long)secIntoYear(dt1);

    // make negative if time is before epoch
    if (flip) out=-out;

    return out;
  }

  // **************************************************************************

  /// \callgraph
  long secSinceEpoch(datetime* dt){

    // get AstroH epoch time
    datetime dt0;
    dt0=parseDate(ASTROH_EPOCH);
    dt0=parseTime("00:00:00",&dt0);

    return secDiff(dt,&dt0);
  }

  // **************************************************************************

}
