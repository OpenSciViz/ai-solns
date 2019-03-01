#if !defined(AHSLICE_C)
#define AHSLICE_C(arg) const char arg##AHTLM_rcsId_svnId[] = "$Id: ahSlice.cxx,v 1.10 2012/07/12 22:14:16 dhon Exp $"; 

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

#include "ahtlm/ahSlice.h"
#include "ahtlm/ahBits.h"

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: ahSlice.cxx,v 1.10 2012/07/12 22:14:16 dhon Exp $";
}

namespace ahtlm {

  /** \addtogroup mod_ahvbslice
   *  @{
   */

/////////////// various types of telemetry data slices
//
//size_t BSlice::size() { return abs(maxlength); }
void BSlice::initSlice(Ubyte* ptr, int len, bool alloc) {
  bufptr= ptr; heap = alloc; if( len < 0 ) heap = true; // neg. len as a heap indicator (for variable len array/slice)?
  (bufptr || heap) ? maxlength = len : maxlength= 0; 
  if( bufptr == 0 && heap && maxlength != 0 ) { 
    size_t bufsz = size(); bufptr = new Ubyte[bufsz]; //calloc(bufsz, sizeof(Ubyte));
    for( size_t i = 0; i < bufsz; ++i ) bufptr[i] = 0;
  }
}
//void BSlice::initSlice(const TLMbyteVec& data) {
void BSlice::initSlice(const basic_string<Ubyte>& data) {
  initSlice(0, (int) data.size(), true);
  for( size_t i = 0; i < size(); ++i ) bufptr[i] = data[i];
}
void BSlice::clear() {
  //if( bufptr && heap ) delete [] bufptr;
  initSlice(0, 0);
}
BSlice::BSlice(int len, bool alloc) { initSlice(0, len, alloc); }
BSlice::BSlice(Ubyte* ptr, int len, bool alloc) { initSlice(ptr, len, alloc); }
BSlice::~BSlice() { clear(); } // forever alloc may reset heap to false?

//size_t ISlice::size() { return abs(maxlength); }
void ISlice::initSlice(Uint16* ptr, int len, bool alloc) {
  bufptr= ptr; heap= alloc; if( len < 0 ) heap = true; // neg. len as a heap indicator?
  (bufptr || heap) ? maxlength = len : maxlength= 0; 
  if( bufptr == 0 && heap && maxlength != 0 ) { 
    size_t bufsz = size(); bufptr = new Uint16[bufsz]; //calloc(bufsz, sizeof(Uint16));
    for( size_t i = 0; i < bufsz; ++i ) bufptr[i] = 0;
  }
}
void ISlice::initSlice(const vector<Uint16>& data) {
  initSlice(0, (int) data.size(), true);
  for( size_t i = 0; i < size(); ++i ) bufptr[i] = data[i];
}
void ISlice::clear() {
  if( bufptr && heap ) delete [] bufptr;
  initSlice(0, 0);
}
ISlice::ISlice(int len, bool alloc) { initSlice(0, len, alloc); }
ISlice::ISlice(Uint16* ptr, int len, bool alloc) { initSlice(0, len, alloc); }
ISlice::~ISlice() { clear(); } // forever alloc may reset heap to false?

//size_t JSlice::size() { return abs(maxlength); }
void JSlice::initSlice(Uint32* ptr, int len, bool alloc) {
  bufptr= ptr; heap= alloc; if( len < 0 ) heap = true; // neg. len as a heap indicator?
  (bufptr || heap) ? maxlength = len : maxlength= 0; 
  if( bufptr == 0 && heap && maxlength != 0 ) { 
    size_t bufsz = size(); bufptr = new Uint32[bufsz]; //calloc(bufsz, sizeof(Uint16));
    for( size_t i = 0; i < bufsz; ++i ) bufptr[i] = 0;
  }
}
void JSlice::initSlice(const vector<Uint32>& data) {
  initSlice(0, (int) data.size(), true);
  for( size_t i = 0; i < size(); ++i ) bufptr[i] = data[i];
}
void JSlice::clear() {
  if( bufptr && heap ) delete [] bufptr;
  initSlice(0, 0);
}
JSlice::JSlice(int len, bool alloc) { initSlice(0, len, alloc); }
JSlice::JSlice(Uint32* ptr, int len, bool alloc) { initSlice(0, len, alloc); }
JSlice:: ~JSlice() { clear(); } // forever alloc may reset heap to false?

//size_t KSlice::size() { return abs(maxlength); }
void KSlice::initSlice(Uint64* ptr, int len, bool alloc) {
  bufptr= ptr; heap= alloc; if( len < 0 ) heap = true; // neg. len as a heap indicator?
  (bufptr || heap) ? maxlength = len : maxlength= 0; 
  if( bufptr == 0 && heap && maxlength != 0 ) { 
    size_t bufsz = size(); bufptr = new Uint64[bufsz]; //calloc(bufsz, sizeof(Uint64));
    for( size_t i = 0; i < bufsz; ++i ) bufptr[i] = 0;
  }
}
void KSlice::initSlice(const vector<Uint64>& data) {
  initSlice(0, (int) data.size(), true);
  for( size_t i = 0; i < size(); ++i ) bufptr[i] = data[i];
}
void KSlice::clear() {
  if( bufptr && heap ) delete [] bufptr;
  initSlice(0, 0);
}
KSlice::KSlice(int len, bool alloc) { initSlice(0, len, alloc); }
KSlice::KSlice(Uint64* ptr, int len, bool alloc) { initSlice(0, len, alloc); }
KSlice::~KSlice() { clear(); } // forever alloc may reset heap to false?

//size_t DSlice::size() { return abs(maxlength); }
  void DSlice::initSlice(double* ptr, int len, bool alloc) {
  bufptr= ptr; heap= alloc; if( len < 0 ) heap = true; // neg. len as a heap indicator?
  (bufptr || heap) ? maxlength = len : maxlength= 0; 
  if( bufptr == 0 && heap && maxlength != 0 ) { 
    size_t bufsz = size(); bufptr = new double[bufsz]; //calloc(bufsz, sizeof(double));
    for( size_t i = 0; i < bufsz; ++i ) bufptr[i] = 0.0;
  }
}
void DSlice::initSlice(const vector<double>& data) {
  initSlice(0, (int) data.size(), true);
  for( size_t i = 0; i < size(); ++i ) bufptr[i] = data[i];
}
void DSlice::clear() {
  if( bufptr && heap ) delete [] bufptr;
  initSlice(0, 0);
}
DSlice::DSlice(int len, bool alloc) { initSlice(0, len, alloc); }
DSlice::DSlice(double* ptr, int len, bool alloc) { initSlice(0, len, alloc); }
DSlice::~DSlice() { clear(); } // forever alloc may reset heap to false?

// this could simply be a std::string, which provides a length() func. and a  const char* c_str() func.
void ASlice::initSlice(const string& ascitext) { *((std::string*)this) = ascitext; bufptr = c_str(); maxlength = (int) size(); }
ASlice::ASlice(const char* c) : std::string() { heap = false; if( c ) initSlice(c); }
ASlice::ASlice(int len) { heap = false; *((std::string*)this) = std::string(abs(len), 'a'); }

//
//size_t CSlice::size() { return abs(maxlength); }
void CSlice::initSlice(char* ptr, int len, bool alloc) {
  bufptr= ptr; heap= alloc; if( len < 0 ) heap = true; // neg. len as a heap indicator?
  (bufptr || heap) ? maxlength = len : maxlength= 0; 
  if( bufptr == 0 && heap && maxlength != 0 ) { 
    size_t bufsz = size(); bufptr = new char[bufsz]; //calloc(bufsz, sizeof(char));
    for( size_t i = 0; i < bufsz; ++i ) bufptr[i] = 0;
  }
}
void CSlice::initSlice(const vector<char>& data) {
  initSlice(0, (int) data.size(), true);
  for( size_t i = 0; i < size(); ++i ) bufptr[i] = data[i];
}
void CSlice::clear() {
  if( bufptr && heap ) delete [] bufptr;
  initSlice(0, 0);
}
CSlice::CSlice(int len, bool alloc) { initSlice(0, len, alloc); }
CSlice::CSlice(char* ptr, int len, bool alloc) { initSlice(ptr, len, alloc); }
CSlice::~CSlice() { clear(); } // forever alloc may reset heap to false?

// or just do the same for text as for binary bytes ? ...
// typedef CSlice ASlice; cfitsio asci text slice as a character buffer, rather than a std::string

// since XSlice is simply a std::vector<bool>, just provide a length() func. and ctor -- perhaps this suffices? 
//int XSlice::length() { return (int) size(); } 
void XSlice::initSlice(const BitsAny& bv) { *((vector<bool>*)this) = bv; }
XSlice::XSlice(int len) : BitsAny(abs(len), 0) {}

  /** @} */

} // namespace ahtlm

#endif // AHSLICE_C
