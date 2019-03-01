
#if !defined(DOSGD_C)
#define DOSGD_C const char doSGD_rcsId_svnId[] = "$Id: doSGD.cxx,v 1.8 2012/07/22 18:14:44 dhon Exp $"; 

#include <math.h>

/** \file Funcs. that handle SGD TLM -- asic and channel remap funcs, internal test "data" and
    related activities. Some of the more general funcs. will be refactored into ahgen and ahfits.
    Others need to move to blobCells.cxx and blobBits.cxx.
    \author David B. Hon
    \date Feb-Mar-Apr 2012
*/

/// note some of these funcs. may move into the ahfits or ahgen namespaces
/// and associated libs...
//
#include "ahtlm/ahBits.h"
AHBITS_H(doSGD)

#include "ahtlm/ahSlice.h"
AHSLICE_H(doSGD)

#include "ahtlm/ahTLM.h"
AHTLM_H(doSGD)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(doSGD)

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(doSGD)

using namespace ahtlm;
using namespace ahsgd;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

// private-local namespace of heplers (which may get refactored to doInstrum or ahgen)
namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: doSGD.cxx,v 1.8 2012/07/22 18:14:44 dhon Exp $";

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

namespace ahsgd {
  /// these funcs act on atomic/individual bitsets/bit-fields...
  /// layerIdx (0-4) in least significant 4 bits 
  Uint16 sgdLayerIdxFromTLMhdr(const Bits48 bits) { 
    Bits4 b; int offset = 8;
    for( size_t i = 0; i < b.size(); ++i ) b[i] = bits[offset + i];
    return (Uint16)bits.to_ulong(); // bits2int(b); 
  }

  /// ASICIdx (0-7) in most significant 4 bits 
  Uint16 sgdAsicIdxFromTLMhdr(const Bits48 bits) { 
    Bits4 b4; int offset = 12;
    for( int i = 0; i < 4; ++i ) b4[i] = bits[offset + i];
    return (Uint16)bits.to_ulong(); // bits2int(b4);
  }

  Uint16 sgdAsicRemap(const Bits48 bits) {
    Uint16 layrIdx = sgdLayerIdxFromTLMhdr(bits);
    Uint16 asicIdx = sgdAsicIdxFromTLMhdr(bits);
    Uint16 asic = 1 + asicIdx + (layrIdx * _SGDnumAsicPerLayer); 
    return asic;
  }

  int sgdAsicVecReMap(vector<Uint16>& remap, vector<Uint16>& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets) {
    orig.clear(); remap.clear();
    int offset = _SGDTLMheaderBytes; // first asic event data telem buffer offset 14 bytes from start 
    Bits48 ehdr;
    int hitAsics = (int)byte_offsets.size();
    for( int i = 0; i < hitAsics; ++i ) {
      offset = sgdEventHdrOf(ehdr, rawtlm, byte_offsets, offset);
      orig.push_back(sgdAsicIdxFromTLMhdr(ehdr));
      remap.push_back(sgdAsicRemap(ehdr));
    }
    return (int) orig.size();
  }

  int sgdChanReMap(vector<Uint16>& remap, vector<Uint16>& orig, const Bits48 bits) {
    // if event data is processed sequentually, this may simply append to given vecs
    //orig.clear(); remap.clear();
    int offset = (int) _SGDTLMheaderBytes; // first asic event data telem buffer offset 14 bytes from start
    int asicIdx = sgdAsicRemap(bits) - 1;
    for( int i = 0; i < (int)_SGDnumChanPerAsic; ++i ) {
      if( bits[offset + i] ) {
        int ochan = 1 + i;
        int rchan  = 1 + i + (asicIdx * _SGDnumChanPerAsic);
        orig.push_back(ochan);
        remap.push_back(rchan);
      }
    }
    return (int) orig.size();
  }

  // the above 2 funcs are simple ... but inefficient for a portable cfitsio interface
  // so here are byte oriented versions that use anonymous unions:
  int sgdAsicVecReMap(TLMbyteVec& remap, TLMbyteVec& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets) {
    orig.clear(); remap.clear();
    int offset = (int) _SGDTLMheaderBytes; // first asic event data telem buffer offset 14 bytes from start 
    Bits48 ehdr;
    int hitAsics = (int)byte_offsets.size();
    union { Uint16 u16tlm; unsigned char bytlm[2]; };
    for( int i = 0; i < hitAsics; ++i ) {
      offset = sgdEventHdrOf(ehdr, rawtlm, byte_offsets, offset);
      u16tlm = sgdAsicIdxFromTLMhdr(ehdr);
      orig.push_back(bytlm[0]); orig.push_back(bytlm[1]);
      u16tlm = sgdAsicRemap(ehdr);
      remap.push_back(bytlm[0]); remap.push_back(bytlm[1]);
    }
    return (int)remap.size();
  }

  /// these funcs act on arrays of telemetry items (of a specific type),
  /// and may either perform on built-in test data, or the real thing
  /// (whenever the real thing becomes available)..
  
  int sgdAsicHeaders(Int32TLM& asic_hdrs, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) { 
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Number of Asics hit: " << _numHitAsics << endl;
    // this is a placeholder 
    vector<Uint32> eval(_numHitAsics, 20);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    asic_hdrs.setSlice(eval);
    return (int)eval.size();
  } 

  int sgdOrigAsicIds(Int16TLM& orig_asicIds, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) { 
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Number of Asics hit: " << _numHitAsics << endl;
    // this is a placeholder -- somehow the asic/tray, tray/driver-board, asic/driver-board info must be preserved?
    vector<Uint16> eval(_numHitAsics, 21);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    orig_asicIds.setSlice(eval);
    return (int)eval.size();
  }

  int sgdRemapAsicIds(Int16TLM&  remapped_asicIds, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Number of Asics hit: " << _numHitAsics << endl;
    // this is a placeholder 
    vector<Uint16> eval(_numHitAsics, 22);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    remapped_asicIds.setSlice(eval);
    return (int)eval.size();
  }

  int sgdOrigChanFlags(Int64TLM& orig_chans, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint64> eval(_numHitAsics, 23);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    orig_chans.setSlice(eval);
    return (int)eval.size();
  }

  int sgdRemapChanIds(Int16TLM& remapped_chanIds, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_totalActiveChans, 24);
    for(int i = 0; i < _totalActiveChans; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    remapped_chanIds.setSlice(eval);
    return (int)eval.size();
  }

  int sgdActiveChanCount(ByteTLM& actv_chans, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    TLMbyteVec eval(_numHitAsics, 25);
    for(int i = 0; i < _numHitAsics; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    actv_chans.setSlice(eval);
     return (int)eval.size();
  }

  int sgdRefChans(Int16TLM& ref_chans, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) { 
    int bit_offset = 0; // need bit offsets to each 10 bit value;
    clog << __PRETTY_FUNCTION__ << " byte offset: " << offset << ", bit offset: " << bit_offset << endl;
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_numHitAsics, 25);
    for(int i = 0; i < _numHitAsics; ++i ) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    ref_chans.setSlice(eval);
    return (int)eval.size();
  } 

  int sgdNoiseChans(Int16TLM& noise_chans, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    int bit_offset = 0; // need bit offsets to each 10 bit value;
    clog << __PRETTY_FUNCTION__ << " byte offset: " << offset << ", bit offset: " << bit_offset << endl;
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_numHitAsics, 26);
    for(int i = 0; i < _numHitAsics; ++i ) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    noise_chans.setSlice(eval);
    return (int)eval.size();
  } 

  int sgdRawADCPulses(Int16TLM& raw_adc_pulse_heights, vector<Bits64>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset) {
    int bit_offset = 0; // need bit offsets to each 10 bit value;
    clog << __PRETTY_FUNCTION__ << " byte offset: " << offset << ", bit offset: " << bit_offset << endl;
    clog << __PRETTY_FUNCTION__ << " Number of 64bit ChannelFlags: " << occur_chan_bitflags.size() << endl;
    clog << __PRETTY_FUNCTION__ << " Total number of active Channels: " << _totalActiveChans << endl;
    vector<Uint16> eval(_totalActiveChans, 27);
    for(int i = 0; i < _totalActiveChans; ++i) {
      eval[i] += i; // must be replaced with telemetry extraction logic -- whenever valid telemtry exists
    }
    raw_adc_pulse_heights.setSlice(eval);
    return (int)eval.size();
  }

  int sgdCalibPulses(Int16TLM& calib_pulse_heights, const Int16TLM& raw_adc_pulse_heights, const Int16TLM& noise_chans, const Int16TLM& ref_chans) {
    return 0;
  }

  int countActiveChans(vector<Bits64>& chanflags) {
    int chan_cnt = 0, asic_cnt = (int)chanflags.size();
    for( int i = 0; i < asic_cnt; ++i ) { 
      Bits64& flags = chanflags[i];
      chan_cnt += flags.count();
    }
    return chan_cnt;
  }

  // process all columns
  int sgdFF2SF(SGDSF& sgdsf, TLMbyteVec& raw_asic_data) {
    string& testInfo = (string&)sgdsf.col01; 
    clog << __PRETTY_FUNCTION__ << " Occurance (test gen.) Info: " << testInfo << endl;

    int testasic_cnt = _numHitAsics = checkAsicCount(testInfo);
    int testchan_cnt = _totalActiveChans = checkChanCount(testInfo);
    clog << __PRETTY_FUNCTION__ << " Occurance (test gen) asics, chans: " << testasic_cnt << ", " << testchan_cnt << endl;
 
    // extract, reformat, and remap telemetry bits in column 16 (raw TLM data) into columns 17 - 26: 
    // inspect the raw tlm buff ti determine count of asics & chans, and offsets to each asic sub-buf
    vector<int> byte_offsets;
    vector<Bits64> occur_chan_bitflags;
    int asic_cnt = asicBuffInfo(byte_offsets, occur_chan_bitflags, raw_asic_data);

    // check that asic and active chan counts are rational:
    clog << __PRETTY_FUNCTION__ << " active asics found in buffer: " << asic_cnt << " " << occur_chan_bitflags.size() << endl;
    //if( asic_cnt <= 0 || asic_cnt > (int)_SGDmaxAsic ) return -1;

    vector<int> actv_chans;
    int actvchan_cnt = countActiveChans(occur_chan_bitflags);
    clog << __PRETTY_FUNCTION__ << " Total number of active channels found in all Occurance bitflags: " << actvchan_cnt << endl; 

    // loop throug all asics and use offsets to access raw tlm to process each column:
    for( int i = 0; i < asic_cnt; ++i ) {
      int offset = byte_offsets[i];
      // these column defs and order may change, but for AstroH_B01 assume this:
      sgdAsicHeaders(sgdsf.col20, occur_chan_bitflags, raw_asic_data, offset);
      sgdOrigAsicIds(sgdsf.col21, occur_chan_bitflags, raw_asic_data, offset);
      sgdRemapAsicIds(sgdsf.col22, occur_chan_bitflags, raw_asic_data, offset);
      sgdOrigChanFlags(sgdsf.col23, occur_chan_bitflags, raw_asic_data, offset);
      sgdRemapChanIds(sgdsf.col24, occur_chan_bitflags, raw_asic_data, offset);
      sgdActiveChanCount(sgdsf.col25, occur_chan_bitflags, raw_asic_data, offset);
      sgdRefChans(sgdsf.col26, occur_chan_bitflags, raw_asic_data, offset);
      sgdNoiseChans(sgdsf.col27, occur_chan_bitflags, raw_asic_data, offset);
      sgdRawADCPulses(sgdsf.col28, occur_chan_bitflags, raw_asic_data, offset);
      //sgdCalibPulses(sgdsf.col29, occur_chan_bitflags, raw_asic_data, offset);
    }
    return 29; // struct SGDSF hard-coded with 29 columns
  }

  //int doSGDwork(InstrumWork& input, InstrumWork& output) {
  int doSGDwork(AhFitsFilePtr infile, AhFitsFilePtr outfile) {
    // double check instrument names of infile and outfile?
    vector<string> instrums; // double check each file's (template?) instrument names
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
    // stack allocation of max SGD raw data buff -- should be used for sgd ff/sf raw data cloumn (june 2012 template)
    //TLMbyteVec raw_asic_data(_maxSGD_TLMbytes, 0); // cleared bytes
    TLMbyteVec* raw_asic_data = new TLMbyteVec(_maxSGD_TLMbytes, 0); // use the heap here and no need to free this at this code-level
    SGDSF* sgdsf = new SGDSF;
    SGDFF* sgdff = (SGDFF*) sgdsf; // memory efficiency ...
    //SGDSF sgdsf;
    //SGDFFr& sgdff = (SGDFF&) sgdsf; // memory efficiency ...

    // default behavior is to use allocation of raw tlm buff if non-null pointer provided 2nd arg:
    // bool alloc = sgdTLMof(sgdsf, &raw_asic_data);
    bool alloc = sgdTLMof(sgdsf, raw_asic_data);
    clog << __PRETTY_FUNCTION__ << " Telemetry buffer allocated yet? (true/false): " << alloc << endl;

    int asic_cnt = 0, ffcolcnt = 0, sfcolcnt = 0;
    //if( alloc ) // ok presumabley the buffers have been allocated and the read may proceed

    for( int rowIdx = 0; rowIdx < row_cnt; ++rowIdx ) {
      // read the first fits file columns -- sets tlm buff lengths to actual read lengths
      ffcolcnt = sgdReadRow(infile, sgdff, rowIdx);
      // this should return the total number of acive asics and/or channel found
      asic_cnt = sgdFF2SF(sgdsf, raw_asic_data);
      clog << __PRETTY_FUNCTION__ << " " << instrum << " Completed FF2SF remap -- active Asic count: " << asic_cnt << endl;
      // write out the sff 
      sfcolcnt = sgdWriteRow(outfile, sgdsf, rowIdx);
      clog << __PRETTY_FUNCTION__ << " " << instrum << " Wrote output file colcount: " << sfcolcnt << endl;
    }

    return row_cnt;
  } // doSGDwork

} // end namespace ahsgd

#endif
