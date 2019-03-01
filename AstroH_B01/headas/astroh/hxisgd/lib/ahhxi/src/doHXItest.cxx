#if !defined(DOHXITEST_C)
#define DOHXITEST_C const char doHXItest_rcsId_svnId[] = "$Id: doHXItest.cxx,v 1.16 2012/07/22 18:14:44 dhon Exp $"; 
/** \file Funcs. that generate internal test "data" and related activities.
    Some of tthe more general funcs. will be refactored into ahgen and ahfits.
    Others need to move to blobCells.cxx and blobBits.cxx.
    \author David B. Hon
    \date Feb-Mar-Apr 2012
*/

/// all includes needed and local function declarations -- namespace ahvbslice
/// note some of these funcs. may move into the ahfits or ahgen namespaces
/// and associated libs...
//
#include "ahtlm/ahSlice.h"
AHSLICE_H(doHXItest)

#include "ahtlm/ahTLM.h"
AHTLM_H(doHXItest)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(doHXItest)

// this is temporary -- need to reconsider location of some typedefs 
#include "ahhxisgd.h"

using namespace ahtlm;
using namespace ahhxisgd;

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: doHXItest.cxx,v 1.16 2012/07/22 18:14:44 dhon Exp $";

  string cfitsioTestFileName(string& filename, string& template_filename, bool clobber = true) {
    // generate the full text string required in the cfistio func. arg./param.: 
    string tlm_template = "ahfits_hxi_tlm.tpl";

    if( filename == "" ) filename = "test_hxi_tlm.fits";
    if( template_filename == "" ) template_filename = tlm_template;

    string cfitsfile_name = (clobber ? "!" : ""); 
    cfitsfile_name += filename;
    cfitsfile_name += "(" + template_filename + ")";
    return cfitsfile_name;
  }

} // end private-local namespace of heplers

namespace ahhxi {
  /// internal test data generation helper
  int setByteTLMVecFromText(TLMbyteVec& vals, string& txtbytes) {
    size_t sz = txtbytes.size(); vals.resize(sz);
    for( size_t i = 0; i < sz; ++i ) vals[i] = txtbytes[i];
    return (int) sz;
  }

  int setBitTLMVecFromText(TLMbyteVec& vals, string& txtbits) {
    // bits as text string, for example: 
    if( txtbits.empty() ) txtbits = "011001110001111000011111000001";
    const size_t _8bits = 8;
    const size_t _numbits = txtbits.size();
    const size_t _numoffsets = _numbits / _8bits;
    vals.clear();

    //BitsAny bitbuf(_numbits, 0);
    std::bitset<_maxSGD_TLMbits> bitbuf(txtbits);
    //std::bitset<_maxHXI_TLMbits> bitbuf(txtbits);
    Bits8 b8;
    size_t offset = 0;
    for( size_t bcnt = 0; ++bcnt <= _numoffsets; offset += _8bits ) {
      b8.reset();
      for( size_t i = 0; i < _8bits; ++i ) b8[i] = bitbuf[i + offset];
      vals.push_back((Ubyte) b8.to_ulong());
    }

    // and handle any remaining partial byte bits 
    const size_t _bitstodo =  _numbits - (vals.size() * _8bits);
    if( _bitstodo <= 0 ) return (int)vals.size();

    b8.reset();
    for( size_t i = 0; i < _bitstodo; ++i ) b8[i] = bitbuf[i + offset];
    vals.push_back((Ubyte) b8.to_ulong());

    return (int)vals.size();
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  AhFitsFilePtr hxiCreateTestfile(string& filename, string& template_filename, HXIFF& hxiff, bool clobber) {
    /// provide internal default test filename:
    string cfitsfile_name = cfitsioTestFileName(filename, template_filename);
    AhFitsFilePtr ahf = new AhFitsFile(cfitsfile_name);
    int status = 0, result = 0;
    // note AhFitsFile::connect_type::iterator is used for btable row iteration
    // test on a single row for now...
    if( ahf ) {
      result = fits_create_file(&(ahf->m_cfitsfp), (char *) cfitsfile_name.c_str(), &status);
      int rowidx = 0, hduidx = 0; 
      result = hxiWriteRow(ahf, hxiff, rowidx, hduidx);
    }
    return ahf;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  AhFitsFilePtr hxiCreateTestfile(string& filename, string& template_filename, HXISF& hxisf, bool clobber) {
    /// provide internal default test filename:
    string cfitsfile_name = cfitsioTestFileName(filename, template_filename);
    clog << __func__ << " cfitsfile_name: " << cfitsfile_name << endl;
    AhFitsFilePtr ahf = new AhFitsFile(cfitsfile_name);
    int status = 0, result = 0;
    // note AhFitsFile::connect_type::iterator is used for btable row iteration
    // test on a single row for now...
    if( ahf ) {
      int rowidx = 0, hduidx = 0; 
      result = fits_create_file(&(ahf->m_cfitsfp), (char *) cfitsfile_name.c_str(), &status);
      // this should simply associate the ordered names in the vec. with column numbers starting at 1 
      result = hxiWriteRow(ahf, hxisf, rowidx, hduidx);
      // or specicially indicate the set of column numbers by using an int vec:
      /// vector<int> cols = { 1, 3, 4, 6 } or such?
    }  
    return ahf;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int hxiTestOutputColCells(vector<string>& col_names, vector<int>& cell_sizes, HXISF& hxisf, int hitcnt, int actvchan, int occurId) {
    // hard-code default test values for 26 column hxi sff
    //vector<string>& col_names = sfwork.col_names;
    //vector<int>& cell_sizes = sfwork.col_sizes;
    col_names.clear() ; cell_sizes.clear();
    int colcount = 26;
    // default initializer of stuct init does not perfom slice alloc, but does provide lenghts
    bool alloc_slice = hxiTLMof(hxisf); // so alloc_slice return == false
    clog << __func__ << "> " << alloc_slice << " ... allocating and setting test bit-buffs ..." << endl;

    //HXIFF& hxiff = (HXIFF&) hxisf; // sff is extension of fff 

    // col1 is an asci text descrition string -- a slice that simply uses c++ string type:
    // this column is temporary and strictly for use in development and test
    // need a vector<int> actvchan to provide actv chans per asic rather than this single value:
    int totalchan = hitcnt*actvchan;
    string describe = describeOccurance("HXI", hitcnt, totalchan, occurId);
    hxisf.col01.setSlice(describe);
    // name of first column -- which should contain a simple example of FFF blob data
    col_names.push_back(hxisf.col01.name);
    cell_sizes.push_back((int) hxisf.col01.size());
    clog << __func__ << " column: " << col_names[0] << ", size: " << cell_sizes[0]
         << ", val: " << (int)hxisf.col01.bufptr[0] << endl;
 
    vector<double> time(1, occurId + 2012.40);
    hxisf.col02.setSlice(time); 
    col_names.push_back(hxisf.col02.name);
    cell_sizes.push_back((int) hxisf.col02.size());
    clog << __func__ << " column: " << col_names[1] << ", size: " << cell_sizes[1]
         << ", val: " << (int)hxisf.col02.bufptr[0] << endl;

    vector<double> stime(1, occurId + 2012.32);
    hxisf.col03.setSlice(stime); 
    col_names.push_back(hxisf.col03.name);
    cell_sizes.push_back((int) hxisf.col03.size());
    clog << __func__ << " column: " << col_names[2] << ", size: " << cell_sizes[2]
         << ", val: " << (int)hxisf.col03.bufptr[0] << endl;

    string tlm_astext = "20byte_CCSDS_PktHdr";
    TLMbyteVec ccsds_pkthdr; setByteTLMVecFromText(ccsds_pkthdr, tlm_astext);
    hxisf.col04.setSlice(ccsds_pkthdr); 
    col_names.push_back(hxisf.col04.name);
    cell_sizes.push_back((int) hxisf.col04.size());
    clog << __func__ << " column: " << col_names[3] << ", size: " << cell_sizes[3]
         << ", val: " << (int)hxisf.col04.bufptr[0] << endl;

    vector<Uint32> time_indicator(1, occurId + 2012); 
    hxisf.col05.setSlice(time_indicator); 
    col_names.push_back(hxisf.col05.name);
    cell_sizes.push_back((int) hxisf.col05.size());
    clog << __func__ << " column: " << col_names[4] << ", size: " << cell_sizes[4]
         << ", val: " << (int)hxisf.col05.bufptr[0] << endl;

    vector<Uint32> occuranceId(1, occurId); 
    hxisf.col06.setSlice(occuranceId); 
    col_names.push_back(hxisf.col06.name);
    cell_sizes.push_back((int) hxisf.col06.size());
    clog << __func__ << " column: " << col_names[5] << ", size: " << cell_sizes[5]
         << ", val: " << (int)hxisf.col06.bufptr[0] << endl;

    vector<Uint32> localtime(1, occurId + 2012); 
    hxisf.col07.setSlice(localtime); 
    col_names.push_back(hxisf.col07.name);
    cell_sizes.push_back((int) hxisf.col07.size());
    clog << __func__ << " column: " << col_names[6] << ", size: " << cell_sizes[6]
         << ", val: " << (int)hxisf.col07.bufptr[0] << endl;

    tlm_astext = "00000001";
    TLMbyteVec seuflag(1, 0); setBitTLMVecFromText(seuflag, tlm_astext);
    hxisf.col08.setSlice(seuflag); 
    col_names.push_back(hxisf.col08.name);
    cell_sizes.push_back((int) hxisf.col08.size());
    clog << __func__ << " column: " << col_names[7] << ", size: " << cell_sizes[7]
         << ", val: " << hxisf.col08.bufptr[0] << endl;

    tlm_astext = "00000011";
    TLMbyteVec lenchkflag(1, 0); setBitTLMVecFromText(lenchkflag, tlm_astext);
    hxisf.col09.setSlice(lenchkflag); 
    col_names.push_back(hxisf.col09.name);
    cell_sizes.push_back((int) hxisf.col09.size());
    clog << __func__ << " column: " << col_names[8] << ", size: " << cell_sizes[8]
         << ", val: " << (int)hxisf.col09.bufptr[0] << endl;

    tlm_astext = "00000111";
    TLMbyteVec trig0(1, 0); setBitTLMVecFromText(trig0, tlm_astext);
    hxisf.col10.setSlice(trig0); 
    col_names.push_back(hxisf.col10.name);
    cell_sizes.push_back((int) hxisf.col10.size());
    clog << __func__ << " column: " << col_names[9] << ", size: " << cell_sizes[9]
         << ", val: " << hxisf.col10.bufptr[0] << endl;

    tlm_astext = "00001111";
    TLMbyteVec trigpat(1, 0); setBitTLMVecFromText(trigpat, tlm_astext);
    hxisf.col11.setSlice(trigpat); 
    col_names.push_back(hxisf.col11.name);
    cell_sizes.push_back((int) hxisf.col11.size());
    clog << __func__ << " column: " << col_names[10] << ", size: " << cell_sizes[10]
         << ", val: " << (int)hxisf.col11.bufptr[0] << endl;
    
    tlm_astext = "00011111";
    TLMbyteVec precisehitpat(1, 0); setBitTLMVecFromText(precisehitpat, tlm_astext);
    hxisf.col12.setSlice(precisehitpat); 
    col_names.push_back(hxisf.col12.name);
    cell_sizes.push_back((int) hxisf.col12.size());
    clog << __func__ << " column: " << col_names[11] << ", size: " << cell_sizes[11] 
         << ", val: " << (int)hxisf.col12.bufptr[0] << endl;

    tlm_astext = "00111111";
    TLMbyteVec fasthitpat(1, 0); setBitTLMVecFromText(fasthitpat, tlm_astext);
    hxisf.col13.setSlice(fasthitpat); 
    col_names.push_back(hxisf.col13.name);
    cell_sizes.push_back((int) hxisf.col13.size());
    clog << __func__ << " column: " << col_names[12] << ", size: " << cell_sizes[12]
         << ", val: " << (int)hxisf.col13.bufptr[0] << endl;

    vector<Uint32> livetime(1, 2014); 
    hxisf.col14.setSlice(livetime); 
    col_names.push_back(hxisf.col14.name);
    cell_sizes.push_back((int) hxisf.col14.size());
    clog << __func__ << " column: " << col_names[13] << ", size: " << cell_sizes[13]
         << ", val: " << (int)hxisf.col14.bufptr[0] << endl;
 
    TLMbyteVec numhits(1, hitcnt);
    hxisf.col15.setSlice(numhits); 
    col_names.push_back(hxisf.col15.name);
    cell_sizes.push_back((int) hxisf.col15.size());
    clog << __func__ << " column: " << col_names[14] << ", size: " << cell_sizes[14]
         << ", val: " << (int)hxisf.col15.bufptr[0] << endl;

    TLMbyteVec raw_asic_data; // need a more useful set of values for more complete testing (later)
    // genTextTLM4Test(tlm_astext) ; setByteTLMVecFromText(raw_asic_data, tlm_astext)
    //tlm_astext = "aabbccddeeffgg"; // 7 asics, 2 channels/per-asic -- keep it simple 
    Bits32 chanflags;
    ahtlm::genTLM4Test(tlm_astext, hitcnt, actvchan, chanflags); // need less simple version ala vector<int>& actvchan
    setByteTLMVecFromText(raw_asic_data, tlm_astext);
    hxisf.col16.setSlice(raw_asic_data); 
    col_names.push_back(hxisf.col16.name);
    cell_sizes.push_back((int) hxisf.col16.size());
    clog << __func__ << " column: " << col_names[15] << ", size: " << cell_sizes[15]
         << ", val: " << (int)hxisf.col16.bufptr[0] << endl;

    // the commented-out funcs. are HXI specific test (bit) pattern generators
    // signature may change and/or be augmented...
 
    vector<Uint32> asic_hdrs(hitcnt, 17); // hxiAsicHdrs(asic_hdrs, raw_asic_data)
    hxisf.col17.setSlice(asic_hdrs); 
    col_names.push_back(hxisf.col17.name);
    cell_sizes.push_back((int) hxisf.col17.size());
    clog << __func__ << " column: " << col_names[16] << ", size: " << cell_sizes[16]
         << ", val: " << (int)hxisf.col17.bufptr[0] << endl;

    TLMbyteVec orig_asicIds(hitcnt, 18); // hxiOrigAsicIds(orig_asicIds, raw_asic_data);
    hxisf.col18.setSlice(orig_asicIds); 
    col_names.push_back(hxisf.col18.name);
    cell_sizes.push_back((int) hxisf.col18.size());
    clog << __func__ << " column: " << col_names[17] << ", size: " << cell_sizes[17]
         << ", val: " << (int)hxisf.col18.bufptr[0] << endl;

    TLMbyteVec remapped_asicIds(hitcnt, 19); // hxiRemapAsicIds(remapped_asicIds, raw_asic_data);
    hxisf.col19.setSlice(remapped_asicIds); 
    col_names.push_back(hxisf.col19.name);
    cell_sizes.push_back((int) hxisf.col19.size());
    clog << __func__ << " column: " << col_names[18] << ", size: " << cell_sizes[18]
         << ", val: " << (int)hxisf.col19.bufptr[0] << endl;

    vector<Uint32> orig_chanFlags(hitcnt, 20); // hxiOrigChanBitFlags(orig_chanIds, raw_asic_data);
    hxisf.col20.setSlice(orig_chanFlags); 
    col_names.push_back(hxisf.col20.name);
    cell_sizes.push_back((int) hxisf.col20.size());
    clog << __func__ << " column: " << col_names[19] << ", size: " << cell_sizes[19]
         << ", val: " << (int)hxisf.col20.bufptr[0] << endl;

    vector<Uint16> remapped_chanIds(totalchan, 21); // hxiRemapChanIds(remapped_chanIds, raw_asic_data)
    hxisf.col21.setSlice(remapped_chanIds);
    col_names.push_back(hxisf.col21.name);
    cell_sizes.push_back((int) hxisf.col21.size());
    clog << __func__ << " column: " << col_names[20] << ", size: " << cell_sizes[20]
         << ", val: " << (int)hxisf.col21.bufptr[0] << endl;

    TLMbyteVec actvchan_bvec(hitcnt, 22); // hxiActiveChans(actvchan_bvec, raw_asic_data);
    hxisf.col22.setSlice(actvchan_bvec); 
    col_names.push_back(hxisf.col22.name);
    cell_sizes.push_back((int) hxisf.col22.size());
    clog << __func__ << " column: " << col_names[21] << ", size: " << cell_sizes[21]
         << ", val: " << (int)hxisf.col22.bufptr[0] << endl;

    vector<Uint16> ref_chans(hitcnt, 23); // hxiRefChans(ref_chans, raw_asic_data)
    hxisf.col23.setSlice(ref_chans);
    col_names.push_back(hxisf.col23.name);
    cell_sizes.push_back((int) hxisf.col23.size());
    clog << __func__ << " column: " << col_names[22] << ", size: " << cell_sizes[22]
         << ", val: " << (int)hxisf.col23.bufptr[0] << endl;

    vector<Uint16> comn_noise(hitcnt, 24); // hxiNoiseChans(comn_noise, raw_asic_data)
    hxisf.col24.setSlice(comn_noise);
    col_names.push_back(hxisf.col24.name);
    cell_sizes.push_back((int) hxisf.col24.size());
    clog << __func__ << " column: " << col_names[23] << ", size: " << cell_sizes[23]
         << ", val: " << (int)hxisf.col24.bufptr[0] << endl;

    vector<Uint16> raw_adc_pulse_heights(totalchan, 25); // hxiRawAdcPulses(raw_adc_pulse_heights, raw_asic_data);
    hxisf.col25.setSlice(raw_adc_pulse_heights); 
    col_names.push_back(hxisf.col25.name);
    cell_sizes.push_back((int) hxisf.col25.size());
    clog << __func__ << " column: " << col_names[24] << ", size: " << cell_sizes[24]
         << ", val: " << (int)hxisf.col25.bufptr[0] << endl;

    vector<Uint16> cal_adc_pulse_heights(totalchan, 26); // hxiCalAdcPulses(cal_adc_pulse_heights, raw_asic_data);
    hxisf.col26.setSlice(cal_adc_pulse_heights); 
    col_names.push_back(hxisf.col26.name);
    cell_sizes.push_back((int) hxisf.col26.size());
    clog << __func__ << " column: " << col_names[25] << ", size: " << cell_sizes[25]
         << ", val: " << (int)hxisf.col26.bufptr[0] << endl;

    return colcount;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  //int hxiTestInputColCells(vector<string>& ffcol_names, vector<int>& ffcell_sizes, HXIFF& hxiff) {
  // given current implementation of HXISF as extension of HXIFF -- do we need this?
  int hxiTestInputColCells(InstrumWork& ffwork, HXIFF& hxiff, int hitcnt= 7, int occurId= 77) {
    InstrumWork sfwork("HXI");
    HXISF hxisf; // an empty HXI FF HDU data buff struct 
    int outcols = hxiTestOutputColCells(ffwork.col_names, ffwork.col_sizes, hxisf); 
    hxiff = (HXIFF& ) hxisf; // hxiff struct is extended by hxisff (sff is superset of ff) 
    return outcols-10;
    /*  this should not allocate data buffs, just sets default column names and data types:
    int maxcols = hxiColumnNames(sfwork, hxisf); 
    ffwork.col_sizes.clear();
    int ffcolcount = (int)ffwork.col_names.size(); 
    hxiff = (HXIFF) hxisf; // sff is extension of fff 
    for( int i = 0; i < ffcolcount; ++i ) {
      string name = ffwork.col_names[i] == sfwork.col_names[i];
      int sz = ffwork.col_sizes[i] == sfwork.col_sizes[i];
      clog << __func__ << "> " << name << " length == " << sz << endl;
    }
    return ffcolcount;
    */ 
  }
     
  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int hxiTestIO(AhFitsFilePtr outfile, HXISF& hxisf, AhFitsFilePtr infile, HXIFF& hxiff) {
    /// perform ahFitsConnects, process input and ouput, then ahFitsDisconnects
    /// loop over hxiff and hxisf columns and "connect" slice buffs somehow to
    /// ahfits i/o internals:
    // ahFitsConnect(infile, inblob.col_name.c_str(), &_hits, &_hits_count);
    // ahFitsConnect(outfile, outblob.col_names[0].c_str(), &_event_per_row, 0);
    // ahFitsConnect(outfile, outblob.col_names[1].c_str(), &_event, &_event_count);


    /// for now, just perform the cfitsio calls here, then refactor later into ahfits, etc.
    fitsfile* infits = fitsFileStructPtr(infile);
    if( ! infits ) { 
      clog << __func__ << "oops need open input (fits) file, abort"<<endl;
      return -1;
    }
    fitsfile* outfits = fitsFileStructPtr(outfile);
    if( ! outfits ) { 
      clog << __func__ << "oops need open output (fits) file, abort"<<endl;
      return -2;
    }

    // full internal test inits input (FFF) file, then reads it back:
    int result = hxiWriteRow(infile, hxiff); 
    HXIFF rhxiff;
    result = hxiReadRow(outfile, rhxiff);

    // full internal test writes output (SFF) file, then reads it back:
    result = hxiWriteRow(outfile, hxisf); 
    HXIFF rhxisf;
    result = hxiReadRow(outfile, rhxisf);
  
    // ahFitsDisconnectAll(outfile);
    // ahFitsDisconnectAll(infile);
    return 0;
  }

  int hxiWriteTestDefaults(AhFitsFilePtr fff, AhFitsFilePtr sff, int rowcnt) {
    HXISF hxisf; // an empty HXI FF HDU data buff struct
    vector<string> col_names; vector<int> col_sizes;
    vector<int> hitcnt, actvchan, occurId;
    hitcnt.push_back(10); hitcnt.push_back(20); hitcnt.push_back(30);
    actvchan.push_back(2); actvchan.push_back(12); actvchan.push_back(22);
    occurId.push_back(1); occurId.push_back(2); occurId.push_back(3);
    // hard-code 1 to 3 rows for now...
    if( rowcnt < 1 ) rowcnt = 1; if( rowcnt > 3 ) rowcnt = 3;
    for( int rowIdx = 0; rowIdx < rowcnt; ++rowIdx ) {
      int outcols = hxiTestOutputColCells(col_names, col_sizes, hxisf, hitcnt[rowIdx], actvchan[rowIdx], occurId[rowIdx]); 
      int wrtcols = hxiWriteRow(sff, hxisf, rowIdx);
      HXIFF& hxiff = (HXIFF& ) hxisf; // hxiff struct is extended by hxisff (sff is superset of ff) 
      wrtcols += hxiWriteRow(fff, hxiff, rowIdx);
      clog << __func__ << " " << rowIdx << " number of columns written: " << wrtcols << " == " << outcols << endl;
    }
    return rowcnt;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  //int hxiTestTLM(vector<string>& incol_names, vector<string>& outcol_names, AhFitsFilePtr outfile, AhFitsFilePtr infile) {
  int hxiTestTLM(InstrumWork& ffwork, InstrumWork& sfwork) {
    /// this allocates data buffs and provides some internal test default data values:
    /// hard-code default test values for 26 column hxi sff
    HXISF hxisf; // an empty HXI FF HDU data buff struct 
    //int outcols = hxiTestOutputColCells(sfwork, hxisf); 
    int outcols = hxiTestOutputColCells(sfwork.col_names, sfwork.col_sizes, hxisf); 
    HXIFF& hxiff = (HXIFF& ) hxisf; // hxiff struct is extended by hxisff (sff is superset of ff) 
    //HXIFF hxiff;
    //int incols = hxiTestInputColCells(ffwork, hxiff);
    clog << __func__ << " number of columns in each input row: " << hxiff.colCount() << endl;
    clog << __func__ << " number of columns in each output row: " << hxisf.colCount() <<  "== " << outcols << endl;

    if( ! ffwork.file ) {
      clog << __func__ << " no infile indicate, creating one for subsequent use/tests..." << endl;
      // use internal test data to create a simple infile
      string fitsfile_name = ""; // ahfits::fileName(infile); // tests null arg... 
      string template_name = "";
      ffwork.file = hxiCreateTestfile(fitsfile_name, template_name, hxiff);
    }

    // read the input (test) (ff) file and process it to create the output (sf) file:
    return hxiTestIO(sfwork.file, hxisf, ffwork.file, hxiff);
  }
} // end namespace ahhxi

#endif // DOHXITEST_C
