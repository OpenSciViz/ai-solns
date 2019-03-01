#if !defined(AHTLMIO_H)
#define AHTLMIO_H(arg) const char arg##AHTLMIO_rcsId_svnId[] = "$Id: ahTLMIO.h,v 1.3 2012/07/11 21:16:02 dhon Exp $"; 

#include "ahtlm/ahBits.h"
#include "ahtlm/ahSlice.h"
#include "ahtlm/ahTLM.h"
#include "ahtlm/ahTLMinfoCache.h"

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

#include <map>
#include <string>
#include <vector>

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace ahtlm {

  /** \addtogroup mod_ahtlm
   *  @{
   */
 
  /// these should be moved to ahfits, but are currently implemented here in ahErrHashFITS.cxx
  int setErrHashFITS(std::map< int, string >& errhash);
  string getCFITSIOversion();
  string textOfErr(int statcode);

  /// candidates for ahfit lib, implemented in ahTLMIO.cxx
  int alloc1ColDataOfRow(AhFitsFilePtr ahf, string& col_name, TLMbyteVec& celldata, int colIdx= 0, int rowIdx= 0, int hduIdx= 1);

  int allocAllColDataOfRow(AhFitsFilePtr ahf, vector<string>& col_names, vector<int>& cell_sizes, HashNamedTLMbyteVec& datacell_hash, int rowIdx= 0, int hduIdx= 1);

  int readSlice(AhFitsFilePtr ahf, FFSliceIO& slice, int colIdx= 0, int rowIdx= 0, int hduIdx= 1);

  int readColumn(AhFitsFilePtr ahf, TLMbyteVec& vals, string& col_name, int colIdx= 0, int rowIdx= 0, int hduIdx= 1);

  int readBlob(AhFitsFilePtr ahf, FFBlob& blob, int colIdx= 0, int rowIdx= 0, int hduIdx= 1);

  int readBlobGroup(AhFitsFilePtr ahf, FFBlobGroup& blobs, vector<int>& colIdxVec, int rowIdx= 0, int hduIdx= 1);

  int writeSlice(AhFitsFilePtr ahf, FFSliceIO& slice, int colIdx= 0, int rowIdx= 0, int hduIdx= 1);

  int writeColumn(AhFitsFilePtr ahf, TLMbyteVec& vals, string& col_name, int colIdx= 0, int rowId= 0, int hduIdx= 1);

  int writeBlob(AhFitsFilePtr ahf, FFBlob& blob, int colIdx= 0, int rowIdx= 0, int hduIdx= 1);

  int writeBlobGroup(AhFitsFilePtr ahf, FFBlobGroup& blobs, vector<int>& colIdxVec, int rowIdx= 0, int hduIdx= 1);

  int writeBlobGroup(AhFitsFilePtr ahf, FFBlobGroup& blobs, int rowIdx= 0, int hduIdx= 1);

  int readFITS(AhFitsFilePtr ahf, void* pdata, int bufsize, string& colname, int rowIdx= 0, int hduIdx= 1);

  int writeFITS(AhFitsFilePtr ahf, void* pdata, long long length, string& colname, string& format, int colIdx, int rowIdx= 0, int hduIdx= 1);

  /** @} */

} // namespace ahtlm

#endif
