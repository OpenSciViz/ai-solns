/** \file testahtime.cxx
 *  \brief Test program for the ahtime library.
 *  \author Mike Witthoeft
 *  \date $Date: 2012/07/12 12:12:51 $
 */

#include <iostream>

#include "ahlog/ahlog.h"
#include "ahtime/ahtime.h"
#include "ahtime/AhTimeUTC.h"
#include "ahtime/AhTimeTAI.h"
#include "ahtime/AhTimeTT.h"
#include "ahtime/ahtimeconv.h"


/// \ingroup mod_ahtime
namespace ahtime {


  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \callgraph
  int utmain() {

    ahlog::setup("testahtime","!DEFAULT",2,false);

    bool chk=true;
    chk&=ut_ahtime();
    chk&=ut_AhTimeUTC();
    chk&=ut_AhTimeTAI();
    chk&=ut_AhTimeTT();
    chk&=ut_ahtimeconv();
    if (chk) {
      AH_OUT << "ahtime.h: all tests passed!" << std::endl;
    } else {
      AH_OUT << "ahtime.h: failure occurred!" << std::endl;
    }

    return 0;
  }

  /** @} */

} // end namespace



int main(int argc, char* argv[]) {

  return ahtime::utmain();
}


/* Revision Log
 $Log: testahtime.cxx,v $
 Revision 1.2  2012/07/12 12:12:51  mwitthoe
 standardize and consolidate ahtime unit tests

 Revision 1.1  2012/06/19 21:31:36  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.8  2012/06/07 11:11:41  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

 Revision 1.7  2012/05/29 14:47:47  mwitthoe
 created class AhDateTime to replace functions in ahdatetime.h; moved leap-second functions into ahleapsec.h

 Revision 1.6  2012/05/23 23:27:47  mwitthoe
 add ahdatetime defining a structure to hold a date and time (and functions operating on it); add functions to ahtime to determine number of leap seconds in a date/time interval; remove AhTimeElem from ahtime (but class still present)

 Revision 1.5  2012/04/27 17:58:02  mwitthoe
 minor changes to ahtime

 Revision 1.4  2012/04/24 19:51:31  mwitthoe
 remove bugs in ahtime

 Revision 1.3  2012/04/24 19:16:18  mwitthoe
 documentation for ahtime


*/
