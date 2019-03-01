#if !defined(SGDTEMPLATE_C)
#define SGDTEMPLATE_C const char sgdtemplate_rcsId_svnId[] = "$Id: SGDtemplate.cxx,v 1.4 2012/07/16 13:43:56 dhon Exp $"; 

// Local includes.
#include "ahhxisgd.h"
AHHXISGD_H(sgdtemplate)

#include "ahsgd/ahSGDtyps.h"
AHSGDTYPS_H(sgdtemplate)

#include "ahhxi/ahHXItyps.h"
AHHXITYPS_H(sgdtemplate)

#include "ahtlm/ahBits.h"
AHBITS_H(sgdtemplate)

#include "ahtlm/ahSlice.h"
AHSLICE_H(sgdtemplate)

#include "ahtlm/ahTLM.h"
AHTLM_H(sgdtemplate)

#include "ahtlm/ahTLMIO.h"
AHTLMIO_H(sgdtemplate)

#include "ahtlm/ahTLMinfoCache.h"
AHTLMINFOCACHE_H(sgdtemplate)

using namespace std;
using namespace ahtlm;

namespace {
  const string cvsId = "$Name: AstroH_B01 $ $Id: SGDtemplate.cxx,v 1.4 2012/07/16 13:43:56 dhon Exp $";
}

namespace ahsgd {

  /// SGD template default content
  int initSGDtemplates(string& content, string& fff_filename, string& sff_filename) {
    const string inpath = "./input/";
    if( fff_filename == "" ) fff_filename = sgdFFVersion(inpath); // "./input/SGD_FFFv00.tpl";
    if( sff_filename == "" ) sff_filename = sgdSFVersion(inpath); // "./inpupt/SGD_SFFv00.tpl";
    content = \
"xtension=bintable\n"
"extname = events\n"
"telescop= astro-h\n"
"instrume = SGD\n"
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
"ttype# = Occurence_ID\n"
"tform# = 1J\n"
"\n"
"ttype# = Local_Time\n"
"tform# = 1J\n"
"\n"
"#ttype# = FLAG_SEU\n"
"#tform# = 1X\n"
"ttype# = FLAG_SingleEventUpset\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_LengthCheck\n"
"#tform# = 1X\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_CalMode\n"
"#tform# = 1X\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_InitialTriggerPattern\n"
"#tform# = 31X\n"
"tform# = 1J\n"
"\n"
"ttype# = FLAG_TriggerPattern\n"
"#tform# = 31X\n"
"tform# = 1J\n"
"\n"
"#ttype# = FLAG_Trigger\n"
"#tform# = 6X\n"
"\n"
"ttype# = FLAG_CCBusy\n"
"#tform# = 3X\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_HitPatternCC\n"
"#tform# = 3X\n"
"tform# = 1B\n"
"\n"
"ttype# = FLAG_HitPattern\n"
"#tform# = 4X\n"
"tform# = 1B\n"
"\n"
"#ttype# = FLAG_FastBGO\n"
"#tform# = 4X\n"
"ttype# = FLAG_FastShieldHitPattern\n"
"tform# = 1B\n"
"\n"
"#1J or 24X (actually 24-bit width)\n"
"ttype# = LiveTime\n"
"tform# = 1J\n"
"\n"
"ttype# = nHitASICs\n"
"#tform# = 1B\n"
"tform# = 1I\n"
"\n"
"#------------------------------------------------------------\n"
"# Original ASIC Event Data\n"
"#------------------------------------------------------------\n"
"# Data output from each ASIC have\n"
"# flags (4 bits),\n"
"# ASIC ID (12 bits),\n"
"# DM (1 bit) + channel data (64 bits) + CM (1 bit),\n"
"# Reference (10 bits),\n"
"# ADC (10 bits x n ; 10*64 = 640 bits max),\n"
"# Common mode noise (10 bits),\n"
"# and 0s for filling.\n"
"#\n"
"# Therefore, the maximum length of one ASIC data is\n"
"# pack_16bits(4+12+1+64+1+10+640+10+fill)\n"
"# = pack_16bits(742+fill) = pack_16bits(752)\n"
"# = 47x2 bytes = 94 bytes\n"
"#\n"
"# One SGD has (208/CC)x3CC=624 ASIC chips, and therefore, the max length of\n"
"# the event data part (of the raw event telemetry) becomes\n"
"# 624 chips x 94 bytes = 58656 bytes\n"
"\n"
"ttype# = RAW_ASIC_DATA\n"
"tform# = 1PB(58656)\n";
    //
    // write fff (fits template) file:
    int sz = writeTextFile(content, fff_filename);
    //
    content += \
"#------------------------------------------------------------\n"
"#------------------------------------------------------------\n"
"# Columns added for SFF\n"
"#------------------------------------------------------------\n"
"#------------------------------------------------------------\n"
"\n"
"# Name       : ASIC Header Data\n"
"# Data type  : 24 bits information packed to 32bits and treated as Unsigned Integer (V)\n"
"# hon: or UJ ?\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"ttype# = ASIC_Header_Data\n"
"# tform# = 1PV(39936)\n"
"tform# = 1PJ(39936)\n"
"\n"
"# Name       : Original ASIC ID\n"
"# Data type  : Signed short (2byte integer = I)\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"ttype# = Original_ASIC_ID\n"
"tform# = 1PI(39936)\n"
"\n"
"# Name       : Remapped ASIC ID\n"
"# Data type  : Signed short (2byte integer = I)\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"ttype# =  Remapped_ASIC_ID\n"
"tform# = 1PI(39936)\n"
"\n"
"# Name       : Original Channel ID\n"
"# Data type  : Unsigned byte (B)\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"#ttype# =  Original_Channel_ID\n"
"#tform# = 1PB(39936)\n"
"\n"
"# Name       : Original Channel BitFlags\n"
"# Data type  : Unsigned byte (B)\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"ttype# =  Original_Channel_BitFlags\n"
"#tform# = 1PB(39936)\n"
"# hon: or 624 64bit ints ?\n"
"# hon: but is we want to avoid 64 bit ints use 2*624 32 bit ints?\n"
"#tform# = 1PUJ(1248)\n"
"tform# = 1PUK(624)\n"
"\n"
"# Name       : Remapped Channel ID\n"
"# Data type  : Unsigned byte (B)\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"ttype# = Remapped_Channel_ID\n"
"#tform# = 1PB(39936)\n"
"tform# = 1PUI(39936)\n"
"\n"
"# Calculated as the sum of the active channel x ASIC.\n"
"# hon -- number of active chanel per asic to sgd 624 asics\n"
"# This value is max of 32 for HXI and 64 for SGD\n"
"\n"
"#ttype# = NUM_ACTIVE_CHAN\n"
"ttype# = NumberOf_Active_Channels\n"
"tform# = 1PB(624)\n"
"\n"
"# Name       : ASIC Reference Channel\n"
"# Data type  : 10 bits after the channel number. It represents the\n"
"# reference channel." 
"# Max length : 624 (limited by the number of ASICs(i.e. 624))\n"
"#ttype# = REF_CHAN\n"
"#tform# = 1PJ(624)\n"
"ttype# = Reference_Channels\n"
"tform# = 1PI(624)\n"
"\n"
"# Name       : ASIC Common Noise Data\n"
"# Data type  : last 10 bits representing the common noise for this asic\n"
"# Max length : 624 (limited by the number of ASICs(i.e. 624))\n"
"#ttype# = ASIC_CMN\n"
"#tform# = 1PJ(624)\n"
"ttype# = CommonNoise_Channels\n"
"tform# = 1PI(624)\n"
"\n"
"# Name       : Raw ADC Data\n"
"# Data type  : 10-bit pulse height data packet to signed short (16bits = I)\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"ttype# = Raw_ADC_Pulse_Heights\n"
"tform# = 1PI(39936)\n"
"\n"
"# Name       : Calibrated ADC Data\n"
"# Data type  : 10-bit pulse height data packet to signed short (16bits = I)\n"
"# Max length : 39936 (624 chips * 64 channels)\n"
"ttype# = Calibrated_ADC_Pulse_Heights\n"
"#tform# = 1PJ(39936)\n"
"tform# = 1PI(39936)\n";
    //
    // write sff (fits template) file:
    sz += writeTextFile(content, sff_filename);
    clog << __PRETTY_FUNCTION__ << " " << sz << " created: " << fff_filename << " and " << sff_filename << endl;
    return sz;
  } // initSGDtemplates

} // namespace ahsgd

#endif // AHSGDTEMPLATE_C
