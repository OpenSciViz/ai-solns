#if !defined(INITIALIZE_C)
#define INITIALIZE_C 

/** \file Implementaion of the iniitlize function for the Astro-H HXI & SGD 
          First Fits File to Second FITS File tool "fff2sff". This derives
          from James Peachey's "ahdemo" Build 0.0 example.
    \author David B. Hon
    \date Feb-Mar 2012
*/

#include "ahhxisgd.h" // all includes needed and local function declarations -- namespace ahhxisgd
AHHXISGD_H(intialize)

using namespace ahhxi;
using namespace ahsgd;
using namespace ahtlm;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: initialize.cxx,v 1.13 2012/07/18 16:41:21 dhon Exp $";

  string genTestFITS(string& infile_name, string& outfile_name, string& template_name, AhFitsFilePtr* infile, AhFitsFilePtr* outfile) {
      // create simulated input and expected output...
      // use a built-in template for our simulated input
      string tplsgd= "", tplhxi= "";
      string intemplate_name = ""; // create simulated template for outfile
      string outemplate_name = template_name; // = "./input/infitsimu.tpl"; // create simulated template for outfile
      // init sgd or hxi ff (input test fits file) template, based on output_template name
      bool dosgd = false, dohxi = false;
      // this is not robust, but adequate for our internal test TLM generation
      if( outemplate_name.find("SGD") != string::npos || outemplate_name.find("sgd") != string::npos ) {
        dosgd = true;
        initSGDtemplates(tplsgd, intemplate_name, outemplate_name);
      }
      else {
        dohxi = true;
        initHXItemplates(tplhxi, intemplate_name, outemplate_name);
      }
      ahFitsCreate(infile_name.c_str(), intemplate_name.c_str(), infile, true);
      if( 0 == infile ) {
        clog <<  __PRETTY_FUNCTION__ << " failed to create sample input (default simu): " << infile_name << endl;
        return "";
      }
      ahFitsCreate(outfile_name.c_str(), outemplate_name.c_str(), outfile, true);
      if( 0 == outfile ) {
        clog <<   __PRETTY_FUNCTION__ << " failed to create expected output (default simu): " << outfile_name << endl;
        return "";
      }

      // a simple approach to create the largest file (most number of rows)
      // then (f)copy subsets of rows into one or more smaller files
      // to provide a collection of testable files. but to do so may require
      // more in/out (pre-opened) AhFitsFilePtr params ...
      // also, need to check cfitsio   behavior on file close and max-column-length setting
      int test_rowcnt = 1; // 2; // 3;
      // don't want to recompile each time we need to change the nature of the test file, nor
      // should this be a parfile option, so just use an env var.
      char* txtcnt = getenv("AHTEST_ROWS");
      if( txtcnt ) test_rowcnt = atoi(txtcnt);
      clog <<  __PRETTY_FUNCTION__ << " test rows: " << test_rowcnt <<" "
           << infile_name << ", " << template_name << ", " << outfile_name << endl;
      if( dohxi ) { 
        hxiWriteTestDefaults(*infile, *outfile, test_rowcnt); // if not SGD, assume HXI
      }
      else {
        sgdWriteTestDefaults(*infile, *outfile, test_rowcnt); // else assume SGD
      }
      return intemplate_name;
    }
}

namespace ahhxisgd {

  bool preCheck(string& version, string& infile_name, string& outfile_name, string& template_name) {
    clog << __PRETTY_FUNCTION__ << version << ", " << infile_name << ", " << outfile_name << ", " << template_name << endl;
    return false; // for debug / eval. force shutdown until this stub is fleshed-out
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  string initialize(string& infile_name, string& outfile_name, string& template_name,
                    AhFitsFilePtr * infile, AhFitsFilePtr * outfile, bool simu) {
    // sumlate input and output in self-consistent fashion
    vector< string > iopaths;
    int np = creatIOpaths(iopaths);
    if( np < (int)iopaths.size() ) {
      clog << __PRETTY_FUNCTION__ << " input/output directory path create/check failed..." << endl; 
      throw runtime_error(string(__PRETTY_FUNCTION__) + " input/output directory path create/check failed...");
    }
    if( simu ) {
      // create simulated input and expected output...
      // use a built-in template for our simulated input
      return genTestFITS(infile_name, outfile_name, template_name, infile, outfile);
    }

    /* Open events extension in the input file. ahFitsOpen is a function in libahfits that opens the file
       (using the @filename convention to open multiple input files). It also sets up the optimal buffering
       for the file(s). */
    // C++ version: this will throw an exception if it fails to open the file.
    if( infile_name == "" )
      throw std::logic_error(string( __PRETTY_FUNCTION__) +  " no infile_name, cannot procede without it.");

    if( template_name == "" )
      throw std::logic_error(string(__PRETTY_FUNCTION__) + " no template_name, cannot procede without it.");

    if( !simu && outfile_name == "" )
      throw std::logic_error(string( __PRETTY_FUNCTION__) + " no outfile_name, cannot procede without it.");

    /* Open output file. ahFitsCreate is a function in libahfits that creates the file using
       a FITS standard template It also sets up the optimal buffering for the file(s). */
    // C++ version: this will throw an exception if it fails to create the file.

    // should first fetch column name from template -- note current version of sff template has no extname?
    string hdu_extname = hduNameDefault();
    try {
      clog << __PRETTY_FUNCTION__ << " " << infile_name << ", " << template_name << ", " << outfile_name << endl;
      ahFitsOpen(infile_name.c_str(), hdu_extname.c_str(), infile); 
      vector<string> extnames; // use built-in default 
      int hduextcnt = getHDUnames(*infile, extnames); // get the extension names from template or input file?
      clog <<  __PRETTY_FUNCTION__ << " HDU count: " << hduextcnt << " in: " << infile_name  << endl;
      if( hduextcnt <= 0 ) throw std::runtime_error(string(__PRETTY_FUNCTION__) + " bad input HDU count " + infile_name);
      for( int i = 0; i < hduextcnt; ++i ) {
        clog <<  __PRETTY_FUNCTION__ << " " << extnames[i] << endl;
      }
      bool clobber = getClobber();
      clog << __PRETTY_FUNCTION__ << " clobber: " << clobber << endl;
      ahFitsCreate(outfile_name.c_str(), template_name.c_str(), outfile, clobber);
      // Go to the the first extension hdu in the output file, if it exists.
      ahFitsMove(*outfile, hdu_extname.c_str());
      //if( hduextcnt > 0 ) ahFitsMove(*outfile, extnames[0].c_str());
    }
    catch( const exception& ex ) {
      clog << ex.what() << endl;
      throw; // pass this up the call stack ...
    }

    return "";
  }

} // namespace ahhxisgd

#endif // INITIALIZE_C
