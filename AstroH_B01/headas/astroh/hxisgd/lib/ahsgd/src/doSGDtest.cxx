#if !defined(DOSGDTEST_C)
#define DOSGDTEST_C const char doSGDtest_rcsId_svnId[] = "$Id: doSGDtest.cxx,v 1.7 2012/07/22 18:14:44 dhon Exp $"; 
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
AHSLICE_H(doSGDtest)

#include "ahtlm/ahTLM.h"
AHTLM_H(doSGDtest)

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(doSGDtest)

// this may change -- need to reconsider location of some typedefs 
#include "ahhxisgd.h"

using namespace ahtlm;
using namespace ahhxisgd;

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: doSGDtest.cxx,v 1.7 2012/07/22 18:14:44 dhon Exp $";

  string cfitsioTestFileName(string& filename, string& template_filename, bool clobber = true) {
    // generate the full text string required in the cfistio func. arg./param.: 
    string tlm_template = "ahfits_sgd_tlm.tpl";

    if( filename == "" ) filename = "test_sgd_tlm.fits";
    if( template_filename == "" ) template_filename = tlm_template;

    string cfitsfile_name = (clobber ? "!" : ""); 
    cfitsfile_name += filename;
    cfitsfile_name += "(" + template_filename + ")";
    return cfitsfile_name;
  }

} // end private-local namespace of heplers

namespace ahsgd {
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
    //std::bitset<_maxSGD_TLMbits> bitbuf(txtbits);
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
  AhFitsFilePtr sgdCreateTestfile(string& filename, string& template_filename, SGDFF& sgdff, bool clobber) {
    /// provide internal default test filename:
    string cfitsfile_name = cfitsioTestFileName(filename, template_filename);
    AhFitsFilePtr ahf = new AhFitsFile(cfitsfile_name);
    int status = 0, result = 0;
    // note AhFitsFile::connect_type::iterator is used for btable row iteration
    // test on a single row for now...
    if( ahf ) {
      result = fits_create_file(&(ahf->m_cfitsfp), (char *) cfitsfile_name.c_str(), &status);
      int rowidx = 0, hduidx = 0; 
      result = sgdWriteRow(ahf, sgdff, rowidx, hduidx);
    }
    return ahf;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  AhFitsFilePtr sgdCreateTestfile(string& filename, string& template_filename, SGDSF& sgdsf, bool clobber) {
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
      result = sgdWriteRow(ahf, sgdsf, rowidx, hduidx);
      // or specicially indicate the set of column numbers by using an int vec:
      /// vector<int> cols = { 1, 3, 4, 6 } or such?
    }  
    return ahf;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  int sgdTestOutputColCells(vector<string>& col_names, vector<int>& cell_sizes, SGDSF& sgdsf, int hitcnt, int actvchan, int occurId) {
    // hard-code default test values for ? column sgd sff
    //vector<string>& col_names = sfwork.col_names;
    //vector<int>& cell_sizes = sfwork.col_sizes;
    col_names.clear() ; cell_sizes.clear();
    int colcount = 29; // there may be a different column count for SGD vs. HXI ... this is for B01 testing
    // default initializer of stuct init does not perfom slice alloc, but does provide lenghts
    bool alloc_slice = sgdTLMof(sgdsf); // so alloc_slice return == false
    clog << __func__ << "> " << alloc_slice << " ... allocating and setting test bit-buffs ..." << endl;

    //SGDFF& sgdff = (SGDFF&) sgdsf; // sff is extension of fff 

    // col1 is an asci text descrition string -- a slice that simply uses c++ string type:
    // need a vector<int> actvchan per asic rather than this single value:
    int totalchan = hitcnt*actvchan;
    string describe = describeOccurance("SGD", hitcnt, totalchan, occurId);
    sgdsf.col01.setSlice(describe);
    // name of this first column
    col_names.push_back(sgdsf.col01.name);
    cell_sizes.push_back((int) sgdsf.col01.size());
    clog << __func__ << " column: " << col_names[0] << ", size: " << cell_sizes[0]
         << ", val: " << (int)sgdsf.col01.bufptr[0] << endl;
 
    vector<double> time(1, occurId + 2012.624);
    sgdsf.col02.setSlice(time); 
    col_names.push_back(sgdsf.col02.name);
    cell_sizes.push_back((int) sgdsf.col02.size());
    clog << __func__ << " column: " << col_names[1] << ", size: " << cell_sizes[1]
         << ", val: " << (int)sgdsf.col02.bufptr[0] << endl;

    vector<double> stime(1, occurId + 2012.64);
    sgdsf.col03.setSlice(stime); 
    col_names.push_back(sgdsf.col03.name);
    cell_sizes.push_back((int) sgdsf.col03.size());
    clog << __func__ << " column: " << col_names[2] << ", size: " << cell_sizes[2]
         << ", val: " << (int)sgdsf.col03.bufptr[0] << endl;

    string tlm_astext = "20byte_CCSDS_PktHdr";
    TLMbyteVec ccsds_pkthdr; setByteTLMVecFromText(ccsds_pkthdr, tlm_astext);
    sgdsf.col04.setSlice(ccsds_pkthdr); 
    col_names.push_back(sgdsf.col04.name);
    cell_sizes.push_back((int) sgdsf.col04.size());
    clog << __func__ << " column: " << col_names[3] << ", size: " << cell_sizes[3]
         << ", val: " << (int)sgdsf.col04.bufptr[0] << endl;

    vector<Uint32> time_indicator(1, occurId + 2012); 
    sgdsf.col05.setSlice(time_indicator); 
    col_names.push_back(sgdsf.col05.name);
    cell_sizes.push_back((int) sgdsf.col05.size());
    clog << __func__ << " column: " << col_names[4] << ", size: " << cell_sizes[4]
         << ", val: " << (int)sgdsf.col05.bufptr[0] << endl;

    vector<Uint32> occuranceId(1, occurId); 
    sgdsf.col06.setSlice(occuranceId); 
    col_names.push_back(sgdsf.col06.name);
    cell_sizes.push_back((int) sgdsf.col06.size());
    clog << __func__ << " column: " << col_names[5] << ", size: " << cell_sizes[5]
         << ", val: " << (int)sgdsf.col06.bufptr[0] << endl;

    vector<Uint32> localtime(1, occurId + 2012); 
    sgdsf.col07.setSlice(localtime); 
    col_names.push_back(sgdsf.col07.name);
    cell_sizes.push_back((int) sgdsf.col07.size());
    clog << __func__ << " column: " << col_names[6] << ", size: " << cell_sizes[6]
         << ", val: " << (int)sgdsf.col07.bufptr[0] << endl;

    tlm_astext = "00000001";
    TLMbyteVec seuflag(1, 0); setBitTLMVecFromText(seuflag, tlm_astext);
    sgdsf.col08.setSlice(seuflag); 
    col_names.push_back(sgdsf.col08.name);
    cell_sizes.push_back((int) sgdsf.col08.size());
    clog << __func__ << " column: " << col_names[7] << ", size: " << cell_sizes[7]
         << ", val: " << sgdsf.col08.bufptr[0] << endl;

    tlm_astext = "00000011";
    TLMbyteVec lenchkflag(1, 0); setBitTLMVecFromText(lenchkflag, tlm_astext);
    sgdsf.col09.setSlice(lenchkflag); 
    col_names.push_back(sgdsf.col09.name);
    cell_sizes.push_back((int) sgdsf.col09.size());
    clog << __func__ << " column: " << col_names[8] << ", size: " << cell_sizes[8]
         << ", val: " << (int)sgdsf.col09.bufptr[0] << endl;

    tlm_astext = "00000111";
    TLMbyteVec calmode(1, 0); setBitTLMVecFromText(calmode, tlm_astext);
    sgdsf.col10.setSlice(calmode); 
    col_names.push_back(sgdsf.col10.name);
    cell_sizes.push_back((int) sgdsf.col10.size());
    clog << __func__ << " column: " << col_names[9] << ", size: " << cell_sizes[9]
         << ", val: " << (int)sgdsf.col10.bufptr[0] << endl;

    tlm_astext = "00011111";
    TLMbyteVec trig0(1, 0); setBitTLMVecFromText(trig0, tlm_astext);
    sgdsf.col11.setSlice(trig0); 
    col_names.push_back(sgdsf.col11.name);
    cell_sizes.push_back((int) sgdsf.col11.size());
    clog << __func__ << " column: " << col_names[10] << ", size: " << cell_sizes[10]
         << ", val: " << sgdsf.col11.bufptr[0] << endl;
    
    tlm_astext = "00111111";
    TLMbyteVec trigpat(1, 0); setBitTLMVecFromText(trigpat, tlm_astext);
    sgdsf.col12.setSlice(trigpat); 
    col_names.push_back(sgdsf.col12.name);
    cell_sizes.push_back((int) sgdsf.col12.size());
    clog << __func__ << " column: " << col_names[11] << ", size: " << cell_sizes[11]
         << ", val: " << (int)sgdsf.col12.bufptr[0] << endl;
    
    tlm_astext = "01111111";
    TLMbyteVec ccbusy(1, 0); setBitTLMVecFromText(ccbusy, tlm_astext);
    sgdsf.col13.setSlice(ccbusy); 
    col_names.push_back(sgdsf.col13.name);
    cell_sizes.push_back((int) sgdsf.col13.size());
    clog << __func__ << " column: " << col_names[12] << ", size: " << cell_sizes[12] 
         << ", val: " << (int)sgdsf.col13.bufptr[0] << endl;

    tlm_astext = "11111111";
    TLMbyteVec hitpatcc(1, 0); setBitTLMVecFromText(hitpatcc, tlm_astext);
    sgdsf.col14.setSlice(hitpatcc); 
    col_names.push_back(sgdsf.col14.name);
    cell_sizes.push_back((int) sgdsf.col14.size());
    clog << __func__ << " column: " << col_names[13] << ", size: " << cell_sizes[13] 
         << ", val: " << (int)sgdsf.col14.bufptr[0] << endl;
    
    tlm_astext = "10101010";
    TLMbyteVec precisehitpat(1, 0); setBitTLMVecFromText(precisehitpat, tlm_astext);
    sgdsf.col15.setSlice(precisehitpat); 
    col_names.push_back(sgdsf.col15.name);
    cell_sizes.push_back((int) sgdsf.col15.size());
    clog << __func__ << " column: " << col_names[14] << ", size: " << cell_sizes[14] 
         << ", val: " << (int)sgdsf.col15.bufptr[0] << endl;

    tlm_astext = "01111111";
    TLMbyteVec fasthitpat(1, 0); setBitTLMVecFromText(fasthitpat, tlm_astext);
    sgdsf.col16.setSlice(fasthitpat); 
    col_names.push_back(sgdsf.col16.name);
    cell_sizes.push_back((int) sgdsf.col16.size());
    clog << __func__ << " column: " << col_names[15] << ", size: " << cell_sizes[15]
         << ", val: " << (int)sgdsf.col16.bufptr[0] << endl;

    vector<Uint32> livetime(1, 2014); 
    sgdsf.col17.setSlice(livetime); 
    col_names.push_back(sgdsf.col17.name);
    cell_sizes.push_back((int) sgdsf.col17.size());
    clog << __func__ << " column: " << col_names[16] << ", size: " << cell_sizes[16]
         << ", val: " << (int)sgdsf.col17.bufptr[0] << endl;
 
    vector<Uint16> numhits(1, hitcnt);
    sgdsf.col18.setSlice(numhits); 
    col_names.push_back(sgdsf.col18.name);
    cell_sizes.push_back((int) sgdsf.col18.size());
    clog << __func__ << " column: " << col_names[17] << ", size: " << cell_sizes[17]
         << ", val: " << (int)sgdsf.col18.bufptr[0] << endl;

    TLMbyteVec raw_asic_data; // need a more useful set of values for more complete testing (later)
    //tlm_astext = "aaabbbcccddddeeefffggghhhjjjkkklllmmmnnnooopppqqq"; // 17 asics, 2 chans each
    Bits64 chanflags;
    ahtlm::genTLM4Test(tlm_astext, hitcnt, actvchan, chanflags); // need better test gen -- diff # of chans/asic
    setByteTLMVecFromText(raw_asic_data, tlm_astext);
    sgdsf.col19.setSlice(raw_asic_data);
    col_names.push_back(sgdsf.col19.name);
    cell_sizes.push_back((int) sgdsf.col19.size());
    clog << __func__ << " column: " << col_names[18] << ", size: " << cell_sizes[18]
         << ", val: " << (int)sgdsf.col19.bufptr[0] << endl;

    // the commented-out funcs. are SGD test (bit) pattern generators
    // signature may change and/or be augmented...
 
    vector<Uint32> asic_hdrs(hitcnt, 20); // sgdAsicHdrs(asic_hdrs, raw_asic_data)
    sgdsf.col20.setSlice(asic_hdrs); 
    col_names.push_back(sgdsf.col20.name);
    cell_sizes.push_back((int) sgdsf.col20.size());
    clog << __func__ << " column: " << col_names[19] << ", size: " << cell_sizes[19]
         << ", val: " << (int)sgdsf.col20.bufptr[0] << endl;

    vector<Uint16> orig_asicIds(hitcnt, 21); // sgdOrigAsicIds(orig_asicIds, raw_asic_data);
    sgdsf.col21.setSlice(orig_asicIds); 
    col_names.push_back(sgdsf.col21.name);
    cell_sizes.push_back((int) sgdsf.col21.size());
    clog << __func__ << " column: " << col_names[20] << ", size: " << cell_sizes[20]
         << ", val: " << (int)sgdsf.col21.bufptr[0] << endl;

    vector<Uint16> remapped_asicIds(hitcnt, 22); // sgdRemapAsicIds(remapped_asicIds, raw_asic_data);
    sgdsf.col22.setSlice(remapped_asicIds); 
    col_names.push_back(sgdsf.col22.name);
    cell_sizes.push_back((int) sgdsf.col22.size());
    clog << __func__ << " column: " << col_names[21] << ", size: " << cell_sizes[21]
         << ", val: " << (int)sgdsf.col22.bufptr[0] << endl;

    vector<Uint64> orig_chanFlags(hitcnt, 23); // sgdOrigChanBitFlags(orig_chanIds, raw_asic_data);
    sgdsf.col23.setSlice(orig_chanFlags); 
    col_names.push_back(sgdsf.col23.name);
    cell_sizes.push_back((int) sgdsf.col23.size());
    clog << __func__ << " column: " << col_names[22] << ", size: " << cell_sizes[22]
         << ", val: " << (int)sgdsf.col23.bufptr[0] << endl;

    vector<Uint16> remapped_chanIds(totalchan, 24); // sgdRemapChanIds(remapped_chanIds, raw_asic_data)
    sgdsf.col24.setSlice(remapped_chanIds);
    col_names.push_back(sgdsf.col24.name);
    cell_sizes.push_back((int) sgdsf.col24.size());
    clog << __func__ << " column: " << col_names[23] << ", size: " << cell_sizes[23]
         << ", val: " << (int)sgdsf.col24.bufptr[0] << endl;

    TLMbyteVec actvchan_bvec(hitcnt, 25); // sgdActiveChans(actvchan_bvec, raw_asic_data);
    sgdsf.col25.setSlice(actvchan_bvec);
    col_names.push_back(sgdsf.col25.name);
    cell_sizes.push_back((int) sgdsf.col25.size());
    clog << __func__ << " column: " << col_names[24] << ", size: " << cell_sizes[24]
         << ", val: " << (int)sgdsf.col25.bufptr[0] << endl;

    vector<Uint16> ref_chans(hitcnt, 26); // sgdRefChans(ref_chans, raw_asic_data)
    sgdsf.col26.setSlice(ref_chans);
    col_names.push_back(sgdsf.col26.name);
    cell_sizes.push_back((int) sgdsf.col26.size());
    clog << __func__ << " column: " << col_names[25] << ", size: " << cell_sizes[25]
         << ", val: " << (int)sgdsf.col26.bufptr[0] << endl;

    vector<Uint16> comn_noise(hitcnt, 27); // sgdNoiseChans(comn_noise, raw_asic_data)
    sgdsf.col27.setSlice(comn_noise);
    col_names.push_back(sgdsf.col27.name);
    cell_sizes.push_back((int) sgdsf.col27.size());
    clog << __func__ << " column: " << col_names[26] << ", size: " << cell_sizes[26]
         << ", val: " << (int)sgdsf.col27.bufptr[0] << endl;

    vector<Uint16> raw_adc_pulse_heights(totalchan, 28); // sgdRawAdcPulses(raw_adc_pulse_heights, raw_asic_data);
    sgdsf.col28.setSlice(raw_adc_pulse_heights); 
    col_names.push_back(sgdsf.col28.name);
    cell_sizes.push_back((int) sgdsf.col28.size());
    clog << __func__ << " column: " << col_names[27] << ", size: " << cell_sizes[27]
         << ", val: " << (int)sgdsf.col28.bufptr[0] << endl;

    vector<Uint16> cal_adc_pulse_heights(totalchan, 29); // sgdCalAdcPulses(cal_adc_pulse_heights, raw_asic_data);
    sgdsf.col29.setSlice(cal_adc_pulse_heights); 
    col_names.push_back(sgdsf.col29.name);
    cell_sizes.push_back((int) sgdsf.col29.size());
    clog << __func__ << " column: " << col_names[28] << ", size: " << cell_sizes[28]
         << ", val: " << (int)sgdsf.col29.bufptr[0] << endl;

    return colcount;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  //int sgdTestInputColCells(vector<string>& ffcol_names, vector<int>& ffcell_sizes, SGDFF& sgdff) {
  // given current implementation of SGDSF as extension of SGDFF -- do we need this?
  int sgdTestInputColCells(InstrumWork& ffwork, SGDFF& sgdff, int hitcnt= 7, int occurId= 77) {
    InstrumWork sfwork("SGD");
    SGDSF sgdsf; // an empty SGD FF HDU data buff struct 
    int outcols = sgdTestOutputColCells(ffwork.col_names, ffwork.col_sizes, sgdsf); 
    sgdff = (SGDFF& ) sgdsf; // sgdff struct is extended by sgdsff (sff is superset of ff) 
    return outcols-10;
    /*  this should not allocate data buffs, just sets default column names and data types:
    int maxcols = sgdColumnNames(sfwork, sgdsf); 
    ffwork.col_sizes.clear();
    int ffcolcount = (int)ffwork.col_names.size(); 
    sgdff = (SGDFF) sgdsf; // sff is extension of fff 
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
  int sgdTestIO(AhFitsFilePtr outfile, SGDSF& sgdsf, AhFitsFilePtr infile, SGDFF& sgdff) {
    /// perform ahFitsConnects, process input and ouput, then ahFitsDisconnects
    /// loop over sgdff and sgdsf columns and "connect" slice buffs somehow to
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
    int result = sgdWriteRow(infile, sgdff); 
    SGDFF rsgdff;
    result = sgdReadRow(outfile, rsgdff);

    // full internal test writes output (SFF) file, then reads it back:
    result = sgdWriteRow(outfile, sgdsf); 
    SGDFF rsgdsf;
    result = sgdReadRow(outfile, rsgdsf);
  
    // ahFitsDisconnectAll(outfile);
    // ahFitsDisconnectAll(infile);
    return 0;
  }

  int sgdWriteTestDefaults(AhFitsFilePtr fff, AhFitsFilePtr sff, int rowcnt) {
    SGDSF sgdsf; // an empty SGD FF HDU data buff struct
    vector<string> col_names; vector<int> col_sizes;
    vector<int> hitcnt, actvchan, occurId;
    hitcnt.push_back(10); hitcnt.push_back(42); hitcnt.push_back(208); // asics range from 1 - 624 (3*208)
    actvchan.push_back(12); actvchan.push_back(32); actvchan.push_back(52);
    occurId.push_back(1); occurId.push_back(2); occurId.push_back(3);
    // hard-code 1 to 3 rows for now...
    if( rowcnt < 1 ) rowcnt = 1; if( rowcnt > 3 ) rowcnt = 3;
    for( int rowIdx = 0; rowIdx < rowcnt; ++rowIdx ) {
      int outcols = sgdTestOutputColCells(col_names, col_sizes, sgdsf, hitcnt[rowIdx], actvchan[rowIdx], occurId[rowIdx]); 
      int wrtcols = sgdWriteRow(sff, sgdsf, rowIdx);
      SGDFF& sgdff = (SGDFF& ) sgdsf; // sgdff struct is extended by sgdsff (sff is superset of ff) 
      wrtcols += sgdWriteRow(fff, sgdff, rowIdx);
      clog << __func__ << " " << rowIdx << " number of columns written: " << wrtcols << " == " << outcols << endl;
    }
    return rowcnt;
  }

  // doxygen tag to generate diagram of func. invocations
  /// \callgraph
  //int sgdTestTLM(vector<string>& incol_names, vector<string>& outcol_names, AhFitsFilePtr outfile, AhFitsFilePtr infile) {
  int sgdTestTLM(InstrumWork& ffwork, InstrumWork& sfwork) {
    /// this allocates data buffs and provides some internal test default data values:
    /// hard-code default test values for 26 column sgd sff
    SGDSF sgdsf; // an empty SGD FF HDU data buff struct 
    //int outcols = sgdTestOutputColCells(sfwork, sgdsf); 
    int outcols = sgdTestOutputColCells(sfwork.col_names, sfwork.col_sizes, sgdsf); 
    SGDFF& sgdff = (SGDFF& ) sgdsf; // sgdff struct is extended by sgdsff (sff is superset of ff) 
    //SGDFF sgdff;
    //int incols = sgdTestInputColCells(ffwork, sgdff);
    clog << __func__ << " number of columns in each input row: " << sgdff.colCount() << endl;
    clog << __func__ << " number of columns in each output row: " << sgdsf.colCount() <<  "== " << outcols << endl;

    if( ! ffwork.file ) {
      clog << __func__ << " no infile indicate, creating one for subsequent use/tests..." << endl;
      // use internal test data to create a simple infile
      string fitsfile_name = ""; // ahfits::fileName(infile); // tests null arg... 
      string template_name = "";
      ffwork.file = sgdCreateTestfile(fitsfile_name, template_name, sgdff);
    }

    // read the input (test) (ff) file and process it to create the output (sf) file:
    return sgdTestIO(sfwork.file, sgdsf, ffwork.file, sgdff);
  }
} // end namespace ahsgd

#endif // DOSGDTEST_C
