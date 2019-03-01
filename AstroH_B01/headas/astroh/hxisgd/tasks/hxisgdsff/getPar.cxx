#if !defined(GETPAR_C)
#define GETPAR_C
/** \file Implementaion of the getPar function for the Astro-H HXI & SGD 
          First Fits File to Second FITS File tool "fff2sff". This derives
          from James Peachey's "ahdemo" Build 0.0 example.
    \author David B. Hon
    \date Feb-Mar 2012
*/

// all includes needed and local function declarations -- namespace ahhxisgd
// note this include stdexcept ... which may include std::string ?
#include "ahhxisgd.h" // all includes needed and local function declarations -- namespace ahhxisgd
AHHXISGD_H(getpar)

using namespace ahhxi;
using namespace ahsgd;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace { 
  const string cvsId = "$Name: AstroH_B01 $ $Id: getPar.cxx,v 1.11 2012/07/19 17:11:36 peachey Exp $";
}

namespace ahhxisgd {
  // helper funcs -- may move to libahgen 
  string lowerCaseOf(const string& s) {
    int slen = (int)s.length();
    string lcs = s;
    for( int i = 0; i < slen; ++i  ) { 
      lcs[i] = std::tolower(lcs[i]);
    }
    return lcs;
  }
  // it is convenient to retrun the full list of runtime (cmd-line) params as a vector
  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int getParList(AppRuntime& app, vector< string >& parlist, map< string, string >& txtpars, map< string, int >& intpars, map< string, double >& fltpars) {
    parlist.clear();
    parlist[0] = "threads"; intpars[parlist[0]] = 1;
    parlist[1] = "infile_name"; txtpars[parlist[1]] = "./input/foo.fits";
    parlist[2] = "template_name"; txtpars[parlist[2]] = "./input/foo.tpl";
    parlist[3] = "outfile_name"; txtpars[parlist[3]] = "./output/bar.fits";
    parlist[4] = "xdim_colname"; txtpars[parlist[4]] = "colX";
    parlist[5] = "ydim_colname"; txtpars[parlist[5]] = "colY";
    parlist[6] = "xdim_sizemax"; intpars[parlist[6]] = 40;
    parlist[7] = "ydim_sizemax"; intpars[parlist[7]] = 32;

    // but the implementation of the AppRuntime struct will change use for stub:
    app._threads = intpars["threads"];
    app._parlist = parlist;
    return (int)app._parlist.size(); 
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int getParList(AppRuntime& app, vector< string >& parlist) {
    map< string, string > txtpars;
    map< string, int > intpars;
    map< string, double > fltpars;
    return getParList(app, parlist, txtpars, intpars, fltpars); 
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  void getParAncil(string& ancilfile, int& threadcnt) {
    // any fon-FITS I/O?
    ancilfile = "";
    if(  ancilfile != "" )
      clog << __func__ <<  __PRETTY_FUNCTION__  << " ancilfile = " <<  ancilfile << endl;
    else
      clog << __func__ <<  __PRETTY_FUNCTION__  << " no ancilfile input ..." << endl;
 
    //
    // if no threadcnt val is available in parfile, consider
    // using cpu count? sysconf suffices for linux, solaris, aix:
    // http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
    threadcnt = sysconf( _SC_NPROCESSORS_ONLN );
    if( threadcnt <= 0 )
      threadcnt = 1;
    
    clog << __func__ <<  __PRETTY_FUNCTION__  << "threadcnt = "<<  threadcnt << endl;
    return;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  bool getParFITS(string& infile_name, string& outfile_name, string& template_name, bool* pclobber, bool* pprecheck) {
    clog << __FILE__ << " " << __LINE__ << " " << __PRETTY_FUNCTION__  << endl;
    // C++: this status is needed because Ape uses int status for errors.
    int status = eOK; /* Ape defines this status code. */
    // hon: how many parameters available in parfile and what are their names?
    // dictionary of all possible paramater names?
    string filename;
    char *fquery = 0;
    //char fquery[BUFSIZ]; memset(fquery,0,BUFSIZ);
  
    /// built-in test filenames:
    string sgd_template_name = sgdSFVersion("./input/");
    string sgd_infile_name =  "./input/ahSGD_tlm.fits";
    string sgd_exp_outfile_name = "./expected_output/ahSGD_tlm_extract.fits";
    string sgd_outfile_name = "./output/ahSGD_tlm_extract.fits";

    string hxi_template_name = hxiSFVersion("./input/");
    string hxi_infile_name =  "./input/ahHXI_tlm.fits";
    string hxi_exp_outfile_name = "./expected_output/ahHXI_tlm_extract.fits";
    string hxi_outfile_name = "./output/ahHXI_tlm_extract.fits";
    
    /* Get the template file name parameter. */
    //memset(fquery,0,BUFSIZ);
    //status = ape_trad_query_file_name("templatefile", &fquery);
    status = ape_trad_query_file_name("templatefile", (char**) &fquery);
    if (eOK != status) {
      setStatus(status);
      throw runtime_error(string(__PRETTY_FUNCTION__) + ": problem with templatefile parameter.");
    }
    template_name = filename = fquery; 
    clog << __PRETTY_FUNCTION__ <<  " template: "<< template_name << endl;
    // override if filenames seem suspiciously short (a.fits or a.tpl are 6 chars)
    if( template_name.length() < 6) {
      template_name = hxi_template_name; // or sgd_template_name;
      //template_name = sgd_template_name; // or hxi_template_name;
      clog << __PRETTY_FUNCTION__ <<  " override template: "<< template_name << endl;
    }

    // hon: enable internal simulation test of instrument indicated in templatefile name
    string sclobber = "no", sprecheck = "no";
    bool simu = false;
    if( pclobber ) {
      char bquery = 0;
      //status = ape_trad_query_bool("clobber", &bquery);
      status = ape_trad_query_bool("clobber", &bquery);
      if( eOK == status ) {
        //sclobber = lowerCaseOf(*query); 
        //if( string::npos == sclobber.find("true") && string::npos == sclobber.find("yes") ) // not yes/true
        if( 0 == bquery ) // not yes/true
          *pclobber = false;
        else
          *pclobber = true;
      }
    }
    if( pprecheck ) {
      char bquery = 0;
      //status = ape_trad_query_bool("precheck", &bquery);
      status = ape_trad_query_bool("precheck", &bquery);
      if( eOK == status ) {
        //sprecheck = lowerCaseOf(*query); 
        //if( string::npos == sprecheck.find("true") && string::npos == sprecheck.find("yes") ) // not yes/true
        if( 0 == bquery ) // not yes/true
          *pprecheck = false;
        else
          *pprecheck = true;
      }
    }
    if( pclobber && pprecheck ) {
      if( *pprecheck && *pclobber ) { 
        simu = true; // create input and expected output files from default FITS template
        // hon: for internal built-in test
        // this app. has no input other than a template,
        // create simulated the input file and (expected) output as part of internal test...
        // test fits file create via custom templates
        if( template_name.find("HXI") != string::npos || template_name.find("hxi") != string::npos ) {
          infile_name = hxi_infile_name; template_name = hxi_template_name; outfile_name = hxi_exp_outfile_name;
        }
        else {
          infile_name = sgd_infile_name; template_name = sgd_template_name; outfile_name = sgd_exp_outfile_name;
        }
        clog << __func__ <<  " template: "<< template_name << endl;
        clog << __func__ <<  " input: "<< infile_name << endl;
        clog << __func__ <<  " output: "<< outfile_name << endl;
        return simu;
      }
    }
 

    /* Get the input file name parameter. */
    //memset(fquery,0,BUFSIZ);
    //status = ape_trad_query_file_name("infile", &fquery);
    status = ape_trad_query_file_name("infile", (char**) &fquery);
    if (eOK != status) {
      setStatus(status);
      throw runtime_error(string(__PRETTY_FUNCTION__) + ": problem with infile parameter.");
    }
    infile_name = filename = fquery; 
    clog << __PRETTY_FUNCTION__ <<  " input: "<< infile_name << endl;
    // override if filenames seem suspiciously short (a.fits or a.tpl are 6 chars)
    if( infile_name.length() < 6) {
      //infile_name = hxi_infile_name; // or sgd_infile_name;
      infile_name = sgd_infile_name; // or hxi_infile_name;
      clog << __PRETTY_FUNCTION__ <<  " override input: "<< infile_name << endl;
    }

    //memset(fquery,0,BUFSIZ);
    //status = ape_trad_query_file_name("outfile", &fquery);
    status = ape_trad_query_file_name("outfile", (char**) &fquery);
    if (eOK != status) {
      setStatus(status);
      throw runtime_error(string(__PRETTY_FUNCTION__) + ": problem with outfile parameter.");
    }
    outfile_name = filename = fquery; 
    clog << __PRETTY_FUNCTION__ <<  " output: "<< outfile_name << endl;
    // override if filenames seem suspiciously short (a.fits or a.tpl are 6 chars)
    if( outfile_name.length() < 6) {
      //outfile_name = hxi_outfile_name; // or sgd_outfile_name;
      outfile_name = sgd_outfile_name; // hxi_outfile_name;
      clog << __PRETTY_FUNCTION__ <<  " override output: "<< outfile_name << endl;
    }

    return simu;
  } // getParFITS
} // namespace ahhxisgd

#endif // GETPAR_C
