/** \brief Implementation of public members of the libahgen library.
    \author James Peachey
    \date 2012-01-29
*/
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

#include "ape/ape_error.h"
#include "ape/ape_trad.h"

#include <stdexcept>
#include <string>
#include <string.h>

namespace ahgen {

static bool s_clobber = true;
static int s_chatter = 2; // Output, error, high-priority warnings and info.
static bool s_debug = false;

void startUp(int argc, char ** argv) {
  std::string banner(__func__); banner += ": ";
  int status = ape_trad_init(argc, argv);
  if (eOK != status) {
    // TODO: report status code along with message.
    throw std::runtime_error(banner + "ape_trad_init returned non-0 status");
  }

  char clobber = 0;
  status = ape_trad_query_bool("clobber", &clobber);
  if (eOK != status) {
    // Ignore error -- s_clobber has default value.
  } else {
    // Set the clobber flag.
    s_clobber = 0 != clobber ? true : false;
  }

  status = ape_trad_query_int("chatter", &s_chatter);

  char debug = 0;
  status = ape_trad_query_bool("debug", &debug);
  if (eOK != status) {
    // Ignore error -- s_debug has default value.
  } else {
    // Set the debug flag.
    s_debug = 0 != debug ? true : false;
  }

  char* logfile;
  status = ape_trad_query_string("logfile",&logfile);
  if (eOK != status) logfile="!DEFAULT";

  ahlog::setup(argv[0], logfile, s_chatter, s_debug);
}

void shutDown(void) {
  ape_trad_close(1);
  ahlog::shutdown();
}

static int s_status = 0;

int getStatus(void) { return s_status; }

int setStatus(int status) { if (0 == s_status) s_status = status; return s_status; }

void resetStatus(void) { s_status = 0; }

bool getClobber(void) {
  return s_clobber;
}

int getChatter(void) {
  return s_chatter;
}

bool getDebug(void) {
  return s_debug;
}

/// \todo make case insensitive
int atoinst(const char* inst_str) {
  if (strcmp(inst_str,(char*)"SXS_PIXEL") == 0)
    return ahgen::INST_SXS_PIXEL;
  else if (strcmp(inst_str,(char*)"SXS_ANTICO") == 0)
    return ahgen::INST_SXS_ANTICO;
  else if (strcmp(inst_str,(char*)"SXI") == 0)
    return ahgen::INST_SXI;
  else if (strcmp(inst_str,(char*)"HXI") == 0)
    return ahgen::INST_HXI;
  else if (strcmp(inst_str,(char*)"SGD_CC") == 0)
    return ahgen::INST_SGD_CC;
  else if (strcmp(inst_str,(char*)"SGD_WAM") == 0)
    return ahgen::INST_SGD_WAM;
  return ahgen::INST_BAD;
}


} // namespace ahgen

/* Revision Log
 $Log: ahgen.cxx,v $
 Revision 1.7  2012/05/23 22:18:54  mwitthoe
 ahgen::startUp() will now take the log file name from the parameter file

 Revision 1.6  2012/05/16 20:13:54  mwitthoe
 change setup() arguments to match updated ahlog; add ahlog::shutdown() to shutDown() to properly close log file

 Revision 1.5  2012/05/10 19:16:08  mwitthoe
 make ahlog::setup() call in ahgen consistent with current ahlog version

 Revision 1.4  2012/04/25 21:34:25  peachey
 Add chatter and debug parameters to the startUp step.

 Revision 1.3  2012/04/24 15:26:07  mwitthoe
 add instrument enumeration and conversion function to ahgen

 Revision 1.2  2012/02/03 15:30:45  peachey
 Add and test getClobber facility, from parameter file.

 Revision 1.1  2012/01/31 22:23:33  peachey
 Add first version of general support library.

*/
