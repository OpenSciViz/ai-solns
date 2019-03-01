/** 
 * \file ahtimeconv.h
 * \brief Functions to act on time objects: UTC, TT, and TAI.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:53 $
 *
 */
 
#ifndef AHTIME_AHTIMECONV_H
#define AHTIME_AHTIMECONV_H

#include "ahtime/AhTimeUTC.h"
#include "ahtime/AhTimeTT.h"
#include "ahtime/AhTimeTAI.h"
#include "ahtime/ahleapsec.h"

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief convert UTC time to TT (MJD)
  /// \param[in] tin UTC time
  /// \param[out] tout TT time
  void convertTime(AhTimeUTC & tin, AhTimeTT & tout);

  /// \brief convert TT (MJD) time to UTC
  /// \param[in] tin TT time
  /// \param[out] tout UTC time
  void convertTime(AhTimeTT & tin, AhTimeUTC & tout);

  /// \brief convert UTC time to TAI
  /// \param[in] tin UTC time
  /// \param[out] tout TAI time
  /// \param[in] epoch desired epoch
  void convertTime(AhTimeUTC & tin, AhTimeTAI & tout, AhTimeUTC & epoch);

  /// \brief convert TAI time to UTC
  /// \param[in] tin TAI time
  /// \param[out] tout UTC time
  void convertTime(AhTimeTAI & tin, AhTimeUTC & tout);

  /// \brief convert TAI time to TT
  /// \param[in] tin TAI time
  /// \param[out] tout TT time
  void convertTime(AhTimeTAI & tin, AhTimeTT & tout);

  /// \brief convert TT time to TAI
  /// \param[in] tin TT time
  /// \param[out] tout TAI time
  /// \param[in] epoch desired epoch
  void convertTime(AhTimeTT & tin, AhTimeTAI & tout, AhTimeUTC & epoch);

  /// \brief unit test for ahtimeconv.h
  bool ut_ahtimeconv();

  /** @} */

}

#endif /* AHTIME_AHTIMECONV_H */

/* Revision Log
 $Log: ahtimeconv.h,v $
 Revision 1.4  2012/07/16 21:02:53  mwitthoe
 update ahtime library/tool header comments

 Revision 1.3  2012/07/13 18:32:07  mwitthoe
 clean up ahtime code

 Revision 1.2  2012/07/12 12:12:50  mwitthoe
 standardize and consolidate ahtime unit tests

 Revision 1.1  2012/07/11 16:26:19  mwitthoe
 new ahtime libraries and test codes: ahleapsec - new library to read leap second data from CALDB; AhTimeTAI - library to store number of seconds relative to an epoch; AhTimeTT - library to store modified Julian date (MJD); AhTimeUTC - library to store UTC times; ahtimeconv - conversion functions between 3 AhTime* classes


*/
