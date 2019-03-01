#if !defined(DOINSTRUM_C)
#define DOINSTRUM_C 

/** \file Implementaion of the doWork function for the Astro-H FITS b-table 'blob slicer'
          to help process First Fits Files. This derives from James Peachey's
          "ahdemo" Build 0.0 example.
    \author David B. Hon
    \date Feb-Mar-Apr 2012
*/

/// all includes needed and local function declarations -- namespace ahhxisgd
/// note some of these funcs. may move into the ahfits or ahgen namespaces
/// and associated libs...

// Local includes.
#include "ahhxisgd.h"
AHHXISGD_H(doinstrum)

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(doinstrum)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(doinstrum)

#include "ahtlm/ahBits.h"
AHBITS_H(doinstrum)

#include "ahtlm/ahSlice.h"
AHSLICE_H(doinstrum)

#include "ahtlm/ahTLM.h"
AHTLM_H(doinstrum)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(doinstrum)

#include "ahtlm/ahTLMinfoCache.h"
AHTLMINFOCACHE_H(doinstrum)

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // isdigit, ispunct, isspace, etc.
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

using namespace ahtlm;
using namespace ahhxi;
using namespace ahsgd;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: doInstrum.cxx,v 1.8 2012/07/18 16:41:21 dhon Exp $";


}

namespace ahhxisgd {
  int creatIOpaths(vector< string >& paths) {
    int np = (int)paths.size();
    if( np <= 0 ) {
      paths.push_back("./input"); paths.push_back("./output"); paths.push_back("./expected_output");
      np = (int)paths.size();
    }

    mode_t m= 0775, m0 = umask(0); // preserve orig. umask...
    for( int i = 0; i  < np; ++i ) {
      string& filename = paths[i];
      clog << __PRETTY_FUNCTION__ << " :: " << filename << endl;
      mkdir(filename.c_str(), m);
    }
    umask(m0);
    return np;
  }

  int creatIOpaths(string& input, string& output, string& expect) {
    vector< string > paths;
    if( input == "" ) input = "./input";
    if( output == "" ) output = "./output";
    if( expect == "" ) expect = "./expected_output";
    paths.push_back(input); paths.push_back(output); paths.push_back(expect);
    return creatIOpaths(paths);
  }

  int writeTextFile(const string& content, const string& filename) {
    string path = filename;
    size_t slashpos = path.rfind("/");
    if( string::npos != slashpos ) { // slash in filename -- be sure path exists
      path = path.substr(0, slashpos);
      vector< string > paths; paths.push_back(path);
      creatIOpaths(paths);
    }
    clog << __PRETTY_FUNCTION__ << " writing " << filename << endl;
    FILE* fp = fopen(filename.c_str(), "w");
    fprintf(fp, "%s", content.c_str());
    fclose(fp);
    return (int)content.length();
  }

  int doInstrum(InstrumWork& input, InstrumWork& output, int* checkpoint) {

    clog <<  __PRETTY_FUNCTION__  << " instrument input : " << input.name << ", output: " << output.name << endl; // "HXI"; // or "SGD";
    if( input.name != output.name ) {
      clog <<  __PRETTY_FUNCTION__  << " must be congruent instrument for inpout and output processing ... " <<endl;
      if( checkpoint ) *checkpoint = -1;
      return -1; 
    }
    string instrum = input.name;

    clog <<  __PRETTY_FUNCTION__  << " inHdu == " << input.hduIdx << ", inRow: " << input.rowIdx << endl;
    clog <<  __PRETTY_FUNCTION__  << " outHdu == " << output.hduIdx << ", outRow: " << output.rowIdx << endl;
  
    // check the current extension hdu setting for the input.file and output.file AhFitsFilePtrs
    vector<string> in_extnames, out_extnames;
    int inhdus = getHDUnames(input.file, in_extnames);
    for( int i = 0; i < inhdus; ++i ) {
      clog <<  __PRETTY_FUNCTION__  << " extension HDU name " << i << ": " << in_extnames[i] << endl;
    }
    int outhdus = getHDUnames(output.file, out_extnames);
    for( int i = 0; i < outhdus; ++i ) {
      clog <<  __PRETTY_FUNCTION__  << " extension HDU name " << i << ": " << out_extnames[i] << endl;
    }

    vector<string> incol_names, outcol_names;
    map<string, int> incol_hash, outcol_hash;
    int incolcnt = getAllColNamesAndNums(input.file, incol_names, incol_hash, input.hduIdx);
    for( int i = 0; i < incolcnt; ++i ) {
      string& name = incol_names[i];
      int colnum = incol_hash[name];
      clog <<  __PRETTY_FUNCTION__  << " input column name and number: " << name << " ==> " << colnum << endl;
    }
    int outcolcnt = getAllColNamesAndNums(output.file, outcol_names, outcol_hash, output.hduIdx);
    for( int i = 0; i < outcolcnt; ++i ) {
      string& name = outcol_names[i];
      int colnum = outcol_hash[name];
      clog <<  __PRETTY_FUNCTION__  << " output column name and number: " << name << " ==> " << colnum << endl;
    }

    if( "HXI" == instrum ) {
      doHXIwork(input.file, output.file);
      if( checkpoint ) *checkpoint = -1;
    }
    else if( "SGD" == instrum ) { 
      doSGDwork(input.file, output.file);
      if( checkpoint ) *checkpoint = -1;
    }
    else {
      clog <<  __PRETTY_FUNCTION__  << " only support Astro-H HXI or SGD ..." << endl;
      if( checkpoint ) *checkpoint = -1;
      return -1; 
    }
    
    if( ! checkpoint )
      return 0;

    (*checkpoint)--;
    clog <<  __PRETTY_FUNCTION__  << " checkpoint: " << *checkpoint << endl;

    return *checkpoint;
  } // end doInstrum

} // end namespace ahhxisgd

#endif // DOINSTRUM_C
