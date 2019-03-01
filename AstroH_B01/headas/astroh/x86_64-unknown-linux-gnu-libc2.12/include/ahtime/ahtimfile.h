/** 
 * \file ahtimefile.h
 * \brief read S_TIME and TI data from TIM file
 * \author Mike Witthoeft
 * \date $Date: 2012/07/16 21:02:53 $
 *
 */
 
#ifndef AHTIME_AHTIMFILE_H
#define AHTIME_AHTIMFILE_H

#include <string>
#include <vector>

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief struct hold TIM data: S_TIME and TI
  struct tim_data {
    int size;                           ///< number of points
    std::vector<double> s_time;         ///< vector with S_TIME
    std::vector<long long> ti;          ///< vector with TI
    std::string s_time_units;           ///< units of S_TIME
    std::string ti_units;               ///< units of TI value
  };

  /// \brief erase data from tim_data struct
  /// \param[out] timdat TIM data struct
  void clearTIMData(tim_data & timdat);

  /// \brief read data from extension in TIM file and put in tim_data struct
  /// \param[in] filename name of TIM file
  /// \param[in] hduname name of extension
  /// \param[out] timdat TIM data struct
  void readTIMData(const std::string & filename, const std::string & hduname,
               tim_data & timdat);

  /** @} */

}

#endif /* AHTIME_AHDELAY_H */

/* Revision Log
 $Log: ahtimfile.h,v $
 Revision 1.4  2012/07/16 21:02:53  mwitthoe
 update ahtime library/tool header comments

 Revision 1.3  2012/07/16 20:09:39  mwitthoe
 move TI and S_TIME units from TIM file into TIM struct; remove reading of TIM file from function in ahtimeassign (reading is now done outside HDU loop)

 Revision 1.2  2012/06/26 19:45:02  mwitthoe
 updates to ahtime library: time assignment and test HK and TIM files

 Revision 1.1  2012/06/22 20:13:39  mwitthoe
 in ahtime, add functions to read data from TIM file; fix compiler warning in ahdelay.cxx


*/
