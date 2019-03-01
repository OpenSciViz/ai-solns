#if !defined(IOSGD_C)
#define IOSGD_C const char ioSGD_rcsId_svnId[] = "$Id: ioSGD.cxx,v 1.5 2012/07/22 18:14:44 dhon Exp $"; 

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
AHBITS_H(ioSGD)

#include "ahtlm/ahSlice.h"
AHSLICE_H(ioSGD)

#include "ahtlm/ahTLM.h"
AHTLM_H(ioSGD)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(ioSGD)

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(ioSGD)

using namespace ahtlm;
using namespace ahsgd;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

// private-local namespace of heplers (which may get refactored to doInstrum or ahgen)
namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: ioSGD.cxx,v 1.5 2012/07/22 18:14:44 dhon Exp $";

  /// 
  ahtlm::Uint16 bits2int(const Bits4 bits) {
    //ahtlm::Uint16 ival = bits[0]; size_t width = (int)bits.size();
    //for( size_t i = 1; i < width; ++i ) if( bits[i] ) ival += i*2;
    ahtlm::Uint16 ival = (ahtlm::Uint16) bits.to_ulong();
    return ival;
  }


} // end private-local namespace of heplers

namespace ahsgd {

  //  return a place-holder of (allocated or not ?) telemtry slices for an entire row of ff and sf b-table
  //  for the specified instrument -- int return bool should indicate if row buffers are allocated
  //  (true) or not (false) -- use init() to setup column info and optionally alloc. buffers:  
  bool sgdTLMof(SGDSF& sgdsf, TLMbyteVec* raw_asic_data) {
    bool alloc = false;
    int maxSGDasic = (int)_SGDmaxAsic;
    int maxSGDchan = (int) _SGDmaxChan;
    int maxSGDraw = (int) _SGDmaxRawTLM;

    // positve length/size values indicate fixe-length buffs...
    // use cfitsio convention that negative typecodes --> neg. lengths indicate variable 
    // length/size buffs
 
    // SGD FF/SFF column 19 is the raw telemtry data buff -- check the allocation buffer pointer
    // to determine if a complete buffer allocation is indicated (o just place-holders
    sgdsf.col19.init(-maxSGDraw, "RAW_ASIC_DATA", "1PB(58656)"); // neg. max length of variable buff
    if( raw_asic_data ) { 
      // if a pre-allocated tlm buffer has been provided, reset/init the slice with it: 
      sgdsf.col19.initSlice((Ubyte*)raw_asic_data->data(), (int)raw_asic_data->size());
      //sgdsf.col19.setSlice(*raw_asic_data, "RAW_ASIC_DATA");
      alloc = true; // and proceed with full allocation
    }
  
    // note that fits table column names should NOT contain any white-space
    // this will be removed or commented-out after development & test 
    sgdsf.col01.init(128, "event_description");
    string describe = "SGD test FF-SFF ...............................................................................................................";
    if( alloc) sgdsf.col01.initSlice(describe);

    sgdsf.col02.init(1, "TIME");
    if( alloc) { vector<double> dvals(1, 2012.0); sgdsf.col02.initSlice(dvals); }

    sgdsf.col03.init(1, "S_TIME");
    if( alloc) { vector<double> dvals(1, 2012.0); sgdsf.col03.initSlice(dvals); }

    sgdsf.col04.init(20, "CCSDS_Packet_Header", "20B");
    if( alloc) { TLMbyteVec bvals(20, 42); sgdsf.col04.initSlice(bvals); }

    sgdsf.col05.init(1, "Time_Indicator");
    if( alloc) { vector<Uint32> ivals(1, 2014); sgdsf.col05.initSlice(ivals); }

    sgdsf.col06.init(1, "Occurence_ID");
    if( alloc) { vector<Uint32> ivals(1, 2014); sgdsf.col06.initSlice(ivals); }

    sgdsf.col07.init(1, "Local_Time");
    if( alloc) { vector<Uint32> ivals(1, 2014); sgdsf.col07.initSlice(ivals); }

    sgdsf.col08.init(1, "FLAG_SingleEvenUpset", "1B"); 
    if( alloc) { TLMbyteVec bvals(1, 8); sgdsf.col08.initSlice(bvals); }

    sgdsf.col09.init(1, "FLAG_LengthCheck", "1B"); // sgdsf.col09.init(5, "FLAG_LengthCheck", "1X");
    if( alloc) { TLMbyteVec bvals(1, 9); sgdsf.col09.initSlice(bvals); }

    sgdsf.col10.init(1, "FLAG_CalMode", "1B"); // sgdsf.col10.init(8, "FLAG_CalMode", "1X");
    if( alloc) { TLMbyteVec bvals(1, 10); sgdsf.col10.initSlice(bvals); }

    sgdsf.col11.init(1, "FLAG_InitialTriggerPattern", "1B"); // sgdsf.col11.init(8, "FLAG_TriggerPattern", "31X");
    if( alloc) { TLMbyteVec bvals(1, 11); sgdsf.col11.initSlice(bvals); }

    sgdsf.col12.init(1, "FLAG_TriggerPattern", "1B"); // sgdsf.col12.init(8, "FLAG_Trigger", "6X");
    if( alloc) { TLMbyteVec bvals(1, 12); sgdsf.col12.initSlice(bvals); }

    sgdsf.col13.init(1, "FLAG_CCBusy", "1B"); // sgdsf.col13.init(8, "FLAG_CCBusy", "3X");
    if( alloc) { TLMbyteVec bvals(1, 13); sgdsf.col13.initSlice(bvals); }

    sgdsf.col14.init(1, "FLAG_HitPatternCC", "1B"); // sgdsf.col12.init(2, "FLAG_HitPatternCC", "3X");
    if( alloc) { TLMbyteVec bvals(1, 14); sgdsf.col14.initSlice(bvals); }

    sgdsf.col15.init(1, "FLAG_PreciseShieldHitPattern", "1B"); // sgdsf.col12.init(2, "FLAG_HitPattern", "4X");
    if( alloc) { TLMbyteVec bvals(1, 15); sgdsf.col15.initSlice(bvals); }

    sgdsf.col16.init(1, "FLAG_FastShieldHitPattern", "1B"); // sgdsf.col13.init(2, "FLAG_FastBGO"); // , "4X");
    if( alloc) { TLMbyteVec bvals(1, 16); sgdsf.col16.initSlice(bvals); }
 
    sgdsf.col17.init(1, "LiveTime");
    if( alloc) { vector<Uint32> ivals(1, 2014); sgdsf.col17.initSlice(ivals); }

    sgdsf.col18.init(1, "nHitASICs", "1UI");
    if( alloc) { vector<Uint16> ivals(1, 18); sgdsf.col18.initSlice(ivals); }

    //
    // column 19: raw TLM buffer handled first -- to determine alloc bool behavior from arg...
    //

    sgdsf.col20.init(-maxSGDasic, "ASIC_Header_Data", "1PUJ(624)");
    if( alloc) { vector<Uint32> ivals(maxSGDasic, 20); sgdsf.col20.initSlice(ivals); }

    sgdsf.col21.init(-maxSGDasic, "Original_ASIC_IDs", "1PB(624)");
    if( alloc) { vector<Uint16> ivals(maxSGDasic, 21); sgdsf.col21.initSlice(ivals); }

    sgdsf.col22.init(-maxSGDasic, "Remapped_ASIC_IDs", "1PB(624)");
    if( alloc) { vector<Uint16> ivals(maxSGDasic, 22); sgdsf.col22.initSlice(ivals); }

    sgdsf.col23.init(-maxSGDasic, "Original_Channel_BitFlags", "1PUK(624)");
    if( alloc) { vector<Uint64> ivals(maxSGDasic, 23); sgdsf.col23.initSlice(ivals); }
 
    sgdsf.col24.init(-maxSGDchan, "Remapped_Channel_IDs", "1PUI(39936)"); 
    if( alloc) { vector<Uint16> ivals(maxSGDasic, 24); sgdsf.col24.initSlice(ivals); }

    sgdsf.col25.init(-maxSGDasic, "NumberOf_Active_Channels", "1PB(624)"); // max value of 64 for SGD
    if( alloc) { TLMbyteVec bvals(maxSGDasic, 25); sgdsf.col25.initSlice(bvals); }

    sgdsf.col26.init(-maxSGDasic, "Reference_Channels", "1PUI(624)"); 
    if( alloc) { vector<Uint16> ivals(maxSGDasic, 26); sgdsf.col26.initSlice(ivals); }

    sgdsf.col27.init(-maxSGDasic, "CommonNoise_Channels", "1PUI(624)");
    if( alloc) { vector<Uint16> ivals(maxSGDasic, 27); sgdsf.col27.initSlice(ivals); }

    sgdsf.col28.init(-maxSGDchan, "Raw_ADC_Pulse_Heights", "1PUI(39936)");
    if( alloc) { vector<Uint16> ivals(maxSGDasic, 28); sgdsf.col28.initSlice(ivals); }

    sgdsf.col29.init(-maxSGDchan, "Calibrated_ADC_Pulse_Heights", "1PUJ(39936)");
    //if( alloc) { vector<Uint16> ivals(maxSGDasic, 29); sgdsf.col29.initSlice(ivals); }

    return alloc; // no buffer allocation yet
  } // sgdTLMof -- telemetry i/o buffers as single struct of slice tyes

  int sgdColumnNames(vector<string>& ffcol_names, vector<string>& sfcol_names, SGDSF& sgdsf) {
    ffcol_names.clear(); sfcol_names.clear();
    sgdTLMof(sgdsf); // (re)set/init the default non-allocation struct
    ffcol_names.push_back(sgdsf.col01.name); ffcol_names.push_back(sgdsf.col02.name);
    ffcol_names.push_back(sgdsf.col03.name); ffcol_names.push_back(sgdsf.col04.name);
    ffcol_names.push_back(sgdsf.col05.name); ffcol_names.push_back(sgdsf.col06.name);
    ffcol_names.push_back(sgdsf.col07.name); ffcol_names.push_back(sgdsf.col08.name);
    ffcol_names.push_back(sgdsf.col09.name); ffcol_names.push_back(sgdsf.col10.name);
    ffcol_names.push_back(sgdsf.col11.name); ffcol_names.push_back(sgdsf.col12.name);
    ffcol_names.push_back(sgdsf.col13.name); ffcol_names.push_back(sgdsf.col14.name);
    ffcol_names.push_back(sgdsf.col15.name); ffcol_names.push_back(sgdsf.col16.name);
    // repeat and supplement for sff:
    sfcol_names.push_back(sgdsf.col01.name); sfcol_names.push_back(sgdsf.col02.name);
    sfcol_names.push_back(sgdsf.col03.name); sfcol_names.push_back(sgdsf.col04.name);
    sfcol_names.push_back(sgdsf.col05.name); sfcol_names.push_back(sgdsf.col06.name);
    sfcol_names.push_back(sgdsf.col07.name); sfcol_names.push_back(sgdsf.col08.name);
    sfcol_names.push_back(sgdsf.col09.name); sfcol_names.push_back(sgdsf.col10.name);
    sfcol_names.push_back(sgdsf.col11.name); sfcol_names.push_back(sgdsf.col12.name);
    sfcol_names.push_back(sgdsf.col13.name); sfcol_names.push_back(sgdsf.col14.name);
    sfcol_names.push_back(sgdsf.col15.name); sfcol_names.push_back(sgdsf.col16.name);
    sfcol_names.push_back(sgdsf.col17.name); sfcol_names.push_back(sgdsf.col18.name);
    sfcol_names.push_back(sgdsf.col19.name); sfcol_names.push_back(sgdsf.col20.name);

    sfcol_names.push_back(sgdsf.col21.name); sfcol_names.push_back(sgdsf.col22.name);
    sfcol_names.push_back(sgdsf.col21.name); sfcol_names.push_back(sgdsf.col24.name);
    sfcol_names.push_back(sgdsf.col21.name); sfcol_names.push_back(sgdsf.col26.name);
    return (int)sfcol_names.size();
  }

  /// read an SGD FF b-table row
  int sgdReadRow(AhFitsFilePtr ahf, SGDFF& sgdff, int rowIdx, int hduIdx) {
    /// for now, just perform the cfitsio calles here, then refactor later into ahfits, etc.
    fitsfile* pfits = fitsFileStructPtr(ahf);
    if( ! pfits ) { 
      clog << __func__ << "oops need open input (fits) file, abort"<<endl;
      return -1;
    }
    // check current row count
    int rowcnt = getRowCount(ahf);
    if( rowIdx >= rowcnt ) {
      clog << __func__ << "oops requested row: " << rowIdx << " exceeds available rows in b-table: " << rowcnt << "... abort"<<endl;
      return -2;
    }

    // use returned length against to set tlm::length ...
    sgdff.col01.length = readFITS(ahf, sgdff.col01.voidPtr(), sgdff.col01.capacity(), sgdff.col01.name, rowIdx, hduIdx);
    sgdff.col02.length = readFITS(ahf, sgdff.col02.voidPtr(), sgdff.col02.capacity(), sgdff.col02.name, rowIdx, hduIdx);
    sgdff.col03.length = readFITS(ahf, sgdff.col03.voidPtr(), sgdff.col03.capacity(), sgdff.col03.name, rowIdx, hduIdx);
    sgdff.col04.length = readFITS(ahf, sgdff.col04.voidPtr(), sgdff.col04.capacity(), sgdff.col04.name, rowIdx, hduIdx);
    sgdff.col05.length = readFITS(ahf, sgdff.col05.voidPtr(), sgdff.col05.capacity(), sgdff.col05.name, rowIdx, hduIdx);
    sgdff.col06.length = readFITS(ahf, sgdff.col06.voidPtr(), sgdff.col06.capacity(), sgdff.col06.name, rowIdx, hduIdx);
    sgdff.col07.length = readFITS(ahf, sgdff.col07.voidPtr(), sgdff.col07.capacity(), sgdff.col07.name, rowIdx, hduIdx);
    sgdff.col08.length = readFITS(ahf, sgdff.col08.voidPtr(), sgdff.col08.capacity(), sgdff.col08.name, rowIdx, hduIdx);
    sgdff.col09.length = readFITS(ahf, sgdff.col09.voidPtr(), sgdff.col09.capacity(), sgdff.col09.name, rowIdx, hduIdx);
    sgdff.col10.length = readFITS(ahf, sgdff.col10.voidPtr(), sgdff.col10.capacity(), sgdff.col10.name, rowIdx, hduIdx);
    sgdff.col11.length = readFITS(ahf, sgdff.col11.voidPtr(), sgdff.col11.capacity(), sgdff.col11.name, rowIdx, hduIdx);
    sgdff.col12.length = readFITS(ahf, sgdff.col12.voidPtr(), sgdff.col12.capacity(), sgdff.col12.name, rowIdx, hduIdx);
    sgdff.col13.length = readFITS(ahf, sgdff.col13.voidPtr(), sgdff.col13.capacity(), sgdff.col13.name, rowIdx, hduIdx);
    sgdff.col14.length = readFITS(ahf, sgdff.col14.voidPtr(), sgdff.col14.capacity(), sgdff.col14.name, rowIdx, hduIdx);
    sgdff.col15.length = readFITS(ahf, sgdff.col15.voidPtr(), sgdff.col15.capacity(), sgdff.col15.name, rowIdx, hduIdx);
    sgdff.col16.length = readFITS(ahf, sgdff.col16.voidPtr(), sgdff.col16.capacity(), sgdff.col16.name, rowIdx, hduIdx);
    sgdff.col17.length = readFITS(ahf, sgdff.col17.voidPtr(), sgdff.col17.capacity(), sgdff.col17.name, rowIdx, hduIdx);
    sgdff.col18.length = readFITS(ahf, sgdff.col18.voidPtr(), sgdff.col18.capacity(), sgdff.col18.name, rowIdx, hduIdx);
    sgdff.col19.length = readFITS(ahf, sgdff.col19.voidPtr(), sgdff.col19.capacity(), sgdff.col19.name, rowIdx, hduIdx);
    
    printRawTLM(sgdff.col19);

    return sgdff.colCount();
  } // sgdReadRow FF

  /// read an SGD SF b-table row
  int sgdReadRow(AhFitsFilePtr ahf, SGDSF& sgdsf, int rowIdx, int hduIdx) {
    /// for now, just perform the cfitsio calles here, then refactor later into ahfits, etc.
    fitsfile* pfits = fitsFileStructPtr(ahf);
    if( ! pfits ) { 
      clog << __func__ << "oops need open input (fits) file, abort"<<endl;
      return -1;
    }
    // check current row count
    int rowcnt = getRowCount(ahf);
    if( rowIdx >= rowcnt ) {
      clog << __func__ << "oops requested row: " << rowIdx << " exceeds available rows in b-table: " << rowcnt << "... abort"<<endl;
      return -2;
    }

    // ff cols of row:
    sgdReadRow(ahf, (SGDFF&) sgdsf, rowIdx, hduIdx);
    
    // use returned length against to set tlm::length ...
    sgdsf.col20.length = readFITS(ahf, sgdsf.col20.voidPtr(), sgdsf.col20.capacity(), sgdsf.col20.name, rowIdx, hduIdx);
    sgdsf.col21.length = readFITS(ahf, sgdsf.col21.voidPtr(), sgdsf.col21.capacity(), sgdsf.col21.name, rowIdx, hduIdx);
    sgdsf.col22.length = readFITS(ahf, sgdsf.col22.voidPtr(), sgdsf.col22.capacity(), sgdsf.col22.name, rowIdx, hduIdx);
    sgdsf.col23.length = readFITS(ahf, sgdsf.col23.voidPtr(), sgdsf.col23.capacity(), sgdsf.col23.name, rowIdx, hduIdx);
    sgdsf.col24.length = readFITS(ahf, sgdsf.col24.voidPtr(), sgdsf.col24.capacity(), sgdsf.col24.name, rowIdx, hduIdx);
    sgdsf.col25.length = readFITS(ahf, sgdsf.col25.voidPtr(), sgdsf.col25.capacity(), sgdsf.col25.name, rowIdx, hduIdx);
    sgdsf.col26.length = readFITS(ahf, sgdsf.col26.voidPtr(), sgdsf.col26.capacity(), sgdsf.col26.name, rowIdx, hduIdx);
    sgdsf.col27.length = readFITS(ahf, sgdsf.col27.voidPtr(), sgdsf.col27.capacity(), sgdsf.col27.name, rowIdx, hduIdx);
    sgdsf.col28.length = readFITS(ahf, sgdsf.col28.voidPtr(), sgdsf.col28.capacity(), sgdsf.col28.name, rowIdx, hduIdx);
    // don't do any calibration processing now
    //sgdsf.col29.length = readFITS(ahf, sgdsf.col29.voidPtr(), sgdsf.col29.capacity(), sgdsf.col29.name, rowIdx, hduIdx);

    return sgdsf.colCount();
  } // sgdReadRow SF

  /// write an SGD FF b-table row
  int sgdWriteRow(AhFitsFilePtr ahf, SGDFF& sgdff, int rowIdx, int hduIdx) { 
    /// for now, just perform the cfitsio calles here, then refactor later into ahfits, etc.
    fitsfile* pfits = fitsFileStructPtr(ahf);
    if( ! pfits ) { 
      clog << __func__ << "oops need open output (fits) file, abort"<<endl;
      return -1;
    }
    int colIdx = 0;
    writeFITS(ahf, sgdff.col01.voidPtr(), sgdff.col01.length, sgdff.col01.name, sgdff.col01.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col02.voidPtr(), sgdff.col02.length, sgdff.col02.name, sgdff.col02.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col03.voidPtr(), sgdff.col03.length, sgdff.col03.name, sgdff.col03.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col04.voidPtr(), sgdff.col04.length, sgdff.col04.name, sgdff.col04.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col05.voidPtr(), sgdff.col05.length, sgdff.col05.name, sgdff.col05.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col06.voidPtr(), sgdff.col06.length, sgdff.col06.name, sgdff.col06.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col07.voidPtr(), sgdff.col07.length, sgdff.col07.name, sgdff.col07.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col08.voidPtr(), sgdff.col08.length, sgdff.col08.name, sgdff.col08.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col09.voidPtr(), sgdff.col09.length, sgdff.col09.name, sgdff.col09.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col10.voidPtr(), sgdff.col10.length, sgdff.col10.name, sgdff.col10.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col11.voidPtr(), sgdff.col11.length, sgdff.col11.name, sgdff.col11.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col12.voidPtr(), sgdff.col12.length, sgdff.col12.name, sgdff.col12.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col13.voidPtr(), sgdff.col13.length, sgdff.col13.name, sgdff.col13.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col14.voidPtr(), sgdff.col14.length, sgdff.col14.name, sgdff.col14.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col15.voidPtr(), sgdff.col15.length, sgdff.col15.name, sgdff.col15.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col16.voidPtr(), sgdff.col16.length, sgdff.col16.name, sgdff.col16.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col17.voidPtr(), sgdff.col17.length, sgdff.col17.name, sgdff.col16.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col18.voidPtr(), sgdff.col18.length, sgdff.col18.name, sgdff.col16.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdff.col19.voidPtr(), sgdff.col19.length, sgdff.col19.name, sgdff.col16.format, colIdx++, rowIdx, hduIdx);

    return colIdx; //sgdff.colCount();
  } // sgdWriteRow FF

  /// write an SGD SF b-table row
  int sgdWriteRow(AhFitsFilePtr ahf, SGDSF& sgdsf, int rowIdx, int hduIdx) { 
     fitsfile* pfits = fitsFileStructPtr(ahf);
     if( ! pfits ) { 
       clog << __func__ << "oops need open output (fits) file, abort"<<endl;
       return -1;
    }

    // check current row count
    getRowCount(ahf);
 
    // ff cols of row:
    sgdWriteRow(ahf, (SGDFF&) sgdsf, rowIdx, hduIdx);
    
    // sf cols:
    int colIdx = 19;
    writeFITS(ahf, sgdsf.col20.voidPtr(), sgdsf.col20.length, sgdsf.col20.name, sgdsf.col20.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col21.voidPtr(), sgdsf.col21.length, sgdsf.col21.name, sgdsf.col21.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col22.voidPtr(), sgdsf.col22.length, sgdsf.col22.name, sgdsf.col22.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col23.voidPtr(), sgdsf.col23.length, sgdsf.col23.name, sgdsf.col23.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col24.voidPtr(), sgdsf.col24.length, sgdsf.col24.name, sgdsf.col24.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col25.voidPtr(), sgdsf.col25.length, sgdsf.col25.name, sgdsf.col25.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col26.voidPtr(), sgdsf.col26.length, sgdsf.col26.name, sgdsf.col26.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col27.voidPtr(), sgdsf.col27.length, sgdsf.col27.name, sgdsf.col27.format, colIdx++, rowIdx, hduIdx);
    writeFITS(ahf, sgdsf.col28.voidPtr(), sgdsf.col28.length, sgdsf.col28.name, sgdsf.col28.format, colIdx++, rowIdx, hduIdx);
    // don't do any calibration processing now
    //writeFITS(ahf, sgdsf.col29.voidPtr(), sgdsf.col29.length, sgdsf.col29.name, sgdsf.col29.format, colIdx++, rowIdx, hduIdx);

    return colIdx; //sgdsf.colCount();
  } //  sgdWriteRow SF

} // end namespace ahsgd

#endif // IOSGD_C
