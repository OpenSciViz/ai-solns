#if !defined(LUTHXI_C)
#define LUTHXI_C const char lutHXI_rcsId_svnId[] = "$Id: lutHXI.cxx,v 1.3 2012/07/18 20:49:47 dhon Exp $"; 

#include <math.h>

/** \file Funcs. that handle HXI TLM -- asic and channel remap funcs, internal test "data" and
    related activities. Some of the more general funcs. will be refactored into ahgen and ahfits.
    Others need to move to blobCells.cxx and blobBits.cxx.
    \author David B. Hon
    \date Feb-Mar-Apr 2012
*/

/// note some of these funcs. may move into the ahfits or ahgen namespaces
/// and associated libs...
//
#include "ahtlm/ahBits.h"
AHBITS_H(lutHXI)

#include "ahtlm/ahSlice.h"
AHSLICE_H(lutHXI)

#include "ahtlm/ahTLM.h"
AHTLM_H(lutHXI)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(lutHXI)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(lutHXI)

using namespace ahtlm;
using namespace ahhxi;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

/*
funcs to fetch or generate LUTs that guide the telemetry (tlm) access and processing.
LUTs should provide integer offsets in units of (number of) bits or bytes or 16
bit words from the start of a buffer of sub-buffer. offsets may be absolute from
the start of the entire buffer, or relative to the start of a (embedded variable-length)
sub-buffer.

the important ones:

1.) asic hit count bitset offset -- absolute from start of full (raw) tlm buffer 
2.) 1st asic sub-buffer header -- absolute from start of full (raw) tlm buffer 
3.) 1st asic active channel bitflags --  absolute from start of full (raw) tlm buffer
4.) ref channel 10 bit adc value -- relative to start of asic sub-buffer (header)
5.) active channel adc 10 bit values -- relative to asic-sub-buff (one offset foreach?)
6.) noise channel adc 10 bit value  -- relative to start of asic sub-buffer (header)

 */

// private-local namespace of heplers (which may get refactored to doInstrum or ahgen)
namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: lutHXI.cxx,v 1.3 2012/07/18 20:49:47 dhon Exp $";

  /// 
  ahtlm::Uint16 bits2int(const Bits4 bits) {
    //ahtlm::Uint16 ival = bits[0]; size_t width = (int)bits.size();
    //for( size_t i = 1; i < width; ++i ) if( bits[i] ) ival += i*2;
    ahtlm::Uint16 ival = (ahtlm::Uint16) bits.to_ulong();
    return ival;
  }


} // end private-local namespace of heplers

namespace ahhxi {

  int asicCountOffset() { return HXI_Occurance_HeaderSizeInBytes - 2; }

  int getAsicHitCount(const TLMbyteVec& raw_asic_data) {
    int offset = asicCountOffset();
    Ubyte hits = raw_asic_data[offset];
    return (int)hits;
  }

  int firstEventOffset() { return HXI_Occurance_HeaderSizeInBytes; } 

  int firstActvChanFlagOffset() { return HXI_Occurance_HeaderSizeInBytes + 2; } 

  int nextEventOffset(vector<int>& byte_offsets, int current_offset) {
    // if the byte_offsets are properl set one can simply index the offset vec
    // and this may be an optional double check on what's next and where is
    // the stop bit & 0 filler, etc...
    // find the next stop bit/filler from the given offset, and
    // return a new offset (for the next event buff) 
    // binary search better for SGD's potentiall loneger vec.
    for(int i = 0; i < (int) byte_offsets.size(); ++i ) {
      if( byte_offsets[i] > current_offset ) return byte_offsets[i];
    }
    return current_offset; // pretty much a stub for now..
  }

  // given a telemetry buffer offset byte count (offset == 14 is due to 2 byte 
  // start flag, 4 byte local time, 4 byte flags, and 3 byte live-time and 1 byte nhitasics),
  // and entire byte telem vec., extract the 48bit event header bitset. the 
  int hxiEventHdrOf(Bits48& ehdr, const TLMbyteVec& tlm, vector<int>& byte_offsets, int tlm_offset) {
    int byt_offset = 0;
    // 6 bytes --> 48 bits
    for( int bytIdx = 0; bytIdx < 6; ++bytIdx ) {
      Bits8 b8(tlm[tlm_offset + bytIdx]); // init 8bit val. from 1 byte of telemetry
      byt_offset = bytIdx * b8.size();
      for( size_t bitIdx = 0; bitIdx < b8.size(); ++bitIdx )
        ehdr[byt_offset + bitIdx] = b8[bitIdx];
    }
    // evaluate the offset to the next event and return it:
    //return hxiNextEventOffset(tlm, tlm_offset);
    return nextEventOffset(byte_offsets, tlm_offset);
  }

  int singleAsicBuffBitCount(int actv_cnt) {
    int asic_layer_miscflags= 16, chanflags= 32, cmflag= 1, noisechan= 10, chan1adc= 10, refchan= 10, stop= 1;
    int bitcnt = asic_layer_miscflags + chanflags + cmflag + noisechan + chan1adc*actv_cnt + refchan + stop;
    return bitcnt;
  }

  int getActiveChanIds(vector<Uint16>& actv_chanIds, Bits32& bitflags) {
    actv_chanIds.clear();
    for( int i = 0; i < (int)bitflags.size(); ++i ) {
      if( bitflags.test(i) ) actv_chanIds.push_back(1+i);
    }
    return (int)actv_chanIds.size();
  }

  // this is an essential/key func that supports telemtry processing of an "occurance"
  // by providing (byte) offsets to the start of each asic hit's raw data buffer,
  // as determined be the active channel bitflags -- which are also extracted and provided for
  // HXI as an "occurance" vector-list Bit32 items. (SGD func must provide Bit64 items)
  int asicBuffInfo(vector<int>& byte_offsets, vector<Bits32>& occur_chan_bitflags, const TLMbyteVec& raw_asic_data) {
    /// given the entire raw telemetry (occurance) buffer, scan through it to find the offsets to the start
    /// of each individual asic's channel data, and return the list of offsets (as byte counts, but must
    /// yield a even number of bytes ==> ensuring 16 bit word alignments)
    /// since this must inspect the active channel bit flags along the way, it should provide those too.

    int asic_cnt = getAsicHitCount(raw_asic_data);

    /// check active channel bitflags to determine the number of data channels in the first asic buff
    /// and calculate the exepcted byte count the next asic buff, if any
    // offset should skip over the 8 bit asic-layer-Id and 8 bits that hold start, chip, trg, seu, dm flags
    int actv_cnt = 0, bitcnt = 0, bittywords = 0;
    int offset = firstEventOffset();
    byte_offsets.clear();
    Bits32 bitflags;
    vector<Uint16> actv_chanIds;
    // loop over asic 'numhits' count:
    for( int i = 0; i < asic_cnt; ++i ) {
      byte_offsets.push_back(offset);
      offset += 2; // find the chan bit flags -- this may be a larger offset for SGD
      extractTLMbits(bitflags, raw_asic_data, offset); // examine hxi 32 bit chan flags
      actv_cnt = getActiveChanIds(actv_chanIds, bitflags); // how many are active?
      bitcnt = singleAsicBuffBitCount(actv_cnt); // calc. expected size of the raw data buffer in bits
      // use bitcnt to calc. byte count by rounding-up to nearest integral number of bytes
      // that produces an integral number of 16 bit words ...
      // and check that is congruent with the 0 fill that should folllow the stop bit and
      offset += bitcnt / 8; 
      bittywords = bitcnt % 16;
      if( bittywords > 0 ) {
        if( bittywords <= 8 ) 
          offset += 1; // add 1 byte
        else
          offset += 2; // add 2 byte
      }
      occur_chan_bitflags.push_back(bitflags); // save the channel bitflags for future use...  
      // preceed the start of the next asic data buff in this occurance?
      // next asic sub buff is at least this much farther into the raw tlm occurance buff
   }
    return asic_cnt;
  }

} // end namespace ahhxi

#endif // LUTHXI_C
