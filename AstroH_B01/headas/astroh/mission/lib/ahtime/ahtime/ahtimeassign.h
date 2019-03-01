/** 
 * \file ahtimeassign.h
 * \brief Assign time to housekeeping and event files.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:53 $
 *
 */
 
#ifndef AHTIME_AHTIMEASSIGN_H
#define AHTIME_AHTIMEASSIGN_H

#include "ahtime/ahcolumndef.h"
#include "ahtime/ahtimfile.h"
#include "ahfits/ahfits.h"

#include <string>

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief fill TIME column in active extension of a HK file
  /// \param[in] fptr The FITS file object.
  /// \param[in] timfile name of TIM file
  /// \param[in] timecolumn name of column holding assigned times
  /// \param[in] timdat data struct for TIM file
  /// \param[in] calctime true if filling TIME column
  /// \param[in] calcutc true if filling UTC columns
  /// \param[in] interpolation interpolation type from enumeration in ahmath
  void assign_time_hk(ahfits::AhFitsFilePtr fptr, 
                      const std::string & timfile,
                      const std::string & timecolumn,
                      const ahtime::tim_data & timdat,
                      const bool & calctime, const bool & calcutc,
                      const int & interpolation);

  /** @} */

}

#endif /* AHTIME_AHTIMEASSIGN_H */

/* Revision Log
 $Log: ahtimeassign.h,v $
 Revision 1.6  2012/07/16 21:02:53  mwitthoe
 update ahtime library/tool header comments

 Revision 1.5  2012/07/16 20:09:39  mwitthoe
 move TI and S_TIME units from TIM file into TIM struct; remove reading of TIM file from function in ahtimeassign (reading is now done outside HDU loop)

 Revision 1.4  2012/07/12 20:37:46  mwitthoe
 time assignment now updates UTC and header times

 Revision 1.3  2012/06/28 23:21:03  mwitthoe
 ahtime library and tool: add support for calctime, calcutc, and interpolation parameters; moved HDU loop from library to tool

 Revision 1.2  2012/06/25 13:40:37  mwitthoe
 working update to ahtime library

 Revision 1.1  2012/06/19 21:31:34  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/06/07 11:11:39  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

*/
