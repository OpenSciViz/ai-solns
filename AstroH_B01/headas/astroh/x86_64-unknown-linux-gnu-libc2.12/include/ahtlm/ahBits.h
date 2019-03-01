#if !defined(AHBITS_H)
#define AHBITS_H(arg) const char arg##AHBITS_rcsId_svnId[] = "$Id: ahBits.h,v 1.6 2012/07/15 20:57:17 dhon Exp $"; 

/// make use of c/c++ stdlib items:
#include <cstdlib>
#include <cmath>
#include <bitset>
#include <map>
#include <vector>
#include <sstream>
#include <string>

using namespace std;

namespace ahtlm {

  /** \addtogroup mod_ahgen
   *  @{
   */

/// bit-sets (flags and such) for time and hit counts are then followed by raw data

typedef unsigned long long Uint64;
typedef unsigned int       Uint32;
typedef unsigned short     Uint16;
typedef unsigned char      Uint8;
typedef unsigned char      Ubyte;

// a general purpose arbitrary size bit field/flag:
typedef std::vector<bool> BitsAny;
typedef std::map< int, BitsAny > BitHash;
//
// these are fixed-size bit flag types:
typedef std::bitset<1> Bits1;
typedef std::bitset<2> Bits2;
typedef std::bitset<3> Bits3;
typedef std::bitset<4> Bits4;
typedef std::bitset<5> Bits5;
typedef std::bitset<6> Bits6;
typedef std::bitset<8> Bits8;
typedef std::bitset<10> Bits10;
typedef std::bitset<16> Bits16;
typedef std::bitset<24> Bits24;
typedef std::bitset<31> Bits31;
typedef std::bitset<32> Bits32;
typedef std::bitset<48> Bits48;
typedef std::bitset<64> Bits64;
typedef std::bitset<96> Bits96;

typedef std::bitset<1280> Bits1280; // 32*40 (hxi max chan*asic)
typedef std::bitset<39936> Bits39936; // 64*624 (sgd max chan*asic)

// all telemtry (TLM) is 16 bit aligned 
typedef std::map< int, Bits16 > Bit16Hash; 

typedef Bits32 HXIChanFlagsAsic;
typedef Bits1280 HXIChanFlagsOccur;
typedef std::map< int, HXIChanFlagsAsic> HXIChanFlagsHash;

typedef Bits64 SGDChanFlagsAsic;
typedef Bits39936 SGDChanFlagsOccur;
typedef std::map< int, SGDChanFlagsAsic> SGDChanFlagsHash;

/// return number of set bits
int remapOccuranceBitFlags(HXIChanFlagsOccur& remapflags, const HXIChanFlagsHash& origchans);

int remapOccuranceBitFlags(SGDChanFlagsOccur& remapflags, const SGDChanFlagsHash& origchans);

//
//
///////////////////////////////////// bitset tranformation and pretty print funcs.
//
// funcs for multiple bits -- bitset <--> vector<bool> conversion
// (loop over the bool vector and bitset [] by index):
// betsets: 1, 2, 3, 4, 5, 6, 8, 24, 31

// return bool indicates success or failure of get:
bool getBitSet(HXIChanFlagsOccur& b, const BitsAny& bv, int idx= 0);
// return int indicates size of resultant bit vec
bool setBitsAny(BitsAny& bv, const HXIChanFlagsOccur& b, int idx= 0);

// return bool indicates success or failure of get:
bool getBitSet(SGDChanFlagsOccur& b, const BitsAny& bv, int idx= 0);
// return int indicates size of resultant bit vec
bool setBitsAny(BitsAny& bv, const SGDChanFlagsOccur& b, int idx= 0);

// return bool indicates success or failure of get:
bool getBitSet(Bits1& b, const BitsAny& bv, int idx= 0);
// return int indicates size of resultant bit vec
bool setBitsAny(BitsAny& bv, const Bits1& b, int idx= 0);

// return bool indicates success or failure of get:
bool getBitSet(Bits2& b, const BitsAny& bv, int idx= 0);
// return int indicates size of resultant BitsAny vec
int setBitsAny(BitsAny& bv, const Bits2& b, int idx= 0);

bool getBitSet(Bits3& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits3& b, int idx= 0);

bool getBitSet(Bits4& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits4& b, int idx= 0); 

bool getBitSet(Bits5& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits5& b, int idx= 0); 

bool getBitSet(Bits6& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits6& b, int idx= 0); 

bool getBitSet(Bits8& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits8& b, int idx= 0); 

bool getBitSet(Bits10& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits10& b, int idx= 0); 

bool getBitSet(Bits16& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits16& b, int idx= 0); 

bool getBitSet(Bits24& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits24& b, int idx= 0); 

bool getBitSet(Bits31& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits31& b, int idx= 0); 

bool getBitSet(Bits32& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits32& b, int idx= 0); 

bool getBitSet(Bits48& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits48& b, int idx= 0);

bool getBitSet(Bits96& b, const BitsAny& bv, int idx= 0);
int setBitsAny(BitsAny& bv, const Bits96& b, int idx= 0);

// return text of bitsets for pretty-printing
int bitText(string& line, const Bits8& b, string prefix= "");

int bitText(string& line, const BitsAny& bv, string prefix= "");

int bitText(vector<string>& printbuff, const BitsAny& bv, int stride= 8, string prefix= "");

  /** @} */

} // namespace ahtlm

#endif // AHBITS_H

