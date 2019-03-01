
#if !defined(DOHXI_C)
#define DOHXI_C const char doHXI_rcsId_svnId[] = "$Id: doHXI.cxx,v 1.13 2012/07/22 18:14:44 dhon Exp $"; 

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
AHBITS_H(doHXI)

#include "ahtlm/ahSlice.h"
AHSLICE_H(doHXI)

#include "ahtlm/ahTLM.h"
AHTLM_H(doHXI)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(doHXI)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(doHXI)

using namespace ahtlm;
using namespace ahhxi;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

// private-local namespace of heplers (which may get refactored to doInstrum or ahgen)
namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: doHXI.cxx,v 1.13 2012/07/22 18:14:44 dhon Exp $";

  int _numHitAsics = 0;
  int _totalActiveChans = 0;

  /// 
  ahtlm::Uint16 bits2int(const Bits4 bits) {
    //ahtlm::Uint16 ival = bits[0]; size_t width = (int)bits.size();
    //for( size_t i = 1; i < width; ++i ) if( bits[i] ) ival += i*2;
    ahtlm::Uint16 ival = (ahtlm::Uint16) bits.to_ulong();
    return ival;
  }


} // end private-local namespace of heplers

namespace ahhxi {
  /// these funcs act on atomic/individual bitsets/bit-fields...
  /// layerIdx (0-4) in least significant 4 bits 
  Uint16 hxiLayerIdxFromTLMhdr(const Bits48 bits) { 
    Bits4 b; int offset = 8;
    for( size_t i = 0; i < b.size(); ++i ) b[i] = bits[offset + i];
    return (Uint16)bits.to_ulong(); // bits2int(b); 
  }

  /// ASICIdx (0-7) in most significant 4 bits 
  Uint16 hxiAsicIdxFromTLMhdr(const Bits48 bits) { 
    Bits4 b4; int offset = 12;
    for( int i = 0; i < 4; ++i ) b4[i] = bits[offset + i];
    return (Uint16)bits.to_ulong(); // bits2int(b4);
  }

  Uint16 hxiAsicRemap(const Bits48 bits) {
    Uint16 layrIdx = hxiLayerIdxFromTLMhdr(bits);
    Uint16 asicIdx = hxiAsicIdxFromTLMhdr(bits);
    Uint16 asic = 1 + asicIdx + (layrIdx * _HXInumAsicPerLayer); 
    return asic;
  }

  int hxiAsicVecReMap(vector<Uint16>& remap, vector<Uint16>& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets) {
    orig.clear(); remap.clear();
    int offset = _HXITLMheaderBytes; // first asic event data telem buffer offset 14 bytes from start 
    Bits48 ehdr;
    int hitAsics = (int)byte_offsets.size();
    for( int i = 0; i < hitAsics; ++i ) {
      offset = hxiEventHdrOf(ehdr, rawtlm, byte_offsets, offset);
      orig.push_back(hxiAsicIdxFromTLMhdr(ehdr));
      remap.push_back(hxiAsicRemap(ehdr));
    }
    return (int) orig.size();
  }

  int hxiChanReMap(vector<Uint16>& remap, vector<Uint16>& orig, const Bits48 bits) {
    // if event data is processed sequentually, this may simply append to given vecs
    //orig.clear(); remap.clear();
    int offset = (int) _HXITLMheaderBytes; // first asic event data telem buffer offset 14 bytes from start
    int asicIdx = hxiAsicRemap(bits) - 1;
    for( int i = 0; i < (int)_HXInumChanPerAsic; ++i ) {
      if( bits[offset + i] ) {
        int ochan = 1 + i;
        int rchan  = 1 + i + (asicIdx * _HXInumChanPerAsic);
        orig.push_back(ochan);
        remap.push_back(rchan);
      }
    }
    return (int) orig.size();
  }

  // the above 2 funcs are simple ... but inefficient for a portable cfitsio interface
  // so here are byte oriented versions that use anonymous unions:
  int hxiAsicVecReMap(TLMbyteVec& remap, TLMbyteVec& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets) {
    orig.clear(); remap.clear();
    int offset = (int) _HXITLMheaderBytes; // first asic event data telem buffer offset 14 bytes from start 
    Bits48 ehdr;
    int hitAsics = (int)byte_offsets.size();
    union { Uint16 u16tlm; unsigned char bytlm[2]; };
    for( int i = 0; i < hitAsics; ++i ) {
      offset = hxiEventHdrOf(ehdr, rawtlm, byte_offsets, offset);
      u16tlm = hxiAsicIdxFromTLMhdr(ehdr);
      orig.push_back(bytlm[0]); orig.push_back(bytlm[1]);
      u16tlm = hxiAsicRemap(ehdr);
      remap.push_back(bytlm[0]); remap.push_back(bytlm[1]);
    }
    return (int)remap.size();
  }

  /// these funcs act on arrays of telemetry items (of a specific type),
  /// and may either perform on built-in test data, or the real thing
  /// (whenever the real thing becomes available)..
  
  int hxiAsicHeaders(Int32TLM& asic_hdrs, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Number of Asics hit: " << _numHitAsics << endl;
    // this is a placeholder 
    vector<Uint32> eval(_numHitAsics, 17);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    asic_hdrs.setSlice(eval);
    return (int)eval.size();
  } 

  int hxiOrigAsicIds(ByteTLM& orig_asicIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) { 
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Number of Asics hit: " << _numHitAsics << endl;
    // this is a placeholder -- preserve the asic/layer & layer Id 8 bit TLM values:
    TLMbyteVec eval(_numHitAsics, 18);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    orig_asicIds.setSlice(eval);
    return (int)eval.size();
  }

  int hxiRemapAsicIds(ByteTLM& remapped_asicIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Number of Asics hit: " << _numHitAsics << endl;
    // this is a placeholder 
    TLMbyteVec eval(_numHitAsics, 19);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    remapped_asicIds.setSlice(eval);
    return (int)eval.size();
  }

  int hxiOrigChanFlags(Int32TLM& orig_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint32> eval(_numHitAsics, 20);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    orig_chans.setSlice(eval);
    return (int)eval.size();
  }

  int hxiRemapChanIds(Int16TLM& remapped_chanIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_totalActiveChans, 21);
    for(int i = 0; i < _totalActiveChans; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    remapped_chanIds.setSlice(eval);
    return (int)eval.size();
  }

  int hxiActiveChanCount(ByteTLM& actv_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    TLMbyteVec eval(_numHitAsics, 22);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    actv_chans.setSlice(eval);
    return (int)eval.size();
  }

  int hxiRefChans(Int16TLM& ref_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) { 
    int bit_offset = 0; // need bit offsets to each 10 bit value;
    clog << __PRETTY_FUNCTION__ << " byte offset: " << offset << ", bit offset: " << bit_offset << endl;
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_numHitAsics, 23);
    for(int i = 0; i < _numHitAsics; ++i ) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    ref_chans.setSlice(eval);
    return (int)eval.size();
  } 

  int hxiNoiseChans(Int16TLM& noise_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    int bit_offset = 0; // need bit offsets to each 10 bit value;
    clog << __PRETTY_FUNCTION__ << " byte offset: " << offset << ", bit offset: " << bit_offset << endl;
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_numHitAsics, 24);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    noise_chans.setSlice(eval);
    return (int)eval.size();
  } 

  int hxiRawADCPulses(Int16TLM& raw_adc_pulse_heights, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    int bit_offset = 0; // need bit offsets to each 10 bit value;
    clog << __PRETTY_FUNCTION__ << " byte offset: " << offset << ", bit offset: " << bit_offset << endl;
    clog << __PRETTY_FUNCTION__ << " Number of 32bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_totalActiveChans, 25);
    for(int i = 0; i < _totalActiveChans; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    raw_adc_pulse_heights.setSlice(eval);
    return (int)eval.size();
  }

  // could also perform nominal calibration
  int hxiCalibPulses(Int16TLM& calib_pulse_heights, const Int16TLM& raw_adc_pulse_heights, const Int16TLM& noise_chans, const Int16TLM& ref_chans) {
    return 0;
  }

  int countActiveChans(vector<Bits32>& chanflags) {
    int chan_cnt = 0, asic_cnt = (int)chanflags.size();
    for( int i = 0; i < asic_cnt; ++i ) { 
      Bits32& flags = chanflags[i];
      chan_cnt += flags.count();
    }
    return chan_cnt;
  }

  // process all columns
  int hxiFF2SF(HXISF& hxisf, TLMbyteVec& raw_asic_data) {
    // for test generated/simulated data, col01 should provide simple cross-check:
    string& testInfo = (string&)hxisf.col01; 
    clog << __PRETTY_FUNCTION__ << " Occurance (test gen.) Info: " << testInfo << endl;
 
    int testasic_cnt = _numHitAsics = checkAsicCount(testInfo);
    int testchan_cnt = _totalActiveChans = checkChanCount(testInfo);
    clog << __PRETTY_FUNCTION__ << " Occurance (test gen) asics, chans: " << testasic_cnt << ", " << testchan_cnt << endl;

    // extract, reformat, and remap telemetry bits in raw data column (raw TLM data) into columns new columns
    // inspect the raw tlm buff ti determine count of asics & chans, and offsets to each asic sub-buf
    vector<int> byte_offsets(testasic_cnt, 0); // offsets to each asic sub-buff
    vector<Bits32> occur_chan_bitflags(testasic_cnt, 0);
    int asic_cnt = testasic_cnt; // asicBuffInfo(byte_offsets, occur_chan_bitflags, raw_asic_data);

    // check that asic and active chan counts are rational:
    clog << __PRETTY_FUNCTION__ << " active asics found in buffer: " << asic_cnt << " " << occur_chan_bitflags.size() << endl;
    //if( asic_cnt <= 0 || asic_cnt > (int)_HXImaxAsic ) return -1;

    vector<int> actv_chans;
    int actvchan_cnt = countActiveChans(occur_chan_bitflags);
    clog << __PRETTY_FUNCTION__ << " Total number of active channels found in all Occurance bitflags: " << actvchan_cnt << endl; 

    // if al is well, proceed without output processing
    // loop throug all asics and use offsets to access raw tlm to process each column:
    for( int i = 0; i < asic_cnt; ++i ) {
      int offset = byte_offsets[i];
      // these column defs and order may change, but for AstroH_B01 assume this:
      hxiAsicHeaders(hxisf.col17, occur_chan_bitflags, raw_asic_data, offset); // Int32TLM 
      hxiOrigAsicIds(hxisf.col18, occur_chan_bitflags, raw_asic_data, offset); // ByteTLM: asic/layer
      hxiRemapAsicIds(hxisf.col19, occur_chan_bitflags, raw_asic_data, offset); // ByteTLM: asic*layer (1-40)
      hxiOrigChanFlags(hxisf.col20, occur_chan_bitflags, raw_asic_data, offset); // Int32TLM: bitflags
      hxiRemapChanIds(hxisf.col21, occur_chan_bitflags, raw_asic_data, offset); // Int16TLM: asic*chan (1-1280)
      hxiActiveChanCount(hxisf.col22, occur_chan_bitflags, raw_asic_data, offset); // ByteTLM: #bits==1 (1-32)
      hxiRefChans(hxisf.col23, occur_chan_bitflags, raw_asic_data, offset); // Int16TLM: 10bit -> 16bit ints
      hxiNoiseChans(hxisf.col24, occur_chan_bitflags, raw_asic_data, offset); // Int16TLM: 10bit -> 16bit ints
      hxiRawADCPulses(hxisf.col25, occur_chan_bitflags, raw_asic_data, offset); // Int16TLM: 10bit -> 16bit ints
      hxiCalibPulses(hxisf.col26, hxisf.col25, hxisf.col24, hxisf.col23); // Int16TLM: raw 16bit ints -> cal. 16bit ints
    }
    return asic_cnt;
  }

  //int doHXIwork(InstrumWork& input, InstrumWork& output) {
  int doHXIwork(AhFitsFilePtr infile, AhFitsFilePtr outfile) {
    // double check instrument names of infile and outfile?
    vector<string> instrums;
    vector<AhFitsFilePtr> filelist; filelist.push_back(infile); filelist.push_back(outfile);
    int cnt = getInstrumNames(instrums, filelist);
    clog << __PRETTY_FUNCTION__ << " " << cnt 
         << " Input file instrument name: " << instrums[0] << " Output file instrument name: " << instrums[1] << endl;

    string instrum = instrums[0];
    if( instrum != instrums[1] ) {
      clog << __PRETTY_FUNCTION__ << " mismatch input/output instrument names? " << endl;
      return -1;
    }

    // get the b-table number of rows:
    long row_cnt = getRowCount(infile);

    // allocate sn i/o buffer for a single row of max content (thus reusable)
    // stack allocation of max HXI raw data buff -- could be used for hxi ff/sf raw data column (june 2012 template)
    //TLMbyteVec raw_asic_data(_maxHXI_TLMbytes, 0); // cleared bytes
    TLMbyteVec* raw_asic_data = new TLMbyteVec(_maxHXI_TLMbytes, 0); // use the heap here and no need to free this at this code-level
    HXISF* hxisf = new HXISF;
    HXIFF* hxiff = (HXIFF*) hxisf; // memory efficiency ...
    // HXISF hxisf;
    //HXIFFr& hxiff = (HXIFF&) hxisf; // memory efficiency ...

    // default behavior is to use allocation of raw tlm buff if non-null pointer provided 2nd arg:
    // bool alloc = hxiTLMof(hxisf, &raw_asic_data);
    bool alloc = hxiTLMof(hxisf, raw_asic_data);
    clog << __PRETTY_FUNCTION__ << " Telemetry buffer allocated yet? (true/false): " << alloc << endl;

    int asic_cnt = 0, ffcolcnt = 0, sfcolcnt = 0;
    //if( alloc ) // ok presumabley the buffers have been allocated and the read may proceed

    for( int rowIdx = 0; rowIdx < row_cnt; ++rowIdx ) {
      // read the first fits file columns -- sets tlm buff lengths to actual read lengths
      ffcolcnt = hxiReadRow(infile, hxiff, rowIdx);
      // this should return the total number of acive asics and/or channel found
      asic_cnt = hxiFF2SF(hxisf, raw_asic_data);
      clog << __PRETTY_FUNCTION__ << " " << instrum << " Completed FF2SF remap -- active Asic count: " << asic_cnt << endl;
      // write out the sff 
      sfcolcnt = hxiWriteRow(outfile, hxisf, rowIdx);
      clog << __PRETTY_FUNCTION__ << " " << instrum << " Wrote output file colcount: " << sfcolcnt << endl;
    }

    return row_cnt;
  } // doHXIwork

} // end namespace ahhxi

#endif
