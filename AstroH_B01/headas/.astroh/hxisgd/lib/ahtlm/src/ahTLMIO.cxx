#if !defined(AHTLMIO_C)
#define AHTLMIO_C(arg) const char arg##AHTLMIO_rcsId_svnId[] = "$Id: ahTLMIO.cxx,v 1.14 2012/07/20 20:58:50 dhon Exp $"; 

/** \file Funcs. that read and write FITS file data and related activities.
    Some of the more general funcs. will be refactored into ahgen and ahfits.
    Others need to move to blobCells.cxx and blobBits.cxx.
    \author David B. Hon
    \date May-June 2012
*/

/// all includes needed and local function declarations -- namespace ahhxisgd
/// note some of these funcs. may move into the ahfits or ahgen namespaces
/// and associated libs...
//
#include "stdint.h" // int limits

#include "ahtlm/ahBits.h"
#include "ahtlm/ahSlice.h"
#include "ahtlm/ahTLM.h"
#include "ahtlm/ahTLMIO.h"
#include "ahtlm/ahTLMinfoCache.h"

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

/** \attention this take directly from the cfitstio api doc:
The following routines write or read data values in the current ASCII or binary table extension.
If a write operation extends beyond the current size of the table, then the number of rows in the
table will automatically be increased and the NAXIS2 keyword value will be updated. Attempts to
read beyond the end of the table will result in an error.

Automatic data type conversion is performed for numerical data types (only) if the data type of the
column (defined by the TFORMn keyword) differs from the data type of the array in the calling routine.
ASCII and binary tables support the following data type values: TSTRING, TBYTE, TSBYTE, TSHORT, TUSHORT,
TINT, TUINT, TLONG, TLONGLONG, TULONG, TFLOAT, or TDOUBLE. Binary tables also support TLOGICAL
(internally mapped to the `char' data type), TCOMPLEX, and TDBLCOMPLEX.

Individual bits in a binary table 'X' or 'B' column may be read/written to/from a *char array by
specifying the TBIT datatype. The *char array will be interpreted as an array of logical TRUE (1)
or FALSE (0) values that correspond to the value of each bit in the FITS 'X' or 'B' column.
Alternatively, the values in a binary table 'X' column may be read/written 8 bits at a time to/from
an array of 8-bit integers by specifying the TBYTE datatype.

Note that within the context of these routines, the TSTRING data type corresponds to a C 'char**' data
type, i.e., a pointer to an array of pointers to an array of characters. This is different from the
keyword reading and writing routines where TSTRING corresponds to a C 'char*' data type, i.e., a single
pointer to an array of characters. When reading strings from a table, the char arrays obviously must
have been allocated long enough to hold the whole FITS table string.

Numerical data values are automatically scaled by the TSCALn and TZEROn keyword values (if they exist).

In the case of binary tables with vector elements, the 'felem' parameter defines the starting element
(beginning with 1, not 0) within the cell (a cell is defined as the intersection of a row and a column
and may contain a single value or a vector of values). The felem parameter is ignored when dealing with
ASCII tables. Similarly, in the case of binary tables the 'nelements' parameter specifies the total
number of vector values to be read or written (continuing on subsequent rows if required) and not the
number of table cells. 

AND (table column info routines, note negative datatype code for variable length arrays!):

Return the data type, vector repeat value, and the width in bytes of a column in an ASCII or binary table.
Allowed values for the data type in ASCII tables are: TSTRING, TSHORT, TLONG, TFLOAT, and TDOUBLE.
Binary tables also support these types: TLOGICAL, TBIT, TBYTE, TCOMPLEX and TDBLCOMPLEX. The negative of
the data type code value is returned if it is a variable length array column. Note that in the case of
a 'J' 32-bit integer binary table column, this routine will return data type = TINT32BIT (which in fact
is equivalent to TLONG). With most current C compilers, a value in a 'J' column has the same size as an
'int' variable, and may not be equivalent to a 'long' variable, which is 64-bits long on an increasing
number of compilers.

The 'repeat' parameter returns the vector repeat count on the binary table TFORMn keyword value. 
(ASCII table columns always have repeat = 1). The 'width' parameter returns the width in bytes of a single
column element (e.g., a '10D' binary table column will have width = 8, an ASCII table 'F12.2' column will
have width = 12, and a binary table'60A' character string column will have width = 60); Note that CFITSIO
supports the local convention for specifying arrays of fixed length strings within a binary table character
column using the syntax TFORM = 'rAw' where 'r' is the total number of characters (= the width of the column)
and 'w' is the width of a unit string within the column. Thus if the column has TFORM = '60A12' then this
means that each row of the table contains 5 12-character substrings within the 60-character field, and thus
in this case this routine will return typecode = TSTRING, repeat = 60, and width = 12. (The TDIMn keyword
may also be used to specify the unit string length; The pair of keywords TFORMn = '60A' and TDIMn = '(12,5)'
would have the same effect as TFORMn = '60A12'). The number of substrings in any binary table character
string field can be calculated by (repeat/width). A null pointer may be given for any of the output parameters
that are not needed.
*/

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: ahTLMIO.cxx,v 1.14 2012/07/20 20:58:50 dhon Exp $";

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
  //ahfits::AhFitsFilePtr _theInputAhFits = 0;
  //ahfits::AhFitsFilePtr _theOutputAhFits = 0;

  // note use of static var. here assumes single-threaded app. -- for multi-threaded apps, need mutex or non-static impl.
  // static  ahhxisgd::TLMbyteVec* _theReUsableMaxSlicePtr = 0; 
  ahtlm::TLMbyteVec* _theReUsableMaxSlicePtr = 0; 

  // note use of static vars. here assume single-threaded app. -- for multi-threaded apps, need mutex or non-static impl.
  // static ahhxisgd::TLMbyteVec* _theReUsableMaxSliceVecPtr = 0;
  //ahhxisgd::TLMbyteVec* _theReUsableMaxSliceVecPtr = 0;
  // static vector< ahhxisgd::TLMbyteVec* > *_theReUsableAsicChanVecPtrs = 0;  
  vector< ahtlm::TLMbyteVec* > *_theReUsableAsicChanVecPtrs = 0;


} // end private-local namespace of heplers

namespace ahtlm {
  // some of these funcs will be moved / refactored into the ahfits namespace and lib.
  // these should work for input FITS files, not sure about FITS templates?

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int alloc1ColDataOfRow(AhFitsFilePtr ahf, string& col_name, TLMbyteVec& celldata, int colIdx, int rowIdx, int hduIdx) {
    // this allocates a (max-sized) read buffer for a variable length data column/cell in a btable row
    // use cfitsio funks to alloc an individual named column's size/length of data array (i.e. a cell in this row)
    // and establish a read buffer allocation for it.
    if( 0 == _theReUsableMaxSlicePtr ) { // perform one-time alloc
      _theReUsableMaxSlicePtr = new TLMbyteVec(_maxTLM, 0);
    }

    // of use for log:
    string& filename = ahf->m_filename;
    int colnum = getColNumberOfName(ahf, col_name, rowIdx);
    /// also set keep track of columne number index
    int colidx = colnum - 1;
    clog << __PRETTY_FUNCTION__ << " FITS file: " << filename << ", " << colidx << " == " << colIdx <<endl;

    int typecode = 0, length = 0;
    // need to check typecode to determine if it is a variable length col. (typecode < 0 for such!)
    length = getColLengthAndType(ahf, typecode, colIdx , rowIdx);
    clog << __PRETTY_FUNCTION__ << " FITS file: " << filename << " indicates column/cell size: " << length << " (assumed) max is " << _maxTLM << endl;
    if( length > (int)_maxTLM ) {
      clog << __PRETTY_FUNCTION__ << " oops, FITS file access indicates column/cell size that exceeds expected max (reset to max?)..." << endl;
      length = _maxTLM;
    }
    celldata = *_theReUsableMaxSlicePtr;
    // note that the (re)used max-size buffer, but only the indicated length return is what will be used for I/O
    return length;
 }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int allocAllColDataOfRow(AhFitsFilePtr ahf, vector<string>& col_names, vector<int>& cell_sizes, HashNamedTLMbyteVec& datacell_hash, int rowIdx, int hduIdx) {
    // while this allocates (max-sized) read buffers for all variable length column/cell data in a row, the specific data size 
    // info is extracted from the opend FITS file cfitsio funcs -- to get the number of data columns in the row (and check their datatype), also get
    // the column names and and size/length of the array content of each cell (which is variable, but maxed to 1280 for HXI)
    // and establish a specific read buffer allocation for the entire row.
    // in theory a non-threaded app can reuse these memory buffs for each individual row in btable:
    // note use of static vars. here assume single-threaded app. -- for multi-threaded apps, need mutex or alt. impl.
    if( 0 == _theReUsableMaxSlicePtr ) { // perform one-time alloc
      _theReUsableMaxSlicePtr = new TLMbyteVec(_maxTLM, 0);
      _theReUsableAsicChanVecPtrs  = new vector<TLMbyteVec*>(1+_maxAsic, 0); // over-alloc for easy indexing
      for( size_t i = 0; i < _maxAsic; ++i ) 
        (*_theReUsableAsicChanVecPtrs)[i] = new TLMbyteVec(_maxChan, 0);
    }

    // of use for log:
    string& filename = ahf->m_filename;

    // i think row and cols are numbered starting at 1?
    col_names.clear(); cell_sizes.clear(); datacell_hash.clear();

    map<string, int> allcol_name_num;
    int colcnt = getAllColNamesAndNums(ahf, col_names, allcol_name_num);
    clog << __PRETTY_FUNCTION__ << " FITS file: " << filename << " indicates column count: " << colcnt << endl;
                              // 
    int typecode = 0, colnum = 0, length = 0;
    // refactored the proper cfitsio func. name & args into convenience (getter) funcs:
    for( int colIdx = 0; colnum < colcnt ; ++colnum ) {  
      string colname = col_names[colIdx];
      colnum = getColNumberOfName(ahf, colname, rowIdx);
      length = getColLengthAndType(ahf, typecode, colIdx , rowIdx);
      col_names.push_back(colname);
      cell_sizes.push_back(length); // keep track of the actual amount of data in this cell, althoug the buffs are max-sized for reuse
      if( length > (int)_maxChan ) { // this is presumably the original telemetry blob
        // note this is maxed, so DO NOT use this vector::size(), use cell_sizes set above
        datacell_hash[col_names[colIdx]] = *_theReUsableMaxSlicePtr;
      }
      else {
        // this is clumsy and inefficient, perhaps the datacell_hash should to vector<>*
        // or just use one mega vector<> for all?
        // this assignmend woud be simpler (legible!):
        // datacell_hash[col_names[colIdx]] = (*_theReUsableAsicChanVecPtrs)[colIdx];
        datacell_hash[col_names[colIdx]] = *((*_theReUsableAsicChanVecPtrs)[colIdx]); 
      }
    }
    return (int) col_names.size();
  } // end allocColDataBuffsOfRow -- may refactor this a bit, i.e. introduce allocCellDataBuff?

  // if rowIdx < 0 read all or index from last row?
  int readSlice(AhFitsFilePtr ahf, FFSliceIO& slice, int colIdx, int rowIdx, int hduIdx) {
    // http://heasarc.gsfc.nasa.gov/docs/software/fitsio/c/c_user/node46.html
    // int fits_read_col(fitsfile *fptr, int datatype, int colnum, LONGLONG firstrow, LONGLONG firstelem,
    // LONGLONG nelements, DTYPE *nulval, DTYPE *array, int *anynul, int *status)

    if( 0 == ahf ) {
      clog << __PRETTY_FUNCTION__ << " no open infile?" << endl;
      return -1;
    }
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename;

    int status = 0;
    int typecode = 0; // need code for variable-length array of shorts
    // this should return the size and set the typecode:
    int length = getColLengthAndType(ahf, typecode, colIdx, rowIdx, hduIdx);
    if( length != slice.length ) {
      clog << __PRETTY_FUNCTION__ << " mismatch in expected data array length: "<< length << " != " << slice.length << filename << endl;
      return -1;
    }
    void* array = slice.dataptr;
    int colnum = 1 + colIdx;
    long long firstrow = 1 + rowIdx;
    long long firstelem = 1;
    short nulval = 0;
    int anynul = 0;

    //if ( 0 != fits_read_col(fp, typecode, colnum, firstrow, firstelem, length, (void*) &nulval, (void*)array,  &anynul, &status) ) {
    if ( 0 != fits_read_col(fp, abs(typecode), colnum, firstrow, firstelem, length, (void*) &nulval, (void*)array,  &anynul, &status) ) {
      clog << __PRETTY_FUNCTION__ << " cfistio column data read error: " << filename << endl;
      throw std::runtime_error(string(__PRETTY_FUNCTION__) + " cfistio column data read error: " + filename);
    }

    return length;
  }

  int readColumn(AhFitsFilePtr ahf, TLMbyteVec& vals, string& col_name, int colIdx, int rowIdx, int hduIdx) {
    // get the column number from the name and call readSlice...
    // still a stub:
    if( 0 == ahf ) {
      clog << __PRETTY_FUNCTION__ << " no open infile?" << endl;
      return -1;
    }
    FFSliceIO slice;
    slice.length = alloc1ColDataOfRow(ahf, col_name, vals, rowIdx, hduIdx);
    slice.dataptr = (Ubyte*) vals.data();
    // invoke readSlice to populate the vector of data in the column cell
    int result = readSlice(ahf, slice, colIdx, rowIdx, hduIdx);
    return result;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int readBlob(AhFitsFilePtr ahf, FFBlob& blob, int colIdx, int rowIdx, int hduIdx) {
    // need to double-check col_name and colnum (1+colIdx)
    clog << __PRETTY_FUNCTION__ << " column name of blob: " << blob.col_name << ", rowIdx: " << rowIdx << endl;
    int result = readColumn(ahf, blob.celldata, blob.col_name, colIdx, rowIdx, hduIdx);
    clog << __PRETTY_FUNCTION__ << " number of vals in " << blob.col_name << " is: " << blob.celldata.size() << endl;
    return result;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int readBlobGroup(AhFitsFilePtr ahf, FFBlobGroup& blobs, vector<int>& colIdxVec, int rowIdx, int hduIdx) {
    // determin the number of data cell/columns in row:
    int colcnt = allocAllColDataOfRow(ahf, blobs.col_names, blobs.cell_sizes, blobs.datacell_hash, rowIdx);
    clog << __PRETTY_FUNCTION__ << " colcnt: " << colcnt << endl;
    size_t sz = blobs.datacell_hash.size();
    int total = 0;
    clog << __PRETTY_FUNCTION__ << " number of blobs in group: " << sz << endl;
    for( size_t i = 0; i < sz; ++i ) {
      string& col_name = blobs.col_names[i];
      TLMbyteVec& vals = blobs.datacell_hash[col_name];
      total += readColumn(ahf, vals, col_name, rowIdx, hduIdx);
    }
    return total;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int writeSlice(AhFitsFilePtr ahf, FFSliceIO& slice, int colIdx, int rowIdx, int hduIdx) {
    // int fits_write_col(fitsfile *fptr, int datatype, int colnum, LONGLONG firstrow,
    // LONGLONG firstelem, LONGLONG nelements, DTYPE *array, > int *status)    // according to the the cfitsio api manual:
    // "In the case of binary tables with vector elements, the 'felem' parameter defines the starting
    // element (beginning with 1, not 0) within the cell (a cell is defined as the intersection of a 
    // row and a column and may contain a single value or a vector of values). The felem parameter is 
    // ignored when dealing with ASCII tables. Similarly, in the case of binary tables the 'nelements' 
    // parameter specifies the total number of vector values to be read or written (continuing on 
    // subsequent rows if required) and not the number of table cells." 
    clog << __PRETTY_FUNCTION__ << " column index: " << colIdx << ", row index: " << rowIdx << endl;
    string& filename = ahf->m_filename;
    int status = 0;
    fitsfile* fp = ahf->m_cfitsfp;
    int colnum = colIdx + 1;
    int rownum = rowIdx + 1;
    long repeat = 0; // one entire blob can be considered one slice
    long width = 0; // total # elements in slice, i.e. length
    long felem = 1; // one entire blob can be considered one slice?
    long long length = (long long)slice.length; // total # elements in slice, i.e. length
    void* data = (void*) slice.dataptr;
    int typecode = TINT; // cfitsio datatype -- not sure if this is just for scalar or also for array of ints?

    // cut-n-pasted from ahfits: 
    // Find out what is in this column so that it may be read/written in its native format.
    if (0 != fits_get_coltype(fp, colnum, &typecode, &repeat, &width, &status)) {
      clog << __PRETTY_FUNCTION__ << " could not get column type for column # " << colnum << endl;
      throw std::runtime_error(string(__PRETTY_FUNCTION__) + "could not get column type for column");
    }
    clog << __PRETTY_FUNCTION__ << " column info column # (typecode, repeat, width)" << colnum << " (" << typecode
         << ", " << repeat << ", " << width << ")" << endl;
    // do cfitsio also cut-n-pasted from ahfits, not sure what the 1 is yet...
    if (0 != fits_write_col(fp, typecode, colnum, rownum, felem, length, data, &status) ) {
      clog << __PRETTY_FUNCTION__ << " error writing column in table of file: " << endl;
      throw std::runtime_error(string(__PRETTY_FUNCTION__) + " error writing column in table of file: " + filename);
    }

    return (int) length;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int writeColumn(AhFitsFilePtr ahf, TLMbyteVec& vals, string& col_name, int colIdx, int rowIdx, int hduIdx) {
    // http://heasarc.gsfc.nasa.gov/docs/software/fitsio/c/c_user/node46.html
    // int fits_write_col(fitsfile *fptr, int datatype, int colnum, LONGLONG firstrow,
    // LONGLONG firstelem, LONGLONG nelements, DTYPE *array, > int *status)    // according to the the cfitsio api manual:
    clog << __PRETTY_FUNCTION__ << " column name of slice: " << col_name << endl;
    int length = (int)vals.size();
    clog << __PRETTY_FUNCTION__ << " of length " << length << " into column: " << col_name << " is: " << endl;
    Ubyte* dataptr = (Ubyte*) vals.data();
    if( length <= 0 || 0 == dataptr ) {
      clog << __PRETTY_FUNCTION__ << " no data?" << endl;
      return -1;
    }
    // std::pair< int, Ubyte*> or struct { int length; Ubyte* dataptr; }
    // if struct, use struct ctor  -- i.e. slice.length = length; slice.dataptr =  dataptr;
    FFSliceIO slice(length, dataptr);

    // param/args require by cfitsio func(s): 
    int status = 0, colnum = 0;
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename;
  
    // either the column does not exist yet so we want to add a new column with this name, or
    // it alreadt exists and we should verify the column name and associated number? 

    // cut-n-pasted from ahfits: 
    // Find the column number associated with the given column name.
    if (0 != fits_get_colnum(fp, CASEINSEN, (char*)col_name.c_str(), &colnum, &status)) {
      clog << __PRETTY_FUNCTION__ << " could not get column # of column name: " + col_name << " at " << colnum << endl;
      colnum = 1 + colIdx;
      clog << __PRETTY_FUNCTION__ << " create column # column name: " + col_name << " at " << colnum << endl;
      string tform = "1PB"; // variable lengt byte array of unknow max-length
      if ( 0 != fits_insert_col(fp, colnum, (char*)col_name.c_str(), (char*)tform.c_str(), &status)) {
        clog << __PRETTY_FUNCTION__ << " could not insert new column name: " + col_name << " at " << colnum << endl;
        throw std::runtime_error(string(__PRETTY_FUNCTION__) + " error inserting new column " + col_name + " of: " + filename);
      }
    }
    colIdx = colnum - 1;
    return writeSlice(ahf, slice, colIdx, rowIdx);
  }

  int writeBlob(AhFitsFilePtr ahf, FFBlob& blob, int colIdx, int rowIdx, int hduIdx) {
    clog << __PRETTY_FUNCTION__ << " column name of blob: " << blob.col_name << endl;
    clog << __PRETTY_FUNCTION__ << " number of vals in " << blob.col_name << " is: " << blob.celldata.size() << endl;
    return writeColumn(ahf, blob.celldata, blob.col_name, colIdx, rowIdx, hduIdx);
  }

  int writeBlobGroup(AhFitsFilePtr ahf, FFBlobGroup& blobs, vector<int>& colIdxVec, int rowIdx, int hduIdx) {
    int sz = (int)blobs.datacell_hash.size();
    int total = 0;
    clog << __PRETTY_FUNCTION__ << " number of blobs in groups: " << sz << endl;
    for( int i = 0; i < sz; ++i ) {
      string& col_name = blobs.col_names[i];
      int colIdx = colIdxVec[i];
      TLMbyteVec& vals = blobs.datacell_hash[col_name];
      total += writeColumn(ahf, vals, col_name, colIdx, rowIdx, hduIdx);
    }
    return total;
  }

  int writeBlobGroup(AhFitsFilePtr ahf, FFBlobGroup& blobs, int rowIdx, int hduIdx) {
    int sz = (int)blobs.datacell_hash.size();
    int total = 0;
    clog << __PRETTY_FUNCTION__ << " number of blobs in groups: " << sz << endl;
    for( int i = 0; i < sz; ++i ) {
      string& col_name = blobs.col_names[i];
      int colIdx = i; // assume column name index in vec. directly associate with col numbers...
      TLMbyteVec& vals = blobs.datacell_hash[col_name];
      total += writeColumn(ahf, vals, col_name, colIdx, rowIdx, hduIdx);
    }
    return total;
  }

  /////////////////////////////// all-in-one read/write low-level FITS I/O
  // some form of this needs to move to libahfits -- refactor other funcs to use this or vice-versa?
  // and shorten the params via a struct -- { void* pdata, long long length, string& colname, string& format } FColOut ?
  int writeFITS(AhFitsFilePtr ahf, void* pdata, long long length, string& colname, string& format, int colIdx, int rowIdx, int hduIdx) {
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename;
    // since the first HDU is the primary (image) header, only use hduIdx id > 1
    string hduname = "events"; // move to this default extname if unable to use hduIdx (<= 0)?
    int status = 0, typecode = 0, hducount = 0, hdunow = 0;
    int hdunum = hduIdx + 1, rownum = rowIdx + 1, colnum = colIdx + 1;
    long repeat = 0, width = 0, felem = 1; // single cell of row, column?
    void* cfits_pdata = pdata;

    if( 0 == cfits_pdata ) {
      clog <<  "\n" << __func__ << " Failed ... sorry, bad data buff pointer ... " << endl;
      return -1;
    }
    if( 0 == fp || filename.length() <= 0 ) {
      clog <<  "\n" << __func__ << " Failed ... sorry, insufficient file info ... " << endl;
      return -1;
    }
    clog << "\n" << __func__ << " file:" << filename << endl;

    if( 0 != fits_get_num_hdus(fp, &hducount, &status) ) {
      clog << __func__ << " Failed ... sorry, unable to get HDU count ... " << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return -2;
    }
    clog << __func__ << " Ok, found HDU count: "<< hducount << " for " << filename << endl;
    hdunow = fits_get_hdu_num(fp, &hdunow);
    clog << __func__ << " Current HDU # "<< hdunow << " for " << filename << endl;

    if( hdunow != hdunum ) {
      if( 0 != fits_movnam_hdu(fp, BINARY_TBL, (char*) hduname.c_str(), 1, &status) ) {
        clog << __func__ << " Failed to find hdu name: " << hduname << ", typecode: " << typecode << " for " << filename << endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      }
      else { // try to move to indicated hdunum or should the hdu be inserted if not found?
        if( 0 != fits_movabs_hdu(fp, hdunum, &typecode, &status) ) {
        clog << __func__ << " Failed to find/move to hdu index: " << hduIdx << " -- hdu #, type: " << hdunum << ", " << typecode << " for " << filename << endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
        return -2;
        }
      }
    }
    if( 0 != fits_get_hdu_type(fp, &typecode, &status) ) {
      clog << __func__ << " Failed to get hdu " << hduname << " typecode for " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
    }
    if( BINARY_TBL != typecode ) {
      clog << __func__ << " Failed ... Hum, this is not a binary table hdu? -- hdu #, type: " << hdunum << ", " << typecode << " for " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return -3;
    }

    hdunum = fits_get_hdu_num(fp, &hdunum);
    clog << "\n" << __func__ << " Ok, found hdu name: " << hduname << " #: " << hdunum << ", B-table typecode: " << typecode << " for " << filename << endl;

    /// presumably the fits file has been opened/created via a template and this is all that we need:
    if( 0 == fits_get_colnum(fp, CASEINSEN, (char*)colname.c_str(), &colnum, &status) ) {
      clog << __PRETTY_FUNCTION__ << " " << colname << " found at column # " << colnum << endl;
      if( 0 != fits_get_coltype(fp, colnum, &typecode, &repeat, &width, &status) ) {
        clog << __PRETTY_FUNCTION__ << " " << colname << " Failed to get type info for column # " << colnum << endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
        return -4;
      }
      clog << __PRETTY_FUNCTION__ << " " << colname << " column info: column # (typecode, repeat, width)" << colnum << " (" << typecode << ", " << repeat << ", " << width << ")" << endl;
      // do cfitsio also cut-n-pasted from ahfits...
      clog << __func__ << " HDU name: " << hduname << " rownum, colname & number: " << rownum << ", " << colname << ", " << colnum << endl;
      clog << __func__ << "typecode, length: " << typecode << ", " << length << " for " << filename << endl;

      // in this case cfitsio casts void* to char** -- this logic should be supported by slice ot TLM struct bufptr access func...
      //if( typecode == TSTRING ) cfits_pdata = &pdata; // in this case cfitsio casts void* to char**

      //if( 0 == fits_write_col(fp, typecode, colnum, rownum, felem, length, cfits_pdata, &status) ) {
      if( 0 == fits_write_col(fp, abs(typecode), colnum, rownum, felem, length, cfits_pdata, &status) ) {
        clog << __PRETTY_FUNCTION__ << " wrote column data length: " << length << ", into file: " << filename << endl;
        //getRowCount(ahf);
        return length;
      } // write on ! it worked ?
      else {
        clog << __PRETTY_FUNCTION__ << " Failed to write column: " << colname << " of length: " << length << " into " << filename << endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      } // write failed ... try to once more ?
    } 

    // if the above fails, try to insert a new column? ...
    clog << __PRETTY_FUNCTION__ << " ? need to insert a new column?: " << colname << " into " << filename << endl;
    colnum = colIdx + 1; // make sure we are using the desired colnum ..
    if( 0 == fits_insert_col(fp, colnum, (char*)colname.c_str(), (char*)format.c_str(), &status) ) {
      clog << __PRETTY_FUNCTION__ << " inserted new column: " + colname << " at " << colnum << endl;
      // and need proper typecode -- could this possibly fail?
      if( 0 != fits_get_coltype(fp, colnum, &typecode, &repeat, &width, &status) ) {
        clog << __PRETTY_FUNCTION__ << " " << colname << " Failed to get type info for column # " << colnum << endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
        return -5;
      }
    } 
    else {
      clog << __PRETTY_FUNCTION__ << " Failed ... could not insert new column: " + colname << " at " << colnum << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return -6;
    }

    // in this case cfitsio casts void* to char** -- this logic should be supported by slice ot TLM struct bufptr access func...
    // if( typecode == TSTRING ) cfits_pdata = &pdata; 

    //if( 0 == fits_write_col(fp, typecode, colnum, rownum, felem, length, cfits_pdata, &status) ) {     
    if( 0 == fits_write_col(fp, abs(typecode), colnum, rownum, felem, length, cfits_pdata, &status) ) {     
      clog << __PRETTY_FUNCTION__ << " wrote column data length: " << length << ", into file: " << filename << endl;
      //getRowCount(ahf);
      return length;
    }
    clog << __PRETTY_FUNCTION__ <<" Failed to write column " << colname << " of length: "<< " into " << filename << endl;
    clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
    //throw std::runtime_error(string(__PRETTY_FUNCTION__) + " error inserting new column " + col_name + " of: " + filename);
    return -7;
  } // writeFITS


  int readFITS(AhFitsFilePtr ahf, void* pdata, int bufsize, string& colname, int rowIdx, int hduIdx) {
    fitsfile* fp = ahf->m_cfitsfp;
    string& filename = ahf->m_filename; 
    string hduname = "events"; // move to this default extname if unable to use hduIdx (<= 0)?
    short nulval = 0;
    int status = 0, typecode = 0, colcount= 0, hducount = 0, hdunow = 0, anynul = 0, colnum = 0;
    int hdunum = hduIdx + 1, rownum = rowIdx + 1;
    long rowcount = 0, repeat = 0, width = 0; // single cell of row, column?
    long long firstelem = 1, firstrow = rownum, llrepeat = 0, llwidth = 0;
 
    if( 0 == fp || filename.length() <= 0 ) {
      clog <<  "\n" << __func__ << " Failed ... sorry, insufficient file info ... " << endl;
      return 0;
    }
    clog << "\n" << __func__ << " file:" << filename << endl;
 
    if( 0 != fits_get_num_hdus(fp, &hducount, &status) ) {
      clog << __func__ << " Failed ... sorry, unable to get HDU count ... " << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return 0;
    }

    clog << __func__ << " Ok, found HDU count: "<< hducount << " for " << filename << endl;
    hdunow = fits_get_hdu_num(fp, &hdunow);
    clog << __func__ << " Current HDU # "<< hdunow << " for " << filename << endl;

    if( hdunow != hdunum ) {
      if( 0 != fits_movnam_hdu(fp, BINARY_TBL, (char*) hduname.c_str(), 1, &status) ) {
        clog << __func__ << " Failed to find hdu name: " << hduname << ", typecode: " << typecode << " for " << filename << endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      }
      else { // try to move to indicated hdunum
        if( 0 != fits_movabs_hdu(fp, hdunum, &typecode, &status) ) {
        clog << __func__ << " Failed to find/move to hdu index: " << hduIdx << " -- hdu #, type: " << hdunum << ", " << typecode << " for " << filename << endl;
        clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
        return -2;
        }
      }
    }
    if( 0 != fits_get_hdu_type(fp, &typecode, &status) ) {
      clog << __func__ << " Failed to get hdu " << hduname << " typecode for " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
    }
    if( BINARY_TBL != typecode ) {
      clog << __func__ << " Failed ... Hum, this is not a binary table hdu? -- hdu #, type: " << hdunum << ", " << typecode << " for " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return 0;
    }

    hdunum = fits_get_hdu_num(fp, &hdunum);
    clog << "\n" << __func__ << " Ok, found hdu name: " << hduname << " #: " << hdunum << ", B-table typecode: " << typecode << " for " << filename << endl;

    int found= 0;
    long axescount[2] = { 0, 0 }; // read the NAXIS1 and NAXIS2 keyword to get table size
    if( 0 != fits_read_keys_lng(fp, "NAXIS", 1, 2, axescount, &found, &status) ) {
      clog << __PRETTY_FUNCTION__ << " unable to get axes count from? FITS file: " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return 0;
    }
    if( 0 != fits_get_num_rows(fp, &rowcount, &status) ) {
      clog << __PRETTY_FUNCTION__ << " unable to get row count from? FITS file: " << filename << endl;
    }
    /// axes_cnt[1] should == rowcount:
    clog << __PRETTY_FUNCTION__ << " got row count from naxis: " << axescount[1] << " and directly via API: " << rowcount << endl;

    /// check validity of file HDU b-table content -- what is the column count and is this colname present, etc. 
    if( 0 != fits_get_num_cols(fp, &colcount, &status) ) {
      clog << __PRETTY_FUNCTION__ << " unable to get column count from? FITS file: " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return 0;
    }

    if( 0 != fits_get_colnum(fp, CASEINSEN, (char*)colname.c_str(), &colnum, &status) ) {
      clog << __PRETTY_FUNCTION__ << " " <<  colname << " could not get colum # from " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return 0;
    }
    clog << __PRETTY_FUNCTION__ << " " <<  colname << " total columns == " << colcount << ", colum # == " << colnum <<  endl;

    if( 0 != fits_get_coltype(fp, colnum, &typecode, &repeat, &width, &status) ) {
      clog << __PRETTY_FUNCTION__ << " " <<  colname << " check column type error from fits_get_coltype?" << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return 0;
    }
    if( typecode < 0 ) {
      clog << __PRETTY_FUNCTION__ << " " <<  colname << " variable length column data info (typecode, repeat, width): " << typecode << ", " << repeat << ", " << width << endl;
    }
    else {
      clog << __PRETTY_FUNCTION__ << " " << colname << " fixed length column data info (typecode, repeat, width): " << typecode << ", " << repeat << ", " << width << endl;
    }

    // FITS acii text columns are peculiar :
    int length = repeat; // some func(repeat, width) ? 
    if( abs(typecode) == TSTRING ) {
      length = (int) (repeat / width); // fixed or variable length text string?
    }
    else if( 0 > typecode ) { // from ahfite readRow:
      // hon: do all these different api func. yield the same info?
      if( 0 != fits_get_coltypell(fp, colnum, &typecode, &llrepeat, &llwidth, &status)) {
        clog << __PRETTY_FUNCTION__ << " unable to get column type elem. info. from? FITS file: " << filename << endl;
        return 0;
      } 
      // hopefully the right way to eval the length of the data vec in the cell/column-element -- but text differs?
      // Read the descriptor of this row to get the number of elements actually contained.
      if( 0 != fits_read_descriptll(fp, colnum, firstrow, &llrepeat, 0, &status) ) {
        clog << __PRETTY_FUNCTION__ << " unable to get column type elem. info. from? FITS file: " << filename << endl;
        return 0;
      }
      length = (int) llrepeat; // or some func(repeat, width) ? 
    }

    clog << __PRETTY_FUNCTION__ << " current buffer size: " << bufsize << ", length of the data vec in the cell/column-element: " << length << endl;

    if( length > bufsize ) { 
      clog << __PRETTY_FUNCTION__ << " may need to resize the buffer? " << endl;
    }

    if( 0 == pdata ) {
      clog << __PRETTY_FUNCTION__ << " need to perform (intial-one-time-max?) buffer allocation? " << endl;
      // perform allocation here on heap or return ...
      return -length; // perhaps return -length as hint for allocation?
    }

    //if( 0 != fits_read_col(fp, typecode, colnum, firstrow, firstelem, length, (void*) &nulval, pdata,  &anynul, &status) ) {
    if( 0 != fits_read_col(fp, abs(typecode), colnum, firstrow, firstelem, length, (void*) &nulval, pdata,  &anynul, &status) ) {
      clog << __PRETTY_FUNCTION__ << " cfistio column data read error: " << filename << endl;
      clog << __PRETTY_FUNCTION__ << textOfErr(status) << endl;
      return 0;
    }
    
    return length;
  } // readFITS

} // end namespace ahtlm
#endif // AHTLM_C

