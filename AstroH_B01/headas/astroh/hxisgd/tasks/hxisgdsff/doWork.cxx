#if !defined(DOWORK_C)
#define DOWORK_C 

/** \file Implementaion of the doWork function for the Astro-H FITS b-table 'blob slicer'
          to help process First Fits Files. This derives from James Peachey's
          "ahdemo" Build 0.0 example.
    \author David B. Hon
    \date Feb-Mar-Apr 2012
*/

/// all includes needed and local function declarations -- namespace ahhxisgd
/// note some of these funcs. may move into the ahfits or ahgen namespaces
/// and associated libs...
//
// Local includes.
#include "ahhxisgd.h"
AHHXISGD_H(dowork)

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(dowork)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(dowork)

#include "ahtlm/ahBits.h"
AHBITS_H(dowork)

#include "ahtlm/ahSlice.h"
AHSLICE_H(dowork)

#include "ahtlm/ahTLM.h"
AHTLM_H(dowork)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(dowork)

#include "ahtlm/ahTLMinfoCache.h"
AHTLMINFOCACHE_H(dowork)

using namespace ahhxi;
using namespace ahsgd;
using namespace ahtlm;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: doWork.cxx,v 1.11 2012/07/18 16:41:21 dhon Exp $";
} 

namespace ahhxisgd {

  // this is curently a stub/placeholder
  int utmain(int argc, char** argv, char** envp) {
    int result = 0, stride = 16;
    BitsAny bits;
    string prefix = string(__PRETTY_FUNCTION__);
    vector<string> bittxt;
    // unit tests of individual and groups of funcitons in the ahhxisgdnamespace:
    //result = utBits(argc, argv, envp);
    HXIChanFlagsOccur hxi_chanbits;
    setBitsAny(bits, hxi_chanbits);
    bitText(bittxt, bits, stride, prefix);
    for( size_t i = 0; i < bittxt.size(); ++i ) { clog << bittxt[i] << endl; }
    //
    SGDChanFlagsOccur sgd_chanbits;
    setBitsAny(bits, sgd_chanbits);
    for( size_t i = 0; i < bittxt.size(); ++i ) { clog << bittxt[i] << endl; }
    return result;
  }

  void saveWork(AhFitsFilePtr outfile, int* checkpoint) {
    if( checkpoint )
      clog <<  __PRETTY_FUNCTION__  << " checkpoint: " << *checkpoint << endl;
    else
      clog <<  __PRETTY_FUNCTION__  << " checkpoint: none" << endl;
    return;
  }

  bool validInstrum(InstrumWork& input, InstrumWork& output, bool& dohxi, bool& dosgd) { 
    // try to find "INSTRUME" keyword in primay and/or first extension HDU
    vector<string> instrum_names_hdu0, instrum_names_hdu1;
    vector<AhFitsFilePtr> filelist;
    filelist.push_back(input.file); filelist.push_back(output.file);
    int cnt0 = ahtlm::getInstrumNames(instrum_names_hdu0, filelist, 0);
    int cnt1 = ahtlm::getInstrumNames(instrum_names_hdu1, filelist, 1);

    if( 0 == cnt0 && 0 == cnt1 ) {
      clog << __PRETTY_FUNCTION__ << " no instrument name keyword in primay and/or first extension HDU?" << endl;
      return false;
    }

    // should be case-insensitive 
    size_t hxipos = 0, sgdpos = 0;
    string instrum;
    for( int i = 0; i < cnt0; ++i ) {
      instrum = lowerCaseOf(instrum_names_hdu0[i]);
      hxipos = instrum.find("hxi");
      sgdpos = instrum.find("sgd");
      if( hxipos != string::npos ) { 
        dohxi = true; input.name = output.name = "HXI";
      }
      if( sgdpos != string::npos ) {
        dosgd = true; input.name = output.name = "SGD";
      }
    }
    for( int i = 0; i < cnt1; ++i ) {
      instrum = lowerCaseOf(instrum_names_hdu1[i]);
      hxipos = instrum.find("hxi");
      sgdpos = instrum.find("sgd");
      if( hxipos != string::npos ) { 
        dohxi = true; input.name = output.name = "HXI";
      }
      if( sgdpos != string::npos ) {
        dosgd = true; input.name = output.name = "SGD";
      }
    }
    // ok hopefully hxi or sgd instument names have been found
    // if somehow both have been found, indicate potential user-input error
    if( dohxi && dosgd ) {
      clog << __PRETTY_FUNCTION__ << " both HXI and SGD instrume keyword in primay and/or first extension HDU?" << endl;
      return false;
    }
    if( dohxi ) clog << __PRETTY_FUNCTION__ << " HXI instrume keyword in primay and/or first extension HDU?" << endl;
    if( dosgd ) clog << __PRETTY_FUNCTION__ << " SGD instrume keyword in primay and/or first extension HDU?" << endl;
    return true;
  }
  
  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  void doWork(AhFitsFilePtr infile, AhFitsFilePtr outfile, int* checkpoint) {
    if( checkpoint ) {
      if( *checkpoint < 0 ) {
        clog <<  __PRETTY_FUNCTION__  << " ?bad checkpoint: " << *checkpoint << endl;
        return;
      }
      clog <<  __PRETTY_FUNCTION__  << " checkpoint: " << *checkpoint << endl;
    }

    // hard-code test scenarios here:
    //InstrumWork hxi_input("HXI", infile), hxi_output("HXI", outfile);
    //InstrumWork sgd_input("SGD", infile), sgd_output("SGD", outfile);

    // fetch the instrument name (HXI or SGD) from the FITS primary and/or 1st extension
    // header and get to work...
    InstrumWork input(infile), output(outfile); // hxi or sgd work;
    bool dohxi = false, dosgd = false;
    bool valid = validInstrum(input, output, dohxi, dosgd); // should (re)set work instrument name

    if( !valid ) {
      clog << __PRETTY_FUNCTION__  << " valid instrument name check indicates something is amiss: dohxi, doshd " << dohxi << ", " << dosgd << endl;
      if( !dohxi && !dosgd ) {
        if( checkpoint ) *checkpoint = -1;
        return;
      }
    }

    /// optional -- if a checkpoint is desired, a non-null int counter pointer should be decremented 
    /// assume doInstrum did any required decrement...
    int checkpnt = doInstrum(input, output, checkpoint);
    clog <<  __PRETTY_FUNCTION__  << " doInstrum: (checkpoint, names): " << checkpnt << " " << input.name << " " << output.name << endl;
    return;
  } // end doWork

} // end namespace ahhxisgd

#endif // DOWORK_C
