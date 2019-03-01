/** 
 * \file ahtime.cxx
 * \brief Assign time to housekeeping and event files.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/20 14:57:32 $
 *
 */
 
#include "ape/ape_error.h"
#include "ape/ape_trad.h"

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"
#include "ahtime/ahtime.h"
#include "ahmath/ahmath.h"
#include "ahtime/ahleapsec.h"
#include "ahtime/ahcolumndef.h"
#include "ahtime/ahdelay.h"
#include "ahtime/ahtimeassign.h"
#include "ahtime/ahtimfile.h"

/** 

\defgroup tool_ahtime Time Assignment (ahtime)
@ingroup mod_mission_tasks

The ahtime tool assigns the time to HK and science data using the preliminary
time (S_TIME) and the time indicator (TI) present in the HK and event files.
There are two assigned times: TIME gives the number of seconds relative to the
mission epoch (2014-01-01) and UTC which lists the date (year, month, day), time
(hour, minute, second), and remaining microseconds. The time assignment is
performed by interpolating data in a TIM file which contains the correspondence
between TIME and TI. This is sufficient for HK data, whereas science data
requires further look-up (e.g instrument delays).

*/


/** \addtogroup tool_ahtime
 *  @{
 */

/// \brief Get parameter values
/// \param[out] input name of input HK or event file
/// \param[out] output name of output file
/// \param[out] timfile name of TIM file
/// \param[out] localhk name HK file with local times
/// \param[out] columndef name of column definition FITS file (or CALDB)
/// \param[out] delayfile name of instrument delay FITS file (or CALDB)
/// \param[out] leapsecfile leap second FITS file location (or CALDB)
/// \param[out] timecolumn name of column holding assigned times
/// \param[out] calctime true to assign TIME column
/// \param[out] calcutc true to calculate UTC columns
/// \param[out] interpolation interpolation type
void getPar(std::string& input, std::string& output, std::string& timfile, 
            std::string& localhk, std::string& columndef, 
            std::string& delayfile, std::string& leapsecfile, 
            std::string& timecolumn, bool& calctime, bool& calcutc, 
            int& interpolation);

/// \brief Copy contents of input file to output file.  Read in leap second 
///        FITS file and get file names containing column definitions and 
///        instrument delays.
/// \param[in] input name of input HK or event file
/// \param[in] output name of output file
/// \param[in] leapsecfile location of leap second FITS file (or CALDB)
/// \param[in,out] columndef name of column definition FITS file (or CALDB)
/// \param[in,out] delayfile name of instrument delay FITS file (or CALDB)
void initialize(const std::string& input, const std::string& output,
                const std::string& leapsecfile, std::string& columndef,
                std::string& delayfile);

/// \brief Calculate TIME column in input file and output results
/// \param[in] input name of input HK or event file (contains S_TIME, TI data
//             and column where TIME is assigned)
/// \param[in] timfile name of TIM file
/// \param[in] columndef name of column definition FITS file (or CALDB)
/// \param[in] delayfile name of instrument delay FITS file (or CALDB)
/// \param[in] timecolumn name of TIME column to be updated
/// \param[in] calctime true to assign TIME column
/// \param[in] calcutc true to calculate UTC columns
/// \param[in] interpolation interpolation type enumeration
void doWork(const std::string& input, const std::string& timfile,
            const std::string& columndef, const std::string& delayfile, 
            const std::string& timecolumn, const bool & calctime, 
            const bool & calcutc, const int & interpolation);

void finalize();


// ****************************************************************************

/// \brief ahtime tool
///
/// Long description
int main(int argc, char** argv) {

  std::string input;               // name of HK or event FITS file
  std::string output;              // name of output file (will copy input)
  std::string timfile;             // name of TIM file
  std::string localhk;             // name of HK local time file
  std::string columndef;           // name of column definitions FITS file
  std::string delayfile;           // name of instrument delay FITS file
  std::string leapsecfile;         // name for leap second FITS file
  std::string timecolumn;          // name of column to be populated with time
  int interpolation;               // interpolation type string
  bool calctime;                   // true to calculate TIME column
  bool calcutc;                    // true to calculate UTC columns

  try {
    ahgen::startUp(argc,argv);

    getPar(input,output,timfile,localhk,columndef,delayfile,leapsecfile,
           timecolumn,calctime,calcutc,interpolation);

    initialize(input,output,leapsecfile,columndef,delayfile);

    doWork(output,timfile,columndef,delayfile,timecolumn,calctime,
           calcutc,interpolation);
  } catch (const std::exception & x) {
    ahgen::setStatus(1);
    AH_ERR << x.what() << std::endl;
  }

  finalize();

  ahgen::shutDown();
  return ahgen::getStatus();

}

// ****************************************************************************

void getPar(std::string& input, std::string& output, std::string& timfile, 
            std::string& localhk, std::string& columndef, 
            std::string& delayfile, std::string& leapsecfile, 
            std::string& timecolumn, bool& calctime, bool& calcutc, 
            int& interpolation) {
  int status=eOK;

  // get input file (HK or event FITS file)
  char* t_input;
  status=ape_trad_query_string("infile",&t_input);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting input parameter");
  }
  input=(std::string)t_input;

  // get input file (HK or event FITS file)
  char* t_output;
  status=ape_trad_query_string("outfile",&t_output);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting output parameter");
  }
  output=(std::string)t_output;

  // get TIME file
  char* t_timfile;
  status=ape_trad_query_string("timfile",&t_timfile);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting TIM file parameter");
  }
  timfile=(std::string)t_timfile;

  // get HK file used with science data (ignored for HK data)
  char* t_localhk;
  status=ape_trad_query_string("localtimehk",&t_localhk);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting HK file with local time");
  }
  localhk=(std::string)t_localhk;

  // get location of columndef file
  char* t_columndef;
  status=ape_trad_query_string("columndef",&t_columndef);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting columndef parameter");
  }
  columndef=(std::string)t_columndef;

  // get location of instrument delays
  char* t_delayfile;
  status=ape_trad_query_string("delayfile",&t_delayfile);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting delayfile parameter");
  }
  delayfile=(std::string)t_delayfile;

  // get location of leap second table
  char* t_leapsecfile;
  status=ape_trad_query_string("leapsec",&t_leapsecfile);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting location of leap second table");
  }
  leapsecfile=(std::string)t_leapsecfile;

  // get name of column to hold assigned time
  char* t_timecolumn;
  status=ape_trad_query_string("timecolumn",&t_timecolumn);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting name of assigned time column");
  }
  timecolumn=(std::string)t_timecolumn;

  // calculate TIME column?
  char t_calctime;
  status=ape_trad_query_bool("calctime",&t_calctime);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting calctime parameter");
  }
  calctime=false;
  if (t_calctime != 0) calctime=true;

  // calculate UTC columns?
  char t_calcutc;
  status=ape_trad_query_bool("calcutc",&t_calcutc);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting calcutc parameter");
  }
  calcutc=false;
  if (t_calcutc != 0) calcutc=true;

  // type of interpolation
  char* t_interpolation;
  status=ape_trad_query_string("interpolation",&t_interpolation);
  if (eOK != status) {
    AH_THROW_RUNTIME("problem getting interpolation type");
  }
  interpolation=ahmath::interpolation_type((std::string)t_interpolation);

  // calctime and calcutc cannot both be false
  if (!calctime && !calcutc)
    AH_THROW_RUNTIME("calctime and calcutc both false; nothing to do!");

}

// ****************************************************************************

void initialize(const std::string& input, const std::string& output,
                const std::string& leapsecfile, std::string& columndef,
                std::string& delayfile) {

  // copy contents of input to output
  ahfits::ahFitsClone(input,output);

  // get filename of column definitions from CALDB, if necessary
  if (columndef == "CALDB") {
    columndef=ahtime::get_caldb_coldef();
  }

  // get filename of instrument delays from CALDB, if necessary
  if (delayfile == "CALDB") {
    delayfile=ahtime::get_caldb_delay();
  }

  // load leap second data
  std::string tmp=ahtime::locateLeapSecFile(leapsecfile);
  ahtime::readLeapSecData(tmp);

}

// ****************************************************************************

void doWork(const std::string& input, const std::string& timfile, 
            const std::string& columndef, const std::string& delayfile, 
            const std::string& timecolumn, const bool & calctime, 
            const bool & calcutc, const int & interpolation) {


  // open FITS file
  ahfits::AhFitsFilePtr fptr;
  ahfits::ahFitsOpen(input.c_str(),"",&fptr);
  if (!ahfits::ahFitsReadOK(fptr))
    AH_THROW_RUNTIME("failed to open FITS file: "+input);

  // read TIM data
  /// \internal
  /// \note reading the TIM file here is okay if file is smallish; if TIM file
  ///   is large, may be better to read it into a buffer as needed in the
  ///   time assignment function
  ahtime::tim_data timdat;   // needs to be outside scope of if-statement
  if (calctime) {
    // get data from TIM file
    /// \internal
    /// \todo Using TIME_PACKETS for the extension name to be used in the TIM
    ///   file since that is what is in Suzaku; need to find out what the 
    ///   AstroH label will be
    std::string timhdu="TIME_PACKETS";
    ahtime::readTIMData(timfile,timhdu,timdat);
    if (timdat.size < 4)
      AH_THROW_RUNTIME("need at least 4 points in TIM file: "+timfile);
  }

  // loop over HDUs
  do {

    // nothing to do if not a bintable
    if (!ahfits::ahFitsIsBintable(fptr)) continue;

    /// \internal
    /// \note Question: should we gather all common header information in a 
    ///   single call (from ahcommon) instead of reading each header value
    ///   separately?  It would be convenient to pass a struct with this info
    ///   to other functions.

    /// \internal
    /// \note I am using HDUCLASS to determine if the input is HK; but this
    ///   information might be in EXTNAME instead (as a substring).

    // get telescop, instrume, detname, and hduclass
    std::string telescop=ahfits::ahFitsGetKeyValStr(fptr,"TELESCOP");
    std::string instrume=ahfits::ahFitsGetKeyValStr(fptr,"INSTRUME");
    std::string detname=ahfits::ahFitsGetKeyValStr(fptr,"DETNAME");
    std::string hduclass=ahfits::ahFitsGetKeyValStr(fptr,"HDUCLASS");

    // check telescop
    /// \internal
    /// \todo in the comparision below, "AstroH" should be replaced with a 
    ///   constant in ahcommon.
    if (telescop != "AstroH") {
      ahfits::ahFitsClose(fptr);
      AH_THROW_RUNTIME("HDU in "+fptr->m_filename+" has wrong value for "+
                       "TELESCOP: "+telescop+" (should be AstroH)");
    }

    // check if HK or use INSTRUME and DETNAME to get column labels for
    // column definition and instrument delay CALDB files
    /// \internal
    /// \todo currently using strings to describe instrument (or HK) type;
    ///   these should be replaced with enumerations once the names have 
    ///   been decided.  The enumerations should be in ahcommon.
    std::string filetype="";
    if (hduclass == "HK") {
      filetype="HK";
    }
    else {
      ahfits::ahFitsClose(fptr);
      AH_THROW_RUNTIME("Only HK data currently supported");
    }

    // assign time
    try {
      ahtime::assign_time_hk(fptr,timfile,timecolumn,timdat,calctime,calcutc,
                             interpolation);
    } catch (const std::exception & x) {
      ahfits::ahFitsClose(fptr);
      AH_THROW_RUNTIME("failed to assign time: " + (std::string)x.what());
    }

  } while (ahfits::ahFitsNextHDU(fptr));

  // close event or HK file
  ahfits::ahFitsClose(fptr);

}

// ****************************************************************************

void finalize(){

}

// ****************************************************************************


/** @} */


/* Revision Log
 $Log: ahtime.cxx,v $
 Revision 1.14  2012/07/20 14:57:32  mwitthoe
 add Doxygen to ahtime tool

 Revision 1.13  2012/07/17 22:36:36  mwitthoe
 allow ahtime tool to access leap second file from REFDATA

 Revision 1.12  2012/07/17 19:01:36  mwitthoe
 ahtime tool: edit par file descriptions, always ask for localtimehk parameter, check if calctime and calcutc are both false

 Revision 1.11  2012/07/16 21:02:53  mwitthoe
 update ahtime library/tool header comments

 Revision 1.10  2012/07/16 20:10:20  mwitthoe
 move reading of TIM data outside of HDU loop in doWork() of ahtime tool

 Revision 1.9  2012/07/12 20:39:32  mwitthoe
 update ahtime tool to use new time assignment function and ahtime classes

 Revision 1.8  2012/06/28 23:21:03  mwitthoe
 ahtime library and tool: add support for calctime, calcutc, and interpolation parameters; moved HDU loop from library to tool


*/

