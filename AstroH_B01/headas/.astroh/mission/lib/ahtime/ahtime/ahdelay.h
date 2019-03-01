/** 
 * \file ahdelay.h
 * \brief functions to act on the CALDB instrument delay file for AstroH.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:52 $
 *
 */
 
#ifndef AHTIME_AHDELAY_H
#define AHTIME_AHDELAY_H

#include <string>
#include <vector>

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief write a template for the instrument delay FITS file
  /// \param[in] tplfile name of output FITS template
  /// \param[in] nrows number of rows applied to each extension
  void write_delay_template(const std::string & tplfile, const int & nrows);

  /// \brief create instrument delay FITS file from template
  /// \param[in] filename name of output FITS file
  /// \param[in] tplfile name of template file to use
  void create_delay_fits(const std::string & filename,
                         const std::string & tplfile);

  /// \brief write list of times and delays for a single instrument
  /// \param[in] filename name of FITS file to modify
  /// \param[in] inst name of instrument
  /// \param[in] times vector of times; seconds from TT
  /// \param[in] delays vector of delays in microseconds
  void write_delay_times(const std::string & filename, 
                         const std::string & inst,
                         const std::vector<int> times,
                         const std::vector<double> delays);

  /// \brief get name of instrument delay FITS file from CALDB
  /// \return filename of instrument delay FITS file
  /// \note currently just returns name of local file: delay.fits
  std::string get_caldb_delay();

  /// \brief get delay time of instrument at given time
  /// \param[in] inst name of instrument
  /// \param[in] time epoch time in seconds
  /// \return delay time in microseconds
  /// \internal
  /// \note this function will read the CALDB file in every call; the function
  /// should not be called repeatedly 
  double get_delay(const std::string inst, const int time);


  /** @} */

}

#endif /* AHTIME_AHDELAY_H */

/* Revision Log
 $Log: ahdelay.h,v $
 Revision 1.2  2012/07/16 21:02:52  mwitthoe
 update ahtime library/tool header comments

 Revision 1.1  2012/06/19 21:31:34  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/06/07 11:11:39  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

*/
