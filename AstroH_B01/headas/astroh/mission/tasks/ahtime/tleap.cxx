/*
 * This code will retrieve the LEAPSECOND table from CALDB and get
 * the number of leap seconds between the AstroH epoch and a given time.
 */

#include <stdexcept>

#include "ape/ape_error.h"
#include "ape/ape_trad.h"

#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"
#include "ahtime/ahtime.h"
#include "ahtime/AhDateTime.h"
#include "ahtime/ahleapsec.h"


/// \brief Get parameter values
/// \param[out] dtstr string with date and time
/// \param[out] leapsecfile leap second FITS file location
void getPar(char*& dtstr, char*& leapsecfile);

/// \brief Read in leap second FITS file
/// \param[in] leapsecfile location of leap second FITS file
/// \param[out] leaptbl table of date/times and number of leap seconds
void initialize(char*& leapsecfile, ahtime::leaptable &leaptbl);

/// \brief Determine number of leap seconds between Astro-H epoch and given
/// time (which is printed to screen).
/// \param[in] dtstr string with date and time
/// \param[in] leaptbl leap second data table
void doWork(std::string dtstr,const ahtime::leaptable & leaptbl);

void finalize();


int main(int argc, char** argv) {

  char* dtstr;                     // hold date and time input
  char* leapsecfile;               // filename for leap second FITS file
  ahtime::leaptable leaptbl;       // container for leap second data

  try {
    ahgen::startUp(argc,argv);

    getPar(dtstr,leapsecfile);

    initialize(leapsecfile,leaptbl);     // load leap second table

    doWork((std::string)dtstr,leaptbl);  // get number of leap seconds to add
  } catch (const std::exception & x) {
    ahgen::setStatus(1);
    AH_ERR << x.what() << std::endl;
  }

  finalize();

  ahgen::shutDown();
  return ahgen::getStatus();

}

// ****************************************************************************

void getPar(char*& dtstr, char*& leapsecfile) {
  int status=eOK;

  // get string with date and time
  status=ape_trad_query_string("datetime",&dtstr);
  if (eOK != status) {
    throw std::runtime_error(AH_PREP("problem getting datetime parameter"));
  }

  // get location of leap second table
  status=ape_trad_query_string("leapsecfile",&leapsecfile);
  if (eOK != status) {
    std::string errmsg="problem getting location of leap second table";
    throw std::runtime_error(AH_PREP(errmsg));
  }

}

// ****************************************************************************

void initialize(char*& leapsecfile, ahtime::leaptable &leaptbl) {
  // get proper file name if leapsecfile is CALDB or REFDATA
  std::string filename=ahtime::getLeapSecTable(leapsecfile);

  // read leap second FITS file and return map of results
  leaptbl=ahtime::getLeapData(filename);
}

// ****************************************************************************

void doWork(std::string dtstr,const ahtime::leaptable & leaptbl) {
  // convert current and epoch times to datetime object
  ahtime::AhDateTime dt(dtstr);
  ahtime::AhDateTime dt0(ahtime::MISSION_EPOCH);

  // get time elapsed since AstroH epoch (without leapseconds)
  int telap=dt.secSince(dt0);
  std::cout << "seconds elapsed (no leap): " << telap << std::endl;
  if (telap < 0) 
    std::cout << "warning: leap second count incorrect for negative intervals"
              <<std::endl;

  int nleap=ahtime::leapSecFromEpoch(dt,leaptbl);
  std::cout << "number leap seconds: " << nleap << std::endl;
}

// ****************************************************************************

void finalize(){

}

// ****************************************************************************


