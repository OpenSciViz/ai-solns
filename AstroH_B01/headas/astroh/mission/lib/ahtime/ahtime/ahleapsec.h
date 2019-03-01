/** 
 * \file ahleapsec.h
 * \brief Functions to deal with leap seconds.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:52 $
 *
 */
 
#ifndef AHTIME_AHLEAPSEC_H
#define AHTIME_AHLEAPSEC_H

#include <string>
#include <map>

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief define type holding leap second table
  typedef std::map<std::string,double> LeapSecData;

  /// \brief return map containing leap second data
  LeapSecData & getLeapSecData();

  /// \brief return filename with leap second data
  /// \param[in] leapsecfile either name of leap second FITS file or keyword
  /// CALDB or REFDATA indicating the location where the file is found
  /// \return name of leap second file
  std::string locateLeapSecFile(const std::string & leapsecfile);

  /// \brief read leap second FITS file and load data in global data map
  /// \param[in] leapsecfile name of leap second file
  void readLeapSecData(const std::string & leapsecfile);


  /** @} */

}

#endif /* AHTIME_AHLEAPSEC_H */

/* Revision Log
 $Log: ahleapsec.h,v $
 Revision 1.5  2012/07/16 21:02:52  mwitthoe
 update ahtime library/tool header comments

 Revision 1.4  2012/07/13 18:32:07  mwitthoe
 clean up ahtime code

 Revision 1.3  2012/07/11 16:26:19  mwitthoe
 new ahtime libraries and test codes: ahleapsec - new library to read leap second data from CALDB; AhTimeTAI - library to store number of seconds relative to an epoch; AhTimeTT - library to store modified Julian date (MJD); AhTimeUTC - library to store UTC times; ahtimeconv - conversion functions between 3 AhTime* classes


*/
