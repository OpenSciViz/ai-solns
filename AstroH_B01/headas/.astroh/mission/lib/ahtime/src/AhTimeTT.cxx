/** 
 * \file AhTimeTT.cxx
 * \brief Container of Terrestrial Time (TT) using the Modified Julian Date 
 * (MJD).
 * \author Mike Witthoeft
 * \date $Date: 2012/07/13 18:32:08 $
 *
 */
 
#include "ahlog/ahlog.h"
#include "ahtime/AhTimeTT.h"

#include <cmath>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  int numSecInInterval(AhTimeTT & t1, AhTimeTT & t2, double* subseconds) {

    // get difference in days
    int out=t2.mjdi()-t1.mjdi();
    double dd=t2.mjdf()-t1.mjdf();
    while (dd < 0.0) {
      dd+=1.0;
      out--;
    }
    if (out < 0) AH_THROW_RUNTIME("first time is larger than second");

    // convert to seconds
    out*=SECS_IN_DAY;
    dd*=(double)SECS_IN_DAY;

    // make sure that subseconds is less than 1
    int lo=(int)dd;
    out+=lo;
    dd=dd-(double)lo;

    if (subseconds != NULL) *subseconds=dd;
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeTT::AhTimeTT() {
    m_mjdi=0;
    m_mjdf=0.0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeTT::AhTimeTT(const int & mjdi, const double & mjdf) {
    set(mjdi,mjdf);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  AhTimeTT::AhTimeTT(const double & mjd) {
    set(mjd);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeTT::set(const int & mjdi, const double & mjdf) {
    m_mjdi=mjdi;
    m_mjdf=mjdf;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void AhTimeTT::set(const double & mjd) {
    m_mjdi=(int)mjd;
    m_mjdf=mjd-(double)m_mjdi;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  double AhTimeTT::mjd() {
    return (double)m_mjdi+m_mjdf;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeTT::mjdi() {
    return m_mjdi;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  double AhTimeTT::mjdf() {
    return m_mjdf;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  int AhTimeTT::compare(AhTimeTT & dt) {
    if (mjdi() < dt.mjdi()) return -1;
    if (mjdi() > dt.mjdi()) return +1;

    if (mjdf() < dt.mjdf()) return -1;
    if (mjdf() > dt.mjdf()) return +1;

    return 0;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_AhTimeTT() {

    double tol=1.e-10;

    bool allgood=true;
    AH_INFO(ahlog::HIGH) << "*** Test AhTimeTT.h:" << std::endl;

    // test initialize with separate whole and fractional time
    {
      std::string section="initialize with separate whole and fractional time";
      bool okay=true;
      ahtime::AhTimeTT time(5777,0.245);
      if (time.mjdi() != 5777) okay=false;
      if (std::abs(time.mjdf()-0.245) > tol) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test initialize with combined whole and fractional time
    {
      std::string section="initialize with combined whole and fractional time";
      bool okay=true;
      ahtime::AhTimeTT time(5777.245);
      if (time.mjdi() != 5777) okay=false;
      if (std::abs(time.mjdf()-0.245) > tol) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test retrieval functions
    {
      std::string section="retrieval functions";
      bool okay=true;
      ahtime::AhTimeTT time(5777,0.245);
      if (time.mjdi() != 5777) okay=false;
      if (std::abs(time.mjdf()-0.245) > tol) okay=false;
      if (std::abs(time.mjd()-5777.245) > tol) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test comparison
    {
      std::string section="comparison functions";
      bool okay=true;
      ahtime::AhTimeTT time1(5777,0.245);
      ahtime::AhTimeTT time2(5777,0.255);
      ahtime::AhTimeTT time3(5787,0.245);
      if (time1.compare(time1) != 0) okay=false;
      if (time1.compare(time2) != -1) okay=false;
      if (time1.compare(time3) != -1) okay=false;
      if (time3.compare(time1) != +1) okay=false;
      allgood&=ut_report(section,okay);
    }

    // test interval calculation
    {
      std::string section="interval calculation";
      bool okay=true;
      ahtime::AhTimeTT time1(5777,0.245);
      ahtime::AhTimeTT time2(5777,0.255);
      ahtime::AhTimeTT time3(5787,0.245);
      ahtime::AhTimeTT time4(5777,0.2456);
      if (numSecInInterval(time1,time2,NULL) != 864) okay=false;
      if (numSecInInterval(time1,time3,NULL) != 864000) okay=false;
      double subseconds;
      if (numSecInInterval(time1,time4,&subseconds) != 51) okay=false;
      if (std::abs(subseconds-0.84) > tol) okay=false;
      allgood&=ut_report(section,okay);
    }

    return allgood;
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: AhTimeTT.cxx,v $
 Revision 1.3  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
