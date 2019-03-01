/** \file make_hktim.cxx
 *  \brief Make test TIM and HK files for testing
 *  \author Mike Witthoeft
 *  \date $Date: 2012/07/12 20:36:58 $
 */

#include "ahtime/ahtime.h"
#include "ahtime/AhTimeUTC.h"
#include "ahtime/AhTimeTAI.h"
#include "ahtime/ahtimeconv.h"
#include "ahtime/ahleapsec.h"
#include "ahlog/ahlog.h"
#include "ahfits/ahfits.h"
#include "ahmath/ahmath.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <stdlib.h>


/// \brief add TI to epoch, returning integers giving date and time
/// \param[in] epoch epoch as AhTimeUTC instance
/// \param[in] ti TI value
/// \param[in] icycle cycle number of given TI value (0 = in 1st cycle)
/// \param[in] ti_cycle length of TI cycle (as ordinal)
/// \param[in] ti_units value of n in (1/2^n s) used for the units of TI
/// \param[out] yyyymmdd output date as integer: YYYYMMDD
/// \param[out] hhmmss output time as integer: HHMMSS
/// \param[out] micro number of microseconds
/// \param[out] s_time number of seconds since epoch
void calc_utc(ahtime::AhTimeUTC & epoch, const int & ti, const int & icycle,
              const int & ti_cycle, const int & ti_units, int & yyyymmdd, 
              int & hhmmss, int & microsec, double & s_time) {

  // get whole and fractional number of seconds to be added
  int addi=ti/ti_units+(icycle*ti_cycle)/ti_units;
  int left=ti%ti_units+(icycle*ti_cycle)%ti_units;
  while (left > ti_units) {
    addi++;
    left-=ti_units;
  }
  double addf=(double)left/(double)ti_units;

  // add time by defining TAI time and converting to UTC
  ahtime::AhTimeTAI tai(addi,addf,epoch);
  ahtime::AhTimeUTC utc;
  ahtime::convertTime(tai,utc);

  // seconds since epoch
  s_time=(double)addi+addf;

  // get pieces
  yyyymmdd=utc.integer_date();
  hhmmss=utc.integer_time();
  microsec=utc.integer_subsecond(6);
}



/// \brief write template for TIM file
/// \param[in] tplfile name of template file
/// \param[in] ti_units_tim value of n in (1/2^n s) used for the units of TI
/// \param[in] ti_cycle length of TI cycle (as ordinal)
/// \param[in] epoch epoch as AhTimeUTC instance
void write_tim_template(const std::string & tplfile, const int & ti_units,
                        ahtime::AhTimeUTC & epoch) {

  // get MJD values of epoch
  ahtime::AhTimeTT mjdtime;
  ahtime::convertTime(epoch,mjdtime);

  int factor=(1 << ti_units);

  std::ofstream out;
  out.open(tplfile.c_str());

  out << "# Template file for AstroH TIM file" << std::endl;
  out << "#" << std::endl;
  out << "xtension=bintable" << std::endl;
  out << "#" << std::endl;
  out << "extname='TIME_PACKETS'" << std::endl;
  out << "naxis2=1" << std::endl;
  out << "#" << std::endl;

  out << "telescop = AstroH" << std::endl;
  out << "instrume = " << std::endl;
  out << "detname= " << std::endl;
  out << "hduclass= TIM" << std::endl;
  out << "mjdrefi=" << mjdtime.mjdi() << std::endl;
  out << "mjdreff=" << std::setprecision(15) << mjdtime.mjdf() << std::endl;
  out << "#" << std::endl;

  out << "date_obs= " << std::endl;
  out << "date_end= " << std::endl;
  out << "tstart= " << std::endl;
  out << "tstop= " << std::endl;
  out << "#" << std::endl;

  out << "ttype# = S_TIME" << std::endl;
  out << "tform# = 1D" << std::endl;
  out << "tunit# = s" << std::endl;
  out << "#" << std::endl;

  out << "ttype# = YYYYMMDD" << std::endl;
  out << "tform# = 1J" << std::endl;
  out << "#" << std::endl;

  out << "ttype# = HHMMSS" << std::endl;
  out << "tform# = 1J" << std::endl;
  out << "#" << std::endl;

  out << "ttype# = MICROSEC" << std::endl;
  out << "tform# = 1J" << std::endl;
  out << "#" << std::endl;

  out << "ttype# = TI" << std::endl;
  out << "tform# = 1J" << std::endl;
  out << "tunit# = 1/" << factor << " s" << std::endl;
  out << "#" << std::endl;

  out << "" << std::endl;
  out.close();
}


/// \brief use template to create TIM file, then fill with data
/// \param[in] filename name of TIM file created
/// \param[in] tplfile name of template file
/// \param[in] ti_units value of n in (1/2^n s) used for the units of TI
/// \param[in] ti0 initial TI value
/// \param[in] dti TI step
/// \param[in] nti number of TI values
/// \param[in] ti_cycle maximum value of TI where it is reset to zero
/// \param[in] epoch epoch date/time as AhTimeUTC instance
void write_tim_fits(const std::string & filename, const std::string & tplfile, 
                    const int & ti_units, const int & ti0, const int & dti,
                    const int & nti, const int & ti_cycle, 
                    ahtime::AhTimeUTC & epoch) {

  // make FITS file from template
  ahfits::AhFitsFilePtr fptr;
  ahfits::ahFitsCreate(filename.c_str(),tplfile.c_str(),&fptr,true);
  ahfits::ahFitsClose(fptr);

  // re-open new file
  ahfits::ahFitsOpen(filename.c_str(),"TIME_PACKETS",&fptr);
  if (!ahfits::ahFitsReadOK(fptr)) {
    AH_THROW_RUNTIME("failed to re-open TIM FITS file");
  }

  // make connections from local variables to FITS columns
  double s_time;
  int yyyymmdd,hhmmss,microsec,ti;
  ahfits::ahFitsConnect(fptr,"S_TIME",&s_time,0);
  ahfits::ahFitsConnect(fptr,"YYYYMMDD",&yyyymmdd,0);
  ahfits::ahFitsConnect(fptr,"HHMMSS",&hhmmss,0);
  ahfits::ahFitsConnect(fptr,"MICROSEC",&microsec,0);
  ahfits::ahFitsConnect(fptr,"TI",&ti,0);

  int icycle=0;
  ti=ti0;
  double s_time0=-1.;
  for (int i=0; i < nti; i++) {
    if (ti >= ti_cycle) {       // allows for ti=0, not ti=ti_cycle
      ti=ti%ti_cycle;           // change >= to > for ti=ti_cycle
      icycle++;
    }

    calc_utc(epoch,ti,icycle,ti_cycle,ti_units,yyyymmdd,hhmmss,microsec,
             s_time);
    ahfits::ahFitsWriteRow(fptr);
    ahfits::ahFitsNextRow(fptr);
    ti+=dti;
    if (s_time0 < 0.) s_time0=s_time;
  }

  // write starting time headers
  ahtime::AhTimeUTC tutc;
  ahtime::AhTimeTAI ttai;
  ttai.set(s_time0,epoch);
  ahtime::convertTime(ttai,tutc);
  ahfits::ahFitsWriteKeyValDbl(fptr,"TSTART",s_time0,"");
  ahfits::ahFitsWriteKeyValStr(fptr,"DATE_OBS",tutc.string_datetime(),"");

  // write stopping time headers
  ttai.set(s_time,epoch);
  ahtime::convertTime(ttai,tutc);
  ahfits::ahFitsWriteKeyValDbl(fptr,"TSTOP",s_time,"");
  ahfits::ahFitsWriteKeyValStr(fptr,"DATE_END",tutc.string_datetime(),"");

  // close FITS file
  ahfits::ahFitsClose(fptr);
}


/// \brief write template for HK file
/// \param[in] tplfile name of template file
/// \param[in] ti_units_tim value of n in (1/2^n s) used for the units of TI
/// \param[in] ti_cycle length of TI cycle (as ordinal)
/// \param[in] epoch epoch date/time as AhTimeUTC instance
void write_hk_template(const std::string & tplfile, const int & ti_units,
                       ahtime::AhTimeUTC epoch) {

  // get MJD values of epoch
  ahtime::AhTimeTT mjdtime;
  ahtime::convertTime(epoch,mjdtime);

  int factor=(1 << ti_units);

  std::ofstream out;
  out.open(tplfile.c_str());

  std::vector<std::string> hdus;
  hdus.push_back("HK_DAT1");
  hdus.push_back("HK_DAT2");
  hdus.push_back("HK_DAT3");

  out << "# Template file for AstroH HK file" << std::endl;
  out << "" << std::endl;

  std::vector<std::string>::iterator it;
  for (it=hdus.begin(); it < hdus.end(); it++) {
    out << "xtension=bintable" << std::endl;
    out << "extname='" << *it << "'" << std::endl;
    out << "naxis2=1" << std::endl;
    out << "#" << std::endl;

    out << "telescop = AstroH" << std::endl;
    out << "instrume = " << std::endl;
    out << "detname= " << std::endl;
    out << "hduclass= HK" << std::endl;
    out << "mjdrefi=" << mjdtime.mjdi() << std::endl;
    out << "mjdreff=" << std::setprecision(15) << mjdtime.mjdf() << std::endl;
    out << "#" << std::endl;

    out << "date_obs= " << std::endl;
    out << "date_end= " << std::endl;
    out << "tstart= " << std::endl;
    out << "tstop= " << std::endl;
    out << "#" << std::endl;

    out << "ttype# = TIME" << std::endl;
    out << "tform# = 1D" << std::endl;
    out << "tunit# = s" << std::endl;
    out << "#" << std::endl;

    out << "ttype# = S_TIME" << std::endl;
    out << "tform# = 1D" << std::endl;
    out << "tunit# = s" << std::endl;
    out << "#" << std::endl;

    out << "ttype# = YYYYMMDD" << std::endl;
    out << "tform# = 1J" << std::endl;
    out << "#" << std::endl;

    out << "ttype# = HHMMSS" << std::endl;
    out << "tform# = 1J" << std::endl;
    out << "#" << std::endl;

    out << "ttype# = MICROSEC" << std::endl;
    out << "tform# = 1J" << std::endl;
    out << "#" << std::endl;

    out << "ttype# = TI" << std::endl;
    out << "tform# = 1J" << std::endl;
    out << "tunit# = 1/" << factor << " s" << std::endl;
    out << "#" << std::endl;
  }

  out << "" << std::endl;
  out.close();
}


/// \brief use template to create HK file, then fill with data
/// \param[in] filename name of HK file created
/// \param[in] tplfile name of template file
/// \param[in] ti_units value of n in (1/2^n s) used for the units of TI
/// \param[in] ti0 initial TI value
/// \param[in] dti TI step
/// \param[in] nti number of TI values
/// \param[in] ti_cycle maximum value of TI where it is reset to zero
/// \param[in] epoch epoch date/time in form YYYY-MM-DDTHH:MM:SS
void write_hk_fits(const std::string & filename, const std::string & tplfile, 
                   const int & ti_units, const int & ti0, const int & dti,
                   const int & nti, const int & ti_cycle, 
                   ahtime::AhTimeUTC & epoch) {

  // see random number generator
  srand(time(NULL));

  // get range of ti and cycle
  // kinda complicated, but method helps to avoid integer overflows
  int cycle0=0;
  int nnti=nti/ti_cycle;                         // nti = nnti*ti_cycle + lnti
  int lnti=nti%ti_cycle;
  int cycle1=(ti0+dti*lnti)/ti_cycle+dti*nnti;   // largest cycle
  int ti1=(ti0+dti*lnti)%ti_cycle;               // max TI in last cycle
  ti1-=dti;                         // since TIM file covers [t0,t1)

  // make FITS file from template
  ahfits::AhFitsFilePtr fptr;
  ahfits::ahFitsCreate(filename.c_str(),tplfile.c_str(),&fptr,true);
  ahfits::ahFitsClose(fptr);

  // re-open new file
  ahfits::ahFitsOpen(filename.c_str(),"",&fptr);
  if (!ahfits::ahFitsReadOK(fptr)) {
    AH_THROW_RUNTIME("failed to re-open HK FITS file");
  }

  // make connections from local variables to FITS columns
  double time,s_time;
  int yyyymmdd,hhmmss,ti;
  ahfits::ahFitsConnect(fptr,"TIME",&time,0);
  ahfits::ahFitsConnect(fptr,"S_TIME",&s_time,0);
//  ahfits::ahFitsConnect(fptr,"YYYYMMDD",&yyyymmdd,0);
//  ahfits::ahFitsConnect(fptr,"HHMMSS",&hhmmss,0);
  ahfits::ahFitsConnect(fptr,"TI",&ti,0);

  // loop over extensions
  do {
    if (!ahFitsIsBintable(fptr)) continue;

    // get random list of cycles
    std::vector<int> r_cycles;       // list of cycle values
    for (int i=0; i < nti; i++) {
      r_cycles.push_back(rand()%cycle1+cycle0);
    }
    std::sort(r_cycles.begin(),r_cycles.end());

    // get random list of TI values
    std::vector<int> cycle_bnd;      // cycle boundaries
    std::vector<int> r_tis;          // list of TI values
    int cur_cycle=r_cycles[0];
    for (int i=0; i < nti; i++) {
      while (1) {
        int r_ti=rand()%ti_cycle;
        if (r_cycles[i] == cycle0 && r_ti < ti0) continue;
        if (r_cycles[i] == cycle1 && r_ti > ti1) continue;
        r_tis.push_back(r_ti);
        break;
      }
      if (r_cycles[i] != cur_cycle) {
        cycle_bnd.push_back(i-1);
        cur_cycle=r_cycles[i];
      }
    }
    cycle_bnd.push_back(r_cycles.size()-1);
    int csize=cycle_bnd.size();

    // sort TI values for each cycle
    int istart=0;
    for (int icycle=0; icycle < csize; icycle++) {
      int iend=cycle_bnd[icycle]+1;
      std::cout << "icycle,start,end: " << icycle << ", " << istart << ", " << iend << std::endl;
      std::sort(r_tis.begin()+istart,r_tis.begin()+iend);
      istart=iend;
    }

    double s_time0=-1.;
    for (int i=0; i < nti; i++) {
      int microsec;
      ti=r_tis[i];
      calc_utc(epoch,r_tis[i],r_cycles[i],ti_cycle,ti_units,yyyymmdd,hhmmss,
               microsec,s_time);
      ahfits::ahFitsWriteRow(fptr);
      ahfits::ahFitsNextRow(fptr);
      if (s_time0 < 0.) s_time0=s_time;
    }

    // write starting time headers
    ahtime::AhTimeUTC tutc;
    ahtime::AhTimeTAI ttai;
    ttai.set(s_time0,epoch);
    ahtime::convertTime(ttai,tutc);
    ahfits::ahFitsWriteKeyValDbl(fptr,"TSTART",s_time0,"");
    ahfits::ahFitsWriteKeyValStr(fptr,"DATE_OBS",tutc.string_datetime(),"");

    // write stopping time headers
    ttai.set(s_time,epoch);
    ahtime::convertTime(ttai,tutc);
    ahfits::ahFitsWriteKeyValDbl(fptr,"TSTOP",s_time,"");
    ahfits::ahFitsWriteKeyValStr(fptr,"DATE_END",tutc.string_datetime(),"");

  } while (ahfits::ahFitsNextHDU(fptr));

  // close FITS file
  ahfits::ahFitsClose(fptr);
}



int main() {

// start log
ahlog::setup("make_hktim","!DEFAULT",2,true);

// define epoch
ahtime::AhTimeUTC epoch("2012-01-01T00:00:00",0.0);

// load leap second data
std::string leapsecfile=ahtime::locateLeapSecFile("CALDB");
ahtime::readLeapSecData(leapsecfile);

// TIM file names
std::string tpl_tim="timfile.tpl";
std::string timfile="test.tim";

// HK file names
std::string tpl_hk="hkfile.tpl";
std::string hkfile="test.hk";

// TIM: TI units 1/2^n; TI cycle
int ti_units_tim=5;
int ti_cycle_tim=1<<20;     // 2^20

// HK: TI units 1/2^n; TI cycle
int ti_units_hk=5;
int ti_cycle_hk=1<<20;     // 2^20

// range of TIM TI values
int ti0=100000;
int dti=2000;
int nti=5000;

// create and fill TIM file
write_tim_template(tpl_tim,ti_units_tim,epoch);
write_tim_fits(timfile,tpl_tim,ti_units_tim,ti0,dti,nti,ti_cycle_tim,epoch);

// create and fill HK file
write_hk_template(tpl_hk,ti_units_hk,epoch);
write_hk_fits(hkfile,tpl_hk,ti_units_hk,ti0,dti,nti,ti_cycle_hk,epoch);


}

/* Revision Log
 $Log: make_hktim.cxx,v $
 Revision 1.6  2012/07/12 20:36:58  mwitthoe
 increase precision of MJDREFF header in make_hktim

 Revision 1.5  2012/07/12 18:25:01  mwitthoe
 add MICROSEC column to sample HK file in make_hktim.cxx; do not fill UTC columns in HK file, let ahtime tool do that

 Revision 1.4  2012/07/12 17:53:18  mwitthoe
 make_hktim now writes header values for TSTART, TSTOP, DATE_OBS, and DATE_END; remove reference to AhDateTime and use new time classes instead

 Revision 1.3  2012/06/29 20:42:21  mwitthoe
 resolve compiler warning in make_hktim.cxx (ahtime test code)

 Revision 1.2  2012/06/28 23:21:03  mwitthoe
 ahtime library and tool: add support for calctime, calcutc, and interpolation parameters; moved HDU loop from library to tool

 Revision 1.1  2012/06/26 19:48:55  mwitthoe
 test code in ahtime library to generate sample TIM and HK filK files for time assignment


*/
