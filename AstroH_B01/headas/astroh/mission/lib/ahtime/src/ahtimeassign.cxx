/** 
 * \file ahtimeassign.cxx
 * \brief Assign time to housekeeping and event files.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 20:09:39 $
 *
 */
 
#include "ahtime/ahtimeassign.h"
#include "ahtime/AhTimeUTC.h"
#include "ahtime/AhTimeTAI.h"
#include "ahtime/AhTimeTT.h"
#include "ahtime/ahtimeconv.h"
#include "ahlog/ahlog.h"
#include "ahmath/ahmath.h"
#include "ahfits/ahfits.h"

#include <iostream>
#include <sstream>
#include <cmath>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  void assign_time_hk(ahfits::AhFitsFilePtr fptr, 
                      const std::string & timfile,
                      const std::string & timecolumn,
                      const ahtime::tim_data & timdat,
                      const bool & calctime, const bool & calcutc,
                      const int & interpolation) {

    /// \internal
    /// \note Much of the work in this function is only needed if calctime is
    ///   true.  Regardless of the state of calctime, the TIME column will
    ///   always be read so that UTC can be calculated.

    // check that timecolumn exists in current HDU
    if (!ahFitsHaveColumn(fptr,timecolumn))
      AH_THROW_RUNTIME("Time column, " + timecolumn + ", missing");

    // get units for TI and S_TIME to compare with TI file
    if (calctime) {
      std::string hk_ti_units, hk_stime_units;
      hk_ti_units=ahFitsColumnUnits(fptr,"TI");
      hk_stime_units=ahFitsColumnUnits(fptr,"S_TIME");
      if (hk_ti_units != timdat.ti_units)
        AH_THROW_RUNTIME("TI units differ in HK and TIM files");
      if (hk_stime_units != timdat.s_time_units)
        AH_THROW_RUNTIME("S_TIME units differ in HK and TIM files");
    }

    // get epoch from header values: MJDREFI & MJDREFF
    int mjdrefi=(int)ahfits::ahFitsGetKeyValLLong(fptr,"MJDREFI");
    double mjdreff=ahfits::ahFitsGetKeyValDbl(fptr,"MJDREFF");
    ahtime::AhTimeTT ttepoch(mjdrefi,mjdreff);
    ahtime::AhTimeUTC epoch;
    ahtime::convertTime(ttepoch,epoch);

    // variables to hold row data
    long long val_ti;
    double val_stime;
    double val_time;
    int val_yyyymmdd;
    int val_hhmmss;
    int val_microsec;

    // make connections
    /// \internal
    /// \todo need to replace TI and S_TIME column names with those in 
    ///   column definitions file (CALDB)
    ahfits::ahFitsConnect(fptr,"TI",&val_ti,0);
    ahfits::ahFitsConnect(fptr,"S_TIME",&val_stime,0);
    ahfits::ahFitsConnect(fptr,timecolumn.c_str(),&val_time,0);
    if (calcutc) {
      ahfits::ahFitsConnect(fptr,"YYYYMMDD",&val_yyyymmdd,0);
      ahfits::ahFitsConnect(fptr,"HHMMSS",&val_hhmmss,0);
      ahfits::ahFitsConnect(fptr,"MICROSEC",&val_microsec,0);
    }

    // loop over rows in FITS file
    int timpos=0;   // position in TIM data vector
    double time0=-1.;
    for (ahfits::ahFitsFirstRow(fptr); ahfits::ahFitsReadOK(fptr);
         ahfits::ahFitsNextRow(fptr)) {
      ahfits::ahFitsReadRow(fptr);

      if (calctime) {
        // get position in TIM file
        while (val_stime > timdat.s_time[timpos]) timpos++;

        // form array of points for interpolation
        bool extrap=false;                           // allow for extrapolation
        std::vector<long long> xarr;
        std::vector<double> yarr;
        int ipos=std::max(0,timpos-2);                 // ensure ipos >= 0
        if (ipos >= timdat.size) ipos=timdat.size-4;   // ensure ipos <= max
        if (timdat.ti[ipos+3]-timdat.ti[ipos] < 0) {   // TI resets in range
          // for any reasonably large ti cycle time and fine-enough granularity
          // of TI values in the TIM file, we can use the HK TI value to choose
          // which side of the discontinuity to take; that is, to which boundary
          // value is the HK TI value closest.
          extrap=true;            // may need to extrapolate in this case
          int ireset=ipos+1;
          while (timdat.ti[ireset] > timdat.ti[ipos]) ireset++;
          if (timdat.ti[ipos]-val_ti < val_ti-timdat.ti[ipos+3]) {
            ipos=ireset-4;                // before reset
          } else {
            ipos=ireset;                  // after reset
          }
        }
        for (int i=0; i < 4; i++) {
          xarr.push_back(timdat.ti[i+ipos]);
          yarr.push_back(timdat.s_time[i+ipos]);
        }

        // interpolate
        val_time=ahmath::interpolate_single(val_ti,xarr,yarr,interpolation,
                                            extrap);

        //  first time is written to header
        if (time0 < 0.) time0=val_time;
      }

      // calculate UTC
      if (calcutc) {
        ahtime::AhTimeTAI ttai(val_time,epoch);
        ahtime::AhTimeUTC tutc;
        ahtime::convertTime(ttai,tutc);
        val_yyyymmdd=tutc.integer_date();
        val_hhmmss=tutc.integer_time();
        val_microsec=tutc.integer_subsecond(6);
      }

      // write TIME and UTC
      ahfits::ahFitsWriteRow(fptr);
    }

    // correct header times
    if (calctime) {
      ahtime::AhTimeUTC tutc;
      ahtime::AhTimeTAI ttai;
      std::string tutcstr;
      ttai.set(time0,epoch);
      ahtime::convertTime(ttai,tutc);
      tutcstr=catTimeSubseconds(tutc.string_datetime(),tutc.subsecond(),6);
      ahfits::ahFitsWriteKeyValDbl(fptr,"TSTART",time0,"");
      ahfits::ahFitsWriteKeyValStr(fptr,"DATE_OBS",tutcstr,"");

      ttai.set(val_time,epoch);
      ahtime::convertTime(ttai,tutc);
      tutcstr=catTimeSubseconds(tutc.string_datetime(),tutc.subsecond(),6);
      ahfits::ahFitsWriteKeyValDbl(fptr,"TSTOP",val_time,"");
      ahfits::ahFitsWriteKeyValStr(fptr,"DATE_END",tutcstr,"");
    }

  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: ahtimeassign.cxx,v $
 Revision 1.7  2012/07/16 20:09:39  mwitthoe
 move TI and S_TIME units from TIM file into TIM struct; remove reading of TIM file from function in ahtimeassign (reading is now done outside HDU loop)

 Revision 1.6  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
