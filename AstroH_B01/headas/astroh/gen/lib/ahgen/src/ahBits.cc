#if !defined(AHBITS_C)
#define AHBITS_C(arg) const char arg##AHBITS_rcsId_svnId[] = "$Id: ahBits.cc,v 1.1 2012/06/19 22:13:44 dhon Exp $"; 

/// make use of c/c++ stdlib items:
#include <cstdlib>
#include <cmath>
#include <bitset>
#include <vector>
#include <sstream>
#include <string>

namespace ahgen {

  /** \addtogroup mod_ahgen
   *  @{
   */

//
///////////////////////////////////// bitset tranformation and pretty print funcs.
//
// funcs for multiple bits -- bitset <--> vector<bool> conversion
// (loop over the bool vector and bitset [] by index):
// betsets: 1, 2, 3, 4, 5, 6, 8, 24, 31

// return bool indicates success or failure of get:
bool getBitSet(Bits1& b, const BitsAny& bv, int idx) { 
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
// return int indicates size of resultant bit vec
bool setBitsAny(BitsAny& bv, const Bits1& b, int idx) { 
  const int nb =(const int) b.size();
  if( (nb + idx) <= (int)bv.size() ) bv[idx] = b[0]; else bv.push_back(b[0]);
  return (int) bv.size();
}

// return bool indicates success or failure of get:
bool getBitSet(Bits2& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
// return int indicates size of resultant BitsAny vec
int setBitsAny(BitsAny& bv, const Bits2& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits3& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits3& b, int idx) { 
  const int nb = 3;
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits4& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits4& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits5& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits5& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits6& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits6& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits8& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits8& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits10& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits10& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits16& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits16& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits24& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits24& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits31& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits31& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits48& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits48& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}

bool getBitSet(Bits96& b, const BitsAny& bv, int idx) {
  const int nb = (const int) b.size(), nvb = (const int) bv.size(); 
  if( nb + idx > (int) bv.size() ) return false;
  for( int i = 0, vidx = idx+i; i < nb && vidx < nbv; ++i, ++vidx ) b[i] = bv[vidx];
  return true;
}
int setBitsAny(BitsAny& bv, const Bits96& b, int idx) { 
  const int nb =(const int) b.size();
  for( int i = 0; i < nb; ++i )
    if( (i+idx) < (int)bv.size() ) bv[i+idx] = b[i]; else bv.push_back(b[i]);
  return (int) bv.size();
}


int bitText(string& line, const Bits8& b, string prefix) {
  stringstream ss;
  ss << prefix << " :: " << b.to_string() << endl;
  line += ss.str();
  return line.length();
}

int bitText(string& line, const BitsAny& bv, string prefix) {
  // put the entire bitvec into one portentially long string
  Bits8 b8;
  int vsz = (int)bv.size(), sz8 = (int)b8.size();
  int numlines = vsz / sz8;

  line = "";
  getBitSet(b8, bv);
  int slen = bitText(line, b8, prefix);  // use 8 bit text string
  if( numlines <= 1 )
    return slen;

  int offset = 0;
  for( int i = 1; i < numlines; ++i, offset = i*sz8 ) {
    getBitSet(b8, bv, offest);
    slen = bitText(line, b8, prefix); // line is appended
  }

  // deal with remaining bits ...
  int nits = vsz - offset;
  if( nits <= 0 )
    return slen;

  getBitSet(b8, bv, offest)
  slen = bitText(line, b8, prefix); // line is appended
  return slen;
}

int bitText(vector<string>& printbuff, const BitsAny& bv, int stride, string prefix) {
  string vline = "";
  printbuff.clear();
  int vsz = (int)bv.size();
  int numlines = vsz / stride;
  int slen = bitText(vline, bv, prefix)
  for( int i = 0; i < numlines; ++i ) {
    string s = vline.substr(i*stride, stride);
    printbuff.push_back(s);
    }
  }

  return (int)printbuff.size();          
}

  /** @} */

} // namespace ahgen

#endif

