#if !defined(HXITEMPLATE_C)
#define HXITEMPLATE_C const char hxitemplate_rcsId_svnId[] = "$Id: HXItemplate.cxx,v 1.6 2012/07/13 20:47:44 dhon Exp $"; 

// Local includes.
#include "ahhxisgd.h"
AHHXISGD_H(hxitemplate)

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(hxitemplate)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(hxitemplate)

#include "ahtlm/ahBits.h"
AHBITS_H(hxitemplate)

#include "ahtlm/ahSlice.h"
AHSLICE_H(hxitemplate)

#include "ahtlm/ahTLM.h"
AHTLM_H(hxitemplate)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(hxitemplate)

#include "ahtlm/ahTLMinfoCache.h"
AHTLMINFOCACHE_H(hxitemplate)

using namespace std;
using namespace ahtlm;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: HXItemplate.cxx,v 1.6 2012/07/13 20:47:44 dhon Exp $";
}

namespace ahhxi {

  /// HXI template default content -- has evolved a bit april-may-june 2012 and some
  /// questions remain for both hxi and sgd. but this allows for full coding and testing...
  int initHXItemplates(string& content, string& fff_filename, string& sff_filename) {
    const string inpath = "./input/";
    if( fff_filename == "" ) fff_filename = hxiFFVersion(inpath); //  "./input/HXI_FFFv00.tpl";
    if( sff_filename == "" ) sff_filename = hxiSFVersion(inpath); // "./input/HXI_SFFv00.tpl";
    content = \
"xtension= bintable\n"
"extname = events\n"
"telescop= astro-h\n"
"instrume = HXI\n"
"\n"
"#number of rows\n"
"naxis2 = 1\n"
"\n"
"ttype# = event_description\n"
"tform# = 128a\n"
"\n"
"#------------------------------------------------------------\n"
"#------------------------------------------------------------\n"
"# Columns for FFF\n"
"#------------------------------------------------------------\n"
"#------------------------------------------------------------\n"
"\n"
"ttype# = TIME\n"
"tform# = 1D\n"
"\n"
"ttype# = S_TIME\n"
"tform# = 1D\n"
"\n"
"ttype# = CCSDS_Packet_Header\n"
"tform# = 20B\n"
"\n"
"#Time Indicator\n"
"#ttype# = TI\n"
"ttype# = Time_Indicator\n"
"tform# = 1J\n"
"\n"
"ttype# = Occurance_ID\n"
"tform# = 1J\n"
"\n"
"ttype# = Local_Time\n"
"tform# = 1J\n"
"\n"
"ttype# = FLAG_SingleEventUpset\n"
"#tform# = 5X\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_LengthCheck\n"
"#tform# = 5X\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_InitialTriggerPattern\n"
"#tform# = 8X\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_TriggerPattern\n"
"#tform# = 8X\n"
"tform# = 1B\n"
"\n"
"#ttype# = FLAG_Trigger\n"
"#tform# = 8X\n"
"\n"
"ttype# = FLAG_PreciseShieldHitPattern\n"
"tform# = 1B\n"
"\n"
"#ttype# = FLAG_HitPattern\n"
"#tform# = 2X\n"
"\n"
"#ttype# = FLAG_FastBGO\n"
"#tform# = 2X\n"
"\n"
"ttype# = FLAG_FastShieldHitPattern\n"
"tform# = 1B\n"
"\n"
"#1J or 24X (actually 24-bit width)\n"
"ttype# = LiveTime\n"
"tform# = 1J\n"
"\n"
"ttype# = nHitASICs\n"
"tform# = 1B\n"
"\n"
"#------------------------------------------------------------\n"
"# Original ASIC Event Data\n"
"#------------------------------------------------------------\n"
"# Data output from each ASIC have\n"
"# ASIC ID(8bits), 000 & 5-bit flag(8bits), Ch Data Bits(4bytes),\n"
"# CM(1bit), Reference Ch(10bits), \n"
"# ADC Data (10bits x 32ch max = 320bits max),\n"
"# Common Mode Noise Data (10bits), and 0s for filling.\n"
"#Therefore, the maximum length of one ASIC data is\n"
"# 1 + 1 + 4 + pack_16bits(1+10+320+10+fill)\n"
"# = 6+pack_16bits(341) = 6+pack_16bits(352)\n"
"# = 6+22 = 28bytes\n"
"\n"
"# One HXI has 40 ASIC chips, and therefore, the max length of\n"
"# the event data part (of the raw event telemetry) becomes\n"
"# 40chips x 28bytes = 1120bytes\n"
"\n"
"# Origin     : Pre-Pipe line, copy from the telemetry"
"\n"
"ttype# = RAW_ASIC_DATA\n"
"#tform# = 1PB(1120)\n"
"#hon: June15 2012 B34 R256 meeting yielded this:\n"
"#hon: each (occurance) group of variable-length blocks of ASIC TLM data\n"
"#hon: (or blob of blocks) has a fixed-length TLM (primary) header sized\n"
"#hon: to 7 shorts (14 bytes). each individual ASIC block of chan adc data\n"
"#hon: has a 59 bit header (of which the active chan bit flag is crucial)\n"     
"#hon: summing header (i.e. active chan bit flag) bits + ref chan + \n"
"#hon: 32 (max) 10bit adc chans + 10bit noise chan + stop bit yields:\n"
"#hon: 16+16+16+11+(32*10)+10+1 == 390 bits\n"
"#hon: 390.0/16 == 24.375 must be rounded up to 25 shorts\n"
"#hon: 25*40 ==> 1000 + 7 (primary) header shorts ==> 1007 shorts\n"
"tform# = 1PB(2014)\n";
    //
    // write fff (fits template) file:
    int sz = writeTextFile(content, fff_filename);
    //
    // sff part needs to be congruent with test i/o hard-coded struct
    // as declared/defined in ahHXItyps.h and implementated i/o
    // in ioHXI.cxx and doHXI.cxx ...
    content += \
"#------------------------------------------------------------\n"
"#------------------------------------------------------------\n"
"# Columns added for SFF\n"
"#------------------------------------------------------------\n"
"#------------------------------------------------------------\n"
"\n"
"# Name       : ASIC Header Data\n"
"# Data type  : 24 bits information packed to 32bits and treated as Unsigned Integer (UK)\n"
"# hon: or UJ ? but fancy envent telemetry chart shows 3*16 == 48bits?"
"# Max length : 40 (limited by the number of ASICs(i.e. 40))\n"
"ttype# = ASIC_Header_Data\n"
"# tform# = 1PUK(40)\n"
"tform# = 1PJ(40)\n"
"\n"
"# Name       : Original ASIC IDs\n"
"# Data type  : Unsinged byte\n"
"# Max length : 40 (limited by the number of ASICs(i.e. 40))\n"
"ttype# = Original_ASIC_IDs\n"
"tform# = 1PB(40)\n"
"\n"
"\n"
"# Name       : Remapped ASIC IDs\n"
"# Data type  : Unsinged byte\n"
"# Max length : 40 (limited by the number of ASICs(i.e. 40))\n"
"ttype# = Remapped_ASIC_IDs\n"
"tform# = 1PB(40)\n"
"\n"
"# Name       : Original Channel bit flags\n"
"# Data type  : Unsigned short (2byte integer = UI) or X bits?\n"
"# Max length : 1280 (40chips*32channels)\n"
"ttype# = Original_Channel_BitFlags\n"
"#hon: bits?\n"
"#tform# = 1PX(1280)\n"
"tform# = 1PUJ(40)\n"
"\n"
"# Name       : Remapped Channel IDs\n"
"# Data type  : Unsigned short (2byte integer = UI)\n"
"# Max length : 1280 (40chips*32channels)\n"
"ttype# = Remapped_Channel_IDs\n"
"tform# = 1PI(1280)\n"
"\n"
"# Calculated as the sum of the active channel x ASIC.\n"
"# hon -- number of active chanel per asic to 40 hxi asics\n"
"# This value is max of 32 for HXI and 64 for SGD\n"
"\n"
"#ttype# = NUM_ACTIVE_CHAN\n"
"#tform# = 1PI(40)\n"
"ttype# = NumberOf_Active_Channels\n"
"tform# = 1PB(40)\n"
"\n"
"# Name       : ASIC Reference Channel\n"
"# Data type  : 10 bits after the channel number. It represents the\n"
"# reference channel." 
"# Max length : 40 (limited by the number of ASICs(i.e. 40))\n"
"#ttype# = REF_CHAN\n"
"#tform# = 1PJ(40)\n"
"ttype# = Reference_Channels\n"
"tform# = 1PI(40)\n"
"\n"
"# Name       : ASIC Common Noise Data\n"
"# Data type  : last 10 bits representing the common noise for this asic\n"
"# Max length : 40 (limited by the number of ASICs(i.e. 40))\n"
"#ttype# = ASIC_CMN\n"
"#tform# = 1PJ(40)\n"
"ttype# = CommonNoise_Channels\n"
"tform# = 1PI(40)\n"
"\n"
"# Name       : Raw ADC Data of each remapped ch\n"
"# Data type  : 10-bit pulse height data packet to Unsigned short (16bits = UI)\n"
"# Max length : 1280 (40chips*32channels)\n"
"#ttype# = ADC_PH\n"
"ttype# = Raw_ADC_Pulse_Heights\n"
"tform# = 1PI(1280)\n"
"\n"
"# Name       : Calibrated ADC Data of each remapped ch\n"
"# Data type  : 10-bit pulse height data packet to Unsigned short (16bits = UI)\n"
"# Max length : 1280 (40chips*32channels)\n"
"#ttype# = ADC_PI\n"
"ttype# = Calibrated_ADC_Pulse_Heights\n"
"#tform# = 1PJ(1280)\n"
"tform# = 1PI(1280)\n";
    //
    // write sff (fits template) file:
    sz += writeTextFile(content, sff_filename);
    clog << __PRETTY_FUNCTION__ << " " << sz << " created: " << fff_filename << " and " << sff_filename << endl;
    return sz;
  } // initHXItemplates

} // namespace ahhxi

#endif //

