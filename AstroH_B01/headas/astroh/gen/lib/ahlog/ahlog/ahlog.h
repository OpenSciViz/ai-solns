/** 
 * \file ahlog.h
 * \brief AstroH logger
 * \author Mike Witthoeft
 * \date $Date: 2012/06/07 11:05:23 $
 *
 * Initializes and provides a global formatter for st_stream.  Also defines 
 * macros for compact logging syntax.
 * 
 */

#ifndef AHLOG_AHLOG_H_
#define AHLOG_AHLOG_H_

#include <sys/time.h>
#include <stdexcept>

#include "st_stream/st_stream.h"
#include "st_stream/StreamFormatter.h"


/// \ingroup mod_ahlog
/// \brief macro to return the current function name (with processing)
/// \param[in] x message
///
/// This macro is used to prepend error messages before 'thrown'; therefore,
/// when AH_ERR is ultimately used at a higher level in the application, the
/// origin of the error is present in the message
#define AH_LOCATE(x) ahlog::prepend(x,__PRETTY_FUNCTION__)
#define AH_PREP(x) ahlog::prepend(x,__PRETTY_FUNCTION__)   ///< \note to be removed


/// \ingroup mod_ahlog
/// \brief macro to throw runtime error with trace information
/// \param[in] x message
#define AH_THROW_RUNTIME(x) (ahlog::macro_trace(), throw std::runtime_error(AH_LOCATE(x)))

/// \ingroup mod_ahlog
/// \brief macro to throw logic error with trace information
/// \param[in] x message
#define AH_THROW_LOGIC(x) (ahlog::macro_trace(), throw std::logic_error(AH_LOCATE(x)))

/// \ingroup mod_ahlog
/// \brief logger for DEBUG messages
#define AH_DEBUG (ahlog::macro_debug(__PRETTY_FUNCTION__).prefix() << "[" << __FILE__ << ":" << __LINE__ << "] ")


/// \ingroup mod_ahlog
/// \brief logger for OUTPUT messages
#define AH_OUT (ahlog::macro_out(__func__).prefix())


/// \ingroup mod_ahlog
/// \brief logger for INFO messages
/// \param[in] x message priority: ahlog::LOW or ahlog::HIGH
#define AH_INFO(x) (ahlog::macro_info(x,__func__).prefix())


/// \ingroup mod_ahlog
/// \brief logger for WARN messages
/// \param[in] x message priority: ahlog::LOW or ahlog::HIGH
#define AH_WARN(x) (ahlog::macro_warn(x,__func__).prefix())


/// \ingroup mod_ahlog
/// \brief logger for ERR messages
#define AH_ERR (ahlog::macro_err(__func__).prefix())


/// \ingroup mod_ahlog
namespace ahlog {

  /** \addtogroup mod_ahlog
   *  @{
   */

  /// \brief minimum and maximum chatter levels
  enum {
    MINCHAT=0,     ///< smallest legal chatter level
    MAXCHAT=3      ///< largest legal chatter level
  };


  /// \brief Message priority levels; small chatter triggers HIGH priority
  enum {
    LOW=3,      ///< low priority
    HIGH=2      ///< high priority
  };


  /// \brief Access global formatter; used by macros.
  st_stream::StreamFormatter & logger();


  /// \brief behavior of AH_OUT macro
  /// \param[in] func result of __func__
  /// \return stream for log message
  st_stream::OStream & macro_out(const std::string & func);


  /// \brief behavior of AH_INFO macro
  /// \param[in] chatter level ahlog::LOW or ahlog::HIGH
  /// \param[in] func result of __func__
  /// \return stream for log message
  st_stream::OStream & macro_info(const int & chatter, const std::string & func);


  /// \brief behavior of AH_WARN macro
  /// \param[in] chatter level ahlog::LOW or ahlog::HIGH
  /// \param[in] func result of __func__
  /// \return stream for log message
  st_stream::OStream & macro_warn(const int & chatter, const std::string & func);


  /// \brief behavior of AH_ERR macro
  /// \param[in] func result of __func__
  /// \return stream for log message
  st_stream::OStream & macro_err(const std::string & func);


  /// \brief behavior of AH_DEBUG macro
  /// \param[in] pfunc result of __PRETTY_FUNCTION__
  /// \return stream for log message
  st_stream::OStream & macro_debug(const std::string & pfunc);


  /// \brief print stacktrace if in DEBUG mode (used by AH_THROW_*)
  void macro_trace();


  /// \brief Pointer to logfile.
  std::ofstream* & logfile();


  /// \brief Answer to question: Is the log file being written?
  bool & writelogfile();


  /// \brief Return true/false whether DEBUG is on/off
  bool & debug();


  /// \brief Overwrite (true) or append (false) log file
  bool & clobber();


  /// \brief Store the time ahlog was initialized.
  struct timeval* & starttime();


  /// \brief Initialize logger.
  /// \param[in] execname executable name
  /// \param[in] logfile name of log file; special values: NONE will prevent a
  ///  log file from being written, DEFAULT will cause the log file to be named
  ///  execname.log where execname is the first parameter value; an empty 
  ///  string will result in an error; in addition, if the first character is 
  ///  and exclamation point (!), then the given log file will be overwritten
  /// \param[in] chatter level; low -> less output
  /// \param[in] debug true if debug statements are to be printed 
  /// \internal
  /// \note if chatter is zero, then only the log file streams are opened
  void setup(std::string execname, std::string logfile, int chatter, 
            bool debug);


  /// \brief Shutdown logger.
  void shutdown();


  /// \brief write header to log file
  /// \param[in] execname executable name to write to header
  void write_header(const std::string & execname);


  /// \brief write footer to log file
  /// \param[in] status true if program ends with no error, false otherwise
  void write_footer(const bool status);


  /// \brief Strip return type and argument list from __PRETTY_FUNCTION__.
  /// \param[in] instr value of __PRETTY_FUNCTION__
  /// \return output string
  /// \internal
  /// \note will misbehave given nested parentheses
  std::string strip_function(const std::string instr);


  /// \brief Remove all path information from given string.
  /// \param[in] instr full filename including path
  /// \return output string without path
  ///
  /// This function is designed to take the value of argv[0] and return just
  /// the executable name.  It tests for forward- and backward-slashes as the
  /// path separator.
  std::string strip_path(const std::string instr);


  /// \brief prepend message message with function
  /// \param[in] msg message
  /// \param[in] func name of function
  ///
  /// This function is designed to be used by the AH_PREP macro to prepend
  /// a message with the name of the function originating the message.
  std::string prepend(const std::string & msg, const std::string & func);


  /** @} */

}

#endif /* AHLOG_AHLOG_H_ */

/* Revision Log
 $Log: ahlog.h,v $
 Revision 1.17  2012/06/07 11:05:23  mwitthoe
 ahlog: rdynamic compiler option and missing include

 Revision 1.16  2012/05/31 18:24:27  mwitthoe
 tweaks to ahlog macros; incorporate traceback into main ahlog library

 Revision 1.15  2012/05/30 18:40:21  mwitthoe
 update ahlog Doxygen

 Revision 1.14  2012/05/30 17:38:10  mwitthoe
 improved the safety of the ahlog macros by creating macro_* functions to carry out the majority of the logic, so that the macros are now atomic; removed AH_CHAT_DEBUG

 Revision 1.13  2012/05/29 19:11:10  mwitthoe
 changed macros in ahlog to implement new method of throwing errors and stacktrace; added test program testthrow.cxx

 Revision 1.12  2012/05/29 17:47:22  mwitthoe
 update documentation for ahlog and ahtime

 Revision 1.11  2012/05/23 17:45:36  mwitthoe
 fixed namespace bug AH_ERR macro which prevented it working outside of ahlog

 Revision 1.10  2012/05/17 21:23:39  mwitthoe
 integrate ahtrace into ahlog's AH_ERR macro; trace is only printed when DEBUG is true

 Revision 1.9  2012/05/16 20:12:10  mwitthoe
 change input for DEFAULT log filename and implement clobber using excalamation point

 Revision 1.8  2012/05/10 19:46:45  mwitthoe
 improve header/footer for ahlog and other minor things

 Revision 1.7  2012/04/30 21:55:29  mwitthoe
 changes to ahlog: name of log file can be specified, new behavior for chatter level 0, standard log footer giving elapsed wall time

 Revision 1.6  2012/04/24 19:48:04  mwitthoe
 added documentation to ahlog


*/
