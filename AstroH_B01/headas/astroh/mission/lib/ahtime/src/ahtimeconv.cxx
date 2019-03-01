/** 
 * \file ahtimeconv.cxx
 * \brief Functions to act on time objects: UTC, TT, and TAI.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/13 18:32:08 $
 *
 */
 
#include "ahlog/ahlog.h"
#include "ahtime/ahtimeconv.h"

#include <cmath>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  void convertTime(AhTimeUTC & tin, AhTimeTT & tout) {

    // leap seconds and 32.184s shift
    double adj=32.184+(double)numLeapSecBefore(tin);

    // from http://en.wikipedia.org/wiki/Julian_day
    int a=(14-tin.month())/12;
    int y=tin.year()+4800-a;
    int m=tin.month()+12*a-3;
    int mjdi=tin.day()+(153*m+2)/5+365*y+y/4-y/100+y/400-32045;
    double mjdf=((double)tin.hour()-12.)/24.+((double)tin.minute())/1440.+
         ((double)tin.second()+tin.subsecond()+adj)/86400.;
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

    tout.set(mjdi,mjdf);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void convertTime(AhTimeTT & tin, AhTimeUTC & tout) {

    // 32.184s shift (leap seconds done later)
    double adj=32.184/(double)SECS_IN_DAY;

    // from http://en.wikipedia.org/wiki/Julian_day
    int J=(int)((double)tin.mjdi()+tin.mjdf()-adj+2400001.);
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
    double t0=tin.mjdf()-adj;
    int H=(int)(t0*24.);
    t0=t0*24.-(double)H;
    int MM=(int)(t0*60.);
    t0=t0*60.-(double)MM;
    int S=(int)(t0*60.);
    t0=t0*60.-(double)S;

    // fill tout with intermediate result and get number of leap seconds
    tout.set(Y,M,D,H,MM,S,t0);
    int nleap=numLeapSecBefore(tout);

    // subtract leap seconds and recount number of leap seconds; if different,
    // then add one second back without rounding so that the final number of
    // seconds can be 60 in the case of being on a leap second.
    tout._subtractSeconds(nleap,0.0);
    int nleap2=numLeapSecBefore(tout);
    if (nleap2 != nleap) tout._addOneSecondForce();

  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void convertTime(AhTimeUTC & tin, AhTimeTAI & tout, AhTimeUTC & epoch) {
    double subseconds;
    int time=numSecInInterval(epoch,tin,&subseconds);
    tout.set(time,subseconds,epoch);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void convertTime(AhTimeTAI & tin, AhTimeUTC & tout) {
    // add seconds to epoch, store in tout
    AhTimeUTC epoch0=tin.epoch();
    tout=tin.epoch();
    tout._addSeconds(tin.truncate(),tin.subsecond());

    // get number of leap seconds between tin epoch and intermediate tout and
    // adjust output UTC time
    int nleap=numLeapSecInInterval(epoch0,tout);
    if (nleap > 0) tout._subtractSeconds(nleap,0.0);

    // check difference again, and add one leap second back as necessary
    int nleap2=numLeapSecInInterval(epoch0,tout);
    if (nleap2 != nleap) tout._addOneSecondForce();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void convertTime(AhTimeTAI & tin, AhTimeTT & tout) {
    AhTimeUTC ttime;            // conversion via UTC
    convertTime(tin,ttime);
    convertTime(ttime,tout);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void convertTime(AhTimeTT & tin, AhTimeTAI & tout, AhTimeUTC & epoch) {
    AhTimeUTC ttime;            // conversion via UTC
    convertTime(tin,ttime);
    convertTime(ttime,tout,epoch);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_ahtimeconv() {

    bool allgood=true;
    AH_INFO(ahlog::HIGH) << "*** Test ahtimeconv.h:" << std::endl;

    // for equality checks of doubles
    double tol=1.e-10;

    // needs leap second data
    std::string leapsecfile=ahtime::locateLeapSecFile("CALDB");
    ahtime::readLeapSecData(leapsecfile);

    // MJD fractional test values
    double mjdf1=0.000754444444444502;
    double mjdf2=0.000766018518518541;
    double mjdf3=0.00077759259259258;

    // test conversion from UTC (just before leap second)
    {
      std::string section="from UTC (just before leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeUTC utc("2012-06-30T23:59:59",0.0);
      ahtime::AhTimeTT tt;
      ahtime::AhTimeTAI tai;

      ahtime::convertTime(utc,tt);
      ahtime::convertTime(utc,tai,epoch);
      if (tt.mjdi() != 56109) okay=false;
      if (std::abs(tt.mjdf()-mjdf1) > tol) okay=false;
      if (tai.truncate() != 15724799) okay=false;
      if (tai.subsecond() > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from UTC (during leap second)
    {
      std::string section="from UTC (during leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeUTC utc("2012-06-30T23:59:60",0.0);
      ahtime::AhTimeTT tt;
      ahtime::AhTimeTAI tai;

      ahtime::convertTime(utc,tt);
      ahtime::convertTime(utc,tai,epoch);
      if (tt.mjdi() != 56109) okay=false;
      if (std::abs(tt.mjdf()-mjdf2) > tol) okay=false;
      if (tai.truncate() != 15724800) okay=false;
      if (tai.subsecond() > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from UTC (just after leap second)
    {
      std::string section="from UTC (just after leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeUTC utc("2012-07-01T00:00:00",0.0);
      ahtime::AhTimeTT tt;
      ahtime::AhTimeTAI tai;

      ahtime::convertTime(utc,tt);
      ahtime::convertTime(utc,tai,epoch);
      if (tt.mjdi() != 56109) okay=false;
      if (std::abs(tt.mjdf()-mjdf3) > tol) okay=false;
      if (tai.truncate() != 15724801) okay=false;
      if (tai.subsecond() > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from TAI (just before leap second)
    {
      std::string section="from TAI (just before leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeTAI tai(15724799,0.0,epoch);
      ahtime::AhTimeUTC utc;
      ahtime::AhTimeTT tt;

      ahtime::convertTime(tai,utc);
      ahtime::convertTime(tai,tt);
      if (utc.string_datetime() != "2012-06-30T23:59:59") okay=false;
      if (utc.subsecond() > tol) okay=false;
      if (tt.mjdi() != 56109) okay=false;
      if (std::abs(tt.mjdf()-mjdf1) > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from TAI (during leap second)
    {
      std::string section="from TAI (during leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeTAI tai(15724800,0.0,epoch);
      ahtime::AhTimeUTC utc;
      ahtime::AhTimeTT tt;

      ahtime::convertTime(tai,utc);
      ahtime::convertTime(tai,tt);
      if (utc.string_datetime() != "2012-06-30T23:59:60") okay=false;
      if (utc.subsecond() > tol) okay=false;
      if (tt.mjdi() != 56109) okay=false;
      if (std::abs(tt.mjdf()-mjdf2) > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from TAI (just after leap second)
    {
      std::string section="from TAI (just after leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeTAI tai(15724801,0.0,epoch);
      ahtime::AhTimeUTC utc;
      ahtime::AhTimeTT tt;

      ahtime::convertTime(tai,utc);
      ahtime::convertTime(tai,tt);
      if (utc.string_datetime() != "2012-07-01T00:00:00") okay=false;
      if (utc.subsecond() > tol) okay=false;
      if (tt.mjdi() != 56109) okay=false;
      if (std::abs(tt.mjdf()-mjdf3) > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from TT (just before leap second)
    {
      std::string section="from TT (just before leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeTT tt(56109,mjdf1);
      ahtime::AhTimeUTC utc;
      ahtime::AhTimeTAI tai;

      ahtime::convertTime(tt,utc);
      ahtime::convertTime(tt,tai,epoch);
      if (utc.string_datetime() != "2012-06-30T23:59:59") okay=false;
      if (utc.subsecond() > tol) okay=false;
      if (tai.truncate() != 15724799) okay=false;
      if (tai.subsecond() > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from TT (during leap second)
    {
      std::string section="from TT (during leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeTT tt(56109,mjdf2);
      ahtime::AhTimeUTC utc;
      ahtime::AhTimeTAI tai;

      ahtime::convertTime(tt,utc);
      ahtime::convertTime(tt,tai,epoch);
      if (utc.string_datetime() != "2012-06-30T23:59:60") okay=false;
      if (utc.subsecond() > tol) okay=false;
      if (tai.truncate() != 15724800) okay=false;
      if (tai.subsecond() > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    // test conversion from TT (just after leap second)
    {
      std::string section="from TT (just after leap second)";
      bool okay=true;

      ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);
      ahtime::AhTimeTT tt(56109,mjdf3);
      ahtime::AhTimeUTC utc;
      ahtime::AhTimeTAI tai;

      ahtime::convertTime(tt,utc);
      ahtime::convertTime(tt,tai,epoch);
      if (utc.string_datetime() != "2012-07-01T00:00:00") okay=false;
      if (utc.subsecond() > tol) okay=false;
      if (tai.truncate() != 15724801) okay=false;
      if (tai.subsecond() > tol) okay=false;

      allgood&=ut_report(section,okay);
    }

    return allgood;
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: ahtimeconv.cxx,v $
 Revision 1.3  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
