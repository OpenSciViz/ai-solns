/** 
 * \file AhTimeUTC.cxx
 * \brief Container for UTC date and time.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/20 14:47:40 $
 *
 */
 
#include "ahlog/ahlog.h"
#include "ahtime/AhTimeUTC.h"

#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <cmath>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  int numLeapSecInInterval(AhTimeUTC & t1, AhTimeUTC & t2) {
    if (getLeapSecData().size() == 0) 
      AH_THROW_RUNTIME("no leap second data loaded!");
    int nleap=0;
    LeapSecData::iterator it;
    for (it=getLeapSecData().begin(); it != getLeapSecData().end(); it++) {
      AhTimeUTC tleap((*it).first,0.);
      if (tleap.compare(t1) <= 0) continue;
      if (tleap.compare(t2) > 0) continue;
      nleap+=(int)(*it).second;
    }

    // if first time is before 1972-01-01, add 10s
    /// \internal
    /// \todo check if should be < or <= in comparison below
    /// \note this is commented out since the minimum date is 1980-1-6
//    AhTimeUTC comp("1972-01-01T00:00:00",0.0);
//    if (t1.compare(comp) <= 0) nleap+=10;

    return nleap;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int numLeapSecBefore(AhTimeUTC & time) {
    if (getLeapSecData().size() == 0) 
      AH_THROW_RUNTIME("no leap second data loaded!");
    int nleap=0;
    LeapSecData::iterator it;
    for (it=getLeapSecData().begin(); it != getLeapSecData().end(); it++) {
      AhTimeUTC tleap((*it).first,0.);
      if (tleap.compare(time) > 0) continue;
      nleap+=(int)(*it).second;
    }
    nleap+=10;      // 10s shift at 1972-01-01
    return nleap;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int numSecInInterval(AhTimeUTC & t1, AhTimeUTC & t2, double* subseconds) {
    int x=t1.compare(t2);
    if (x > 0) {
      AH_THROW_RUNTIME("first time is larger than second");
    } else if (x == 0) {
      if (subseconds != NULL) *subseconds=0.0;
      return 0;
    }

    // get number of leap seconds in range
    int out=numLeapSecInInterval(t1,t2);

    // get number of (non-leap) seconds included in first and last years
    double sub_first, sub_last;
    int nsec_first=t1._secLeftInYear(&sub_first);
    int nsec_last=t2._secIntoYear(&sub_last);
    out+=nsec_first+nsec_last;

    // take care of sub-seconds
    double tsub=sub_first+sub_last;
    while (tsub > 1.0-SUBSECOND_THRESHOLD) {
      tsub-=1.0;
      out++;
    }

    // this will take care of slightly negative subseconds (from threshold
    // test above) and very small positive subseconds
    if (tsub < SUBSECOND_THRESHOLD) tsub=0.0;

    // assign subseconds if requested
    if (subseconds != NULL) *subseconds=tsub;

    // count rest of seconds
    if (t1.year() == t2.year()) {
      out-=SECS_IN_YEAR;
      if (isLeapYear(t1.year())) out-=SECS_IN_DAY;
    } else {
      for (int iyear=t1.year()+1; iyear < t2.year(); iyear++) {
        out+=SECS_IN_YEAR;
        if (isLeapYear(iyear)) out+=SECS_IN_DAY;
      }
    }
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeUTC::AhTimeUTC() {
    set(GPS_EPOCH,0.0);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeUTC::AhTimeUTC(const int & year, const int & month, const int & day, 
                       const int & hour, const int & minute, 
                       const int & second, const double & subsecond) {
    set(year,month,day,hour,minute,second,subsecond);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeUTC::AhTimeUTC(const std::string & date, const std::string & time,
                       const double & subsecond) {
    set(date,time,subsecond);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeUTC::AhTimeUTC(const std::string & datetime, const double & subsecond) {
    set(datetime,subsecond);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeUTC::AhTimeUTC(const int & date, const int & time, 
                       const double & subsecond) {
    set(date,time,subsecond);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeUTC::set(const int & year, const int & month, const int & day, 
                      const int & hour, const int & minute, const int & second,
                      const double & subsecond) {

    // reject if date is before GPS epoch
    bool reject=false;
    if (year < MINDATE_YEAR) reject=true;
    if (year == MINDATE_YEAR && month == MINDATE_MON && day < MINDATE_DAY)
      reject=true;
    if (reject) {
      std::stringstream tstr;
      tstr << MINDATE_YEAR << "-" << MINDATE_MON << "-" << MINDATE_DAY;
      AH_THROW_RUNTIME("times before "+tstr.str()+" not supported");
    }

    m_datetime.tm_year=year-1900;    // counted from 1900
    m_datetime.tm_mon=month-1;       // month range 0-11
    m_datetime.tm_mday=day;          // day range 1-31
    m_datetime.tm_hour=hour;         // range: 0-23
    m_datetime.tm_min=minute;        // range: 0-59
    m_datetime.tm_sec=second;        // range: 0-61  (account for leap seconds)
    m_subsecond=subsecond;
    if ((1.-m_subsecond) <= SUBSECOND_THRESHOLD) {
      m_subsecond=0.0;
      m_datetime.tm_sec++;
    }
    if (m_subsecond < SUBSECOND_THRESHOLD) m_subsecond=0.0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeUTC::set(const std::string & date, const std::string & time, 
                      const double & subsecond) {
    // format error messages
    std::string errdate="invalid date format: "+date+"; must be YYYY-MM-DD";
    std::string errtime="invalid time format: "+time+"; must be HH:MM:SS";

    // check for proper format of date
    if (date.size() != 10 || date[4] != '-' || date[7] != '-')
      AH_THROW_RUNTIME(errdate);

    // check for proper format of time
    if (time.size() != 8 || time[2] != ':' || time[5] != ':')
      AH_THROW_RUNTIME(errtime);

    // get year, month, and day
    int year, mon, mday;
    try {
      year=atoi(date.substr(0,4).c_str());
      mon=atoi(date.substr(5,2).c_str());
      mday=atoi(date.substr(8,2).c_str());
    } catch(const std::exception &x) {
      AH_THROW_RUNTIME(errdate);
    }

    // get hour, min, and sec
    int hour,min,sec;
    try {
      hour=atoi(time.substr(0,2).c_str());
      min=atoi(time.substr(3,2).c_str());
      sec=atoi(time.substr(6,2).c_str());
    } catch(const std::exception &x) {
      AH_THROW_RUNTIME(errtime);
    }

    // fill structure
    set(year,mon,mday,hour,min,sec,subsecond);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeUTC::set(const std::string & datetime, const double & subsecond) {
    std::string date=datetime.substr(0,10);
    std::string time=datetime.substr(11,8);
    set(date,time,subsecond);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeUTC::set(const int & date, const int & time,
                      const double & subsecond) {
    // split date
    int year,mon,mday;
    year=date/10000;
    int tmp=date-year*10000;
    mon=tmp/100;
    mday=tmp-mon*100;

    // split time
    int hour,min,sec;
    hour=time/10000;
    tmp=time-hour*10000;
    min=tmp/100;
    sec=tmp-min*100;

    set(year,mon,mday,hour,min,sec,subsecond);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::year() const {
    return m_datetime.tm_year+1900;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::month() const {
    return m_datetime.tm_mon+1;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::day() const {
    return m_datetime.tm_mday;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::hour() const {
    return m_datetime.tm_hour;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::minute() const {
    return m_datetime.tm_min;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::second() const {
    return m_datetime.tm_sec;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  double AhTimeUTC::subsecond() const {
    return m_subsecond;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  /// \internal
  /// \note alternate implementation, could just compare string order
  int AhTimeUTC::compare(const AhTimeUTC & dt) const {
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

    if (subsecond() < dt.subsecond()) return -1;
    if (subsecond() > dt.subsecond()) return +1;

    return 0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string AhTimeUTC::string_date() {
    std::stringstream out;
    out << std::setw(4) << std::setfill('0') << year();
    out << "-";
    out << std::setw(2) << std::setfill('0') << month();
    out << "-";
    out << std::setw(2) << std::setfill('0') << day();
    return out.str();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string AhTimeUTC::string_time() {
    std::stringstream out;
    out << std::setw(2) << std::setfill('0') << hour();
    out << ":";
    out << std::setw(2) << std::setfill('0') << minute();
    out << ":";
    out << std::setw(2) << std::setfill('0') << second();
    return out.str();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string AhTimeUTC::string_datetime() {
    return string_date()+"T"+string_time();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::integer_date() {
    return 10000*year()+100*month()+day();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::integer_time() {
    return 10000*hour()+100*minute()+second();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::integer_subsecond(const int & ndigits) {
    return floor(0.5+pow(10.,ndigits)*subsecond());
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::_secIntoYear(double* subseconds) {

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
    if (subseconds != NULL) *subseconds=subsecond();
    int out=ndays*SECS_IN_DAY
           +hour()*3600+minute()*60+second();
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeUTC::_secLeftInYear(double* subseconds) {

    // include leap day?
    bool leap=false;
    if (isLeapYear(year())) leap=true;

    // use _secIntoYear() to get result
    int out=SECS_IN_YEAR;
    if (leap) out+=SECS_IN_DAY;
    out-=_secIntoYear(subseconds);
    if (subseconds != NULL && *subseconds > 0.0) {
      out--;
      *subseconds=1.0-*subseconds;
    }
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeUTC::_addSeconds(const int & seconds, const double & subseconds) {
    if (seconds < 0 || subseconds < 0.) 
      AH_THROW_RUNTIME("can only add positive seconds/subseconds");

    m_subsecond+=subseconds;
    while (m_subsecond >= 1.0) {
      m_subsecond-=1.0;
      m_datetime.tm_sec++;
    }
    m_datetime.tm_sec+=seconds;
    while (m_datetime.tm_sec > 59) {
      m_datetime.tm_sec-=60;
      m_datetime.tm_min++;
    }
    while (m_datetime.tm_min > 59) {
      m_datetime.tm_min-=60;
      m_datetime.tm_hour++;
    }
    while (m_datetime.tm_hour > 23) {
      m_datetime.tm_hour-=24;
      m_datetime.tm_mday++;
    }
    while (m_datetime.tm_mday > daysInMonth(m_datetime.tm_mon+1,
                                            m_datetime.tm_year+1900)) {
      m_datetime.tm_mday-=daysInMonth(m_datetime.tm_mon+1,
                                      m_datetime.tm_year+1900);
      m_datetime.tm_mon++;
      if (m_datetime.tm_mon+1 > 12) {
        m_datetime.tm_mon-=12;
        m_datetime.tm_year++;
      }
    }
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeUTC::_subtractSeconds(const int & seconds, 
                                   const double & subseconds) {
    if (seconds < 0 || subseconds < 0.) 
      AH_THROW_RUNTIME("can only subtract positive seconds/subseconds");

    m_subsecond-=subseconds;
    while (m_subsecond < 0.0) {
      m_subsecond+=1.0;
      m_datetime.tm_sec--;
    }
    m_datetime.tm_sec-=seconds;
    while (m_datetime.tm_sec < 0) {
      m_datetime.tm_sec+=60;
      m_datetime.tm_min--;
    }
    while (m_datetime.tm_min < 0) {
      m_datetime.tm_min+=60;
      m_datetime.tm_hour--;
    }
    while (m_datetime.tm_hour < 0) {
      m_datetime.tm_hour+=24;
      m_datetime.tm_mday--;
    }
    while (m_datetime.tm_mday < 1) {
      m_datetime.tm_mon--;
      m_datetime.tm_mday+=daysInMonth(m_datetime.tm_mon+1,
                                      m_datetime.tm_year+1900);
      if (m_datetime.tm_mon+1 < 1) {
        m_datetime.tm_mon+=12;
        m_datetime.tm_year--;
      }
    }
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeUTC::_addOneSecondForce() {
    m_datetime.tm_sec++;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_AhTimeUTC() {

    bool allgood=true;
    AH_INFO(ahlog::HIGH) << "*** Test AhTimeUTC.h:" << std::endl;

    // needs leap second data
    std::string leapsecfile=ahtime::locateLeapSecFile("CALDB");
    ahtime::readLeapSecData(leapsecfile);

    // test initialize with single date/time string
    {
      std::string section="initialize with single date/time string";
      bool okay=true;
      ahtime::AhTimeUTC time("2000-01-01T02:38:10",0.123456789);
      if (time.string_datetime() != "2000-01-01T02:38:10") okay=false;
      if (time.subsecond() != 0.123456789) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test initialize with separate date/time strings
    {
      std::string section="initialize with separate date/time strings";
      bool okay=true;
      ahtime::AhTimeUTC time("2000-01-01","02:38:10",0.123456789);
      if (time.string_datetime() != "2000-01-01T02:38:10") okay=false;
      if (time.subsecond() != 0.123456789) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test initialize with separate year, month, etc
    {
      std::string section="initialize with year, month, day, etc";
      bool okay=true;
      ahtime::AhTimeUTC time(2000,1,1,2,38,10,0.123456789);
      if (time.string_datetime() != "2000-01-01T02:38:10") okay=false;
      if (time.subsecond() != 0.123456789) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test initialize with integer date and time
    {
      std::string section="initialize with integer date and time";
      bool okay=true;
      ahtime::AhTimeUTC time(20000101,23810,0.123456789);
      if (time.string_datetime() != "2000-01-01T02:38:10") okay=false;
      if (time.subsecond() != 0.123456789) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test "get" functions
    {
      std::string section="test get functions";
      bool okay=true;
      ahtime::AhTimeUTC time("2000-01-01T02:38:10",0.123456789);
      if (time.year() != 2000) okay=false;
      if (time.month() != 1) okay=false;
      if (time.day() != 1) okay=false;
      if (time.hour() != 2) okay=false;
      if (time.minute() != 38) okay=false;
      if (time.second() != 10) okay=false;
      if (time.subsecond() != 0.123456789) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test formatting functions
    {
      std::string section="test formatting functions";
      bool okay=true;
      ahtime::AhTimeUTC time("2000-01-01T02:38:10",0.123456789);
      if (time.string_datetime() != "2000-01-01T02:38:10") okay=false;
      if (time.string_date() != "2000-01-01") okay=false;
      if (time.string_time() != "02:38:10") okay=false;
      if (time.integer_date() != 20000101) okay=false;
      if (time.integer_time() != 23810) okay=false;
      if (time.integer_subsecond(6) != 123457) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test comparison
    {
      std::string section="test comparison";
      bool okay=true;
      ahtime::AhTimeUTC dt0("2000-01-01T02:38:10",0.0);
      ahtime::AhTimeUTC dt1("2000-01-01T02:38:10",0.0);
      ahtime::AhTimeUTC dt2("2000-01-01T02:38:10",0.1);
      ahtime::AhTimeUTC dt3("2000-01-01T02:38:20",0.0);
      ahtime::AhTimeUTC dt4("2000-01-01T02:40:10",0.0);
      ahtime::AhTimeUTC dt5("2000-01-01T04:38:10",0.0);
      ahtime::AhTimeUTC dt6("2000-01-03T02:38:10",0.0);
      ahtime::AhTimeUTC dt7("2000-02-01T02:38:10",0.0);
      ahtime::AhTimeUTC dt8("2001-01-01T02:38:10",0.0);
      if (dt0.compare(dt1) != 0) okay=false;
      if (dt0.compare(dt2) >= 0) okay=false;
      if (dt0.compare(dt3) >= 0) okay=false;
      if (dt0.compare(dt4) >= 0) okay=false;
      if (dt0.compare(dt5) >= 0) okay=false;
      if (dt0.compare(dt6) >= 0) okay=false;
      if (dt0.compare(dt7) >= 0) okay=false;
      if (dt0.compare(dt8) >= 0) okay=false;
      if (dt8.compare(dt0) <= 0) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test interval calculation
    {
      std::string section="test interval calculation";
      bool okay=true;
      ahtime::AhTimeUTC dt1("2000-01-01T02:38:10",0.0);
      ahtime::AhTimeUTC dt2("2000-01-01T02:38:10",0.1);
      ahtime::AhTimeUTC dt3("2000-01-01T02:38:20",0.0);
      ahtime::AhTimeUTC dt4("2000-01-01T02:40:10",0.0);
      ahtime::AhTimeUTC dt5("2000-01-01T04:38:10",0.0);
      ahtime::AhTimeUTC dt6("2000-01-03T02:38:10",0.0);
      ahtime::AhTimeUTC dt7("2000-02-01T02:38:10",0.0);
      ahtime::AhTimeUTC dt8("2001-01-01T02:38:10",0.0);
      ahtime::AhTimeUTC dt9("2001-02-03T04:40:20",0.1);

      double subseconds=0.0;
      if (numSecInInterval(dt1,dt2,&subseconds) != 0) okay=false;
      if (subseconds != 0.1) okay=false;

      if (numSecInInterval(dt1,dt3,NULL) != 10) okay=false;
      if (numSecInInterval(dt1,dt4,NULL) != 120) okay=false;
      if (numSecInInterval(dt1,dt5,NULL) != 7200) okay=false;
      if (numSecInInterval(dt1,dt6,NULL) != 172800) okay=false;
      if (numSecInInterval(dt1,dt7,NULL) != 2678400) okay=false;
      if (numSecInInterval(dt1,dt8,NULL) != 31622400) okay=false;

      if (numSecInInterval(dt1,dt9,&subseconds) != 34480930) okay=false;
      if (subseconds != 0.1) okay=false;
      allgood&=ut_report(section,okay);
    }

    return allgood;
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: AhTimeUTC.cxx,v $
 Revision 1.6  2012/07/20 14:47:40  mwitthoe
 add const to ahtime year(), month(), etc functions

 Revision 1.5  2012/07/16 20:09:39  mwitthoe
 move TI and S_TIME units from TIM file into TIM struct; remove reading of TIM file from function in ahtimeassign (reading is now done outside HDU loop)

 Revision 1.4  2012/07/13 19:17:22  mwitthoe
 set 1972-01-01 as the minimum date supported by AhTimeUTC

 Revision 1.3  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
