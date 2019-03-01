/** 
 * \file AhTimeTAI.cxx
 * \brief Container of International Atomic Time (TAI) relative to a UTC epoch.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/13 18:32:08 $
 *
 */
 
#include "ahlog/ahlog.h"
#include "ahtime/AhTimeTAI.h"

#include <cmath>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  int numSecInInterval(AhTimeTAI & t1, AhTimeTAI & t2, double* subseconds) {

    // get time difference between epochs
    AhTimeUTC epoch1,epoch2;
    epoch1=t1.epoch();
    epoch2=t2.epoch();
    int dep=0;     // number of seconds between epochs    
    int ecomp=epoch1.compare(epoch2);
    switch (ecomp) {
      case 0:
        dep=0;
        break;
      case -1:
        dep=numSecInInterval(epoch1,epoch2,NULL);
        break;
      case +1:
        dep=-numSecInInterval(epoch2,epoch1,NULL);
        break;
    }

    // get integral and fractional time difference of TAI seconds
    int out=t2.truncate()-t1.truncate();
    double dsecf=t2.subsecond()-t1.subsecond();
    while (dsecf < 0.0) {
      dsecf+=1.0;
      out--;
    }

    // combine and check
    out+=dep;
    if (out < 0) AH_THROW_RUNTIME("first time is larger than second");
    if (subseconds != NULL) *subseconds=dsecf;

    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeTAI::AhTimeTAI() {
    m_epoch.set(GPS_EPOCH,0.0);
    m_seci=0;
    m_secf=0.0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeTAI::AhTimeTAI(int seci, double secf, AhTimeUTC & epoch) {
    set(seci,secf,epoch);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeTAI::AhTimeTAI(double sec, AhTimeUTC & epoch) {
    set(sec,epoch);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeTAI::set(int seci, double secf, AhTimeUTC & epoch) {
    if (epoch.subsecond() != 0.0)
      AH_THROW_RUNTIME("epoch must have zero subseconds");
    m_epoch=epoch;
    m_seci=seci;
    m_secf=secf;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeTAI::set(double sec, AhTimeUTC & epoch) {
    if (epoch.subsecond() != 0.0)
      AH_THROW_RUNTIME("epoch must have zero subseconds");
    m_epoch=epoch;
    m_seci=(int)sec;
    m_secf=sec-(double)m_seci;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeTAI::set_epoch(AhTimeUTC & epoch) {
    if (epoch.subsecond() != 0.0)
      AH_THROW_RUNTIME("epoch must have zero subseconds");

    // get order of current and new epoch
    int x=epoch.compare(m_epoch);
    if (x == 0) return;              // nothing to do

    // easy if new epoch is earlier than old one
    if (x == -1) {
      int dt=ahtime::numSecInInterval(epoch,m_epoch,NULL);
      m_epoch=epoch;
      m_seci+=dt;
      return;
    }

    // if epoch later, need to check if time goes negative
    int dt=ahtime::numSecInInterval(m_epoch,epoch,NULL);
    if (dt > m_seci)
      AH_THROW_RUNTIME("changing epoch results in negative time");
    m_epoch=epoch;
    m_seci-=dt;
    return;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeUTC AhTimeTAI::epoch() {
     return m_epoch;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  double AhTimeTAI::seconds() {
    return (double)m_seci+m_secf;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeTAI::round() {
    int out=m_seci;
    if (m_secf >= 0.5) out++;
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeTAI::truncate() {
    return m_seci;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  double AhTimeTAI::subsecond() {
    return m_secf;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeTAI::compare(AhTimeTAI dt) {
    // compare epochs
    int dep;     // number of seconds between epochs
    int ecomp=m_epoch.compare(dt.m_epoch);
    switch (ecomp) {
      case 0:
        if (m_seci < dt.m_seci) return -1;
        if (m_seci > dt.m_seci) return +1;
        if (m_secf < dt.m_secf) return -1;
        if (m_secf > dt.m_secf) return +1;
        return 0;
      case -1:
        dep=numSecInInterval(m_epoch,dt.m_epoch,NULL);
        if (m_seci < dt.m_seci+dep) return -1;
        if (m_seci > dt.m_seci+dep) return +1;
        if (m_secf < dt.m_secf) return -1;
        if (m_secf > dt.m_secf) return +1;
        return 0;
      case +1:
        dep=numSecInInterval(dt.m_epoch,m_epoch,NULL);
        if (m_seci+dep < dt.m_seci) return -1;
        if (m_seci+dep > dt.m_seci) return +1;
        if (m_secf < dt.m_secf) return -1;
        if (m_secf > dt.m_secf) return +1;
        return 0;
    }
    return 0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_AhTimeTAI() {

    bool allgood=true;
    AH_INFO(ahlog::HIGH) << "*** Test AhTimeTAI.h:" << std::endl;

    // needs leap second data
    std::string leapsecfile=ahtime::locateLeapSecFile("CALDB");
    ahtime::readLeapSecData(leapsecfile);

    // test initialize with separate whole and fractional time
    {
      std::string section="initialize with separate whole and fractional time";
      bool okay=true;
      ahtime::AhTimeUTC epoch("2000-01-01T00:00:00",0.);
      ahtime::AhTimeTAI time(100,0.5,epoch);
      if (time.truncate() != 100) okay=false;
      if (time.subsecond() != 0.5) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test initialize with combined whole and fractional time
    {
      std::string section="initialize with combined whole and fractional time";
      bool okay=true;
      ahtime::AhTimeUTC epoch("2000-01-01T00:00:00",0.);
      ahtime::AhTimeTAI time(100.5,epoch);
      if (time.truncate() != 100) okay=false;
      if (time.subsecond() != 0.5) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test "get" functions
    {
      std::string section="retrieval functions";
      bool okay=true;
      ahtime::AhTimeUTC epoch("2000-01-01T00:00:00",0.);
      ahtime::AhTimeTAI time(100.5,epoch);
      if (time.truncate() != 100) okay=false;
      if (time.round() != 101) okay=false;
      if (time.seconds() != 100.5) okay=false;
      if (time.subsecond() != 0.5) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test comparison functions (same epoch)
    {
      std::string section="comparison functions w/ same epoch";
      bool okay=true;
      ahtime::AhTimeUTC epoch1("2000-01-01T00:00:00",0.);
      ahtime::AhTimeUTC epoch2("2000-01-01T00:00:00",0.);
      ahtime::AhTimeTAI dt1(100.5,epoch1);
      ahtime::AhTimeTAI dt2(111.4,epoch2);
      if (dt1.compare(dt1) != 0) okay=false;
      if (dt1.compare(dt2) != -1) okay=false;
      if (dt2.compare(dt1) != +1) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test comparison functions (different epoch)
    {
      std::string section="comparison functions w/ different epoch";
      bool okay=true;
      ahtime::AhTimeUTC epoch1("2000-01-01T00:00:10",0.);
      ahtime::AhTimeUTC epoch2("2000-01-01T00:00:00",0.);
      ahtime::AhTimeTAI dt1(100.5,epoch1);
      ahtime::AhTimeTAI dt2(111.4,epoch2);
      if (dt1.compare(dt1) != 0) okay=false;
      if (dt1.compare(dt2) != -1) okay=false;
      if (dt2.compare(dt1) != +1) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test interval calculation (same epoch)
    {
      std::string section="interval calculation w/ same epoch";
      bool okay=true;
      ahtime::AhTimeUTC epoch1("2000-01-01T00:00:00",0.);
      ahtime::AhTimeUTC epoch2("2000-01-01T00:00:00",0.);
      ahtime::AhTimeTAI dt1(100.5,epoch1);
      ahtime::AhTimeTAI dt2(111.4,epoch2);
      double subseconds=0.0;
      if (numSecInInterval(dt1,dt2,&subseconds) != 10) okay=false;
      if (std::abs(subseconds-0.9) > 1.e-10) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test interval calculation (different epoch)
    {
      std::string section="interval calculation w/ different epoch";
      bool okay=true;
      ahtime::AhTimeUTC epoch1("2000-01-01T00:00:10",0.);
      ahtime::AhTimeUTC epoch2("2000-01-01T00:00:00",0.);
      ahtime::AhTimeTAI dt1(100.5,epoch1);
      ahtime::AhTimeTAI dt2(111.4,epoch2);
      double subseconds=0.0;
      if (numSecInInterval(dt1,dt2,&subseconds) != 0) okay=false;
      if (std::abs(subseconds-0.9) > 1.e-10) okay=false;
      allgood&=ut_report(section,okay);
    }

    return allgood;
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: AhTimeTAI.cxx,v $
 Revision 1.3  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
