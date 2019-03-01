#if !defined(AHSGDTYPS_H)
#define AHSGDTYPS_H(arg) const char arg##AHSGDTYPS_rcsId_svnId[] = "$Id: ahSGDtyps.h,v 1.12 2012/07/22 18:14:44 dhon Exp $"; 

// headers have gotten too lengthy so moved some stuff here ...
// FF-SF bits and column data typedefs and (some) related funcs.
// FF and SF structs below make use of the TLM slices defined
// here, and should reflect b-table column org. of assoc. FITS files:

// Local includes.
#include "ahtlm/ahBits.h"
#include "ahtlm/ahSlice.h"
#include "ahtlm/ahTLM.h"
#include "ahtlm/ahTLMinfoCache.h"

#include "ahfits/ahfits.h" /// core ahfits namespace
//#include "ahfits/vcslice.h" /// supplements ahfits namespace

#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

// Regional includes.
#include "ape/ape_error.h"
#include "ape/ape_trad.h"

// system.
#include <stdio.h>
#include <unistd.h>

// ISO includes.
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <map>
#include <bitset>
#include <vector>
#include <valarray>

using namespace ahtlm;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

/// \ingroup mod_ahsgd
namespace ahsgd {

/** \addtogroup mod_ahsgd
  *  @{
  */

  //const int _maxSGDraw  = 58656;
  //const int _maxSGDchan = 39936; // int16 count
    
  /// each sgd asic event telemtry blob/slice has at least 4*16 == 64bit header
  /// offsets are bits counts from header first (least sig.) bit..
  
  const size_t _SGDASICTLMheaderBytes = 8; // up to 624 such headers in an occurance tlm buff
  const size_t _SGDTLMheaderBytes = 14; // occurance tlm header is 7 shorts (same as HXI?)
  const size_t _SGDnumTrays =          8 + 8 + 6;
  const size_t _SGDnumTraysPerDriver = 4 + 1 + 2;
  const size_t _SGDnumAsicPerDriver = 32 + 8 + 12;
  const size_t _SGDnumAsicSiPad = 8*4*32;
  const size_t _SGDnumAsicCdTeBottom = 8*1*8;
  const size_t _SGDnumAsicCdTeSide = 6*2*12;

  // placeholder that may prove irrelevant:
  const size_t _SGDnumAsicPerLayer = _SGDnumAsicSiPad + _SGDnumAsicCdTeBottom + _SGDnumAsicCdTeSide; 
  const size_t _SGDmaxAsic = 3 * _SGDnumAsicPerLayer; // 3*208 == 624
  const size_t _SGDnumChanPerAsic = 64; 
  const size_t _SGDmaxChan = _SGDmaxAsic * _SGDnumChanPerAsic;  // 39936
  const size_t _SGDmaxRawTLM = 58656; // byte count

  inline string sgdFFVersion(string path= "./input/") { return path + "SGD_FF2012June.tpl"; }
  inline string sgdSFVersion(string path= "./input/") { return path + "SGD_SF2012June.tpl"; }

//////////////////  SGD FF and SFF structs associated with default FITS templates:
//
//  this corresponds to the iApril May 2012 version of the FITS templates updated
//  to be analogous to the HXIi June 15 2012 -- hon (after re-org. of directories and libs)
//
//  typedef struct {
  struct SGDFF {
    TextTLM   col01; // event_description (for test-id and ?)
    Real64TLM col02; // TIME
    Real64TLM col03; // S_TIME
    ByteTLM   col04; // CCSDS_Packet_Header
    Int32TLM  col05; // Time_Indicator
    Int32TLM  col06; // Occurence_ID
    Int32TLM  col07; // Local_Time
    ByteTLM   col08; // FLAG_SingleEvenUpset
    ByteTLM   col09; // FLAG_LengthCheck
    ByteTLM   col10; // FLAG_CalMode
    ByteTLM   col11; // FLAG_InitialTriggerPattern
    ByteTLM   col12; // FLAG_TriggerPattern
    ByteTLM   col13; // FLAG_CCBusy
    ByteTLM   col14; // FLAG_HitPatternCC
    ByteTLM   col15; // FLAG_PreciseShieldHitPattern // FLAG_HitPattern
    ByteTLM   col16; // FLAG_FastShieldHitPattern // FLAG_FastBGO
    Int32TLM  col17; // LiveTime
    Int16TLM  col18; // nHitASICs
    ByteTLM   col19; // RAW_ASIC_DATA
    inline int colCount() { return 19; }
    inline ~SGDFF() {}
//  } SGDFF;
  };

//  typedef struct  { 
//    SGDFF ff;
  struct SGDSF : SGDFF {
    Int32TLM  col20; // ASIC_Header
    Int16TLM  col21; // Original_ASIC_IDs 
    Int16TLM  col22; // Remapped_ASIC_IDs
    Int64TLM  col23; // Original_Channel_BitFlags // not IDs
    Int16TLM  col24; // Remapped_Channel_IDs
    ByteTLM   col25; // NumberOf_Active_Channels
    Int16TLM  col26; // Reference_Channel
    Int16TLM  col27; // CommonNoise_Channel
    Int16TLM  col28; // Raw_ADC_Pulse_Heights
    Int16TLM  col29; // Calibrated_ADC_Pulse_Heights
    inline int colCount() { return 29; }
    inline ~SGDSF() {}
//  } SGDSF;
  };

  //
  // SGDtemplate.cxx:
  int initSGDtemplates(string& content, string& fff_filename, string& sff_filename);

  //
  ////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in lutSGD.cxx -- i.e. tlm offset(s) lut(s)
  //
  int asicCountOffset();
  int firstEventOffset();
  int firstActvChanFlagOffset();
  int nextEventOffset(vector<int>& byte_offsets, int current_offset= 0);
  //int nextEventOffset(const TLMbyteVec& rawtlm, int current_offset= 0);
  // still need this or no?
  //inline int sgdNextEventOffset(const TLMbyteVec& rawtlm, int current_offset= 0) { return nextEventOffset(rawtlm, current_offset); } 
  int sgdEventHdrOf(Bits48& ehdr, const TLMbyteVec& tlm, vector<int>& byte_offsets, int tlm_offset= _SGDTLMheaderBytes);
  int singleAsicBuffBitCount(int actv_cnt);
  int getActiveChanIds(vector<Uint16>& actv_chanIds, Bits32& bitflags);
  // returns hitAsics int count, and scans tlm to establish asic sub-buff byte offfset from
  // active channel bitflags and returns assoc. vecs:
  int asicBuffInfo(vector<int>& byte_offsets, vector<Bits64>& occur_chan_bitflags, const TLMbyteVec& raw_asic_data);

  //
  ////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in ioSGD.cxx
  //
  int sgdReadRow(AhFitsFilePtr ahf, SGDFF& sgdff, int rowIdx= 0, int hduIdx= 1);
  inline int sgdReadRow(AhFitsFilePtr ahf, SGDFF* sgdff, int rowIdx= 0, int hduIdx= 1) { if( sgdff ) return sgdReadRow(ahf, *sgdff, rowIdx, hduIdx); return -1; }
  int sgdReadRow(AhFitsFilePtr ahf, SGDSF& sgdff, int rowIdx= 0, int hduIdx= 1);
  inline int sgdReadRow(AhFitsFilePtr ahf, SGDSF* sgdsf, int rowIdx= 0, int hduIdx= 1) { if( sgdsf ) return sgdReadRow(ahf, *sgdsf, rowIdx, hduIdx); return -1; }
  int sgdWriteRow(AhFitsFilePtr ahf, SGDFF& sgdff, int rowIdx= 0, int hduIdx= 1);
  
  inline int sgdWriteRow(AhFitsFilePtr ahf, SGDFF* sgdff, int rowIdx= 0, int hduIdx= 1) { if( sgdff ) return sgdWriteRow(ahf, *sgdff, rowIdx, hduIdx); return -1; }
  int sgdWriteRow(AhFitsFilePtr ahf, SGDSF& sgdsf, int rowIdx= 0, int hduIdx= 1);
  inline int sgdWriteRow(AhFitsFilePtr ahf, SGDSF* sgdsf, int rowIdx= 0, int hduIdx= 1) { if( sgdsf ) return sgdWriteRow(ahf, *sgdsf, rowIdx, hduIdx); return -1; }

  bool sgdTLMof(SGDSF& sgdsf, TLMbyteVec* raw_asic_data = 0); // reusable raw data buff ptr != 0 (max size) 
  inline bool sgdTLMof(SGDSF& sgdsf, TLMbyteVec& raw_asic_data) { return sgdTLMof(sgdsf, &raw_asic_data); }
  inline bool sgdTLMof(SGDSF* sgdsf, TLMbyteVec& raw_asic_data) { if( sgdsf ) return sgdTLMof(*sgdsf, raw_asic_data); return false; }
  inline bool sgdTLMof(SGDSF* sgdsf, TLMbyteVec* raw_asic_data) { if( sgdsf ) return sgdTLMof(*sgdsf, raw_asic_data); return false; }
  int sgdColumnNames(vector<string>& ffcol_names, vector<string>&  sfcol_names, SGDSF& sgdsf);
  
  //
  ////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in doSGD.cxx
  //
  /// return a place-holder of unallocated telemtry slices for an entire row of ff and sf b-table
  //  for the specified instrument -- int return bool should indicate if row buffers are allocated
  //  (true) or not (false) -- use init() to setup column info and optionally alloc. buffers:  
  //
  //int doSGDwork(InstrumWork& input, InstrumWork& output);
  int doSGDwork(AhFitsFilePtr infile, AhFitsFilePtr outfile);
  int sgdFF2SF(SGDSF& sgdsf, TLMbyteVec& raw_asic_data);
  inline int sgdFF2SF(SGDSF& sgdsf, TLMbyteVec* data) { if( data ) return sgdFF2SF(sgdsf, *data); return -1; }
  inline int sgdFF2SF(SGDSF* sgdsf, TLMbyteVec& data) { if( sgdsf ) return sgdFF2SF(*sgdsf, data); return -1; }
  inline int sgdFF2SF(SGDSF* sgdsf, TLMbyteVec* data) { if( sgdsf && data ) return sgdFF2SF(*sgdsf, *data); return -1; }
  // asic buffer funcs (some deprecated)
  Uint16 sgdLayerIdxFromTLMhdr(const Bits48 bits);
  Uint16 sgdAsicIdxFromTLMhdr(const Bits48 bits); 
  Uint16 sgdAsicRemap(const Bits48 bits);
  int sgdAsicVecReMap(vector<Uint16>& remap, vector<Uint16>& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets);
  int sgdAsicVecReMap(TLMbyteVec& remap, TLMbyteVec& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets);
  int sgdChanReMap(vector<Uint16>& remap, vector<Uint16>& orig, const Bits48 bits);
  int sgdAsicHeaders(Int32TLM& asic_hdrs, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int sgdOrigAsicIds(Int16TLM& orig_asicIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int sgdRemapAsicIds(Int16TLM& remapped_asicIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  // channel funcs
  int countActiveChans(vector<Bits64>& chanflags);
  int sgdOrigChanFlags(Int64TLM& orig_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int sgdRemapChanIds(Int16TLM& remapped_chanIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  // not sure if this is needed -- preserve bit flags or integer ids (10 bit orig. tlm to 16 bit int)? 
  //int sgdOrigChanIds(Int16TLM& orig_chanIds, vector<Bits32>& occur_chan_bitflags);
  int sgdActiveChanCount(ByteTLM& actv_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int sgdRefChans(Int16TLM& ref_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int sgdNoiseChans(Int16TLM& noise_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  // channel adc pulse data
  int sgdRawADCPulses(vector<Uint16>& raw_adc_pulse_heights, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  //int sgdCalibPulses(vector<Uint32>& cal_adc_pulse_heights, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int sgdCalibPulses(Int16TLM& calib_pulse_heights, const Int16TLM& raw_adc_pulse_heights, const Int16TLM& noise_chans, const Int16TLM& ref_chans);

  //
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in doSGDtest.cxx
  //
  AhFitsFilePtr sgdCreateTestfile(string& filename, string& template_filename, SGDFF& sgdff, bool clobber= true);
  AhFitsFilePtr sgdCreateTestfile(string& filename, string& template_filename, SGDSF& sgdsf, bool clobber= true);
  int sgdWriteTestDefaults(AhFitsFilePtr fff, AhFitsFilePtr sff, int rowcnt= 1);
  int sgdTestIO(AhFitsFilePtr outfile, SGDSF& sgdsf, AhFitsFilePtr infile, SGDFF& sgdff);
  int setByteTLMVecFromText(TLMbyteVec& vals, string& txtbytes);
  int setBitTLMVecFromText(TLMbyteVec& vals, string& txtbits);
  int sgdTestOutputColCells(vector<string>& col_names, vector<int>& cell_sizes, SGDSF& sgdsf, int hitcnt= 7, int actvchan= 17, int occurId= 77);
  //int sgdTestOutputColCells(InstrumWork& sfwork, SGDSF& sgdsf, int hitcnt= 7, int occurId= 77);
  //int sgdTestInputColCells(InstrumWork& ffwork, SGDFF& sgdff, int hitcnt= 7, int occurId= 77);

/** @} */

} /// namespace ahsgd

#endif
