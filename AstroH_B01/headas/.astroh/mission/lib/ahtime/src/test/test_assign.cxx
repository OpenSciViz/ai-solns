/** \file test_assign.cxx
 *  \brief Test time assignment functions.
 *  \author Mike Witthoeft
 *  \date $Date: 2012/06/19 21:31:36 $
 */

#include <iostream>

#include "ahtime/ahtime.h"
#include "ahtime/ahcolumndef.h"
#include "ahtime/ahtimeassign.h"
#include "ahlog/ahlog.h"

int main() {

  ahlog::setup("test_assign","NONE",3,false);

  const char* coldeffile="columndef.fits";
  const char* filename="ae400015010.hk";

  ahtime::assign_time(filename,"HK",coldeffile);



}

/* Revision Log
 $Log: test_assign.cxx,v $
 Revision 1.1  2012/06/19 21:31:36  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/06/07 11:11:41  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

*/
