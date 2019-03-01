#if !defined(AHTLMINFOCACHE_H)
#define AHTLMINFOCACHE_H(arg) const char arg##AHTLMINFOCACHE_rcsId_svnId[] = "$Id: ahTLMinfoCache.h,v 1.4 2012/07/19 21:55:06 dhon Exp $"; 

#include "ahtlm/ahBits.h"
#include "ahtlm/ahSlice.h"
#include "ahtlm/ahTLM.h"
#include "ahtlm/ahTLMIO.h"

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

#include <stdint.h> // int limits

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace ahtlm {

  /** \addtogroup mod_ahtlm
   *  @{
   */
  typedef struct { int index; int count; int type; int version; int rows; string typname; string extname; string instrum; } ahHDUinfo;

  inline fitsfile* fitsFileStructPtr(AhFitsFilePtr ahf) { if( 0 != ahf ) { return ahf->m_cfitsfp; } return 0; }

  string describeOccurance(const string& instrum, int asic_hits, int actv_chans, int occurId);

  int creatIOpaths();
  int creatIOpaths(vector< string >& paths);
  int writeTextFile(const string& content, const string& filename);

  int getHDUnames(AhFitsFilePtr ahf, vector<string>& extnames, int count= 1);

  int getHDUinfo(AhFitsFilePtr ahf, vector<ahHDUinfo>& hdu_info);

  int getHDUinfo(AhFitsFilePtr ahf, ahHDUinfo& hdu_info, int hduIdx= 1, bool move= false);

  int getHDUinfo(AhFitsFilePtr ahf, ahHDUinfo& hdu_info, string hdu_name= "events", int hdutype= BINARY_TBL, int versnum= 0, bool move= true);

  int getInstrumNames(vector<string>& instrum_names, const vector<AhFitsFilePtr>& filelist, int hduIdx= 1);

  int getRowCountFromHeader(AhFitsFilePtr ahf, int hduIdx= 1, bool move= true);

  long getRowCount(AhFitsFilePtr ahf, int hduIdx= 1);

  int getColCount(AhFitsFilePtr ahf, int rowIdx= 0, int hduIdx= 1);

  int getColNumberOfName(AhFitsFilePtr ahf, const string& col_name, int rowIdx= 0, int hduIdx= 1);

  int getColLengthAndType(AhFitsFilePtr ahf, int& typecode, int colIdx= 0, int rowIdx= 0, int hduIdx= 1);

  int getAllColNamesAndNums(AhFitsFilePtr ahf, vector<string>& col_names, map<string, int>& colnamenum_hash, int hduIdx= 1);

  int getDataColNamesAndSizes(AhFitsFilePtr ahf, vector<string>& col_names, map<string, int>& colnamesize_hash, int rowIdx= 0, int hduIdx= 1);

  int setAllColInfo(AhFitsFilePtr ahf, vector<string>& col_names, map<string, int>& colnum_hash, map<string, string>& coltype_hash, int hduIdx= 1);

  /** @} */

} // namespace ahtlm

#endif
