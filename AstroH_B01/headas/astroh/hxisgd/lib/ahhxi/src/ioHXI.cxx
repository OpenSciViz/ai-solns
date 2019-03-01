#if !defined(IOHXI_C)
#define IOHXI_C const char ioHXI_rcsId_svnId[] = "$Id: ioHXI.cxx,v 1.6 2012/07/22 18:14:44 dhon Exp $"; 

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
AHBITS_H(ioHXI)

#include "ahtlm/ahSlice.h"
AHSLICE_H(ioHXI)

#include "ahtlm/ahTLM.h"
AHTLM_H(ioHXI)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(ioHXI)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(ioHXI)

using namespace ahtlm;
using namespace ahhxi;
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

// private-local namespace of heplers (which may get refactored to doInstrum or ahgen)
namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: ioHXI.cxx,v 1.6 2012/07/22 18:14:44 dhon Exp $";

  /// 
  ahtlm::Uint16 bits2int(const Bits4 bits) {
    //ahtlm::Uint16 ival = bits[0]; size_t width = (int)bits.size();
    //for( size_t i = 1; i < width; ++i ) if( bits[i] ) ival += i*2;
    ahtlm::Uint16 ival = (ahtlm::Uint16) bits.to_ulong();
    return ival;
  }


} // end private-local namespace of heplers

namespace ahhxi {

  //  return a place-holder of (allocated or not ?) telemtry slices for an entire row of ff and sf b-table
  //  for the specified instrument -- int return bool should indicate if row buffers are allocated
  //  (true) or not (false) -- use init() to setup column info and optionally alloc. buffers:  
  bool hxiTLMof(HXISF& hxisf, TLMbyteVec* raw_asic_data) {
    bool alloc = false;
    int maxHXIasic = (int)_HXImaxAsic;
    int maxHXIchan = (int) _HXImaxChan;
    int maxHXIraw = (int) _HXImaxRawTLM;

    // positve length/size values indicate fixe-length buffs...
    // use cfitsio convention that negative typecodes --> neg. lengths indicate variable 
    // length/size buffs
 
    // column 16 is the raw telemtry data buff -- check the allocation buffer pointer
    // to determine if a complete buffer allocation is indicated (o just place-holders
    // hxisf.col16.init(-maxHXIraw, "RAW_ASIC_DATA", "1PB(1120)");
    hxisf.col16.init(-maxHXIraw, "RAW_ASIC_DATA", "1PB(2014)"); // neg. max length of variable buff
    if( raw_asic_data ) { 
      // if a pre-allocated tlm buffer has been provided, reset/init the slice with it: 
      hxisf.col16.initSlice((Ubyte*)raw_asic_data->data(), (int)raw_asic_data->size());
      alloc = true; // and proceed with full allocation
    }
  
    // note that fits table column names should NOT contain any white-space
    // this will be removed or commented-out after development & test 
    hxisf.col01.init(128, "event_description");
    string describe = "HXI test FF-SFF ...............................................................................................................";
    if( alloc) hxisf.col01.initSlice(describe); // 128

    hxisf.col02.init(1, "TIME");
    if( alloc) { vector<double> dvals(1, 2012.0); hxisf.col02.initSlice(dvals); }

    hxisf.col03.init(1, "S_TIME");
    if( alloc) { vector<double> dvals(1, 2012.0); hxisf.col03.initSlice(dvals); }

    hxisf.col04.init(20, "CCSDS_Packet_Header", "20B");
    if( alloc) { TLMbyteVec bvals(20, 42); hxisf.col04.initSlice(bvals); }

    hxisf.col05.init(1, "Time_Indicator");
    if( alloc) { vector<Uint32> ivals(1, 2014); hxisf.col05.initSlice(ivals); }

    hxisf.col06.init(1, "Occurence_ID");
    if( alloc) { vector<Uint32> ivals(1, 2014); hxisf.col06.initSlice(ivals); }

    hxisf.col07.init(1, "Local_Time");
    if( alloc) { vector<Uint32> ivals(1, 2014); hxisf.col07.initSlice(ivals); }

    hxisf.col08.init(1, "FLAG_SingleEvenUpset", "1B"); // hxisf.col08.init(5, "FLAG_SEU", "5X");
    if( alloc) { TLMbyteVec bvals(1, 8); hxisf.col08.initSlice(bvals); }

    hxisf.col09.init(1, "FLAG_LengthCheck", "1B"); // hxisf.col09.init(5, "FLAG_LengthCheck", "5X");
    if( alloc) { TLMbyteVec bvals(1, 9); hxisf.col09.initSlice(bvals); }

    hxisf.col10.init(1, "FLAG_InitialTriggerPattern", "1B"); // hxisf.col10.init(8, "FLAG_TriggerPattern", "8X");
    if( alloc) { TLMbyteVec bvals(1, 10); hxisf.col10.initSlice(bvals); }

    hxisf.col11.init(1, "FLAG_TriggerPattern", "1B"); // hxisf.col11.init(8, "FLAG_Trigger", "8X");
    if( alloc) { TLMbyteVec bvals(1, 11); hxisf.col11.initSlice(bvals); }

    hxisf.col12.init(1, "FLAG_PreciseShieldHitPattern", "1B"); // hxisf.col12.init(2, "FLAG_HitPattern"); // , "2X");
    if( alloc) { TLMbyteVec bvals(1, 12); hxisf.col12.initSlice(bvals); }

    hxisf.col13.init(1, "FLAG_FastShieldHitPattern", "1B"); // hxisf.col13.init(2, "FLAG_FastBGO"); // , "2X");
    if( alloc) { TLMbyteVec bvals(1, 13); hxisf.col13.initSlice(bvals); }

    hxisf.col14.init(1, "LiveTime");
    if( alloc) { vector<Uint32> ivals(1, 2014); hxisf.col14.initSlice(ivals); }

    hxisf.col15.init(1, "nHitASICs", "1B");
    if( alloc) { TLMbyteVec bvals(1, 15); hxisf.col15.initSlice(bvals); }

    //
    // column 16: raw TLM buffer handled first -- to determine alloc bool behavior from arg...
    //

    hxisf.col17.init(-maxHXIasic, "ASIC_Header_Data", "1PUJ(40)");
    if( alloc) { vector<Uint32> ivals(maxHXIasic, 17); hxisf.col17.initSlice(ivals); }

    hxisf.col18.init(-maxHXIasic, "Original_ASIC_IDs", "1PB(40)");
    if( alloc) { TLMbyteVec bvals(maxHXIasic, 18); hxisf.col18.initSlice(bvals); }

    hxisf.col19.init(-maxHXIasic, "Remapped_ASIC_IDs", "1PB(40)");
    if( alloc) { TLMbyteVec bvals(maxHXIasic, 19); hxisf.col19.initSlice(bvals); }

    //hxisf.col20.init(-maxHXIchan, "Original_Channel_Ids", "1PUI(1280)");
    hxisf.col20.init(-maxHXIasic, "Original_Channel_BitFlags", "1PUJ(40)");
    if( alloc) { vector<Uint32> ivals(maxHXIasic, 20); hxisf.col20.initSlice(ivals); }
 
    hxisf.col21.init(-maxHXIchan, "Remapped_Channel_IDs", "1PUI(1280)"); 
    if( alloc) { vector<Uint16> ivals(maxHXIasic, 21); hxisf.col21.initSlice(ivals); }

    hxisf.col22.init(-maxHXIasic, "NumberOf_Active_Channels", "1PB(40)"); // max of 32 for HXI and 64 for SGD
    if( alloc) { TLMbyteVec bvals(maxHXIasic, 22); hxisf.col22.initSlice(bvals); }

    hxisf.col23.init(-maxHXIasic, "Reference_Channels", "1PUI(40)"); // hxisf.col18.init(-40, "Original_ASIC_IDs" "1PB(40)");
    if( alloc) { vector<Uint16> ivals(maxHXIasic, 23); hxisf.col23.initSlice(ivals); }

    hxisf.col24.init(-maxHXIasic, "CommonNoise_Channels", "1PUI(40)");
    if( alloc) { vector<Uint16> ivals(maxHXIasic, 24); hxisf.col24.initSlice(ivals); }

    hxisf.col25.init(-maxHXIchan, "Raw_ADC_Pulse_Heights", "1PUI(1280)");
    if( alloc) { vector<Uint16> ivals(maxHXIasic, 25); hxisf.col25.initSlice(ivals); }

    hxisf.col26.init(-maxHXIchan, "Calibrated_ADC_Pulse_Heights", "1PUJ(1280)");
    //if( alloc) { vector<Uint16> ivals(maxHXIasic, 26); hxisf.col26.initSlice(ivals); }

    return alloc; // no buffer allocation yet
  } // hxiTLMof -- telemetry i/o buffers as single struct of slice tyes

  int hxiColumnNames(vector<string>& ffcol_names, vector<string>& sfcol_names, HXISF& hxisf) {
    ffcol_names.clear(); sfcol_names.clear();
    hxiTLMof(hxisf); // (re)set/init the default non-allocation struct
    ffcol_names.push_back(hxisf.col01.name); ffcol_names.push_back(hxisf.col02.name);
    ffcol_names.push_back(hxisf.col03.name); ffcol_names.push_back(hxisf.col04.name);
    ffcol_names.push_back(hxisf.col05.name); ffcol_names.push_back(hxisf.col06.name);
    ffcol_names.push_back(hxisf.col07.name); ffcol_names.push_back(hxisf.col08.name);
    ffcol_names.push_back(hxisf.col09.name); ffcol_names.push_back(hxisf.col10.name);
    ffcol_names.push_back(hxisf.col11.name); ffcol_names.push_back(hxisf.col12.name);
    ffcol_names.push_back(hxisf.col13.name); ffcol_names.push_back(hxisf.col14.name);
    ffcol_names.push_back(hxisf.col15.name); ffcol_names.push_back(hxisf.col16.name);
    // repeat and supplement for sff:
    sfcol_names.push_back(hxisf.col01.name); sfcol_names.push_back(hxisf.col02.name);
    sfcol_names.push_back(hxisf.col03.name); sfcol_names.push_back(hxisf.col04.name);
    sfcol_names.push_back(hxisf.col05.name); sfcol_names.push_back(hxisf.col06.name);
    sfcol_names.push_back(hxisf.col07.name); sfcol_names.push_back(hxisf.col08.name);
    sfcol_names.push_back(hxisf.col09.name); sfcol_names.push_back(hxisf.col10.name);
    sfcol_names.push_back(hxisf.col11.name); sfcol_names.push_back(hxisf.col12.name);
    sfcol_names.push_back(hxisf.col13.name); sfcol_names.push_back(hxisf.col14.name);
    sfcol_names.push_back(hxisf.col15.name); sfcol_names.push_back(hxisf.col16.name);
    sfcol_names.push_back(hxisf.col17.name); sfcol_names.push_back(hxisf.col18.name);
    sfcol_names.push_back(hxisf.col19.name); sfcol_names.push_back(hxisf.col20.name);

    sfcol_names.push_back(hxisf.col21.name); sfcol_names.push_back(hxisf.col22.name);
    sfcol_names.push_back(hxisf.col21.name); sfcol_names.push_back(hxisf.col24.name);
    sfcol_names.push_back(hxisf.col21.name); sfcol_names.push_back(hxisf.col26.name);
    return (int)sfcol_names.size();
  }

  /// read an HXI FF b-table row
  int hxiReadRow(AhFitsFilePtr ahf, HXIFF& hxiff, int rowIdx, int hduIdx) {
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
    hxiff.col01.length = readFITS(ahf, hxiff.col01.voidPtr(), hxiff.col01.capacity(), hxiff.col01.name, rowIdx, hduIdx);
    hxiff.col02.length = readFITS(ahf, hxiff.col02.voidPtr(), hxiff.col02.capacity(), hxiff.col02.name, rowIdx, hduIdx);
    hxiff.col03.length = readFITS(ahf, hxiff.col03.voidPtr(), hxiff.col03.capacity(), hxiff.col03.name, rowIdx, hduIdx);
    hxiff.col04.length = readFITS(ahf, hxiff.col04.voidPtr(), hxiff.col04.capacity(), hxiff.col04.name, rowIdx, hduIdx);
    hxiff.col05.length = readFITS(ahf, hxiff.col05.voidPtr(), hxiff.col05.capacity(), hxiff.col05.name, rowIdx, hduIdx);
    hxiff.col06.length = readFITS(ahf, hxiff.col06.voidPtr(), hxiff.col06.capacity(), hxiff.col06.name, rowIdx, hduIdx);
    hxiff.col07.length = readFITS(ahf, hxiff.col07.voidPtr(), hxiff.col07.capacity(), hxiff.col07.name, rowIdx, hduIdx);
    hxiff.col08.length = readFITS(ahf, hxiff.col08.voidPtr(), hxiff.col08.capacity(), hxiff.col08.name, rowIdx, hduIdx);
    hxiff.col09.length = readFITS(ahf, hxiff.col09.voidPtr(), hxiff.col09.capacity(), hxiff.col09.name, rowIdx, hduIdx);
    hxiff.col10.length = readFITS(ahf, hxiff.col10.voidPtr(), hxiff.col10.capacity(), hxiff.col10.name, rowIdx, hduIdx);
    hxiff.col11.length = readFITS(ahf, hxiff.col11.voidPtr(), hxiff.col11.capacity(), hxiff.col11.name, rowIdx, hduIdx);
    hxiff.col12.length = readFITS(ahf, hxiff.col12.voidPtr(), hxiff.col12.capacity(), hxiff.col12.name, rowIdx, hduIdx);
    hxiff.col13.length = readFITS(ahf, hxiff.col13.voidPtr(), hxiff.col13.capacity(), hxiff.col13.name, rowIdx, hduIdx);
    hxiff.col14.length = readFITS(ahf, hxiff.col14.voidPtr(), hxiff.col14.capacity(), hxiff.col14.name, rowIdx, hduIdx);
    hxiff.col15.length = readFITS(ahf, hxiff.col15.voidPtr(), hxiff.col15.capacity(), hxiff.col15.name, rowIdx, hduIdx);
    hxiff.col16.length = readFITS(ahf, hxiff.col16.voidPtr(), hxiff.col16.capacity(), hxiff.col16.name, rowIdx, hduIdx);
    
    printRawTLM(hxiff.col16);

    return hxiff.colCount();
  } // hxiReadRow FF

  /// read an HXI SF b-table row
  int hxiReadRow(AhFitsFilePtr ahf, HXISF& hxisf, int rowIdx, int hduIdx) {
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
    hxiReadRow(ahf, (HXIFF&) hxisf, rowIdx, hduIdx);
    
    // use returned length against to set tlm::length ...
    hxisf.col17.length = readFITS(ahf, hxisf.col17.voidPtr(), hxisf.col17.capacity(), hxisf.col17.name, rowIdx, hduIdx);
    hxisf.col18.length = readFITS(ahf, hxisf.col18.voidPtr(), hxisf.col18.capacity(), hxisf.col18.name, rowIdx, hduIdx);
    hxisf.col19.length = readFITS(ahf, hxisf.col19.voidPtr(), hxisf.col19.capacity(), hxisf.col19.name, rowIdx, hduIdx);
    hxisf.col20.length = readFITS(ahf, hxisf.col20.voidPtr(), hxisf.col20.capacity(), hxisf.col20.name, rowIdx, hduIdx);
    hxisf.col21.length = readFITS(ahf, hxisf.col21.voidPtr(), hxisf.col21.capacity(), hxisf.col21.name, rowIdx, hduIdx);
    hxisf.col22.length = readFITS(ahf, hxisf.col22.voidPtr(), hxisf.col22.capacity(), hxisf.col22.name, rowIdx, hduIdx);
    hxisf.col23.length = readFITS(ahf, hxisf.col23.voidPtr(), hxisf.col23.capacity(), hxisf.col23.name, rowIdx, hduIdx);
    hxisf.col24.length = readFITS(ahf, hxisf.col24.voidPtr(), hxisf.col24.capacity(), hxisf.col24.name, rowIdx, hduIdx);
    hxisf.col25.length = readFITS(ahf, hxisf.col25.voidPtr(), hxisf.col25.capacity(), hxisf.col25.name, rowIdx, hduIdx);
    // no calibration processingi/testing for build 1
    //hxisf.col26.length = readFITS(ahf, hxisf.col26.voidPtr(), hxisf.col26.capacity(), hxisf.col26.name, rowIdx, hduIdx);

    return hxisf.colCount();
  } // hxiReadRow SF

  /// write an HXI FF b-table row
  int hxiWriteRow(AhFitsFilePtr ahf, HXIFF& hxiff, int rowIdx, int hduIdx) { 
    /// for now, just perform the cfitsio calles here, then refactor later into ahfits, etc.
    fitsfile* pfits = fitsFileStructPtr(ahf);
    if( ! pfits ) { 
      clog << __func__ << "oops need open output (fits) file, abort"<<endl;
      return -1;
    }
    writeFITS(ahf, hxiff.col01.voidPtr(), hxiff.col01.length, hxiff.col01.name, hxiff.col01.format, 0, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col02.voidPtr(), hxiff.col02.length, hxiff.col02.name, hxiff.col02.format, 1, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col03.voidPtr(), hxiff.col03.length, hxiff.col03.name, hxiff.col03.format, 2, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col04.voidPtr(), hxiff.col04.length, hxiff.col04.name, hxiff.col04.format, 3, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col05.voidPtr(), hxiff.col05.length, hxiff.col05.name, hxiff.col05.format, 4, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col06.voidPtr(), hxiff.col06.length, hxiff.col06.name, hxiff.col06.format, 5, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col07.voidPtr(), hxiff.col07.length, hxiff.col07.name, hxiff.col07.format, 6, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col08.voidPtr(), hxiff.col08.length, hxiff.col08.name, hxiff.col08.format, 7, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col09.voidPtr(), hxiff.col09.length, hxiff.col09.name, hxiff.col09.format, 8, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col10.voidPtr(), hxiff.col10.length, hxiff.col10.name, hxiff.col10.format, 9, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col11.voidPtr(), hxiff.col11.length, hxiff.col11.name, hxiff.col11.format, 10, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col12.voidPtr(), hxiff.col12.length, hxiff.col12.name, hxiff.col12.format, 11, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col13.voidPtr(), hxiff.col13.length, hxiff.col13.name, hxiff.col13.format, 12, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col14.voidPtr(), hxiff.col14.length, hxiff.col14.name, hxiff.col14.format, 13, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col15.voidPtr(), hxiff.col15.length, hxiff.col15.name, hxiff.col15.format, 14, rowIdx, hduIdx);
    writeFITS(ahf, hxiff.col16.voidPtr(), hxiff.col16.length, hxiff.col16.name, hxiff.col16.format, 15, rowIdx, hduIdx);

    return hxiff.colCount();
  } // hxiWriteRow FF

  /// write an HXI SF b-table row
  int hxiWriteRow(AhFitsFilePtr ahf, HXISF& hxisf, int rowIdx, int hduIdx) { 
     fitsfile* pfits = fitsFileStructPtr(ahf);
     if( ! pfits ) { 
       clog << __func__ << "oops need open output (fits) file, abort"<<endl;
       return -1;
    }

    // check current row count
    getRowCount(ahf);
 
    // ff cols of row:
    hxiWriteRow(ahf, (HXIFF&) hxisf, rowIdx, hduIdx);
    
    // sf cols:
    writeFITS(ahf, hxisf.col17.voidPtr(), hxisf.col17.length, hxisf.col17.name, hxisf.col17.format, 16, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col18.voidPtr(), hxisf.col18.length, hxisf.col18.name, hxisf.col18.format, 17, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col19.voidPtr(), hxisf.col19.length, hxisf.col19.name, hxisf.col19.format, 18, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col20.voidPtr(), hxisf.col20.length, hxisf.col20.name, hxisf.col20.format, 19, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col21.voidPtr(), hxisf.col21.length, hxisf.col21.name, hxisf.col21.format, 20, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col22.voidPtr(), hxisf.col22.length, hxisf.col22.name, hxisf.col22.format, 21, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col23.voidPtr(), hxisf.col23.length, hxisf.col23.name, hxisf.col23.format, 22, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col24.voidPtr(), hxisf.col24.length, hxisf.col24.name, hxisf.col24.format, 23, rowIdx, hduIdx);
    writeFITS(ahf, hxisf.col25.voidPtr(), hxisf.col25.length, hxisf.col25.name, hxisf.col25.format, 24, rowIdx, hduIdx);
    // no calibration processingi/testing for build 1
    //writeFITS(ahf, hxisf.col26.voidPtr(), hxisf.col26.length, hxisf.col26.name, hxisf.col26.format, 25, rowIdx, hduIdx);

    return hxisf.colCount();
  } //  hxiWriteRow SF

} // end namespace ahhxi

#endif // IOHXI_C
