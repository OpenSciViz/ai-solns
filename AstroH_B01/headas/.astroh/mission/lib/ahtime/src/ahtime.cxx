/** 
 * \file ahtime.cxx
 * \brief General time constants and functions.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/13 18:32:08 $
 * 
 */

#include "ahtime/ahtime.h"
#include "ahlog/ahlog.h"

#include <stdlib.h>
#include <stdio.h>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool isLeapYear(const int & year) {
    if (year%400 == 0) return true;
    if (year%100 == 0) return false;
    if (year%4 == 0) return true;
    return false;
  } 

  // ---------------------------------------------------------------------------

  /// \callgraph
  int daysInMonth(const int & imon) {
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

  // ---------------------------------------------------------------------------

  /// \callgraph
  int daysInMonth(const int & imon, const int & year) {
    int out=daysInMonth(imon);
    if (imon != 2) return out;
    if (isLeapYear(year)) out++;
    return out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string catTimeSubseconds(const std::string & time, 
                                const double & subseconds, 
                                const int & digits) {
    char tmp[99],fmt[9];
    sprintf(fmt,"%%.%if",digits);
    sprintf(tmp,fmt,subseconds);
    std::string out=(std::string)tmp;
    out.erase(out.begin());  // get rid of leading zero
    return time+out;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool uncatTimeSubseconds(const std::string & instr, std::string & time, 
                           double & subseconds) {
    size_t idx=instr.rfind(".");

    // no decimal point found
    if (idx == std::string::npos) {
      time=instr;
      subseconds=0.0;
      return false;
    }

    // split
    time=instr.substr(0,idx);
    subseconds=atof(instr.substr(idx).c_str());
    return true;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_report(std::string section, bool okay) {
    if (okay) {
      AH_INFO(ahlog::HIGH) << "Passed: " << section << std::endl;
    } else {
      AH_INFO(ahlog::HIGH) << "FAILED: " << section << std::endl;
    }
    return okay;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  bool ut_ahtime() {

    bool allgood=true;
    AH_INFO(ahlog::HIGH) << "*** Test ahtime.h:" << std::endl;

    // leap year function
    {
      std::string section="isLeapYear";
      bool okay=true;
      if (isLeapYear(1995)) okay=false;
      if (!isLeapYear(1996)) okay=false;
      if (!isLeapYear(2000)) okay=false;
      if (isLeapYear(1900)) okay=false;
      if (!isLeapYear(2012)) okay=false;
      if (isLeapYear(2014)) okay=false;
      allgood&=ut_report(section,okay);
    }

    // days in months
    {
      std::string section="daysInMonth";
      bool okay=true;
      if (daysInMonth(1) != 31) okay=false;
      if (daysInMonth(2) != 28) okay=false;
      if (daysInMonth(3) != 31) okay=false;
      if (daysInMonth(4) != 30) okay=false;
      if (daysInMonth(5) != 31) okay=false;
      if (daysInMonth(6) != 30) okay=false;
      if (daysInMonth(7) != 31) okay=false;
      if (daysInMonth(8) != 31) okay=false;
      if (daysInMonth(9) != 30) okay=false;
      if (daysInMonth(10) != 31) okay=false;
      if (daysInMonth(11) != 30) okay=false;
      if (daysInMonth(12) != 31) okay=false;
      if (daysInMonth(1,2012) != 31) okay=false;
      if (daysInMonth(1,2013) != 31) okay=false;
      if (daysInMonth(2,2012) != 29) okay=false;
      if (daysInMonth(2,2013) != 28) okay=false;
      allgood&=ut_report(section,okay);
    }

    // combine/split whole and fractional seconds
    {
      std::string section="(un)catTimeSubseconds";
      bool okay=true;

      std::string time="10:13:45";
      double subseconds=0.123456789;
      std::string out=catTimeSubseconds(time,subseconds,6);
      if (out != "10:13:45.123457") okay=false;

      std::string newtime;
      double newsub;
      uncatTimeSubseconds(out,newtime,newsub);
      if (newtime != time) okay=false;
      if (newsub != 0.123457) okay=false;

      uncatTimeSubseconds(time,newtime,newsub);
      if (newtime != time) okay=false;
      if (newsub != 0.0) okay=false;

      allgood&=ut_report(section,okay);
    }

    return allgood;
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: ahtime.cxx,v $
 Revision 1.4  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
