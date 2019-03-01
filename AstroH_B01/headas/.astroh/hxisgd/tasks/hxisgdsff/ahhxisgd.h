#if !defined(AHHXISGD_H)
#define AHHXISGD_H(arg) const char arg##AHHXISGD_rcsId_svnId[] = "$Id: ahhxisgd.h,v 1.10 2012/07/18 16:41:21 dhon Exp $"; 

// Local includes.

// headers have gotten too lengthy so moved some stuff here ...
// FF-SF bits and column data typedefs and (some) related funcs.
// FF and SF structs below make use of the TLM slices defined
// here, and should reflect b-table column org. of assoc. FITS files:

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(ahhxisgd)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(ahhxisgd)

#include "ahtlm/ahBits.h"
AHBITS_H(ahhxisgd)

#include "ahtlm/ahSlice.h"
AHSLICE_H(ahhxisgd)

#include "ahtlm/ahTLM.h"
AHTLM_H(ahhxisgd)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(ahhxisgd)

#include "ahtlm/ahTLMinfoCache.h"
AHTLMINFOCACHE_H(ahhxisgd)

// Local includes.
#include "ahfits/ahfits.h" /// core ahfits namespace
//#include "ahfits/vcslice.h" /// supplements ahfits namespace

#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

// Regional includes.
#include "ape/ape_error.h"
#include "ape/ape_trad.h"

// system.
#include <stdio.h>
#include <unistd.h>

// ISO includes.
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <bitset>
#include <vector>
#include <valarray>

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

/// \ingroup mod_ahhxisgd
namespace ahhxisgd {

  /** \addtogroup mod_ahhxisgd
   *  @{
   */

  /// simple inlines:
  inline string hduNameDefault() { return string("EVENTS"); }
  inline int imax(int a, int b) { if(a > b) return (a); return(b); }
  inline size_t imax(size_t a, size_t b) { if(a > b) return (a); return(b); }

  struct InstrumWork { 
    string name; AhFitsFilePtr file;
    vector<string> col_names;
    vector<int> col_sizes; // do we need col. type info too?
    int rowIdx, rowCnt, hduIdx; 
    InstrumWork(AhFitsFilePtr ahp, string instrum= "HXI", int row_idx= 0, int num_rows= 1, int hdu_idx= 1) {
      file = ahp; name = instrum; rowIdx = row_idx; rowCnt = num_rows; hduIdx = hdu_idx;
    }
    InstrumWork(string instrum, AhFitsFilePtr ahp= 0, int row_idx= 0, int num_rows= 1, int hdu_idx= 1) {
      file = ahp; name = instrum; rowIdx = row_idx; rowCnt = num_rows; hduIdx = hdu_idx;
    }
  };
  
  /// the app. runtime param. struct
  typedef struct {
    /// conveyed from ahdemo
    string _outfile_name;
    string _template_name;
    string _infile_name;
 
    /// more application runtime info
    string _instrum_name; // "HXI" or "SGD"
    int _threads; // single or multi-threaded app.
 
    /// all pfile and cmd-line opts. for convenience 
    vector< string > _parlist;
    map< string, vector<bool> > _boolpars;
    map< string, vector<int> > _intpars;
    map< string, vector<double> > _dblpars;
    map< string, string > _txtpars;
    map< string, FILE* > _ancilfiles;
    map< string, AhFitsFilePtr > _infiles;
    map< string, AhFitsFilePtr > _outfiles;
   } AppRuntime;

  /// alt. to struct initializer/constructor, note optional infile and threading spec.:
  inline void setAppRuntime(AppRuntime& app, char* instrum, char* outfile, char *tmplat, char* infile = 0, int threads = 1) {
    /// default is single-threaded app.
    app._threads = threads;
    /// note potentially empty strings -- imply built-in defaults for internal tests?
    instrum ? app._instrum_name = *instrum : app._instrum_name = "HXI";
    outfile ? app._outfile_name = *outfile : app._outfile_name = "";
    tmplat ? app._template_name = *tmplat : app._template_name = "";
    infile ? app._infile_name = *infile : app._infile_name = ""; 
  }

  void startUp(int argc, char ** argv, string version, int* checkpoint);
  int doInstrum(InstrumWork& input, InstrumWork& output, int* checkpoint= 0);
  void finalize(AhFitsFilePtr infile, AhFitsFilePtr outfile, int* checkpoint= 0);
  void shutDown(string version, int* checkpoint= 0);

  // in doWork.cxx:
  int utmain(int argc, char** argv, char** envp);
  void saveWork(AhFitsFilePtr outfile, int* checkpoint= 0);
  bool validInstrum(InstrumWork& input, InstrumWork& output, bool& dohxi, bool& dosgd);
  void doWork(AhFitsFilePtr infile, AhFitsFilePtr outfile, int* checkpoint= 0);

  // in getPar.cxx: 
  string lowerCaseOf(const string& s);
  int getParList(AppRuntime& app, vector< string >& parlist, map< string, string >& txtpars, map< string, int >& intpars, map< string, double >& fltpars);
  int getParList(AppRuntime& app, vector< string >& parlist);
  void getParAncil(string& ancilfile, int& threadcnt);
  bool getParFITS(string& infile_name, string& outfile_name, string& template_name, bool* pclobber, bool* pprecheck);

  // in initialize.cxx:
  string initialize(string& infile_name, string& outfile_name, string& template_name, AhFitsFilePtr * infile, AhFitsFilePtr * outfile, bool simu);
  bool preCheck(string& version, string& infile_name, string& outfile_name, string& template_name);

  /** @} */

} // namespace ahhxisgd

#endif
