/** \brief Declarations of public members of the test support library.
    \author James Peachey
    \date 2012-01-29
*/
#ifndef ahgen_ahtest_h
#define ahgen_ahtest_h

/* Error macros. The final "if (true)" is so that one can end with a semicolon. */
#define TRY(CODE, DESCRIPTION) \
  try { \
    CODE \
    AH_INFO(1) << "PASSED: test \"" << DESCRIPTION << "\" threw no exception as expected" << std::endl; \
  } catch (const std::exception & x) { \
    setStatus(1); \
    AH_ERR << "FAILED: test \"" << DESCRIPTION << "\" unexpectedly threw exception: \"" \
                          << x.what() << "\"" << std::endl; \
  } \
  if (true)

#define TRYEXCEPTION(CODE, DESCRIPTION) \
  try { \
    CODE \
    setStatus(1); \
    AH_ERR << "FAILED: test \"" << DESCRIPTION << "\" unexpectedly did not throw exception" << std::endl; \
  } catch (const std::exception & x) { \
    AH_INFO(1) << "PASSED: test \"" << DESCRIPTION << "\" threw expected exception: \"" \
                          << x.what() << "\"" << std::endl; \
  } \
  if (true)

/* #endif ahgen_ahtest_h */
#endif

/* Revision Log
 $Log: ahtest.h,v $
 Revision 1.4  2012/04/13 17:30:30  peachey
 Use new ahlog macros (that use st_stream.

 Revision 1.3  2012/03/27 21:03:33  peachey
 Use AhLog::info() instead of AhLog::log() because this change was just
 made in ahlog.

 Revision 1.2  2012/02/03 15:17:36  peachey
 Remove unnecessary "using" statements from TRY and TRYEXCEPTION macros.
 Bad practice to include such things in macros that would be used in many
 places.

 Revision 1.1  2012/01/31 22:23:33  peachey
 Add first version of general support library.

*/
