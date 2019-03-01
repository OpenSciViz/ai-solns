#include "ahlog/ahtrace.h" // supplemental ahlog funcs for call-stack-trace

using namespace ahlog;

/// test ahtrace callstack util.
int main(int argc, char **argv) {
  // int n =  ahlog::utmain(argc, argv);
  int nt = ahlog::ahtrace::utmain(argc, argv);
//  int ntv = ahlog::ahtracevec::utmain(argc, argv);
//  clog << argv[0] << " nt: " << nt << " and ntv: " << ntv << endl;

  // more tests:
//  ahlog::ahtrace aht;
//  aht.print();

//  ahlog::ahtracevec ahtv;
//  ahtv.print();

//  vector<string> symbvec;
//  ahlog::getSymbols(symbvec);
//  ahlog::printTrace(symbvec);

  return 0;
}
