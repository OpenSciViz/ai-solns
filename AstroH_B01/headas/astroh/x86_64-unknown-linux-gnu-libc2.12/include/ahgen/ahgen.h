/** \brief Declarations of public members of the libahgen library.
    \author James Peachey
    \date 2012-01-29
*/
#ifndef ahgen_ahgen_h
#define ahgen_ahgen_h

#include "ahlog/ahlog.h"

/// \ingroup mod_ahgen
namespace ahgen {

/** \addtogroup mod_ahgen
 *  @{
 */


/// \brief aliases for instruments
enum ahinstnames {
  INST_BAD,
  INST_SXS_PIXEL,
  INST_SXS_ANTICO,
  INST_SXI,
  INST_HXI,
  INST_SGD_CC,
  INST_SGD_WAM
};

/** \brief Perform required standard start-up operations required for
      all tools. Open log files, pass the command line to ape to open parameter
      file, and handle global parameters such as clobber and chatter.
    \param argc The number of command line arguments.
    \param argv The command line arguments.
*/
void startUp(int argc, char ** argv);

/** \brief Perform required standard shut-down operations required for
      all tools. Close log files, call ape to close parameter file, etc.
*/
void shutDown(void);

/** \brief Get the global status variable, 0 for normal execution; other codes
      denote errors.
*/
int getStatus(void);

/** \brief Set the global status variable to the given value, and return the
      new status. If the global status is non-0, the status will *not* be changed.
    \param status The new status to set.
*/
int setStatus(int status);

/** \brief Reset the global status variable to 0.
*/
void resetStatus(void);

/** \brief Get the current value for the clobber parameter, if present. */
bool getClobber(void);

/** \brief Get the current value for the chatter parameter, if present. */
int getChatter(void);

/** \brief Get the current value for the debug parameter, if present. */
bool getDebug(void);

/// \brief convert instrument name to enumerated type
/// \param inst_str instrument string
/// \return instrument index from enumerated type, ahinstnames
/// \internal
/// \todo overload function to allow for string arguments
int atoinst(const char* inst_str);

} // namespace ahgen

/** @} */

/* #endif ahgen_ahgen_h */
#endif

/* Revision Log
 $Log: ahgen.h,v $
 Revision 1.6  2012/07/20 14:54:01  mwitthoe
 add Doxygen ahgen

 Revision 1.5  2012/05/14 17:19:30  mwitthoe
 add const to argument of atoinst to match .cxx file

 Revision 1.4  2012/04/25 21:34:25  peachey
 Add chatter and debug parameters to the startUp step.

 Revision 1.3  2012/04/24 15:26:07  mwitthoe
 add instrument enumeration and conversion function to ahgen

 Revision 1.2  2012/02/03 15:30:45  peachey
 Add and test getClobber facility, from parameter file.

 Revision 1.1  2012/01/31 22:23:33  peachey
 Add first version of general support library.

*/
