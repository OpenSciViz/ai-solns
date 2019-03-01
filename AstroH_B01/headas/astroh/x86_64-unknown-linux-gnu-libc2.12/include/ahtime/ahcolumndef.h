/** 
 * \file ahcolumndef.h
 * \brief functions to act on the CALDB columndef file for AstroH.
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:52 $
 *
 */
 
#ifndef AHTIME_AHCOLUMNDEF_H
#define AHTIME_AHCOLUMNDEF_H

#include <string>

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief struct to hold column names from columndef FITS file
  struct coldef_names {
    std::string colum1;       ///< 1st column from current file
    std::string colum2;       ///< 2nd column from current file
    std::string ltime1evt;    ///< 1st column from event file
    std::string ltime2evt;    ///< 2nd column from event file
    std::string ltime1hk;     ///< 1st column from HK file
    std::string ltime2hk;     ///< 2nd column from HK file
  };

  /// \brief write a template for the columndef FITS file
  /// \param[in] filename name of output FITS template
  void write_coldef_template(const std::string & tplfile);

  /// \brief create and fill columndef FITS file
  /// \param[in] filename name of output FITS file
  /// \param[in] tplfile name of template file to use
  void write_coldef_fits(const std::string & filename,
                         const std::string & tplfile);

  /// \brief get name of columndef FITS file from CALDB
  /// \return filename of columndef FITS file
  /// \note currently just returns name of local file: columndef.fits
  std::string get_caldb_coldef();

  /// \brief get length of column widths from primary header
  /// \param[in] filename name of columndef FITS file
  int get_coldef_width(const std::string & filename);

  /// \brief get list of column names for given instrument
  /// \param[in] filename name of columndef FITS file
  /// \param[in] inst name of instrument
  /// \param[out] cols column names for instrument
  void get_coldef_columns(const std::string & filename, 
                          const std::string & inst,
                          coldef_names & cols);


  /** @} */

}

#endif /* AHTIME_AHCOLUMNDEF_H */

/* Revision Log
 $Log: ahcolumndef.h,v $
 Revision 1.3  2012/07/16 21:02:52  mwitthoe
 update ahtime library/tool header comments

 Revision 1.2  2012/07/13 18:32:07  mwitthoe
 clean up ahtime code

 Revision 1.1  2012/06/19 21:31:33  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.1  2012/06/07 11:11:39  mwitthoe
 ahtime: add ahcolumndef, ahdelay, and ahtimeassign with test programs and HK FITS file for testing

*/
