#if !defined(SHUTDOWN_C)
#define SHUTDOWN_C 

#include "ahhxisgd.h" // all includes needed and local function declarations -- namespace ahhxisgd
AHHXISGD_H(shutdown)

#include "ahgen/ahgen.h"
#include "ape/ape_error.h"
#include "ape/ape_trad.h"
#include <stdexcept>
#include <string>

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: shutDown.cxx,v 1.5 2012/07/17 19:08:50 dhon Exp $";
}

namespace ahhxisgd {
  void shutDown(string version, int* checkpoint) {
    ape_trad_close(1);
  }
} // namespace ahhxisgd

#endif // SHUTDOWN_C
