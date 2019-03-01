#if !defined(AHTLM_H)
#define AHTLM_H(arg) const char arg##AHTLM_rcsId_svnId[] = "$Id: ahTLM.h,v 1.16 2012/07/22 18:14:44 dhon Exp $"; 

// according to the HXI and SGD SFF templates, we need these

/// using CFITSIO conventions:
/// X (bits), B (bytes), I (shorts), J (ints), K (longlongs), D (double)

/// note that according to page 181-2 of the CFITSIO doc:
/// http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio.pdf
/// on-line:
/// http://heasarc.gsfc.nasa.gov/docs/software/fitsio/c/c_user/node123.html
/// 'X' indicate a datatype of bits.

/// but according to the on-line quickfits guide:
/// http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/quick.pdf
/// on-line:
/// http://heasarc.gsfc.nasa.gov/docs/software/fitsio/quick/node12.html
/// 'X' indicates a datatype of complex.

/// make use of c/c++ stdlib items:
#include <cstdlib>
#include <cmath>
#include <bitset>
#include <vector>
#include <sstream>
#include <string>
#include <valarray>

#include "ahtlm/ahBits.h"
#include "ahtlm/ahSlice.h"

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

namespace ahtlm {

  /** \addtogroup mod_ahtlm
   *  @{
   */

// need to reationalize these const throughout source code...
const size_t HXI_Occurance_HeaderSizeInBytes = 2 + 4 + 4 + 4;
const size_t SGD_Occurance_HeaderSizeInBytes = 2 + 4 + 4 + 4; // placeholder

const size_t _maxSGD_TLMbytes = 58656;
const size_t _maxSGD_TLMbits  = 8 * _maxSGD_TLMbytes;
//const size_t _maxHXI_TLMbytes = 1120; 
const size_t _maxHXI_TLMbytes = 2014;
const size_t _maxHXI_TLMbits  = 8 * _maxHXI_TLMbytes;

// this provides a data() buffer pointer func:
typedef basic_string<Ubyte> TLMbyteVec;

// these do not provide the "data()" buff pointer func., 
// but presumably the address of the first element suffices?
// valarray presumably allows a cast to its first element pointer
// equiv. of data() -- TLMu16Vec vec; short* bufptr = &vec[0]; ... etc.
typedef valarray<Uint16> TLMu16Vec;
typedef valarray<Uint32> TLMu32Vec;
typedef valarray<Uint64> TLMu64Vec;

typedef map< int, TLMbyteVec > HashTLMbyteVec;
typedef map< string, TLMbyteVec > HashNamedTLMbyteVec;
typedef map< int, TLMbyteVec* > HashTLMbyteVecPtr; 
typedef map< string, TLMbyteVec* > HashNamedTLMbyteVecPtr;


// each hxi asic event telemtry blob/slice has a (1+2)*16 == 48bit header :
typedef Bits48 BitsHXIeventTLMhdr;

// each sgd asic event telemtry blob/slice has a (2+4)*16 == 96bit ? header :
typedef Bits96 BitsSGDeventTLMhdr;

// hxi max raw telemetry column:
//const int hxi_maxbits = 8*1120;
const int hxi_maxbits = 8*2014;
typedef std::bitset<hxi_maxbits> BitsHXIMaxRaw;

// sgd max raw telemetry column:
const int sgd_maxbits = 8*58656;
typedef std::bitset<sgd_maxbits> BitsSGDMaxRaw;

/// usefull structs to help shorten potentially long function param arg lists
/// need to consider blob-slice vs. full-blobs vs. sets of blobs, etc...
/// again, these may move to ahfits namespace
typedef struct { 
  string col_name;
  vector<int> cell_sizelist; // note these are the actual size/lengths of interest
  // data vecs here may be reused so are pre-alloc'd once to max size..
  TLMbyteVec celldata;
  // ? or:
  // TLMbyteVec* celldata;
} FFBlob;

/// (something like) this has moved to ahfits?
/// this is a (hopefully simple) way to represent the FFBlob typedef for use with cfitsio funcs:
/// where the Ubyte* == FFBlob.celdata.data() i.e. use the vector::data() func for pointer
/// int* (and cast to void*) and the the FFBlob::cell_sizelist::size() to provide
/// the number of variable length arrays embedded in celldata, with the i'th variable 
/// length array's size/length/elem-count provided by FFBlob::cell_sizelist[i]
//
// typedef struct { int length; Ubyte* dataptr; } FFSliceIO; // c or c++
// this may no longer be nedded -- see nrew slice types in ahSlice.h ...
struct FFSliceIO { int length; Ubyte* dataptr; inline FFSliceIO(int len= 0, Ubyte* dat=0) { length= len; dataptr= dat; } }; // c++
typedef std::vector< FFSliceIO > FFBlobIO;
typedef std::pair< int, Ubyte* > FFSlice;

typedef struct { 
  vector<string> col_names; // should be congruent with hash named int vec keys
  vector<int> cell_sizes; // note these are the actual size/lengths of interest
  // data vecs here may be reused so are pre-alloc'd once to max size
  // HashTLMVec datacell_hash; 
  HashNamedTLMbyteVec datacell_hash;
  // ? or:
  // HashNamedTLMbyteVecPtr datacell_hash;
} FFBlobGroup;

/// these are specific to the HXI and SGD ASIC channel context:
/// establish a convention for asic/channel slices: index < 0 (e.g. -1) implies ALL asics/channels Ids
/// otherwise index >= 0 implies the specific slice (start) element, and cnts indicate size of slice
/// i.e. slice == [asic_index : asic_index+ascic_cnt-1][chan_index : chan_index+chan_cnt-1] array access,
/// where array elems. are ASIC-Id 1 to 40 and ASIC-Channel-Id 1 to 32 (for HXI)
typedef struct { string describe; int asic_cnt; int chan_cnt; } BlobInfo;
typedef struct { BlobInfo blob; int asic_index; int chan_index; } BlobSlice;

/// all bitset typdfs and related bitset funcs refactored into ahTLM.h (or ahcTLM.h)
//typedef struct { Bits16 msb; Bits16 lsb; } StartTimeFlagB16;

//alt. live-time nhitasic buff -- a 24 bit time int and an 8 bit int count val: 
//typedef struct { char livetime[3] ; char nHitASICs; } LiveTimeAndNHitASICs; 

//
///////////////////////// simple test telemetry pattern
int genTLM4Test(string& tlm_astext, int hitcnt, int actvchan, int tlmIdx);
// max for HXI: 40, 32
int genTLM4Test(string& tlm_astext, int hitcnt, int actvchan, Bits32& chanflags);
// max for SGD: 624, 64
int genTLM4Test(string& tlm_astext, int hitcnt, int actvchan, Bits64& chanflags);
//

//
///////////////////////// instrument telemetry bitset specialization funcs
//

// HXI fancy chart indicates this implicit asic hit buffer size in bitss:
// CM bit + ref-chan + active-chans + noise-chan + stop bit
// SGD should be simliar ...
// presumably the 32 or 64 bit active channel flag tlm has been inspected to
// provide the int finc-arg-param:
inline int singleAsicBuffBitCount(int actv_chan_cnt) { int bits = 1 + 10 + 10*actv_chan_cnt + 10 + 1; return bits; }

/// start at the most-sig. bit and step in direction of least-sig.
/// to find first non-0 (stop) bit, and return its index
/// if no stop bit is found, this should return -1.
int findStopBit(const BitsHXIMaxRaw& bits);
int findStopBit(const BitsSGDMaxRaw& bits);

int checkAsicCount(const string& testInfo);
int checkChanCount(const string& testInfo);

// may use these or the ByteTLM versions below, depending on use-case:
int extractTLMbits(Bits1& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits2& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits3& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits4& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits5& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits6& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits10& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits8& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0);
int extractTLMbits(Bits16& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0);
int extractTLMbits(Bits24& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0);
int extractTLMbits(Bits32& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0);
int extractTLMbits(Bits64& bitflags, const TLMbyteVec& raw_asic_data, int offset = 0);

// if no real telemetry is forthcoming anytime soon, need to develop more
// realistic telemetry simulation/test-data generation features
// the extraction logic should then be tested with telemetry produced by:
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits1& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits2& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits3& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits4& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits5& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits6& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits10& bitflags, int offset = 0, int bit_offset = 0);
int emdedTLMbits(TLMbyteVec& raw_asic_data, const Bits16& bitflags, int offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits32& bitflags, int offset = 0);
int embedTLMbits(TLMbyteVec& raw_asic_data, const Bits64& bitflags, int offset = 0);

/// HXI active chan flags -- examines bitflag to set number of active chan Ids int vec. and returns count
int getActiveChanIds(vector<int>& actvChanIds, Bits32& bitflags, int rowIdx= 0, int hduIdx= 1);
/// HXI all occurance active chan Ids -- hashmap key should be remapped asic Id #
int getAllActiveChanIds(map< int, vector<int> >& actv_chanIds, vector<Bits32>& bitflags, int rowIdx= 0, int hduIdx= 1);

/// SGD active chan flags -- etc.
int getActiveChansIds(vector<int>& actv_chanIds, Bits64& bitflags, int rowIdx= 0, int hduIdx= 1);
/// SGD all occurance active chan Ids -- hashmap key should be remapped asic Id #
int getAllActiveChans(map< int, vector<int> >& actv_chanIds, vector<Bits64>& bitflags, int rowIdx= 0, int hduIdx= 1);

struct BitTLM : XSlice {
  string name; string format; int length; // length <= capacity
  inline int capacity() { return (int) size(); }
  void* voidPtr(); // TBD (need XSlice (ie BitAny) to BSlice func. to stuff bits into bytes and fetch void*)  
  void init(int len= 2, string nam= "bittlm", string fm= "2X");
  void setSlice(BitsAny& bv, string nam="");
  BitTLM(int len= 2, string nam= "bittlm", string fm= "2X");
};

struct ByteTLM : BSlice {
  string name; string format; int length; // length <= capacity
  inline int capacity() { return (int) size(); }
  inline void* voidPtr() { return (void*) bufptr; } 
  void init(int len= -40, string nam= "bytetlm", string fm= "1PB(40)");
  void setSlice(TLMbyteVec& bv, string nam="");
  ByteTLM(int len= -40, string nam= "bytetlm", string fm= "1PB(40)");
  ~ByteTLM();
};

// these are essential for 10-bit adc, 32 and 64 bit chan flags, general 16bit aligned telemetry items
// bitsets that are not word aligned need two offsets to properly find them in the telemetry buffer
int extractTLMbits(Bits1& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits2& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits3& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits4& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits5& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits6& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits10& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0);
int extractTLMbits(Bits31& bitflags, const ByteTLM& raw_asic_data, int offset = 0, int bit_offset = 0); // bit offset 0 or 1

// byte/word aligned bitsets that appear in the telemetry buffer and asci-channel sub-buffers 
// should only need the byte offset,
int extractTLMbits(Bits8& bitflags, const ByteTLM& raw_asic_data, int offset = 0);
int extractTLMbits(Bits16& bitflags, const ByteTLM& raw_asic_data, int offset = 0);
int extractTLMbits(Bits24& bitflags, const ByteTLM& raw_asic_data, int offset = 0);
int extractTLMbits(Bits32& bitflags, const ByteTLM& raw_asic_data, int offset = 0);
int extractTLMbits(Bits64& bitflags, const ByteTLM& raw_asic_data, int offset = 0);

// for test data generation:
int embedTLMbits(ByteTLM& raw_asic_data, const Bits1& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits2& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits3& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits4& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits5& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits6& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits10& bitflags, int offset = 0, int bit_offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits31& bitflags, int offset = 0, int bit_offset = 0);

int embedTLMbits(ByteTLM& raw_asic_data, const Bits8& bitflags, int offset = 0);
int emdedTLMbits(ByteTLM& raw_asic_data, const Bits16& bitflags, int offset = 0);
int emdedTLMbits(ByteTLM& raw_asic_data, const Bits24& bitflags, int offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits32& bitflags, int offset = 0);
int embedTLMbits(ByteTLM& raw_asic_data, const Bits64& bitflags, int offset = 0);

// pretty print func(s)
void printRawTLM(const ByteTLM& rawtlm);

struct Int16TLM : ISlice {
  string name; string format; int length; // length <= capacity
  inline int capacity() { return (int) size(); }
  inline void* voidPtr() { return (void*) bufptr; } 
  void init(int len= 1280, string nam= "uint16tlm", string fm= "1PUI(1280)");
  void setSlice(vector<Uint16>& iv, string nam=""); // may optionally set name too
  Int16TLM(int len= 1280, string nam= "uint16tlm", string fm= "1PUI(1280)");
  ~Int16TLM();
};

struct Int32TLM : JSlice {
  string name; string format; int length; // length <= capacity
  inline int capacity() { return (int) size(); }
  inline void* voidPtr() { return (void*) bufptr; } 
  void init(int len= 1, string nam= "uint32tlm", string fm= "1J");
  void setSlice(vector<Uint32>& iv, string nam=""); // may optionally set name too
  Int32TLM(int len= 1, string nam= "uint32tlm", string fm= "1J");
  ~Int32TLM();
};

struct Int64TLM : KSlice {
  string name; string format; int length; // length <= capacity
  inline int capacity() { return (int) size(); }
  inline void* voidPtr() { return (void*) bufptr; } 
  void init(int len= 40, string nam= "uint64tlm", string fm= "1PUK(40)");
  void setSlice(vector<Uint64>& iv, string nam=""); // may optionally set name too
  Int64TLM(int len= 40, string nam= "uint64tlm", string fm= "1PUK(40)");
  ~Int64TLM();
};

struct Real64TLM : DSlice {
  string name; string format; int length; // length <= capacity
  inline int capacity() { return (int) size(); }
  inline void* voidPtr() { return (void*) bufptr; } 
  void init(int len= 1, string nam= "real64tlm", string fm= "1D");
  void setSlice(vector<double>& rv, string nam=""); // may optionally set name too
  Real64TLM(int len= 1, string nam= "real64tlm", string fm= "1D");
  ~Real64TLM();
};

struct TextTLM : ASlice {
  string name; string format; int length; // length <= capacity
  inline int capacity() { return (int) size(); }
  inline void* voidPtr() { return (void*) &bufptr; } 
  void init(int len= 128, string nam= "texttlm", string fm= "128a");
  void setSlice(string& text, string nam=""); // may optionally set name too
  //void setSlice(vector<string>& text); TBD
  TextTLM(int len= 128, string nam= "texttlm", string fm= "128a");
};


///////////////// example instances/usages of telemetry slices:
//
inline TextTLM* allocTextTLM(string name= "event_description", int cnt= 1) { 
   TextTLM atxttlm(128, "event_description"); // or
   TextTLM btxttlm(128, "event_description", "128a"); // or
   TextTLM* txttlm_ptr = new TextTLM[cnt]; 
   for( int i= 0; i<cnt; ++i ) {
     // append i to name
     stringstream ss; ss << name << "_" << i << ends;
     txttlm_ptr[i].name = ss.str(); 
   } 
   return txttlm_ptr;
 }

inline Real64TLM* allocReal64TLM(string name= "TIME", int cnt= 1) {
  Real64TLM time_tlm(1, name);
  Real64TLM stime_tlm(1, "S_TIME"); // or
  Real64TLM* r64tlm_ptr = new Real64TLM[cnt]; 
  for( int i= 0; i<cnt; ++i ) {
    // append i to name
    stringstream ss; ss << name << "_" << i << ends;
    r64tlm_ptr[i].name = ss.str(); 
  } 
  return r64tlm_ptr;
}

//ByteTLM pkthd(20, "Packet_Header", "20B");

//Int32TLM ti(1, "TI");
//Int32TLM oid(1, "Occurence_ID");
//Int32TLM loctime(1, "Local_Time");

//BitTLM seuflag(5, "string FLAG_SEU", "5X");
//BitTLM lenchkflag(5, "FLAG_LengthCheck", "5X");

//BitTLM trigpatflag(8, "FLAG_TriggerPattern", "8X");
//BitTLM trigflag(8, "FLAG_Trigger", "8X"); 

//BitTLM hitpatflag(2, "FLAG_HitPattern");
//BitTLM fastbgoflag(2, "FLAG_FastBGO");

//Int32TLM livtime(1, "LiveTime");

//ByteTLM asic_cnt(1, "nHitASICs", "1B");
//ByteTLM asic_raw(2014, "RAW_ASIC_DATA", "1PB(2014)");

//ByteTLM remap_asic_id(40, "Remapped_ASIC_IDs");
//ByteTLM orig_asic_id(40, "Original_ASIC_IDs");

//Int64TLM asic_hdr(40, "ASIC_Header_Data");
//Int32TLM asic_hdr(40, "ASIC_Header_Data", "1PUJ(40)");

//Int16TLM remap_chan_id(1280, "Remapped_Channel_IDs");
//Int16TLM orig_chan_id(1280, "Original_Channel_IDs");
//BitTLM orig_chan_bitflag(1280, "Original_Channel_IDs");
//Int16TLM raw_adc_pulse(1280, "Raw_ADC_Pulse_Heights");
//Int16TLM cal_adc_pulse(1280, "Calibrated_ADC_Pulse_Heights");

  /** @} */

} // namespace ahtlm

#endif

