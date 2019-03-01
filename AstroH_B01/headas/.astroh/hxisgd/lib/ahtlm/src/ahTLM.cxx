#if !defined(AHTLM_C)
#define AHTLM_C(arg) const char arg##AHTLM_rcsId_svnId[] = "$Id: ahTLM.cxx,v 1.12 2012/07/22 18:14:44 dhon Exp $"; 

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
#include <cstring>
#include <cmath>
#include <bitset>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include "ahtlm/ahSlice.h"
#include "ahtlm/ahBits.h"
#include "ahtlm/ahTLM.h"
/*
#include "ahhxi/ahHXItyps.h"
#include "ahsgd/ahSGDtyps.h"
*/

namespace { // private local helpers

  const string cvsId = "$Name: AstroH_B01 $ $Id: ahTLM.cxx,v 1.12 2012/07/22 18:14:44 dhon Exp $";

  const int _HXInumAsicPerLayer = 8; // duplicated from ahHXItyps.h
  const int _SGDnumAsicPerLayer = 208; // duplicated from ahSGDtyps.h

  // according to a comment in getcols.c (~line 700):
  // "if string length is greater than a FITS block (2880 char) then must only read 1 string at a time..."
  // so let's force the issue 
  char _cfitstext_buff[2882];

  int clearFITSstr(string& cfstr) {
    //memset(_cfitstext_buff, 'F', sizeof(_cfitstext_buff)); _cfitstext_buff[2881] = '\0';
    memset(_cfitstext_buff, 0, sizeof(_cfitstext_buff));
    cfstr = _cfitstext_buff;
    return (int) cfstr.length();
  }
 
  // assume offsets were sanity checked with buffer length by TLMbyteVec or ByteTLM
  // invokers of these helpers:

  int extractTLMbits(ahtlm::Bits10& bitflags, ahtlm::Ubyte* rawdata, int byt_offset, int bit_offset) {
    union { ahtlm::Uint16 u16tlm; unsigned char bytlm[2]; };
    bitflags.reset(); memset(&u16tlm, 0, sizeof(u16tlm));
    if( bitflags.count() <= 0 ) return 0; // something is amiss -- bad sim/test telemetry -- do nothing now
    for( int i = 0; i < 2; ++i ) { bytlm[i] = rawdata[byt_offset + i]; }
    // must re-align the returned bitflag item based on any bit offset:
    ahtlm::Bits16 alignme = ahtlm::Bits16(u16tlm);
    // shift the 10 bits from the bit-offset down to the lsb at 0
    for( int i = 0; i < 10; ++i ) {
      bitflags[i] = alignme[bit_offset + i];
    }
    return bitflags.count();
  }

  int extractTLMbits(ahtlm::Bits16& bitflags, ahtlm::Ubyte* rawdata, int byt_offset) {
    union { ahtlm::Uint16 u16tlm; unsigned char bytlm[2]; };
    bitflags.reset(); memset(&u16tlm, 0, sizeof(u16tlm));
    if( bitflags.count() <= 0 ) return 0; // something is amiss -- bad sim/test telemetry -- do nothing now
    for( int i = 0; i < 2; ++i ) { bytlm[i] = rawdata[byt_offset + i]; }
    bitflags = ahtlm::Bits16(u16tlm);
    return (int)bitflags.count();
  }
  
  int extractTLMbits(ahtlm::Bits32& bitflags, ahtlm::Ubyte* rawdata, int byt_offset) {
    union { ahtlm::Uint32 u32tlm; unsigned char bytlm[4]; };
    bitflags.reset(); memset(&u32tlm, 0, sizeof(u32tlm));
    if( bitflags.count() <= 0 ) return 0; // something is amiss -- bad sim/test telemetry -- do nothing now
    for( int i = 0; i < 4; ++i ) { bytlm[i] = rawdata[byt_offset + i]; }
    bitflags = ahtlm::Bits32(u32tlm);
    return (int)bitflags.count();
  }

  int extractTLMbits(ahtlm::Bits64& bitflags, ahtlm::Ubyte* rawdata, int byt_offset) {
    union { ahtlm::Uint64 u64tlm; ahtlm::Uint32 u32tlm[2]; unsigned char bytlm[8]; };
    bitflags.reset(); memset(&u64tlm, 0, sizeof(u32tlm));
    if( bitflags.count() <= 0 ) return 0; // something is amiss -- bad sim/test telemetry -- do nothing now
    for( int i = 0; i < 8; ++i ) { bytlm[i] = rawdata[byt_offset + i]; }
    // need a Bits64(Unit64 and/or Bits64(Uint32[2]) initializor
    // bitflags = ahtlm::Bits64(u64tlm); // later
    ahtlm::Bits32 lsb = ahtlm::Bits32(u32tlm[0]);
    ahtlm::Bits32 msb = ahtlm::Bits32(u32tlm[1]);
    for( int i = 0; i < 8; ++i ) {
      bitflags[i] = lsb[i]; bitflags[8+i] = msb[i]; 
    }
    return (int)bitflags.count();
  }
} // private local helpers

namespace ahtlm {

  /** \addtogroup mod_ahvbslice
   *  @{
   */

  // placeholder/pretend TLM header that places an HXI or SGD nHitAsics count (1-40 or 1-624)
  // into the header at byte offset == 12 (i.e byte idx 12 and 13), and asics id in the
  // next two bytes (offset = 14 -- idx 14 and 15), and then the 32 or 64 bit chan flags

  // append raw data to occurance buffer
  int genTLM4Test(string& tlm_astext, int hitcnt, int actvchan, int tlmIdx) {
    //tlm_astext = "aabbccddeeff"; // example: 6 asics, 2 chan each
    string ascii = "abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; // our TLM raw data range! :)
    for( int asicIdx = 0; asicIdx < hitcnt; ++asicIdx ) {
      char tlm = ascii[asicIdx];
      for( int chanIdx = 0; chanIdx < actvchan; ++chanIdx ) tlm_astext[tlmIdx++] = tlm;
    }
    return (int)tlm_astext.length();
  }
  
  int genTLM4Test(string& tlm_astext, int hitcnt, int actvchan, Bits32& chanbits) {
    int occurance_data_offset = 16 + 4;
    tlm_astext.clear(); tlm_astext.resize(occurance_data_offset + hitcnt*actvchan);
    // the first 12 bytes should contain flags and local-time, but just put text:
    string byte_hdr = "HXI_HXI_HXI\n";
    int offset = 12; 
    for( int i = 0; i < offset; ++i ) tlm_astext[i] = byte_hdr[i];
    // set the asic hit count, asic ids, and the active channel flags:
    union { unsigned short asic; unsigned char abuf[2]; };
    asic = (unsigned short)hitcnt; 
    tlm_astext[offset] = abuf[0]; tlm_astext[1+offset] = abuf[1];

    offset = 14;
    Bits16 asic_ids;
    for( int i = 0; i < hitcnt % _HXInumAsicPerLayer; ++i ) asic_ids.set(i);
    asic = (unsigned short) asic_ids.to_ulong();
    tlm_astext[offset] = abuf[0]; tlm_astext[1+offset] = abuf[1];

    offset = 16;
    chanbits.reset();
    //for( int i = 0; i < actvchan % 32; ++i ) chanbits.set(i);
    for( int i = 0; i < actvchan % 16; ++i ) chanbits.set(i);
    union { unsigned int chanflags ; unsigned char cbuf[4]; };
    chanflags = (unsigned int) chanbits.to_ulong();
    for( int i = 0; i < 4; ++i ) tlm_astext[i+offset] = cbuf[i];
   
    // and append some "raw" data
    return genTLM4Test(tlm_astext, hitcnt, actvchan, occurance_data_offset);
  }

  // this placeholder is less than bogus, due to lack of details about the bit lengths
  // and organization of SGD nhit-asics, and asic Id telemetry elements -- need SGD
  // fancy chart event telemtry schematic comparable to HXI's.
  int genTLM4Test(string& tlm_astext, int hitcnt, int actvchan, Bits64& chanbits) {
    int occurance_data_offset = 16 + 8;
    tlm_astext.clear(); tlm_astext.resize(occurance_data_offset + hitcnt*actvchan);
    string byte_hdr = "SGD_SGD_SGD\n";
    // ? bogus SGD offsets...
    int offset = 12; 
    for( int i = 0; i < offset; ++i ) tlm_astext[i] = byte_hdr[i];
    // set the asic hit count, asic ids, and the active channel flags:
    union { unsigned short asic; char abuf[2]; };
    asic = (unsigned short)hitcnt; 
    tlm_astext[offset] = abuf[0]; tlm_astext[1+offset] = abuf[1];

    offset = 14;
    Bits16 asic_ids; // ? bogus _SGDnumAsicPerLayer value
    //for( int i = 0; i < hitcnt % _SGDnumAsicPerLayer; ++i ) asic_ids.set(i);
    for( int i = 0; i < hitcnt % 16; ++i ) asic_ids.set(i);
    asic = (unsigned short) asic_ids.to_ulong();
    tlm_astext[offset] = abuf[0]; tlm_astext[1+offset] = abuf[1];

    offset = 16;
    chanbits.reset();
    for( int i = 0; i < actvchan % 64; ++i ) chanbits.set(i);
    union { unsigned long long chanflags ; unsigned char cbuf[8]; };
    chanflags = (unsigned int) chanbits.to_ulong();
    for( int i = 0; i < 8; ++i ) tlm_astext[i+offset] = cbuf[i];

    // and append some "raw" data
    return genTLM4Test(tlm_astext, hitcnt, actvchan, occurance_data_offset);
  }


  int checkAsicCount(const string& testInfo) {
    // testInfo should look something like:
    // "Default Occurance test values SGD -- Id: 1, asics: 10, chans: 120"
    string searchme = "asics:";
    size_t slen = searchme.length();
    size_t colon = testInfo.find(searchme);
    if( colon == string::npos ) {
      clog << __PRETTY_FUNCTION__ << " ? No Asic count in Occurance (test gen.) Info: " << testInfo << endl;
      return -1;
    }
    colon += slen;
    size_t comma = testInfo.find_last_of(',');
    string s = testInfo.substr(colon, comma - colon);
    int cnt = atoi(s.c_str());
    return cnt;
  }

  int checkChanCount(const string& testInfo) {
    // testInfo should look something like:
    // "Default Occurance test values SGD -- Id: 1, asics: 10, chans: 120"
    string searchme = "chans:";
    size_t colon = testInfo.find_last_of(searchme);
    if( colon == string::npos ) {
      clog << __PRETTY_FUNCTION__ << " ? No Channel count in Occurance (test gen.) Info: " << testInfo << endl;
      return -1;
    }
    string s = testInfo.substr(1+colon);
    int cnt = atoi(s.c_str());
    return cnt;
  }

void printRawTLM(const ByteTLM& tlm) {
  clog << __func__ << " length of raw telemetry buffer: " << tlm.length << endl; 
  char chbuff[BUFSIZ]; memset(chbuff, 0, sizeof(chbuff));
  memcpy(chbuff, tlm.bufptr, BUFSIZ); // tlm.length);
  string tlmtxt = chbuff;
  if( tlmtxt.length() <=  0 ) return;
  clog << __func__ << tlmtxt << endl;
  /*
  int offset = 0, cnt = tlm.length / 16;
  Bits16 bitflags;
  vector<string> textbuff;
  BitsAny bv;
  for( int excnt = 0; excnt < cnt; ++excnt ) {
    extractTLMbits(bitflags, rawtlm, offset);
    setBitsAny(bv, bitflags);
    bitText(textbuff, bv, (int)bitflags.size(), " raw TLM as 16 bit bitsets: ");
    clog << __func__ << textbuff[0];
  }
  */
  return;
}

int findStopBit(const BitsHXIMaxRaw& bits) {
  int stopIdx = hxi_maxbits - 1;
  do {
    if( bits[stopIdx] ) break;
  } while ( --stopIdx > 0 );
  return stopIdx;
}

int findStopBit(const BitsSGDMaxRaw& bits) {
  int stopIdx = sgd_maxbits - 1;
  do {
    if( bits[stopIdx] ) break;
  } while ( --stopIdx > 0 );
  return stopIdx;
}

/// use private local helper func., but it does not perform sanity check of offset vs buff size ...
int extractTLMbits(Bits10& bitflags, const ByteTLM& raw_asic_data, int byt_offset, int bit_offset) {
  if( ! raw_asic_data.bufptr ) return 0;
  if( byt_offset >= raw_asic_data.length - 2 ) return 0;
  return extractTLMbits(bitflags, raw_asic_data.bufptr, byt_offset, bit_offset);
}

int extractTLMbits(Bits8& bitflags, const ByteTLM& raw_asic_data, int offset) {
  if( ! raw_asic_data.bufptr ) return 0;
  if( offset >= raw_asic_data.length - 1 ) return 0;
  return extractTLMbits(bitflags, raw_asic_data.bufptr, offset);
}

int extractTLMbits(Bits16& bitflags, const ByteTLM& raw_asic_data, int offset) {
  if( ! raw_asic_data.bufptr ) return 0;
  if( offset >= raw_asic_data.length - 2 ) return 0;
  return extractTLMbits(bitflags, raw_asic_data.bufptr, offset);
}

int extractTLMbits(Bits32& bitflags, const ByteTLM& raw_asic_data, int offset) {
  if( ! raw_asic_data.bufptr ) return 0;
  if( offset >= raw_asic_data.length - 4 ) return 0;
  return extractTLMbits(bitflags, raw_asic_data.bufptr, offset);
}

int extractTLMbits(Bits64& bitflags, const ByteTLM& raw_asic_data, int offset) {
  if( ! raw_asic_data.bufptr ) return 0;
  if( offset >= raw_asic_data.length - 8 ) return 0;
  return extractTLMbits(bitflags, raw_asic_data.bufptr, offset);
}

int extractTLMbits(Bits8& bitflags, const TLMbyteVec& raw_asic_data, int offset) {
  if( ! raw_asic_data.data() ) return 0;
  if( offset >= (int)raw_asic_data.size() - 1 ) return 0;
  return extractTLMbits(bitflags, (Ubyte*)raw_asic_data.data(), offset);
}

int extractTLMbits(Bits10& bitflags, const TLMbyteVec& raw_asic_data, int byt_offset, int bit_offset) {
  if( ! raw_asic_data.data() ) return 0;
  if( byt_offset >= (int)raw_asic_data.size() - 2 ) return 0;
  return extractTLMbits(bitflags, (Ubyte*)raw_asic_data.data(), byt_offset, bit_offset);
}

int extractTLMbits(Bits16& bitflags, const TLMbyteVec& raw_asic_data, int offset) {
  if( ! raw_asic_data.data() ) return 0;
  if( offset >= (int)raw_asic_data.size() - 2 ) return 0;
  return extractTLMbits(bitflags, (Ubyte*)raw_asic_data.data(), offset);
}

int extractTLMbits(Bits32& bitflags, const TLMbyteVec& raw_asic_data, int offset) {
  if( ! raw_asic_data.data() ) return 0;
  if( offset >= (int)raw_asic_data.size() - 4 ) return 0;
  return extractTLMbits(bitflags, (Ubyte*)raw_asic_data.data(), offset);
}

int extractTLMbits(Bits24& bitflags, const TLMbyteVec& raw_asic_data, int offset) {
  if( ! raw_asic_data.data() ) return 0;
  if( offset >= (int)raw_asic_data.size() - 8 ) return 0;
  return extractTLMbits(bitflags, (Ubyte*)raw_asic_data.data(), offset);
}

/// this demonstrates how to deal with bitsets > 32
int extractTLMbits(Bits64& bitflags, const TLMbyteVec& raw_asic_data, int offset) {
  Bits32 lsb, msb;
  extractTLMbits(lsb, raw_asic_data, offset);
  extractTLMbits(msb, raw_asic_data, offset);
  for( int i = 0; i < 32; ++i ) {
    bitflags[i] = lsb[i];
    bitflags[32+i] = msb[i];
  }
  return (int)bitflags.size(); 
}

int getActiveChanIds(vector<int>& actv_chanIds, Bits32& bitflags, int rowIdx, int hduIdx) {
  actv_chanIds.clear();
  int sz = (int)bitflags.size();
  for( int i = 0; i <  sz; ++i ) {
    if( bitflags.test(i) ) actv_chanIds.push_back(i + 1); // channel Ids range from 1-32 (rampped from 1-32*40)
  }
  return (int) actv_chanIds.size();
}

/// needs pre-allocated hashmap of (empty) vecs and accociated vec of bitflags
int getAllActiveChans(map<int, vector<int> >& actv_chanIds, vector<Bits32>& bitflags, int rowIdx, int hduIdx) {
  if( actv_chanIds.empty() ) {
    clog<< __PRETTY_FUNCTION__ << " please pre-allocate the hash map with empty vecs ... " << endl;
    return -1;
  }
  int i= 0, cnt= 0;
  map< int, vector<int> >::iterator it;
  for( it = actv_chanIds.begin(); it != actv_chanIds.end(); ++it ) {
    int asic = it->first;
    clog<< __PRETTY_FUNCTION__ << " fetching active chanIds for (remapped) ASIC # " << asic << endl;
    vector<int>& chanIds = it->second;
    Bits32& bits = bitflags[i++];
    cnt += getActiveChanIds(chanIds, bits, rowIdx, hduIdx);
  }
  return cnt;
}

int getActiveChanIds(vector<int>& actv_chanIds, Bits64& bitflags, int rowIdx, int hduIdx) {
  actv_chanIds.clear();
  int sz = (int)bitflags.size();
  for( int i = 0; i <  sz; ++i ) {
    if( bitflags.test(i) ) actv_chanIds.push_back(i + 1);
  }
  return (int) actv_chanIds.size();
}

/// needs pre-allocated hashmap of (empty) vecs and accociated vec of bitflags
int getAllActiveChans(map< int, vector<int> >& actv_chanIds, vector<Bits64>& bitflags, int rowIdx, int hduIdx) {
  if( actv_chanIds.empty() ) {
    clog<< __PRETTY_FUNCTION__ << " please pre-allocate the hash map with empty vecs ... " << endl;
    return -1;
  }
  int i= 0, cnt= 0;
  map< int, vector<int> >::iterator it;
  for( it = actv_chanIds.begin(); it != actv_chanIds.end(); ++it ) {
    int asic = it->first;
    clog<< __PRETTY_FUNCTION__ << " fetching active chanIds for (remapped) ASIC # " << asic << endl;
    vector<int>& chanIds = it->second;
    Bits64& bits = bitflags[i++];
    cnt += getActiveChanIds(chanIds, bits, rowIdx, hduIdx);
  }
  return cnt;
}

//int BitTLM::capacity() { return (int) size(); }
void BitTLM::init(int maxlen, string nam, string fm) { name= nam; format= fm; length= 0; resize(maxlen, 0); }
void BitTLM::setSlice(BitsAny& bv, string nam) { initSlice(bv); length = (int)bv.size(); if( nam.length() > 0 ) name = nam; }
BitTLM::BitTLM(int maxlen, string nam, string fm) : XSlice(maxlen) { init(maxlen, nam, fm); }

//int ByteTLM::capacity() { return (int) size(); }
void ByteTLM::init(int maxlen, string nam, string fm) { name= nam; format= fm; length= 0; initSlice(0, maxlen); }
void ByteTLM::setSlice(TLMbyteVec& bv, string nam) { initSlice(bv); length = (int)bv.size(); if( nam.length() > 0 ) name = nam; }
ByteTLM::ByteTLM(int maxlen, string nam, string fm) : BSlice(maxlen) { init(maxlen, nam, fm); }
ByteTLM::~ByteTLM() { if( length && maxlength ) clear(); }

//int Int16TLM::capacity() { return (int) size(); }
void Int16TLM::init(int maxlen, string nam, string fm) { name= nam; format= fm; length= 0; initSlice(0, maxlen); }
void Int16TLM::setSlice(vector<Uint16>& iv, string nam) { initSlice(iv); length = (int)iv.size(); if( nam.length() > 0 ) name = nam; }
Int16TLM::Int16TLM(int maxlen, string nam, string fm) : ISlice(maxlen) { init(maxlen, nam, fm); }
Int16TLM::~Int16TLM() { if( length && maxlength ) clear(); }

//int Int32TLM::capacity() { return (int) size(); }
void Int32TLM::init(int maxlen, string nam, string fm) { name= nam; format= fm; length= 0; initSlice(0, maxlen); }
void Int32TLM::setSlice(vector<Uint32>& iv, string nam) { initSlice(iv); length = (int)iv.size(); if( nam.length() > 0 ) name = nam; }
Int32TLM::Int32TLM(int maxlen, string nam, string fm) : JSlice(maxlen) { init(maxlen, nam, fm); }
Int32TLM::~Int32TLM() { if( length && maxlength ) clear(); }

//int Int64TLM::capacity() { return (int) size(); }
void Int64TLM::init(int maxlen, string nam, string fm) { name= nam; format= fm; length= 0; initSlice(0, maxlen); }
void Int64TLM::setSlice(vector<Uint64>& iv, string nam) { initSlice(iv); length = (int)iv.size(); if( nam.length() > 0 ) name = nam; }
Int64TLM::Int64TLM(int maxlen, string nam, string fm) : KSlice(maxlen) { init(maxlen, nam, fm); }
Int64TLM::~Int64TLM() { if( length && maxlength ) clear(); }

//int Real64TLM::capacity() { return (int) size(); }
void Real64TLM::init(int maxlen, string nam, string fm) { name= nam; format= fm; length= 0; initSlice(0, maxlen); }
void Real64TLM::setSlice(vector<double>& rv, string nam) { initSlice(rv); length = (int)rv.size(); if( nam.length() > 0 ) name = nam; }
Real64TLM::Real64TLM(int maxlen, string nam, string fm) : DSlice(maxlen) { init(maxlen, nam, fm); }
Real64TLM::~Real64TLM() { if( length && maxlength ) clear(); }

//int TextTLM::capacity() { return 1; }
void TextTLM::init(int maxlen, string nam, string fm) { name= nam; format= fm; length= 0; resize(maxlen, 'a'); }
void TextTLM::setSlice(string& text, string nam) { 
  string maxtext; clearFITSstr(maxtext); // ensure a viable (fits text) max capacity
  maxtext = text; // should not resize capacity
  initSlice(maxtext); 
  length = 1;
  if( nam.length() > 0 ) name = nam;
}
TextTLM::TextTLM(int maxlen, string nm, string fm) : ASlice(maxlen) { init(maxlen, nm, fm); }

  /** @} */

} // namespace ahtlm

#endif

