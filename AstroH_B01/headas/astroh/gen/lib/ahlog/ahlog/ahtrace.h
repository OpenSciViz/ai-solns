/** 
 * \file ahtrace.h
 * \brief Traceback support for AstroH logger, ahlog
 * \author David Hon
 * \date $Date: 2012/05/17 21:23:39 $
 *
 * Initializes and provides a global formatter for st_stream.  Also defines 
 * macros for compact logging syntax.
 * 
 */

#if !defined(__AHtrace_h__)
#define __AHtrace_h__ "$Name: AstroH_B01 $ $Id: ahtrace.h,v 1.2 2012/05/17 21:23:39 mwitthoe Exp $"

// backtrace & backtrace_symbols: must compile with gcc/g++ -rdynamic 
#include <execinfo.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cerrno>

// c++ stdlib
//#include <strstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <string>
#include <cstring>

//using namespace std;

namespace ahlog {
  // c with a bit of c++ via stdlib:
  int getSymbols(std::vector<std::string>& trace);
  void printTrace(std::vector<std::string>& trace);

  int testTrace5(int cnt= 0);
  int testTrace4(int cnt= 0);
  int testTrace3(int cnt= 0);
  int testTrace2(int cnt= 0);
  int testTrace1(int cnt= 0);
  int testTrace(int cnt= 1);

  // a bit oo-like (a bit more of c++):
  struct ahtrace {
    // all struct elements are by default "public"
    // we could also place char** and void* buff here..
    // for now the only attribute is the trace vector:
    std::vector<std::string> _trace;

    // and these static funcs
    // example invoke -- { ahlog::ahtrace aht; ahlog::ahtrace::print(aht); }
    inline static void print(ahtrace& aht) { ahlog::printTrace(aht._trace); }
    inline static int utmain(int argc, char **argv) {
      if( argc > 1 && isdigit(argv[1][0]) ) return ahlog::testTrace(atoi(argv[1]));
      return ahlog::testTrace(5);
    }

    // and these non-static funcs
    // example invoke -- { ahlog::ahtrace aht; aht.print(); }
    inline void print() { ahlog::printTrace(_trace); }
    inline int getSymbols() { return ahlog::getSymbols(_trace); }

    // and the default constructor:
    inline ahtrace() { getSymbols(); }
  };

/*
  // and/or more oo-like:
  struct ahtracevec : public vector<string> {
    inline static void print(ahtracevec& ahtvec) { ahlog::printTrace(ahtvec); }

    inline static int utmain(int argc, char **argv) {
      if( argc > 1 && isdigit(argv[1][0]) ) return ahlog::testTrace(atoi(argv[1]));
      return ahlog::testTrace(5);
    }

    inline void print() { ahlog::printTrace(*this); }

    inline int getSymbols() { return ahlog::getSymbols(*this); }

    inline ahtracevec() { getSymbols(); }
  };
*/

} // namespace ahlog

#endif

/* Revision Log
 $Log: ahtrace.h,v $
 Revision 1.2  2012/05/17 21:23:39  mwitthoe
 integrate ahtrace into ahlog's AH_ERR macro; trace is only printed when DEBUG is true



*/
