/** 
 * \file ahleapsec.cxx
 * \brief Functions to deal with leap seconds.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/17 22:36:36 $
 *
 */
 
#include "hdcal.h"
#include "ahlog/ahlog.h"
#include "ahfits/ahfits.h"
#include "ahtime/ahleapsec.h"

#include <sstream>
#include <stdlib.h>

namespace ahtime {

  // ---------------------------------------------------------------------------

  /// \callgraph
  LeapSecData & getLeapSecData() {
    static LeapSecData s_leapsecdata;
    return s_leapsecdata;
  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  std::string locateLeapSecFile(const std::string & leapsecfile) {

    if (leapsecfile == "CALDB") {
      const char* tele="gen";
      const char* instr="ins";
      const char* detname="-";
      const char* filt="-";
      const char* codenam="leapsecs";
      const char* strtdate="-";
      const char* strttime="-";
      const char* stpdate="-";
      const char* stptime="-";
      const char* expr="-";

      int maxret=1;
      int fnamesize=256;
      char* filename=new char[fnamesize];
      long extno=0;
      char* online=new char[10];
      int nret=0;
      int nfound=0;
      int status=0;

      HDgtcalf(tele,instr,detname,filt,codenam,strtdate,strttime,
               stpdate,stptime,expr,maxret,fnamesize,&filename,&extno,
               &online,&nret,&nfound,&status);

      if (status != 0) {
        std::stringstream serr;
        serr << "Could not get leapsecond file from CALDB; "
             << "HDgtcalf status: " << status;
        AH_THROW_RUNTIME(serr.str());
      }

      if (nfound == 0) {
        AH_THROW_RUNTIME("No file found");
      }

      return (std::string)filename;
    } else if (leapsecfile == "REFDATA") {
      char* tmp=getenv("LHEA_DATA");
      if (tmp == NULL) {
        std::string msg="using REFDATA for leap second file requires ";
        msg+="LHEA_DATA environment variable";
        AH_THROW_RUNTIME(msg);
      }
      return (std::string)tmp+"/leapsec.fits";
    }

    // not special value, must be file name    
    return leapsecfile;

  }

  // ---------------------------------------------------------------------------

  /// \callgraph
  void readLeapSecData(const std::string & leapsecfile) {
    // open leap second FITS file
    ahfits::AhFitsFilePtr infile;
    const char * infile_name=leapsecfile.c_str();
    ahfits::ahFitsOpen(infile_name, "LEAPSEC", &infile);
    if (!ahfits::ahFitsReadOK(infile)) {
      AH_THROW_RUNTIME("failed to open leap second table");
    }

    // connect local variables to FITS columns
    char fdate[11];
    char* fdptr=fdate;
    ahFitsConnect(infile,"DATE",&fdptr,0);

    char ftime[16];
    char* ftptr=ftime;
    ahFitsConnect(infile,"TIME",&ftptr,0);

    double fleapsecs;
    ahFitsConnect(infile,"LEAPSECS",&fleapsecs,0);

    // loop over rows and check if date is after Astro-H epoch
    LeapSecData* out=&getLeapSecData();
    out->clear();
    for (ahfits::ahFitsFirstRow(infile); ahfits::ahFitsReadOK(infile); 
         ahfits::ahFitsNextRow(infile)) {

      ahfits::ahFitsReadRow(infile);
      std::string dtstr=(std::string)fdate+(std::string)"T"+(std::string)ftime;
      (*out)[dtstr]=fleapsecs;
    }

    // close leap second FITS file
    ahfits::ahFitsDisconnectAll(infile);
    ahfits::ahFitsClose(infile);
  }

  // ---------------------------------------------------------------------------

}

/* Revision Log
 $Log: ahleapsec.cxx,v $
 Revision 1.5  2012/07/17 22:36:36  mwitthoe
 allow ahtime tool to access leap second file from REFDATA

 Revision 1.4  2012/07/13 18:32:08  mwitthoe
 clean up ahtime code


*/
