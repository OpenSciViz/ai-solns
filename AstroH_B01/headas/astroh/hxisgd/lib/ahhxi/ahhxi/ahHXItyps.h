#if !defined(AHHXITYPS_H)
#define AHHXITYPS_H(arg) const char arg##AHHXITYPS_rcsId_svnId[] = "$Id: ahHXItyps.h,v 1.14 2012/07/20 20:58:49 dhon Exp $"; 

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

/// \ingroup mod_ahhxi
namespace ahhxi {

/** \addtogroup mod_ahhxi
  *  @{
  */

  //const int _maxHXIraw  = 2014; // 1120; // byte count
  //const int _maxHXIchan = 1280; // int16 count
    
  /// each hxi asic event telemtry blob/slice has a 3*16 == 48bit header
  /// offsets are bits counts from header first (least sig.) bit..
  
  const size_t _HXIASICTLMheaderBytes = 6; // up to 40 such headers in an occurance tlm buff
  const size_t _HXITLMheaderBytes = 14; // occurance tlm header is 7 shorts
  const size_t _HXInumLayer = 5;
  const size_t _HXInumAsicPerLayer = 8;
  const size_t _HXImaxAsic = _HXInumLayer * _HXInumAsicPerLayer; // 40 == 5*8
  const size_t _HXInumChanPerAsic = 32; 
  const size_t _HXImaxChan = _HXImaxAsic * _HXInumChanPerAsic;  // int16 count of 1280 == 40*32
  const size_t _HXImaxRawTLM = 2014; // byte count June 2012 consensus // 1120; // byte count April & May 2012

  inline string hxiFFVersion(string path= "./input/") { return path + "HXI_FF2012June.tpl"; }
  inline string hxiSFVersion(string path= "./input/") { return path + "HXI_SF2012June.tpl"; }

//////////////////  HXI FF and SFF structs associated with default FITS templates:
//
//  this corresponds to the April-May 2012 version of the FITS templates - updated
//  to reflect June 15 2012 discussion -- hon (after re-org. of directories and libs)
//
//  typedef struct {
  struct HXIFF {
    TextTLM   col01; // event_description (for test-id and ?)
    Real64TLM col02; // TIME
    Real64TLM col03; // S_TIME
    ByteTLM   col04; // CCSDS_Packet_Header
    Int32TLM  col05; // Time_Indicator
    Int32TLM  col06; // Occurence_ID
    Int32TLM  col07; // Local_Time
    ByteTLM   col08; // FLAG_SingleEvenUpset // BitTLM    col08;
    ByteTLM   col09; // FLAG_LengthCheck // BitTLM    col09;
    ByteTLM   col10; // FLAG_InitialTrigger // BitTLM    col10;
    ByteTLM   col11; // FLAG_TriggerPattern // BitTLM    col11;
    ByteTLM   col12; // FLAG_PreciseShieldHitPattern  // BitTLM    col12;
    ByteTLM   col13; // FLAG_FastShieldHitPattern // BitTLM    col13;
    Int32TLM  col14; // LiveTime
    ByteTLM   col15; // nHitASICs
    ByteTLM   col16; // RAW_ASIC_DATA
    inline int colCount() { return 16; }
    inline ~HXIFF() {}
//  } HXIFF;
  };

//  typedef struct  { 
//    HXIFF ff;
  struct HXISF : HXIFF {
    Int32TLM  col17; // ASIC_Header // ByteTLM   col18;
    ByteTLM   col18; // Original_ASIC_IDs 
    ByteTLM   col19; // Remapped_ASIC_IDs
    Int32TLM  col20; // Original_Channel_BitFlags // not IDs
    Int16TLM  col21; // Remapped_Channel_IDs
    ByteTLM   col22; // NumberOf_Active_Channels
    Int16TLM  col23; // Reference_Channel
    Int16TLM  col24; // CommonNoise_Channel
    Int16TLM  col25; // Raw_ADC_Pulse_Heights
    Int16TLM  col26; // Calibrated_ADC_Pulse_Heights
    inline int colCount() { return 26; }
    inline ~HXISF() {}
//  } HXISF;
  };

  //
  // HXItemplate.cxx:
  int initHXItemplates(string& content, string& fff_filename, string& sff_filename);

  //
  ////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in lutHXI.cxx -- i.e. tlm offset(s) lut(s)
  //
  int asicCountOffset();
  int firstEventOffset();
  int firstActvChanFlagOffset();
  int nextEventOffset(vector<int>& byte_offsets, int current_offset= 0);
  //int nextEventOffset(const TLMbyteVec& rawtlm, int current_offset= 0);
  // still need this or no?
  //inline int hxiNextEventOffset(const TLMbyteVec& rawtlm, int current_offset= 0) { return nextEventOffset(rawtlm, current_offset); } 
  int hxiEventHdrOf(Bits48& ehdr, const TLMbyteVec& tlm, vector<int>& byte_offsets, int tlm_offset= _HXITLMheaderBytes);
  int singleAsicBuffBitCount(int actv_cnt);
  int getActiveChanIds(vector<Uint16>& actv_chanIds, Bits32& bitflags);
  // returns hitAsics int count, and scans tlm to establish asic sub-buff byte offfset from
  // active channel bitflags and returns assoc. vecs:
  int asicBuffInfo(vector<int>& byte_offsets, vector<Bits32>& occur_chan_bitflags, const TLMbyteVec& raw_asic_data);

  //
  ////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in ioHXI.cxx
  //
  int hxiReadRow(AhFitsFilePtr ahf, HXIFF& hxiff, int rowIdx= 0, int hduIdx= 1);
  inline int hxiReadRow(AhFitsFilePtr ahf, HXIFF* hxiff, int rowIdx= 0, int hduIdx= 1) { if( hxiff ) return hxiReadRow(ahf, *hxiff, rowIdx, hduIdx); return -1; }
  int hxiReadRow(AhFitsFilePtr ahf, HXISF& hxiff, int rowIdx= 0, int hduIdx= 1);
  inline int hxiReadRow(AhFitsFilePtr ahf, HXISF* hxisf, int rowIdx= 0, int hduIdx= 1) { if( hxisf ) return hxiReadRow(ahf, *hxisf, rowIdx, hduIdx); return -1; }
  int hxiWriteRow(AhFitsFilePtr ahf, HXIFF& hxiff, int rowIdx= 0, int hduIdx= 1);
  
  inline int hxiWriteRow(AhFitsFilePtr ahf, HXIFF* hxiff, int rowIdx= 0, int hduIdx= 1) { if( hxiff ) return hxiWriteRow(ahf, *hxiff, rowIdx, hduIdx); return -1; }
  int hxiWriteRow(AhFitsFilePtr ahf, HXISF& hxisf, int rowIdx= 0, int hduIdx= 1);
  inline int hxiWriteRow(AhFitsFilePtr ahf, HXISF* hxisf, int rowIdx= 0, int hduIdx= 1) { if( hxisf ) return hxiWriteRow(ahf, *hxisf, rowIdx, hduIdx); return -1; }

  bool hxiTLMof(HXISF& hxisf, TLMbyteVec* raw_asic_data = 0); // reusable raw data buff ptr != 0 (max size) 
  inline bool hxiTLMof(HXISF& hxisf, TLMbyteVec& raw_asic_data) { return hxiTLMof(hxisf, &raw_asic_data); }
  inline bool hxiTLMof(HXISF* hxisf, TLMbyteVec& raw_asic_data) { if( hxisf ) return hxiTLMof(*hxisf, raw_asic_data); return false; }
  inline bool hxiTLMof(HXISF* hxisf, TLMbyteVec* raw_asic_data) { if( hxisf ) return hxiTLMof(*hxisf, raw_asic_data); return false; }
  int hxiColumnNames(vector<string>& ffcol_names, vector<string>&  sfcol_names, HXISF& hxisf);
  
  //
  ////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in doHXI.cxx
  //
  /// return a place-holder of unallocated telemtry slices for an entire row of ff and sf b-table
  //  for the specified instrument -- int return bool should indicate if row buffers are allocated
  //  (true) or not (false) -- use init() to setup column info and optionally alloc. buffers:  
  //
  //int doHXIwork(InstrumWork& input, InstrumWork& output);
  int doHXIwork(AhFitsFilePtr infile, AhFitsFilePtr outfile);
  int hxiFF2SF(HXISF& hxisf, TLMbyteVec& raw_asic_data);
  inline int hxiFF2SF(HXISF& hxisf, TLMbyteVec* data) { if( data ) return hxiFF2SF(hxisf, *data); return -1; }
  inline int hxiFF2SF(HXISF* hxisf, TLMbyteVec& data) { if( hxisf ) return hxiFF2SF(*hxisf, data); return -1; }
  inline int hxiFF2SF(HXISF* hxisf, TLMbyteVec* data) { if( hxisf && data ) return hxiFF2SF(*hxisf, *data); return -1; }
  // asic buffer funcs (some deprecated)
  Uint16 hxiLayerIdxFromTLMhdr(const Bits48 bits);
  Uint16 hxiAsicIdxFromTLMhdr(const Bits48 bits); 
  Uint16 hxiAsicRemap(const Bits48 bits);
  int hxiAsicVecReMap(vector<Uint16>& remap, vector<Uint16>& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets);
  int hxiAsicVecReMap(TLMbyteVec& remap, TLMbyteVec& orig, const TLMbyteVec& rawtlm, vector<int>& byte_offsets);
  int hxiChanReMap(vector<Uint16>& remap, vector<Uint16>& orig, const Bits48 bits);
  int hxiAsicHeaders(Int32TLM& asic_hdrs, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int hxiOrigAsicIds(ByteTLM& orig_asicIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int hxiRemapAsicIds(ByteTLM& remapped_asicIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  // channel funcs
  int countActiveChans(vector<Bits32>& chanflags);
  int hxiOrigChanFlags(Int32TLM& orig_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int hxiRemapChanIds(Int16TLM& remapped_chanIds, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  // not sure if this is needed -- preserve bit flags or integer ids (10 bit orig. tlm to 16 bit int)? 
  //int hxiOrigChanIds(Int16TLM& orig_chanIds, vector<Bits32>& occur_chan_bitflags);
  int hxiActiveChanCount(ByteTLM& actv_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int hxiRefChans(Int16TLM& ref_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int hxiNoiseChans(Int16TLM& noise_chans, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  // channel adc pulse data
  int hxiRawADCPulses(vector<Uint16>& raw_adc_pulse_heights, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  //int hxiCalibPulses(vector<Uint32>& cal_adc_pulse_heights, vector<Bits32>& occur_chan_bitflags, TLMbyteVec& raw_asic_data, int offset);
  int hxiCalibPulses(Int16TLM& calib_pulse_heights, const Int16TLM& raw_adc_pulse_heights, const Int16TLM& noise_chans, const Int16TLM& ref_chans);

  //
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // these funcs are implement in doHXItest.cxx
  //
  AhFitsFilePtr hxiCreateTestfile(string& filename, string& template_filename, HXIFF& hxiff, bool clobber= true);
  AhFitsFilePtr hxiCreateTestfile(string& filename, string& template_filename, HXISF& hxisf, bool clobber= true);
  int hxiWriteTestDefaults(AhFitsFilePtr fff, AhFitsFilePtr sff, int rowcnt=1);
  int hxiTestIO(AhFitsFilePtr outfile, HXISF& hxisf, AhFitsFilePtr infile, HXIFF& hxiff);
  int setByteTLMVecFromText(TLMbyteVec& vals, string& txtbytes);
  int setBitTLMVecFromText(TLMbyteVec& vals, string& txtbits);
  int hxiTestOutputColCells(vector<string>& col_names, vector<int>& cell_sizes, HXISF& hxisf, int hitcnt= 7, int actvchan= 17, int occurId= 77);
  //int hxiTestOutputColCells(InstrumWork& sfwork, HXISF& hxisf, int hitcnt= 7, int occurId= 77);
  //int hxiTestInputColCells(InstrumWork& ffwork, HXIFF& hxiff, int hitcnt= 7, int occurId= 77);

/** @} */

} /// namespace ahhxi

#endif
