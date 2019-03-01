/** 
 * \file ahtimefile.cxx
 * \brief read S_TIME and TI data from TIM file
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 20:09:39 $
 *
 */
 
#include "ahtime/ahtimfile.h"
#include "ahfits/ahfits.h"
#include "ahlog/ahlog.h"

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  void clearTIMData(tim_data & timdat) {
    timdat.size=0;
    timdat.s_time.clear();
    timdat.ti.clear();
    timdat.s_time_units="";
    timdat.ti_units="";
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void readTIMData(const std::string & filename, const std::string & hduname,
               tim_data & timdat) {

    // open file
    ahfits::AhFitsFilePtr fptr;
    ahfits::ahFitsOpen(filename.c_str(),hduname.c_str(),&fptr);
    if (!ahfits::ahFitsReadOK(fptr)) {
      AH_THROW_RUNTIME("failed to open TIM file");
    }

    // erase old data
    clearTIMData(timdat);

    // column names
    std::string ti_colname="TI";
    std::string stime_colname="S_TIME";

    // get units
    timdat.ti_units=ahFitsColumnUnits(fptr,ti_colname.c_str());
    timdat.s_time_units=ahFitsColumnUnits(fptr,stime_colname.c_str());

    // make connections to local variables
    long long ti;
    ahfits::ahFitsConnect(fptr,ti_colname.c_str(),&ti,0);
    double r_time;
    ahfits::ahFitsConnect(fptr,stime_colname.c_str(),&r_time,0);

    // get new data
    for (ahfits::ahFitsFirstRow(fptr); ahfits::ahFitsReadOK(fptr);
         ahfits::ahFitsNextRow(fptr)) {

      ahfits::ahFitsReadRow(fptr);
      timdat.s_time.push_back(r_time);
      timdat.ti.push_back(ti);
    }
    timdat.size=timdat.s_time.size();

    // close FITS file
    ahfits::ahFitsClose(fptr);
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: ahtimfile.cxx,v $
 Revision 1.5  2012/07/16 20:09:39  mwitthoe
 move TI and S_TIME units from TIM file into TIM struct; remove reading of TIM file from function in ahtimeassign (reading is now done outside HDU loop)

 Revision 1.4  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
