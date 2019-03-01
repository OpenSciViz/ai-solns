#include "ahlog/ahlog.h"

#include <execinfo.h> 
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace ahlog {

  /// \callgraph
  st_stream::StreamFormatter & logger() {
    static st_stream::StreamFormatter s_format("", "", ahlog::MAXCHAT);
    return s_format;
  }

  /// \callgraph
  st_stream::OStream & macro_out(const std::string & func) {
    ahlog::logger().setMethod(func);
    return ahlog::logger().out();
  }

  /// \callgraph
  st_stream::OStream & macro_info(const int & chatter, const std::string & func) {
    ahlog::logger().setMethod(func);
    return ahlog::logger().info(chatter);
  }

  /// \callgraph
  st_stream::OStream & macro_warn(const int & chatter, const std::string & func) {
    ahlog::logger().setMethod(func);
    return ahlog::logger().warn(chatter);
  }

  /// \callgraph
  st_stream::OStream & macro_err(const std::string & func) {
    ahlog::logger().setMethod(func);
    return ahlog::logger().err();
  }

  /// \callgraph
  st_stream::OStream & macro_debug(const std::string & pfunc) {
    ahlog::logger().setMethod(ahlog::strip_function(pfunc));
    return ahlog::logger().debug();
  }

  /// \callgraph
  void macro_trace() {
    if(!ahlog::debug()) return;

    const int bufsz = BUFSIZ;
    void *buff[bufsz];
    memset(buff, 0, bufsz);

    int nb = backtrace(buff, bufsz);
    if( nb <= 0 ) return;

    char **text = backtrace_symbols(buff, nb);
    if( 0 == text ) return;

    ahlog::logger().err() << "*** STACK TRACE ***" << std::endl;
    for( int i = 1; i < nb; ++i ) {    // i=0 is always the macro function; skip
      if( 0 == text[i] ) continue;
      ahlog::logger().err() << i << ". " << text[i] << std::endl;
    }
    ahlog::logger().err() << "*** END TRACE ***" << std::endl;
  }

  /// \callgraph
  std::ofstream* & logfile() {
    static std::ofstream* std_os;
    return std_os;
  }

  /// \callgraph
  bool & writelogfile() {
    static bool logfileon;
    return logfileon;
  }

  /// \callgraph
  bool & debug() {
    static bool debugon;
    return debugon;
  }

  /// \callgraph
  bool & clobber() {
    static bool clobberlog;
    return clobberlog;
  }

  /// \callgraph
  struct timeval* & starttime() {
    static struct timeval* logopentime;
    return logopentime;
  }

  /// \callgraph
  void setup(std::string execname, std::string logfile, int chatter, 
             bool debug) {
    // store start time
    starttime()=new struct timeval;
    gettimeofday(starttime(),NULL);

    // set debug state
    ahlog::debug()=debug;

    // strip path information from given executable name
    std::string execbase=ahlog::strip_path(execname);

    // initialize stream (if chatter=0, then write everything only to log file)
    if (0 == chatter) {
      chatter=ahlog::MAXCHAT;     // want all message printed to log file
      st_stream::InitGlobal(execbase, chatter, debug);
    } else {
      st_stream::InitStdStreams(execbase, chatter, debug);
    }

    // get clobber
    clobber()=false;
    if (logfile[0] == '!') {
      clobber()=true;
      logfile.erase(0,1);
    }

    // if empty logfile, base filename on execname
    writelogfile()=true;
    if ("" == logfile) {
      throw std::runtime_error("the name of the log file was set to an empty string (\"\"); this is an illegal value");
    } else if ("DEFAULT" == logfile) {
      std::stringstream tstr;
      tstr << execname << ".log";
      logfile=tstr.str();
    } else if ("NONE" == logfile) {
      writelogfile()=false;
    }

    if (writelogfile()) {
      if (clobber()) {
        ahlog::logfile()=new std::ofstream(logfile.c_str());
      } else {
        ahlog::logfile()=new std::ofstream(logfile.c_str(),std::fstream::app);
      }
      if (ahlog::logfile()->bad()) throw "could not open log file";

      // write date and time to top of header file
      ahlog::write_header(execbase);

      st_stream::stlog.connect(*ahlog::logfile());
      st_stream::sterr.connect(*ahlog::logfile());
      st_stream::stout.connect(*ahlog::logfile());
    }

  }

  /// \callgraph
  void shutdown() {
    ahlog::write_footer(true);
    if (writelogfile()) ahlog::logfile()->close();
  }

  /// \callgraph
  void write_header(const std::string & execname) {
    if (!writelogfile()) return;    // only write header to log file

    struct tm *current;
    time_t now;
    time(&now);
    current=localtime(&now);

    char buff[100];
    sprintf(buff,"%i/%02i/%02i %i:%02i:%02i",1900+current->tm_year,
            1+current->tm_mon,current->tm_mday,current->tm_hour,
            current->tm_min,current->tm_sec);

    *ahlog::logfile() << "STARTLOG: " << buff << std::endl;
    *ahlog::logfile() << "EXECNAME: " << execname << std::endl;
    *ahlog::logfile() << "RUNPATH:  " << getcwd(NULL,0) << std::endl;
    *ahlog::logfile() << std::endl;
  }

  /// \callgraph
  void write_footer(const bool status) {
    if (!writelogfile()) return;    // only write footer to log file

    // get current time
    struct timeval* endtime;
    endtime=new struct timeval;
    gettimeofday(endtime,NULL);

    // get program elapsed seconds
    double wtime=(endtime->tv_sec-starttime()->tv_sec)+ // integral seconds
                 (endtime->tv_usec-starttime()->tv_usec)/1.e6; // fractional

    // convert to minutes if over 60s
    std::string units=" s";
    /// \internal
    /// \note uncomment below to allow for conversion of wall time to minutes
    /// if greater than 60s.
    //if (wtime > 60.) {
    //  wtime/=60.;
    //  units=" min";
    //}

    *ahlog::logfile() << std::endl; 
    /// \internal
    /// \note uncomment below to give exit states: normal or error
    //if (status) {
    //  *ahlog::logfile() << "EXIT:     NORMAL" << std::endl;
    //} else {
    //  *ahlog::logfile() << "EXIT:     ERROR" << std::endl;
    //}
    *ahlog::logfile() << "ENDLOG:   " << wtime << units << std::endl;
  }

  /// \callgraph
  std::string strip_function(const std::string instr) {
    std::string x=instr;

    // for the last argument list, gut contents but leave parentheses;
    // remove other argument lists entirely including parentheses;
    // will misbehave for nested parentheses
    unsigned int i,istart,iend=0;
    while (iend < x.size()-1) {
      istart=-1;
      for (i=0; i<x.size(); i++) {
        if (x[i] == '(') istart=i;
        if (x[i] == ')') {
          iend=i;
          break;
        }
      }
      if (istart < 0) break;          // no parentheses found

      std::string tx=x.substr(0,istart);
      if (iend < x.size()-1) tx+=x.substr(iend+1,x.size()-1);
      if (iend == x.size()-1) tx+="()";
      x=tx;
    }

    // remove return type (this is done after removing argument lists
    // to avoid the problem of no return type and a space in an 
    // argument list)
    for (i=0; i < x.size(); i++) {
      if (x[i] == ' ') break;
    }
    istart=i+1;
    if (istart < x.size()) {        // be sure a space is found
      x=x.substr(istart,x.size()-1);
    }

    return x;
  }

  /// \callgraph
  std::string strip_path(const std::string instr) {
    int n=instr.size();
    while ( n > 0 && instr[n-1] != '/' && instr[n-1] != '\\') n--;
    return instr.substr(n,instr.size()-n);
  }

  /// \callgraph
  std::string prepend(const std::string & msg, const std::string & func) {
    std::stringstream tstr;
    tstr << ahlog::strip_function(func) << " -> " << msg;
    return tstr.str();
  }

}
