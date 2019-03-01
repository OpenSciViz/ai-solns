/** \file ahtimeconv.cxx
    \brief AstroH instrument time converter
    \author Mike Witthoeft
    \date $Date: 2012/06/20 17:26:59 $
*/


// Local includes.
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"
#include "ahtime/ahtime.h"

// Regional includes.
#include "ape/ape_error.h"
#include "ape/ape_trad.h"

// ISO includes.
#include <cstring>
#include <iostream>
#include <stdexcept>

/*
using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;
*/


// Local function declarations.
void getPar(int & step, double & lt, double & ti, double & fine, double & tt,
            char*& instr);
void initialize();
void doWork(int step, double & lt, double & ti, 
            double & fine, double & tt, char* instr);
void finalize();


int main(int argc, char ** argv) {

  int step;
  double lt,ti,fine,tt;
  char* instr;

  try {
    ahgen::startUp(argc, argv);

    /// \todo make the instr parameter an enumeration
    getPar(step,lt,ti,fine,tt,instr);

    initialize();

    doWork(step,lt,ti,fine,tt,instr);
  } catch (const std::exception & x) {
    ahgen::setStatus(1);
    AH_ERR << x.what() << std::endl;
  }

  finalize();

  /* Clean up local variables. */

  /* Perform global clean-up in parallel to startUp function. Implemented in libahgen. Close parameter file,
     close logging streams, etc. REQUIRED IN ALL APPLICATIONS. */
  // C++: this function handles its own exceptions.
  ahgen::shutDown();

  /* Return the global status. Passing a 0 will not affect the status if it was already set to 1. */
  return ahgen::getStatus();
}

void getPar(int & step, double & lt, double & ti, double & fine, double & tt,
            char*& instr) {
  // C++: this status is needed because Ape uses int status for errors.
  int status = eOK; /* Ape defines this status code. */

  // determine at which step to start conversion
  status = ape_trad_query_int("step", &step);

  // initialize all times to zero
  lt=0.0;
  ti=0.0;
  fine=0.0;
  tt=0.0;

  // get input time depending on step
  if (eOK == status) {

    if (step == 1) {
      status = ape_trad_query_double("localtime", &lt);

      if (eOK == status) {
        status = ape_trad_query_double("timeind", &ti);
      }
    } else if (step == 2) {
      status = ape_trad_query_double("fineti", &fine);
    } else {
      status = ape_trad_query_double("terrtime", &tt);
    }
  }

  // get instrument, if needed
  if (step < 3 && eOK == status) {
    status = ape_trad_query_string("instr", &instr);
  }

  if (eOK != status) {
    ahgen::setStatus(status);
    // TODO: make this message more specific.
    AH_ERR << "problem with parameters" << std::endl;
    throw std::runtime_error("getPar: problem with parameters.");
  }
}

void initialize() {
  return;
}

void doWork(int step, double & lt, double & ti, 
            double & fine, double & tt, char* instr) {

  // get instrument enumeration, if needed
  /// \todo should make argument of ahgen::atoinst a const
  int en_instr=0;
  if (step < 3) en_instr=ahgen::atoinst(instr);

  ahtime::AhTimeElem el_lt(lt);
  ahtime::AhTimeElem el_ti(ti);
  ahtime::AhTimeElem el_fine(fine);
  ahtime::AhTimeElem el_tt(tt);

  if (step < 2) el_fine=ahtime::step1(el_lt,el_ti);
  if (step < 3) el_tt=ahtime::step2(el_fine,en_instr);
  ahtime::AhTimeElem el_utc=ahtime::step3(el_tt);
  el_utc.minimizeExponent();

  // output UTC time in two formats
  AH_OUT << "UTC mantissa, exponent: " << el_utc.mantissa() << ", "
         << el_utc.exponent() << std::endl;
  AH_OUT << "UTC (s): " << el_utc.seconds() << std::endl;

}


void finalize() {
  return;
}

/* Revision Log
   $Log: ahtimeconv.cxx,v $
   Revision 1.1  2012/06/20 17:26:59  mwitthoe
   move ahtime.cxx/par to ahtimeconv.cxx/par; these are the initial versions of the time converter tool; the new version of the tool was overwritten in the directory directory reorganization

   Revision 1.1  2012/04/27 17:56:26  mwitthoe
   add ahtimeconv into gen/tasks

*/
