#if !defined(AHLMINFOCACHE_C)
#define AHTLMINFOCACHE_C(arg) const char arg##AHTLMINFOCACHE_rcsId_svnId[] = "$Id: ahTLMinfoCache.cxx,v 1.9 2012/07/19 21:55:06 dhon Exp $"; 

/** \file Funcs. that fetch HDU and table info. with and without a "cache"
    and related activities. Some of tthe more general funcs. will be refactored
    into ahgen and ahfits.
    \author David B. Hon
    \date May-June 2012
*/

/// all includes needed and local function declarations -- namespace ahhxisgd
/// note some of these funcs. may move into the ahfits or ahgen namespaces
/// and associated libs...
//

#include "ahtlm/ahBits.h"
#include "ahtlm/ahSlice.h"
#include "ahtlm/ahTLM.h"
#include "ahtlm/ahTLMIO.h"
#include "ahtlm/ahTLMinfoCache.h"

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h> // int limits
#include <cstring>

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: ahTLMinfoCache.cxx,v 1.9 2012/07/19 21:55:06 dhon Exp $";

  // private-local namespace of heplers

  // as described in the "fancy" HXI telemetry chart...
  // according to hxi_sff_120418.tpl
  const size_t _maxHXIasic = 40; // SGD has many more...
  const size_t _maxHXIchan = 34; // 32 + 1 ref. + 1 common noise
  // const size_t _maxHXItlm = 8 + _maxHXIasic*(3+_maxHXIchan) + 1; // telemetry blob size as count of shorts
  //const size_t _maxHXIfullTLMbytes = 1120; // count of bytes taken from hxi_sff_120418.tpl
  const size_t _maxHXIfullTLMbytes = 2014; // count of bytes taken from hxi_sff_120418.tpl
  const size_t _maxHXIasicTLMshorts = 1280; // count of shorts taken from hxi_sff_120418.tpl

  // SGD  -- taken from sgd_sff_120418.tpl
  const size_t _maxSGDasic = 624;
  const size_t _maxSGDchan = 64; // but is there a ref and common noise chan? 
  // const size_t _maxSGDfullTLM = 8 + _maxSGDasic*(3+_maxSGDchan) + 1; // telemetry blob size as count of shorts
  const size_t _maxSGDfullTLMbytes = 58656; // count of bytes taken from sgd_sff_120418.tpl
  const size_t _maxSGDasicTLMshorts = 39936; // count of shorts taken from sgd_sff_120418.tpl

  // max byte buffer size for general use:
  //const size_t _maxTLM = imax(_maxSGDfullTLMbytes, sizeof(short) * _maxSGDasicTLMshorts);
  const size_t _maxTLM = 2 * _maxSGDasicTLMshorts;
  const size_t _maxAsic = _maxSGDasic;
  const size_t _maxChan = _maxSGDchan;  

  // consider these reusables
  // but not sure what to do if app. is modifying a file, i.e. input == outfile:
  //AhFitsFilePtr _theInputAhFits = 0;
  //AhFitsFilePtr _theOutputAhFits = 0;

  // note use of static var. here assumes single-threaded app. -- for multi-threaded apps, need mutex or non-static impl.
  // static  ahhxisgd::TLMbyteVec* _theReUsableMaxSlicePtr = 0; 
  //ahtlm::TLMbyteVec* _theReUsableMaxSlicePtr = 0; 

  // note use of static vars. here assume single-threaded app. -- for multi-threaded apps, need mutex or non-static impl.
  // static ahhxisgd::TLMbyteVec* _theReUsableMaxSliceVecPtr = 0;
  //ahhxisgd::TLMbyteVec* _theReUsableMaxSliceVecPtr = 0;
  // static vector< ahhxisgd::TLMbyteVec* > *_theReUsableAsicChanVecPtrs = 0;  
  //vector< ahtlm::TLMbyteVec* > *_theReUsableAsicChanVecPtrs = 0;


} // end private-local namespace of heplers

namespace ahtlm {
   // this is useful for testing and debugging:
   string describeOccurance(const string& instrum, int asic_hits, int actv_chans, int occurId) {
     string describe = "Default Occurance test values ";
     stringstream ss;
     ss << instrum << " -- Id: " << occurId << ", asics: " << asic_hits << ", chans: " << actv_chans << endl;
     describe += ss.str();
     return describe;
   }

  // some of these funcs will be moved / refactored into the ahfits namespace and lib.
  // these should work for input FITS files, not sure about FITS templates?

  //typedef unsigned short AHTLMbits; // use this typedef rather than int or short or bitset for now?

  // some utilities -- may move/refactor elsewhere
  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int getHDUnames(AhFitsFilePtr ahf, vector<string>& extnames, int count) {
    extnames.clear();
    // cfitsio func. to fetch all hdu extension names is tbd ...
    // this is a bit more than a stub, providing built-in default of one EVENT HDU ...
    //
    string extname = ""; // "EVENTS"; /// built-in default for typical x-ray btable, according to james
    int status = 0, hducount = 0;
    fitsfile* fp = ahf->m_cfitsfp;

    if( !fp ) {
     clog << __func__ << "oops need open output (fits) file, abort" << endl;
     return -1;
    }

    if( 0 != fits_get_num_hdus(fp, &hducount, &status) ) {
      // Return the total number of HDUs in the FITS file. This returns the number of completely defined HDUs in the file. 
      // If a new HDU has just been added to the FITS file, then that last HDU will only be counted if it has been closed,
      // or if data has been written to the HDU. The current HDU remains unchanged by this routine. 
      clog <<__func__ << "oops HDU total count unavaliable ?" << endl;
      return -1;
    }

    const char* keyname = "extname";
    char keyval[81]; memset(keyval, 0, sizeof(keyval));
    char comment[81]; memset(comment, 0, sizeof(comment));
    if( 0 != fits_read_key_str(ahf->m_cfitsfp, keyname, keyval, comment, &status) ) {
      clog << __func__ << " keyname: " << keyname << " not found ... " << endl;
      return -2;
    }
    extname = keyval;
    extnames.push_back(extname);
    // if request count > 1 anf hcucount > 1 move to next ... TBD
    return (int)extnames.size(); //hducount;
  }

  // fecth the most relevant info about an hdu -- istrument name may be blank
    
  int getHDUinfo(AhFitsFilePtr ahf, vector<ahHDUinfo>& hdu_info) { return 0; } // all

  // some utilities -- may move/refactor elsewhere
  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int getHDUinfo(AhFitsFilePtr ahf, ahHDUinfo& hdu_info, int hduIdx, bool move) {
    hdu_info.index = hdu_info.count = hdu_info.type = hdu_info.version = hdu_info.rows = 0;
    hdu_info.index = hdu_info.count = hdu_info.type = hdu_info.rows = 0;

    fitsfile* fp = ahf->m_cfitsfp;
    if( ! fp ) { 
      clog << __func__ << "oops need open output (fits) file, abort" << endl;
      return -1;
    }

    int status = 0, hducount = 0, hdunum = 1 + hduIdx;

    if( 0 != fits_get_num_hdus(fp, &hducount, &status) ) {
      // Return the total number of HDUs in the FITS file. This returns the number of completely defined HDUs in the file. 
      // If a new HDU has just been added to the FITS file, then that last HDU will only be counted if it has been closed,
      // or if data has been written to the HDU. The current HDU remains unchanged by this routine. 
      clog <<__func__ << "oops HDU total count unavaliable ?" << endl;
      return -1;
    }

    if( hdunum > hducount ) {
      clog << __func__ << "hum, requested HDU id-number: " << hdunum << " exceeds HDU total count: " << hducount << endl;
    }

    // Return the number of the current HDU (CHDU) in the FITS file (where the primary array = 1).
    // This function returns the HDU number rather than a status value.
    int hducur = fits_get_hdu_num(fp, &hducur);
    if( hducur != hdunum ) {
      if( !move ) {
        clog << __func__ << "oops requested HDU index/number does not match current -- need to allow move (absolute). returning current info ..." << endl;
      }
      else {
        if( 0 != fits_movabs_hdu(fp, hdunum, &hdu_info.type, &status) ) {
          clog << __func__ << "oops absolute move to requested HDU number failed: " << hdunum << endl;
          return -3;
        }
      }
    }
    else { // just get the typecode
      if( 0 != fits_get_hdu_type( fp, &hdu_info.type, &status) ) {
        clog << __func__ << "oops typecode unavaliable ? of requested HDU number: " << hdunum << endl;
        return -4;
      }
    }
     
    hdu_info.index = hdunum - 1;

    switch(hdu_info.type) {
      case IMAGE_HDU: hdu_info.typname = "IMAGE_HDU"; break;
      case ASCII_TBL: hdu_info.typname = "ASCII_TBL"; break;
      case BINARY_TBL: hdu_info.typname = "BINARY_TBL"; break;
      default: break;
    }

    // get extname and optional instrument name via standard HDU keywords
    char buff[BUFSIZ]; memset(buff, 0, sizeof(buff));
    string instrumkey = "instrume", hdunamekey = "extname", rowcntkey = "naxis2";
    memset(buff, 0, sizeof(buff));
    if( 0 != fits_read_keyword(fp,  hdunamekey.c_str(), buff, 0, &status) ) {
      clog << "\n" << __PRETTY_FUNCTION__ << " Failed keyowrd fetch ... key: " << hdunamekey << " ... hdunum: " << hdunum<< endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
    }
    else {
      hdu_info.extname = buff;
    }

    memset(buff, 0, sizeof(buff));
    if( 0 != fits_read_keyword(fp,  instrumkey.c_str(), buff, 0, &status) ) {
      clog << "\n" << __PRETTY_FUNCTION__ << " Failed keyowrd fetch ... key: " << instrumkey << " ... hdunum: " << hdunum<< endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
    }
    else {
      hdu_info.instrum = buff;
    }

    memset(buff, 0, sizeof(buff));
    if( 0 != fits_read_keyword(fp, rowcntkey.c_str(), buff, 0, &status) ) {
      clog << "\n" << __PRETTY_FUNCTION__ << " Failed keyowrd fetch ... key: " << rowcntkey << " ... hdunum: " << hdunum<< endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
    }
    else {
      hdu_info.rows = atoi(buff);
    }

    // ok is this all the info we need?
    return hducount;
  }

  int getHDUinfo(AhFitsFilePtr ahf, ahHDUinfo& hdu_info, string hdu_name, int hdutype, int versnum, bool move) {
    hdu_info.index = hdu_info.count = hdu_info.type = hdu_info.version = hdu_info.rows = 0;
    hdu_info.typname = hdu_info.extname = hdu_info.instrum = "";
    
    fitsfile* fp = ahf->m_cfitsfp;
    if( ! fp ) { 
      clog << __func__ << "oops need open output (fits) file, abort" << endl;
      return -1;
    }
 
    // get extname and optional instrument name via standard HDU keywords
    char buff[BUFSIZ]; memset(buff, 0, sizeof(buff));
    string hdunamekey = "extname" ; 
    memset(buff, 0, sizeof(buff));
    int status = 0;
    if( 0 != fits_read_keyword(fp,  hdunamekey.c_str(), buff, 0, &status) ) {
      clog << "\n" << __PRETTY_FUNCTION__ << " Failed keyowrd fetch ... key: " << hdunamekey << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return -2;
    }
    else {
      hdu_info.extname = buff;
    }
    if( hdu_info.extname.find(hdu_name) == string::npos ) {
      if( !move ) {
        clog << __func__ << "requested HDU extension name: " << hdu_name << " current hdu: " << hdu_info.extname << endl;
        clog << __func__ << "oops requested HDU name does not match current -- need to allow move ? (absolute) ..." << endl;
        return -3;
      }
      if( 0 != fits_movnam_hdu(fp, hdutype, (char*)hdu_name.c_str(), versnum, &status) ) {
        clog << __func__ << "oops move to requested HDU name failed: " << hdu_name << endl;
        return -3;
      }
    }

    int hducur = fits_get_hdu_num(fp, &hducur);
    return getHDUinfo(ahf, hdu_info, hducur-1);
  }
 
  /// fetch instrume name keyword value from (default) primary header of each open file
  int getInstrumNames(vector<string>& instrum_names, const vector<AhFitsFilePtr>& filelist, int hduIdx) {
    instrum_names.clear();
    int status = 0, hdunum = 1 + hduIdx;
    // double check instrument names of infile and outfile?
    char buff[BUFSIZ]; memset(buff, 0, sizeof(buff));
    string key = "instrume";

    int filecnt = (int)filelist.size();
    for( int i = 0; i < filecnt; ++i ) {
      AhFitsFilePtr ahf = filelist[i];
      fitsfile* fp = fitsFileStructPtr(ahf);
      if( ! fp ) { 
        clog << __func__ << "oops need open output (fits) file, abort" << endl;
        return -1;
      }
      memset(buff, 0, sizeof(buff));
      if( 0 != fits_read_keyword(fp,  key.c_str(), buff, 0, &status) ) {
        clog << "\n" << __PRETTY_FUNCTION__ << " Failed keyowrd fetch ... key: " << key << " ... hdunum: " << hdunum<< endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
        instrum_names.push_back(""); // place an empty string in our list
      }
      else {
        instrum_names.push_back(string(buff));
      }
    }
    return (int) instrum_names.size(); 
  }

  // b-table tow-count via naxis2 keyword?
  int getRowCountFromHeader(AhFitsFilePtr ahf, int hduIdx, bool move) {
    int hdunum = 1 + hduIdx;
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename;
    int rowcnt = 0, typecode = 0, status = 0;
    if( 0 == fp || filename.length() <= 0 ) {
      clog <<  "\n" << __func__ << " Failed ... sorry, insufficient file info ... " << endl;
      return -1;
    }
    int current_hdu = fits_get_hdu_num(fp, &current_hdu);
    if( current_hdu != hdunum ) {
      if( move ) {
        clog << __PRETTY_FUNCTION__ << " currnent HDU differs from requested and move indicated ..." <<endl;
        if( 0 != fits_movabs_hdu(fp, hdunum, &typecode, &status) ) {
          clog << __func__ << " Failed to find hdu index: " << hduIdx << " -- hdu #, type: " << hdunum << ", " << typecode << " for " << filename << endl;
          clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
          return -2;
        }
      }
      else {
       clog << __PRETTY_FUNCTION__ << " currnent HDU differs from requested and move NOT indicated ..." <<endl;
       return -1;
      }
    }
    string key = "NAXIS2";
    if( 0 != fits_read_key(fp, TINT, key.c_str(), &rowcnt, 0, &status) ) {
      clog << "\n" << __PRETTY_FUNCTION__ << " Failed ... ... " << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return -2;
    }
    clog << __PRETTY_FUNCTION__ << " :: " << rowcnt << endl;
    return rowcnt;
  }

  long getRowCount(AhFitsFilePtr ahf, int hduIdx) {
    long nrows = 0;
    int status = 0;
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename;
    if( 0 != fits_get_num_rows(fp, &nrows, &status) ) {
      clog << __PRETTY_FUNCTION__ << " unable to get row count from? FITS file: " << filename << endl;
    }
    return nrows;
  }

  int getColCount(AhFitsFilePtr ahf, int rowIdx, int hduIdx) {
    int status = 0, ncols = 0;
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename; 
    if( 0 != fits_get_num_cols(fp, &ncols, &status) ) {
      clog << __PRETTY_FUNCTION__ << " unable to get row count from? FITS file: " << filename << endl;
    }
    return ncols;
  }

  int getColNumberOfName(AhFitsFilePtr ahf, const string& col_name, int rowIdx, int hduIdx) {
    int colnum = 0, status = 0;
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename; 
    // cut-n-pasted from ahfits: 
    // Find the column number associated with the given column name.
    if (0 != fits_get_colnum(fp, CASEINSEN, (char*)col_name.c_str(), &colnum, &status)) {
      clog << __PRETTY_FUNCTION__ << " could not get colum # of: " + col_name << " from " << filename << endl;
      throw std::runtime_error(string(__PRETTY_FUNCTION__) + "could not get column number of: " + col_name);
    }
    clog << __PRETTY_FUNCTION__ << " colum # of: " << col_name << " == " << colnum << endl;

    return colnum;
  }

  int getColLengthAndType(AhFitsFilePtr ahf, int& typecode, int colIdx, int rowIdx, int hduIdx) {
    //_theInputAhFits = ahf;
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename; 
    // according to the cfitsio manual:
    // The negative of the data type code value is returned if it is a variable length array column.
    int status = 0;
    int colnum = 1+colIdx;
    long repeat= 0, width= 0;  
    typecode = 0;

    if ( 0 != fits_get_coltype(fp, colnum, &typecode, &repeat, &width, &status) ) {
      clog << __PRETTY_FUNCTION__ << " column data info fetch error from fits_get_coltype?" << endl;
      throw std::runtime_error(string(__PRETTY_FUNCTION__) + " could not get column name and size info: " + filename);
    }
    if( typecode < 0 ) {
      clog << __PRETTY_FUNCTION__ << colIdx << " Variable length column data info (typecode, repeat, width): " << typecode << ", " << repeat << ", " << width << endl;
    }
    else {
      clog << __PRETTY_FUNCTION__ << colIdx << " Fixed length column data info (typecode, repeat, width): " << typecode << ", " << repeat << ", " << width << endl;
    }
    // assuming width is always in units of bytes, this should work for all typecodes with exception of acii text:
    int length = repeat * sizeof(char) / width;
    return length;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int getAllColNamesAndNums(AhFitsFilePtr ahf, vector<string>& col_names, map<string, int>& colnamenum_hash, int hduIdx) {
    //_theInputAhFits = ahf;
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename;
    /// use a wild card iteration to fetch all column names
    char wild[] = "*"; 
    char colname[81]; memset(colname, 0, sizeof(colname));
    int status = 0;
    colnamenum_hash.clear();
    do { // this loop should terminate on COL_NOT_FOUND
      int colnum = 0; memset(colname, 0, sizeof(colname));
      if( 0 != fits_get_colname(fp, CASEINSEN, wild, colname, &colnum, &status) ) {
        clog << __PRETTY_FUNCTION__ << " wildcard column name fetch error from fits_get_colname? file: " << filename << endl;
        if( status == COL_NOT_FOUND ) break;
      }
      string col_name = colname;
      col_names.push_back(col_name);
      colnamenum_hash[col_name] = colnum; // should this hash store column number (>= 1) or index?
    } while( status == 0 );

    int colcnt = (int) col_names.size();
    clog << __PRETTY_FUNCTION__ << " wildcard column name fetch returned column count: " << colcnt << endl;
    return colcnt;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int getDataColNamesAndSizes(AhFitsFilePtr ahf, vector<string>& col_names, map<string, int>& colnamesize_hash, int rowIdx, int hduIdx) {
    // use cfitsio funs to get the number of data columns in the row (and check their datatype), also get
    // the column names and and size/length of the array content of each cell (which is variable, but maxed to 1280 for HXI)
    //_theInputAhFits = ahf;
    //fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename;
    clog << __PRETTY_FUNCTION__ << " from: " << filename << endl;
    // get all the column names then identify which ones contain the data "blobs"
    map<string, int> allcol_name_num;
    vector< string > allcol_names;
    map< string, int > allcol_name_nums;
    int allcolcnt = getAllColNamesAndNums(ahf, allcol_names, allcol_name_nums);
    //
    // now deal with the data columns somehow, that is, look at all the columns and 
    // select data columns for return list (presumabley by datatype and of course the variable length cells?):
    int typecode = 0, length = 0;
    for( int cIdx = 0; cIdx < allcolcnt; ++cIdx ) {
      /// calculate length from repeat and width -- width is byte count ie sizeof(datatype)
      length = getColLengthAndType(ahf, typecode, cIdx, rowIdx);
      clog << __PRETTY_FUNCTION__ << cIdx << " column data info (typecode, length): " << typecode << ", " << length << endl;
      if( typecode != TSTRING && typecode < 0 ) { 
        // assume any non-text variable length column is data (at least for internal testing)
        col_names.push_back(allcol_names[cIdx]);
        colnamesize_hash[col_names[cIdx]] = length;
      }
    }
    return (int)col_names.size();
  }


  /// check if columns already exist -- if so this should issue a confirmation log and return ...
  /// otherwise, create any columns that may be missing and indicate what is what in confirmation log?
  /// \callgraph
  int setAllColInfo(AhFitsFilePtr ahf, vector<string>& col_names, map<string, int>& colnum_hash, map<string, string>& coltype_hash, int hduIdx) {
    // this needs more work...
    // if the vector and hash maps provided are empty, this should try to fetch and set content from pre-opened fits file?
    // but if not empty, this should cross-check fits file with vector/map content and reconcile them in some clever fashion... 
    string& filename = ahf->m_filename;
    int colcnt = (int)col_names.size();
    for( int colIdx = 0; colIdx < colcnt; ++colIdx ) {
      int typecode = 0;
      string colname = col_names[colIdx];
      try {
        int colsz = getColLengthAndType(ahf, typecode, colIdx, 0, hduIdx);  
        clog << __PRETTY_FUNCTION__ << " " << colIdx << ".) found column name: " << colname << " length: " << colsz << endl;
      }
      catch (...) {
        clog << __PRETTY_FUNCTION__ << " column not found, name: " << colname << endl;
        fitsfile* fp = ahf->m_cfitsfp;
        int status= 0, colnum= 1 + colIdx;
        clog << __PRETTY_FUNCTION__ << " create column # column name: " + colname << " at " << colnum << endl;
        string tform = "1PB"; // default variable length byte array of unknow max-length
        if( coltype_hash.find(colname) != coltype_hash.end() ) tform = coltype_hash[colname];
        if ( 0 != fits_insert_col(fp, colnum, (char*)colname.c_str(), (char*)tform.c_str(), &status)) {
          clog << __PRETTY_FUNCTION__ << " could not insert new column name: " + colname << " at " << colnum << endl;
          throw std::runtime_error(string(__PRETTY_FUNCTION__) + " error inserting new column " + colname + " of: " + filename);
        }
      }
    }
 
    return 0;
  } // setAllColInfo

  /// these are useful for setting-up test env. and caching files and FITS templates 
  int creatIOpaths(vector< string >& paths) {
    if( paths.empty() ) {
      paths.push_back("./cache"); paths.push_back("./input");
      paths.push_back("./output"); paths.push_back("./expected_output");
    }

    int np = (int)paths.size();
    mode_t m= 0775, m0 = umask(0); // preserve orig. umask...
    for( int i = 0; i  < np; ++i ) {
      string& filename = paths[i];
      clog << __PRETTY_FUNCTION__ << " :: " << filename << endl;
      mkdir(filename.c_str(), m);
    }
    umask(m0);
    return np;
  }

  int creatIOpaths() {
    vector< string > paths;
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
  
} // end namespace ahtlm
#endif // AHTLMINFOCACHE_C

