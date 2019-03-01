/** 
 * \file ahdelay.cxx
 * \brief functions to act on the CALDB instrument delay file for AstroH.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/13 18:32:08 $
 *
 */
 
#include "ahtime/ahdelay.h"
#include "ahfits/ahfits.h"
#include "ahlog/ahlog.h"

#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  void write_delay_template(const std::string & tplfile, const int & nrows) {

    std::vector<std::string> ext;
    ext.push_back("SXS PSP 1");
    ext.push_back("SXS PSP 2");
    ext.push_back("HXI1");
    ext.push_back("HXI2");
    ext.push_back("SGD1");
    ext.push_back("SGD2");
    ext.push_back("SXI");

    std::ofstream out;
    out.open(tplfile.c_str());
    out << "# Template file for instrument delay FITS file" << std::endl;
    out << "#" <<std::endl;
    out << "# each extension has two columns: " << std::endl;
    out << "#   1) time since epoch in seconds" << std::endl;
    out << "#   2) instrument delay in microseconds" << std::endl;
    out << "" << std::endl;

    std::vector<std::string>::iterator it;
    for (it=ext.begin(); it < ext.end(); it++) {
      out << "" << std::endl;
      out << "# ===== " << *it << " =====" << std::endl;
      out << "xtension = bintable" << std::endl;
      out << "extname = '" << *it << "'" << std::endl;
      out << "naxis2 = " << nrows << std::endl;
      out << "" << std::endl;
      out << "ttype# = TIME" << std::endl;
      out << "tform# = J" << std::endl;
      out << "tunit# = 's'" <<std::endl;
      out << "" << std::endl;
      out << "ttype# = DELAY" << std::endl;
      out << "tform# = D" << std::endl;
      out << "tunit# = 'us'" <<std::endl;
      out << "" << std::endl;
    }

    out.close();
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void create_delay_fits(const std::string & filename, 
                         const std::string & tplfile) {

    // make FITS file from template
    ahfits::AhFitsFilePtr fptr;
    ahfits::ahFitsCreate(filename.c_str(),tplfile.c_str(),&fptr,true);
    ahfits::ahFitsClose(fptr);

  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void write_delay_times(const std::string & filename, 
                         const std::string & inst,
                         const std::vector<int> times,
                         const std::vector<double> delays) {

    // check if times and delays arrays are of equal size
    if (times.size() != delays.size()) {
      AH_THROW_RUNTIME("arrays for times and delays have different lengths");
    }

    // open file
    ahfits::AhFitsFilePtr fptr;
    ahfits::ahFitsOpen(filename.c_str(),inst.c_str(),&fptr);
    if (!ahfits::ahFitsReadOK(fptr)) {
      AH_THROW_RUNTIME("failed to open instrument delay FITS file");
    }

    // make connections with local variable
    int time;
    double delay;
    ahfits::ahFitsConnect(fptr,"TIME",&time,0);
    ahfits::ahFitsConnect(fptr,"DELAY",&delay,0);

    // loop through times/delays, local=vector[i], write row
    // be sure to check if at last row
    ahfits::ahFitsFirstRow(fptr);
    if (!ahfits::ahFitsReadOK(fptr)) {
      ahfits::ahFitsClose(fptr);
      std::stringstream msg;
      msg << "no rows in FITS file? " << filename << " : " << inst;
      AH_THROW_RUNTIME(msg.str());
    }
    int nrow=times.size();
    for (int irow=0; irow < nrow; irow++) {
      time=times[irow];
      delay=delays[irow];
      ahfits::ahFitsWriteRow(fptr);

      if (irow == nrow-1) break;
      ahfits::ahFitsNextRow(fptr);
      if (!ahfits::ahFitsReadOK(fptr)) {
        ahfits::ahFitsClose(fptr);
        std::stringstream msg;
        msg << "cannot go to next row: " << irow << "(" << filename << " : " 
            << inst << ")";
        AH_THROW_RUNTIME(msg.str());
      }
    }

    // close FITS file
    ahfits::ahFitsClose(fptr);
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string get_caldb_delay() {
    return "delay.fits";
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  double get_delay(const std::string inst, const int time) {
    // open file
    std::string filename=get_caldb_delay();
    ahfits::AhFitsFilePtr fptr;
    ahfits::ahFitsOpen(filename.c_str(),inst.c_str(),&fptr);
    if (!ahfits::ahFitsReadOK(fptr)) {
      AH_THROW_RUNTIME("failed to open instrument delay FITS file");
    }

    // make connections to local variables
    int l_time;
    ahfits::ahFitsConnect(fptr,"TIME",&l_time,0);
    double l_delay;
    ahfits::ahFitsConnect(fptr,"DELAY",&l_delay,0);

    // read in table and simultaneously sort
    std::vector<int> times;
    std::vector<double> delays;
    for (ahfits::ahFitsFirstRow(fptr); ahfits::ahFitsReadOK(fptr);
         ahfits::ahFitsNextRow(fptr)) {
      
      ahfits::ahFitsReadRow(fptr);

      // if l_time and l_delay are zero; skip
      if (l_time == 0 && l_delay == 0.0) continue;

      // find where to insert delay
      int j;
      int ntimes=times.size();
      for (j=0; j < ntimes; j++) {
        if (l_time < times[j]) break;
      }
      times.insert(times.begin()+j,l_time);
      delays.insert(delays.begin()+j,l_delay);
    }

    // if only one point in file, return that delay
    if (times.size() == 1) {
      AH_WARN(ahlog::HIGH) << "only one delay in CALDB, no interpolation"
                           << std::endl;
      return delays[0];
    }

    // close FITS file
    ahfits::ahFitsClose(fptr);


    // *** linear interpolation ***

    // find where time fits in times list
    int j;
    for (j=0; j < (int)times.size(); j++) {
      if (time < times[j]) break;
    }

    // check if time matches point exactly
    if (time == times[j]) return delays[j];

    // bracket solution, giving warning if extrapolating
    j--;
    if ( j < 0 ) {
      AH_WARN(ahlog::HIGH) << "time below range in CALDB; extrapolating"
                           << std::endl;
      j++;
    }
    if ( (j+1) == times.size() ) {
      AH_WARN(ahlog::HIGH) << "time above range in CALDB; extrapolating"
                           << std::endl;
      j--;
    }

    // interpolate
    double out=delays[j]+(delays[j+1]-delays[j])*(double)(time-times[j])/
                                                 (double)(times[j+1]-times[j]);

    return out;
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: ahdelay.cxx,v $
 Revision 1.3  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
