#if !defined(STARTUP_C)
#define STARTUP_C 

/** \brief Implementation of public members of the libahgen library.
    \author James Peachey
    \date 2012-01-29
*/

#include "ahhxisgd.h" // all includes needed and local function declarations -- namespace ahhxisgd
AHHXISGD_H(startup)

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

// c++
#include <limits>
#include <cerrno>
#include <iostream>
//#include <strstream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <map>
//#include <multimap.h>
#include <string>
#include <cstring>

//using namespace ahtlm;
//using namespace ahhxi;
//using namespace ahsgd;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: startUp.cxx,v 1.8 2012/07/19 17:09:35 peachey Exp $";

  static int* theCheckPoint = 0;

  static const int sigSet(std::map<int, string>& sigs) {
    const int sigcnt = 16;
    const int signum[sigcnt] = { SIGABRT, SIGALRM, SIGCHLD, SIGCONT, SIGFPE, SIGHUP, SIGINT, SIGPIPE, /*SIGPWR,*/
                                 SIGSTOP, SIGTSTP, SIGTERM, SIGURG, SIGUSR1, SIGUSR2, SIGVTALRM, SIGXFSZ };
    const string signame[sigcnt] = { "SIGABRT", "SIGALRM", "SIGCHLD", "SIGCONT", "SIGFPE", "SIGHUP", "SIGINT", "SIGPIPE", /*SIGPWR,*/
                                     "SIGSTOP", "SIGTSTP", "SIGTERM", "SIGURG", "SIGUSR1", "SIGUSR2", "SIGVTALRM", "SIGXFSZ" };
    for( int i = 0; i < sigcnt; ++i )
      sigs[signum[i]] = signame[i];

    return sigcnt;
  }

  /// posix sighandler
  static void theSigHandler(int signum) {
    switch( signum ) {
      case SIGABRT:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGABRT -- abort!"<<endl;
                      if( theCheckPoint ) *theCheckPoint = -1;
                      //abort();
                      break;
      case SIGALRM:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGALRM"<<endl; break;
      case SIGCHLD:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGCHLD"<<endl; break;
      case SIGCONT:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGCONT"<<endl; break;
      case SIGFPE:    clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGFPE"<<endl; break;
      case SIGHUP:    clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGHUP -- checkpoint!"<<endl;
                      if( theCheckPoint ) (*theCheckPoint)--; // or = -1;
                      break;
      case SIGINT:    clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGINT  -- checkpoint!"<<endl;
                      if( theCheckPoint ) (*theCheckPoint)--; // or = -1;
                      break;
      case SIGPIPE:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGPIPE"<<endl; break;
      //case SIGPWR:  clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGPWR"<<endl; break;
      case SIGSTOP:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGPIPE"<<endl; break;
      case SIGTSTP:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGPIPE"<<endl; break;
      case SIGTERM:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGTERM"<<endl;
                      if( theCheckPoint ) *theCheckPoint = -1;
                      break;
      case SIGURG:    clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGURG"<<endl; break;
      case SIGUSR1:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGG"<<endl; break;
      case SIGUSR2:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGUSR2"<<endl; break;
      case SIGVTALRM: clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGVTALRM"<<endl; break;
      case SIGXFSZ:   clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" SIGXFSZ"<<endl; break;
      default: clog<<__PRETTY_FUNCTION__ <<" caught signal "<<signum<<" unexpected SIG?"<<endl; break;
    }
    return;
  } // sigHandler
} // private namespace

namespace ahhxisgd {

  bool s_clobber = true;
  // since MAXFLOAT is "hidden" on macos under /usr/include/architecture/*/math.h
  // rather than simply in /usr/include/math.h ... just use this as quick-fix:
  const double maxfloat = std::numeric_limits<float>::max(); // 3.4028234e+38;
  const double timeres = 1.0/maxfloat;

  int sleep(double time) {
    if( time < timeres ) time = maxfloat; // (nearly) zero or negative time == 'til interrupt?
    time_t sec = static_cast< time_t >( time );  // truncate to sec.
    double fracsec = time - sec; // fractional sec.
    // nano seconds of timespec cannot be more than 10e9
    long nsec = static_cast< long >( fracsec * 1e9 ) ;
    struct timespec sleep = { sec, nsec };
    int sval = nanosleep( &sleep, &sleep );
    if( sval < 0 ) {
      clog << __PRETTY_FUNCTION__  << " interrupted by signal? " << strerror(errno) << endl;
    }
    return sval;
  } // sleep

  static void setSigHandler(int* checkpoint) {
    if( checkpoint ) theCheckPoint = checkpoint;
    //
    std::map<int, string> sigs;
    const int sigcnt = sigSet(sigs);
    if( sigcnt <= 0 ) 
      return;

    sigset_t sigset, prev_sigset;
    int stat = sigemptyset(&sigset); /* sigmask == 0 */
    map<int, string>::iterator ip;
    //for( int i = 0; i < sigcnt; i++ ) {
    for( ip = sigs.begin(); ip != sigs.end(); ++ip ) {
      int signum = ip->first; // signums[i];
      stat = sigaddset(&sigset, signum);
    }
    pthread_sigmask(SIG_SETMASK, &sigset, &prev_sigset);
    struct sigaction act, actold;
    act.sa_handler = theSigHandler; // one handler for all signals
    act.sa_flags = SA_NOCLDSTOP; // only POSIX... | SA_RESTART; // | SA_SIGINFO; for SOGRTMIN-MAX
    act.sa_mask = sigset; // block all the signals during handling of any
    for( ip = sigs.begin(); ip != sigs.end(); ++ip ) {
    //for( int i = 0; i < sigcnt; ++i ) {
      int signum = ip->first; // signums[i];
      string signame = ip->second; // signames[i];
      //clog << __PRETTY_FUNCTION__  << "> add handler for sig: "<< signame << " == " << signum <<endl;
      sigaction(signum, &act, &actold);
    }
    pthread_sigmask(SIG_UNBLOCK, &sigset, &prev_sigset);
    return;
  } // setSigHandler

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  void startUp(int argc, char ** argv, string version, int* checkpoint) {
    std::string banner(__func__); banner += ": ";
    int status = ape_trad_init(argc, argv);
    if (eOK != status) {
      // TODO: report status code along with message.
      throw std::runtime_error(banner + "ape_trad_init returned non-0 status");
    }

    if( checkpoint ) {
      // setSigHandler(checkpoint);
    }

    // Ignore error -- s_clobber has default value of true.
    char clobber = 0;
    status = ape_trad_query_bool("clobber", &clobber);
    if( eOK == status ) {
      // Set the clobber flag.
      s_clobber = 0 != clobber ? true : false;
    }
  } // startUp

} // namespace ahhxisgd

#endif // STARTUP_C
