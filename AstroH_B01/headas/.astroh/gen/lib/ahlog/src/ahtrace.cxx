// backtrace & backtrace_symbols: must compile with gcc/g++ -rdynamic 
#include "ahlog/ahtrace.h" 
#include "ahlog/ahlog.h"

namespace ahlog {
  int getSymbols(std::vector<std::string>& trace) {
    const int bufsz = BUFSIZ;
    void *buff[bufsz]; memset(buff, 0, bufsz);

    trace.clear();

    int nb = backtrace(buff, bufsz);
    ahlog::logger().err() << __PRETTY_FUNCTION__ << " backtrace() returned buffer count: " << nb << std::endl;
    if( nb <= 0 ) {
      return -1;
    }

    char **text = backtrace_symbols(buff, nb);
    if( 0 == text ) return -1;

    for( int i = 0; i < nb; ++i ) {
      if( 0 != text[i] ) trace.push_back(text[i]);
    }

    return (int) trace.size();
  }

  void printTrace(std::vector<std::string>& trace) {
    int ns = (int)trace.size();
    for(int i = 0; i < ns; ++i )
      ahlog::logger().err() << i << ".) " << trace[i] << std::endl;
  }  
  
  // used by unit tester:
  int testTrace5(int cnt) {
//    ahtracevec ahtv;
//    ahtv.print();
    ahtrace aht;
    aht.print();
    return 5;
  }

  int testTrace4(int cnt) {
    if( --cnt >= 0 ) return testTrace5(cnt);

    ahtrace aht;
    aht.print();
    return 3;
  }

  int testTrace3(int cnt) {
    if( --cnt >= 0 ) return testTrace4(cnt);

//    ahtracevec ahtv;
//    ahtv.print();
    ahtrace aht;
    aht.print();
    return 3;
  }

  int testTrace2(int cnt) {
    if( --cnt >= 0 ) return testTrace3(cnt);

    ahtrace aht;
    aht.print();
    return 2;
  } 

  int testTrace1(int cnt) {
    if( --cnt >= 0 ) return testTrace2(cnt);
 
//    ahtracevec ahtv;
//    ahtv.print();
    ahtrace aht;
    aht.print();
    return 1;
  }

  int testTrace(int cnt) {
    testTrace1(--cnt);
    return 0;
  }  
} // namespace ahlog
