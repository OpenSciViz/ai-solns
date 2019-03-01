#if !defined(AHERRHASHFITS_C)
#define AHERRHASHFITS_C const char ahErrHashFITS_rcsId_svnId[] = "$Id: ahErrHashFITS.cxx,v 1.3 2012/06/28 16:31:43 dhon Exp $"; 

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

/// all includes needed and local function declarations -- namespace ahhxisgd
#include <map>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

/* hon:
 * this shell script can be used for the first draft of the hash assignments
 * but some hand editing is required...
#!/bin/csh -f
if ( ! -e cfitsio_errcodes.html ) then 
  wget http://heasarc.gsfc.nasa.gov/fitsio/c/c_user/node124.html -O cfitsio_errcodes.html
endif
set start = `cat -n cfitsio_errcodes.html | grep -i '<pre>' | awk '{print $1}'`
#echo $start
set stop = `cat -n cfitsio_errcodes.html | grep -i '</pre>' | awk '{print $1}'`
#echo $stop
set hcnt = $stop
@ hcnt = $hcnt - 1
set tcnt = $stop
@ tcnt = $tcnt - $start
@ tcnt = $tcnt - 3
echo $hcnt $tcnt
#echo "head -225 cfitsio_ecodes.html | tail  -157 | awk '{print $2, $1, $3}'"
#if ( -r cfitsio_errhash.cc ) rm cfitsio_errhash.cc
head -$hcnt cfitsio_errcodes.html | tail -$tcnt |\
awk '{printf "%s%s%s%s ", "errhash[", $2, "] = \"", $1; for (i=3; i<NF; i++) {printf " %s", $i;}; printf "%s\n", "\";"}' 
*
*/

/// may be refactored into lib ahfits

namespace {
const string cvsId = "$Name: AstroH_B01 $ $Id: ahErrHashFITS.cxx,v 1.3 2012/06/28 16:31:43 dhon Exp $";

std::map< int, string > _theFITSerrhash;

int setErrHashFITS(std::map< int, string >& errhash) {
  errhash.clear();
  errhash[0] = "Ok";
  errhash[101] = "SAME_FILE  input and output files are the";
  errhash[103] = "TOO_MANY_FILES  tried to open too many FITS files at";
  errhash[104] = "FILE_NOT_OPENED  could not open the named";
  errhash[105] = "FILE_NOT_CREATED  could not create the named";
  errhash[106] = "WRITE_ERROR  error writing to FITS";
  errhash[107] = "END_OF_FILE  tried to move past end of";
  errhash[108] = "READ_ERROR  error reading from FITS";
  errhash[110] = "FILE_NOT_CLOSED  could not close the";
  errhash[111] = "ARRAY_TOO_BIG  array dimensions exceed internal";
  errhash[112] = "READONLY_FILE  Cannot write to readonly";
  errhash[113] = "MEMORY_ALLOCATION  Could not allocate";
  errhash[114] = "BAD_FILEPTR  invalid fitsfile";
  errhash[115] = "NULL_INPUT_PTR  NULL input pointer to";
  errhash[116] = "SEEK_ERROR  error seeking position in";
  errhash[121] = "BAD_URL_PREFIX  invalid URL prefix on file";
  errhash[122] = "TOO_MANY_DRIVERS  tried to register too many IO";
  errhash[123] = "DRIVER_INIT_FAILED  driver initialization";
  errhash[124] = "NO_MATCHING_DRIVER  matching driver is not";
  errhash[125] = "URL_PARSE_ERROR  failed to parse input file";
  errhash[126] = "RANGE_PARSE_ERROR  parse error in range";
  errhash[151] = "SHARED_BADARG  bad argument in shared memory";
  errhash[152] = "SHARED_NULPTR  null pointer passed as an";
  errhash[153] = "SHARED_TABFULL  no more free shared memory";
  errhash[154] = "SHARED_NOTINIT  shared memory driver is not";
  errhash[155] = "SHARED_IPCERR  IPC error returned by a system";
  errhash[156] = "SHARED_NOMEM  no memory in shared memory";
  errhash[157] = "SHARED_AGAIN  resource deadlock would";
  errhash[158] = "SHARED_NOFILE  attempt to open/create lock file";
  errhash[159] = "SHARED_NORESIZE  shared memory block cannot be resized at the";
  errhash[201] = "HEADER_NOT_EMPTY  header already contains";
  errhash[202] = "KEY_NO_EXIST  keyword not found in";
  errhash[203] = "KEY_OUT_BOUNDS  keyword record number is out of";
  errhash[204] = "VALUE_UNDEFINED  keyword value field is";
  errhash[205] = "NO_QUOTE  string is missing the closing";
  errhash[206] = "BAD_INDEX_KEY  illegal indexed keyword name (e.g.";
  errhash[207] = "BAD_KEYCHAR  illegal character in keyword name or";
  errhash[208] = "BAD_ORDER  required keywords out of";
  errhash[209] = "NOT_POS_INT  keyword value is not a positive";
  errhash[210] = "NO_END  couldn't find END";
  errhash[211] = "BAD_BITPIX  illegal BITPIX keyword";
  errhash[212] = "BAD_NAXIS  illegal NAXIS keyword";
  errhash[213] = "BAD_NAXES  illegal NAXISn keyword";
  errhash[214] = "BAD_PCOUNT  illegal PCOUNT keyword";
  errhash[215] = "BAD_GCOUNT  illegal GCOUNT keyword";
  errhash[216] = "BAD_TFIELDS  illegal TFIELDS keyword";
  errhash[217] = "NEG_WIDTH  negative table row";
  errhash[218] = "NEG_ROWS  negative number of rows in";
  errhash[219] = "COL_NOT_FOUND  column with this name not found in";
  errhash[220] = "BAD_SIMPLE  illegal value of SIMPLE";
  errhash[221] = "NO_SIMPLE  Primary array doesn't start with";
  errhash[222] = "NO_BITPIX  Second keyword not";
  errhash[223] = "NO_NAXIS  Third keyword not";
  errhash[224] = "NO_NAXES  Couldn't find all the NAXISn";
  errhash[225] = "NO_XTENSION  HDU doesn't start with XTENSION";
  errhash[226] = "NOT_ATABLE  the CHDU is not an ASCII table";
  errhash[227] = "NOT_BTABLE  the CHDU is not a binary table";
  errhash[228] = "NO_PCOUNT  couldn't find PCOUNT";
  errhash[229] = "NO_GCOUNT  couldn't find GCOUNT";
  errhash[230] = "NO_TFIELDS  couldn't find TFIELDS";
  errhash[231] = "NO_TBCOL  couldn't find TBCOLn";
  errhash[232] = "NO_TFORM  couldn't find TFORMn";
  errhash[233] = "NOT_IMAGE  the CHDU is not an IMAGE";
  errhash[234] = "BAD_TBCOL  TBCOLn keyword value &lt; 0 or &gt;";
  errhash[235] = "NOT_TABLE  the CHDU is not a";
  errhash[236] = "COL_TOO_WIDE  column is too wide to fit in";
  errhash[237] = "COL_NOT_UNIQUE  more than 1 column name matches";
  errhash[241] = "BAD_ROW_WIDTH  sum of column widths not =";
  errhash[251] = "UNKNOWN_EXT  unrecognizable FITS extension";
  errhash[252] = "UNKNOWN_REC  unknown record; 1st keyword not SIMPLE or";
  errhash[253] = "END_JUNK  END keyword is not";
  errhash[254] = "BAD_HEADER_FILL  Header fill area contains non-blank";
  errhash[255] = "BAD_DATA_FILL  Illegal data fill bytes (not zero or";
  errhash[261] = "BAD_TFORM  illegal TFORM format";
  errhash[262] = "BAD_TFORM_DTYPE  unrecognizable TFORM data type";
  errhash[263] = "BAD_TDIM  illegal TDIMn keyword";
  errhash[264] = "BAD_HEAP_PTR  invalid BINTABLE heap pointer is out of";
  errhash[301] = "BAD_HDU_NUM  HDU number &lt;";
  errhash[302] = "BAD_COL_NUM  column number &lt; 1 or &gt;";
  errhash[304] = "NEG_FILE_POS  tried to move to negative byte location in";
  errhash[306] = "NEG_BYTES  tried to read or write negative number of";
  errhash[307] = "BAD_ROW_NUM  illegal starting row number in";
  errhash[308] = "BAD_ELEM_NUM  illegal starting element number in";
  errhash[309] = "NOT_ASCII_COL  this is not an ASCII string";
  errhash[310] = "NOT_LOGICAL_COL  this is not a logical data type";
  errhash[311] = "BAD_ATABLE_FORMAT  ASCII table column has wrong";
  errhash[312] = "BAD_BTABLE_FORMAT  Binary table column has wrong";
  errhash[314] = "NO_NULL  null value has not been";
  errhash[317] = "NOT_VARI_LEN  this is not a variable length";
  errhash[320] = "BAD_DIMEN  illegal number of dimensions in";
  errhash[321] = "BAD_PIX_NUM  first pixel number greater than last";
  errhash[322] = "ZERO_SCALE  illegal BSCALE or TSCALn keyword =";
  errhash[323] = "NEG_AXIS  illegal axis length &lt;";
  errhash[340] = "NOT_GROUP_TABLE  Grouping function";
  errhash[341] = "HDU_ALREADY_MEMBER ";
  errhash[342] = "MEMBER_NOT_FOUND ";
  errhash[343] = "GROUP_NOT_FOUND ";
  errhash[344] = "BAD_GROUP_ID ";
  errhash[345] = "TOO_MANY_HDUS_TRACKED ";
  errhash[346] = "HDU_ALREADY_TRACKED ";
  errhash[347] = "BAD_OPTION ";
  errhash[348] = "IDENTICAL_POINTERS ";
  errhash[349] = "BAD_GROUP_ATTACH ";
  errhash[350] = "BAD_GROUP_DETACH ";
  errhash[360] = "NGP_NO_MEMORY  malloc";
  errhash[361] = "NGP_READ_ERR  read error from";
  errhash[362] = "NGP_NUL_PTR  null pointer passed as an null Passing  pointer as a name file template raises this";
  errhash[363] = "NGP_EMPTY_CURLINE  line read seems to be empty (internally)";
  errhash[364] = "NGP_UNREAD_QUEUE_FULL  cannot unread more then 1 line (or twice)";
  errhash[365] = "NGP_INC_NESTING  too deep include file nesting template loop,  includes itself";
  errhash[366] = "NGP_ERR_FOPEN  fopen() failed, cannot open template";
  errhash[367] = "NGP_EOF  end of file encountered and not";
  errhash[368] = "NGP_BAD_ARG  bad arguments passed. Usually parser internal  error. Should not";
  errhash[369] = "NGP_TOKEN_NOT_EXPECT  token not expected";
  errhash[401] = "BAD_I2C  bad int to formatted string";
  errhash[402] = "BAD_F2C  bad float to formatted string";
  errhash[403] = "BAD_INTKEY  can't interpret keyword value as";
  errhash[404] = "BAD_LOGICALKEY  can't interpret keyword value as";
  errhash[405] = "BAD_FLOATKEY  can't interpret keyword value as";
  errhash[406] = "BAD_DOUBLEKEY  can't interpret keyword value as";
  errhash[407] = "BAD_C2I  bad formatted string to int";
  errhash[408] = "BAD_C2F  bad formatted string to float";
  errhash[409] = "BAD_C2D  bad formatted string to double";
  errhash[410] = "BAD_DATATYPE  illegal datatype code";
  errhash[411] = "BAD_DECIM  bad number of decimal places";
  errhash[412] = "NUM_OVERFLOW  overflow during data type";
  errhash[413] = "DATA_COMPRESSION_ERR  error compressing";
  errhash[414] = "DATA_DECOMPRESSION_ERR  error uncompressing";
  errhash[420] = "BAD_DATE  error in date or time";
  errhash[431] = "PARSE_SYNTAX_ERR  syntax error in parser";
  errhash[432] = "PARSE_BAD_TYPE  expression did not evaluate to desired";
  errhash[433] = "PARSE_LRG_VECTOR  vector result too large to return in";
  errhash[434] = "PARSE_NO_OUTPUT  data parser failed not sent an out";
  errhash[435] = "PARSE_BAD_COL  bad data encounter while parsing";
  errhash[436] = "PARSE_BAD_OUTPUT  Output file not of proper";
  errhash[501] = "ANGLE_TOO_BIG  celestial angle too large for";
  errhash[502] = "BAD_WCS_VAL  bad celestial coordinate or pixel";
  errhash[503] = "WCS_ERROR  error in celestial coordinate";
  errhash[504] = "BAD_WCS_PROJ  unsupported type of celestial";
  errhash[505] = "NO_WCS_KEY  celestial coordinate keywords not";
  errhash[506] = "APPROX_WCS_KEY  approximate wcs keyword values were";

  return (int)errhash.size();
}

string getCFITSIOversion() {
  stringstream ss;
  float versnum = fits_get_version(&versnum);
  ss << " CFITSIO version: " << versnum << " " << ends;
  return ss.str();
}

} // private anonymous namespace

namespace ahtlm {
   
string textOfErr(int statcode) {
  stringstream ss;
  int errcnt = 0;
  if( _theFITSerrhash.empty() ) {
    errcnt = setErrHashFITS(_theFITSerrhash);
    //clog << __PRETTY_FUNCTION__ << " initialized FITS error status code text hash: " << errcnt << endl;
  }
  else {
    errcnt = (int)_theFITSerrhash.size();
  }
  if( _theFITSerrhash.find(statcode) == _theFITSerrhash.end() ) {
    ss << " " << statcode << " :: no description available for this status code! errcnt: " << errcnt << ends;
    return ss.str();
  }   
  ss << getCFITSIOversion() << statcode << " :: " << _theFITSerrhash[statcode] << ends;
  return ss.str();
}   

} // namespace ahtlm

#endif
