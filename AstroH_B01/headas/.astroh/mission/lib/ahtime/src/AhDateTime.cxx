#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <stdlib.h>
#include "ahlog/ahlog.h"
#include "ahtime/ahtime.h"
#include "ahtime/AhDateTime.h"

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhDateTime::AhDateTime(std::string date, std::string time) {
    set(date,time);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhDateTime::AhDateTime(std::string datetime) {
    set(datetime);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhDateTime::AhDateTime(int mjdi, double mjdf) {
    setMJD(mjdi,mjdf);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhDateTime::set(std::string date, std::string time) {
    // just in case...
    std::string errdate="invalid date format: "+date+"; must be YYYY-MM-DD";
    std::string errtime="invalid time format: "+time+"; must be HH:MM:SS";

    // check for proper format of date
    if (date.size() != 10 || date[4] != '-' || date[7] != '-')
      throw std::runtime_error(AH_PREP(errdate));

    // check for proper format of time
    if (time.size() != 8 || time[2] != ':' || time[5] != ':')
      throw std::runtime_error(AH_PREP(errtime));

    // get year, month, and day
    int year, mon, mday;
    try {
      year=atoi(date.substr(0,4).c_str());
      mon=atoi(date.substr(5,2).c_str());
      mday=atoi(date.substr(8,2).c_str());
    } catch(const std::exception &x) {
      throw std::runtime_error(AH_PREP(errdate));
    }

    // get hour, min, and sec
    int hour,min,sec;
    try {
      hour=atoi(time.substr(0,2).c_str());
      min=atoi(time.substr(3,2).c_str());
      sec=atoi(time.substr(6,2).c_str());
    } catch(const std::exception &x) {
      throw std::runtime_error(AH_PREP(errtime));
    }

    // fill structure
    m_datetime.tm_year=year-1900;    // counted from 1900
    m_datetime.tm_mon=mon-1;         // month range 0-11
    m_datetime.tm_mday=mday;         // day range 1-31
    m_datetime.tm_hour=hour;         // range: 0-23
    m_datetime.tm_min=min;           // range: 0-59
    m_datetime.tm_sec=sec;           // range: 0-61  (account for leap seconds)
    m_subsecond=0.;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhDateTime::set(std::string datetime) {
    std::string date=datetime.substr(0,10);
    std::string time=datetime.substr(11,8);
    set(date,time);
  }

  // ---------------------------------------------------------------------------

  /// \internal
  /// \note: method taken from http://en.wikipedia.org/wiki/Julian_day (2012-06-21)
  void AhDateTime::setMJD(int mjdi, double mjdf) {
    // get year, month, day
//    int J=mjdi+2400000;
    int J=(int)((double)mjdi+mjdf+2400001.);
    int j=J+32044;
    int g=j/146097;
    int dg=j%146097;
    int c=(dg/36524+1)*3/4;
    int dc=dg-c*36524;
    int b=dc/1461;
    int db=dc%1461;
    int a=(db/365+1)*3/4;
    int da=db-a*365;
    int y=g*400+c*100+b*4+a;
    int m=(da*5+308)/153-2;
    int d=da-(m+4)*153/5+122;
    int Y=y-4800+(m+2)/12;
    int M=(m+2)%12+1;
    int D=d+1;

    // get hour, minute, second
    double t0=mjdf;
    int H=(int)(t0*24.);
    t0=t0*24.-(double)H;
    int MM=(int)(t0*60.);
    t0=t0*60.-(double)MM;
    int S=(int)(t0*60.);
    t0=t0*60.-(double)S;

    // assign time
    m_datetime.tm_year=Y-1900;
    m_datetime.tm_mon=M-1;
    m_datetime.tm_mday=D;
    m_datetime.tm_hour=H;
    m_datetime.tm_min=MM;
    m_datetime.tm_sec=S;
    m_subsecond=t0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::year() {
    return m_datetime.tm_year+1900;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::month() {
    return m_datetime.tm_mon+1;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::day() {
    return m_datetime.tm_mday;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::hour() {
    return m_datetime.tm_hour;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::minute() {
    return m_datetime.tm_min;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::second() {
    return m_datetime.tm_sec;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  double AhDateTime::subsecond() {
    return m_subsecond;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string AhDateTime::format() {
    std::stringstream out;
    out << std::setw(4) << std::setfill('0') << year();
    out << "-";
    out << std::setw(2) << std::setfill('0') << month();
    out << "-";
    out << std::setw(2) << std::setfill('0') << day();
    out << "T";
    out << std::setw(2) << std::setfill('0') << hour();
    out << ":";
    out << std::setw(2) << std::setfill('0') << minute();
    out << ":";
    out << std::setw(2) << std::setfill('0') << second();
    return out.str();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  /// \internal
  /// \note alternate implementation, could just compare string order
  int AhDateTime::compare(AhDateTime dt) {
    if (year() < dt.year()) return -1;
    if (year() > dt.year()) return +1;

    if (month() < dt.month()) return -1;
    if (month() > dt.month()) return +1;

    if (day() < dt.day()) return -1;
    if (day() > dt.day()) return +1;

    if (hour() < dt.hour()) return -1;
    if (hour() > dt.hour()) return +1;

    if (minute() < dt.minute()) return -1;
    if (minute() > dt.minute()) return +1;

    if (second() < dt.second()) return -1;
    if (second() > dt.second()) return +1;

    return 0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool AhDateTime::isBefore(AhDateTime dt) {
    if (compare(dt) < 0) return true;
    return false;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool AhDateTime::isBeforeOrSame(AhDateTime dt) {
    if (compare(dt) <= 0) return true;
    return false;
  }

  // ---------------------------------------------------------------------------

  bool AhDateTime::isAfter(AhDateTime dt) {
    if (compare(dt) > 0) return true;
    return false;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool AhDateTime::isAfterOrSame(AhDateTime dt) {
    if (compare(dt) >= 0) return true;
    return false;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::secIntoYear() {

    // include leap day?   (don't need to re-add 1900 here since 1900%4==0)
    bool leap=false;
    if (isLeapYear(year())) {
      if (month() > 2) leap=true;
    }

    // number of days before current
    // note: jan=0 in datetime object, but jan=1 in daysInMonth()
    int ndays=day()-1;
    for (int imon=1; imon < month(); imon++) {
      ndays+=daysInMonth(imon);
    }
    if (leap) ndays++;

    // prepare output
    int out=ndays*SECS_IN_DAY
           +hour()*3600+minute()*60+second();
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::secLeftInYear() {

    // include leap day?
    bool leap=false;
    if (isLeapYear(year())) leap=true;

    // use secIntoYear() to get result
    int out=SECS_IN_YEAR;
    if (leap) out+=SECS_IN_DAY;
    out-=secIntoYear();
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::secSince(AhDateTime dt) {

    // make sure dt is in the past
    switch (compare(dt)) {
      case  0: return 0;
      case -1: return -dt.secSince(*this);
    }

    // get number of leap days ( x%4 == 0 and x%100 > 0) not including
    // first and last years
    int nleapday=0;
    int iyear=dt.year()+1;
    while (iyear < year()) {
      if (iyear%4 == 0 && iyear%100 != 0) {
        nleapday++;
      }
      iyear++;
    }

    // time in intervening years
    int nyear=year()-dt.year()-1;
    int out=nyear*SECS_IN_YEAR;
    out+=nleapday*SECS_IN_DAY;

    // too large a time span?
    if (nyear > 68) {
      AH_WARN(ahlog::HIGH) << "In danger of exceeding integer range: "
                           << nyear << " > 68 years" << std::endl;
    }

    // include time from first and last years
    out+=dt.secLeftInYear();
    out+=secIntoYear();

    if (out < 0)
      throw std::runtime_error("negative time span obtained, int limits probably exceeded");

    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhDateTime::secSince(std::string datetime) {
    AhDateTime dt(datetime);
    return secSince(dt);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhDateTime::getMJD(int & mjdi, double & mjdf) {
    int a=(14-month())/12;
    int y=year()+4800-a;
    int m=month()+12*a-3;
    mjdi=day()+(153*m+2)/5+365*y+y/4-y/100+y/400-32045;
    mjdf=((double)hour()-12.)/24.+((double)minute())/1440.+
         (((double)second()+subsecond()))/86400.;
    mjdi-=2400000;    // shift from JD to MJD
    mjdf-=0.5;        // shift from JD to MJD

    // fractional part must be between 0 and 1
    while (mjdf > 1.) {
      mjdi++;
      mjdf-=1.;
    }
    while (mjdf < 0.) {
      mjdi--;
      mjdf+=1.;
    }
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_comparison_tests(AhDateTime dt1, AhDateTime dt2, 
                           AhDateTime dt3, AhDateTime dt4) {
    bool okay=true;

    if ( dt1.compare(dt2) !=  1) okay=false;
    if ( dt1.compare(dt3) !=  0) okay=false;
    if ( dt1.compare(dt4) != -1) okay=false;

    if ( dt1.isBefore(dt2))       okay=false;
    if ( dt1.isBeforeOrSame(dt2)) okay=false;
    if (!dt1.isAfter(dt2))        okay=false;
    if (!dt1.isAfterOrSame(dt2))  okay=false;

    if ( dt1.isBefore(dt3))       okay=false;
    if (!dt1.isBeforeOrSame(dt3)) okay=false;
    if ( dt1.isAfter(dt3))        okay=false;
    if (!dt1.isAfterOrSame(dt3))  okay=false;

    if (!dt1.isBefore(dt4))       okay=false;
    if (!dt1.isBeforeOrSame(dt4)) okay=false;
    if ( dt1.isAfter(dt4))        okay=false;
    if ( dt1.isAfterOrSame(dt4))  okay=false;

    return okay;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_AhDateTime() {

    const std::string TEST_DATETIME="1976-09-25T13:14:15";


    bool allgood=true;
    AH_INFO(ahlog::HIGH) << "Test AhDateTime:" << std::endl;

    // test initializer - one argument
    {
      std::string section="initialize w/ one argument";
      bool okay=true;
      AhDateTime test(TEST_DATETIME);
      if (test.format() != TEST_DATETIME) okay=false;
      allgood=ut_report(section,okay);
    }

    // test initializer - two arguments
    {
      std::string section="initialize w/ two arguments";
      bool okay=true;
      std::string date=TEST_DATETIME.substr(0,10);
      std::string time=TEST_DATETIME.substr(11,8);
      AhDateTime test(date,time);
      if (test.format() != TEST_DATETIME) okay=false;
      allgood=ut_report(section,okay);
    }

    // test year, month, day, hour, minute, and second
    {
      std::string section="check date/time components";
      bool okay=true;
      AhDateTime test(TEST_DATETIME);
      if (test.year() != atoi(TEST_DATETIME.substr(0,4).c_str())) okay=false;
      if (test.month() != atoi(TEST_DATETIME.substr(5,2).c_str())) okay=false;
      if (test.day() != atoi(TEST_DATETIME.substr(8,2).c_str())) okay=false;
      if (test.hour() != atoi(TEST_DATETIME.substr(11,2).c_str())) okay=false;
      if (test.minute() != atoi(TEST_DATETIME.substr(14,2).c_str())) okay=false;
      if (test.second() != atoi(TEST_DATETIME.substr(17,18).c_str())) okay=false;
      allgood=ut_report(section,okay);
    }

    // test comparison functions on years
    {
      std::string section="compare years";
      bool okay=true;
      AhDateTime test1("2012-05-25T13:14:15");
      AhDateTime test2("2011-05-25T13:14:15");
      AhDateTime test3("2012-05-25T13:14:15");
      AhDateTime test4("2013-05-25T13:14:15");
      okay=ut_comparison_tests(test1,test2,test3,test4);
      allgood=ut_report(section,okay);
    }

    // test comparison functions on months
    {
      std::string section="compare months";
      bool okay=true;
      AhDateTime test1("2012-05-25T13:14:15");
      AhDateTime test2("2012-04-25T13:14:15");
      AhDateTime test3("2012-05-25T13:14:15");
      AhDateTime test4("2012-06-25T13:14:15");
      okay=ut_comparison_tests(test1,test2,test3,test4);
      allgood=ut_report(section,okay);
    }

    // test comparison functions on days
    {
      std::string section="compare days";
      bool okay=true;
      AhDateTime test1("2012-05-25T13:14:15");
      AhDateTime test2("2012-05-24T13:14:15");
      AhDateTime test3("2012-05-25T13:14:15");
      AhDateTime test4("2012-05-26T13:14:15");
      okay=ut_comparison_tests(test1,test2,test3,test4);
      allgood=ut_report(section,okay);
    }

    // test comparison functions on hours
    {
      std::string section="compare hours";
      bool okay=true;
      AhDateTime test1("2012-05-25T13:14:15");
      AhDateTime test2("2012-05-25T12:14:15");
      AhDateTime test3("2012-05-25T13:14:15");
      AhDateTime test4("2012-05-25T14:14:15");
      okay=ut_comparison_tests(test1,test2,test3,test4);
      allgood=ut_report(section,okay);
    }

    // test comparison functions on minutes
    {
      std::string section="compare minutes";
      bool okay=true;
      AhDateTime test1("2012-05-25T13:14:15");
      AhDateTime test2("2012-05-25T13:13:15");
      AhDateTime test3("2012-05-25T13:14:15");
      AhDateTime test4("2012-05-25T13:15:15");
      okay=ut_comparison_tests(test1,test2,test3,test4);
      allgood=ut_report(section,okay);
    }

    // test comparison functions on seconds
    {
      std::string section="compare seconds";
      bool okay=true;
      AhDateTime test1("2012-05-25T13:14:15");
      AhDateTime test2("2012-05-25T13:14:14");
      AhDateTime test3("2012-05-25T13:14:15");
      AhDateTime test4("2012-05-25T13:14:16");
      okay=ut_comparison_tests(test1,test2,test3,test4);
      allgood=ut_report(section,okay);
    }

    // test seconds into and left in year
    {
      std::string section="seconds into and left in year";
      bool okay=true;

      // non-leap year
      AhDateTime test1("2011-03-15T11:30:15");
      if (test1.secIntoYear() != 6348615) okay=false;
      if (test1.secLeftInYear() != 25187385) okay=false;

      // leap year
      AhDateTime test2("2012-03-15T11:30:15");
      if (test2.secIntoYear() != 6435015) okay=false;
      if (test2.secLeftInYear() != 25187385) okay=false;

      // on a leap day
      AhDateTime test3("2012-02-29T11:30:15");
      if (test3.secIntoYear() != 5139015) okay=false;
      if (test3.secLeftInYear() != 26483385) okay=false;
      allgood=ut_report(section,okay);
    }

    // test seconds between two dates
    {
      std::string section="seconds in interval";
      bool okay=true;
      std::string datetime1="2012-05-22T15:44:25";
      std::string datetime2="1976-09-25T13:25:18";
      AhDateTime test1(datetime1);
      AhDateTime test2(datetime2);
      if (test1.secSince(test2) != 1125109147) okay=false;
      if (test1.secSince(datetime2) != 1125109147) okay=false;
      if (test2.secSince(test1) != -1125109147) okay=false;
      if (test2.secSince(datetime1) != -1125109147) okay=false;
      allgood=ut_report(section,okay);
    }

    // test conversion to MJD
    {
      std::string section="conversion to MJD";
      bool okay=true;
      int mjdi;
      double mjdf;
      std::string datetime="1976-09-25T13:25:13";
      AhDateTime test(datetime);
      test.getMJD(mjdi,mjdf);
      if (mjdi != 43046) okay=false;
      if ((int)(1.e6*mjdf) != 559178) okay=false;
      allgood=ut_report(section,okay);
    }

    // test initialization using MJD
    {
      std::string section="conversion from MJD and back";
      bool okay=true;
      int mjdi=51544;
      double mjdf=0.00074287037037037;
      AhDateTime test(mjdi,mjdf);
      if (test.format() != "2000-01-01T00:01:04") okay=false;
      if ((int)(1.e3*test.subsecond()) != 183) okay=false;   // rounding

      int mjdi_new;
      double mjdf_new;
      test.getMJD(mjdi_new,mjdf_new);
      if (mjdi_new != mjdi) okay=false;
      if (abs(mjdf_new-mjdf) > 1.e-7) okay=false;
      allgood=ut_report(section,okay);
    }

    return allgood;
  }

  // ---------------------------------------------------------------------------

}
