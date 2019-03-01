/** \brief Unit test of ahgen library.
    \author James Peachey
    \date 2012-01-29
*/

#include "ahgen/ahgen.h"
#include "ahgen/ahtest.h"
#include "ahlog/ahlog.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace ahgen;
using namespace ahlog;

static void testExceptionHandling(void);
static void testStartUp(void);
static void testShutDown(void);
static void testClobber(void);
static void testInst(void);

// Variable to hold banner (reused throughout the test).
static std::string s_banner;

int main(int, char **) {
  // Test exception handling facilities.
  testExceptionHandling();

  // Test startUp function.
  testStartUp();

  // Test shutDown function.
  testShutDown();

  // Test clobber function.
  if (0 != getStatus) testClobber(); else AH_WARN(4) << "Problems upstream: skipping testClobber" << std::endl;

  // Test instrument constants
  testInst();

  return (0 != getStatus()) ? 1 : 0;
}

static void testExceptionHandling(void) {
  // This test is somewhat confusing, because it tests the very macros used to write all the other tests.
  // Also it implicitly tests resetStatus, getStatus and setStatus.
  // This test needs to reset the global status, so start by preserving that value.
  int status = getStatus();

  // We also need a local variable for the status from this test.
  int teststatus = 0;

  // Test TRY macro with a success case (no exception thrown). Confirm status is 0 before and after.
  resetStatus();
  s_banner = "TRY macro with no code";
  TRY(, s_banner);
  if (0 != getStatus()) {
    teststatus = 1;
    AH_ERR << "After \"" << s_banner << "\", status was unexpectedly " << getStatus() << ", not 0" << std::endl;
  }

#ifdef YOU_WANT_TO_SEE_TRY_MACRO_CATCH_AN_EXCEPTION
  // Test TRY macro with a failure case (exception thrown). Confirm status is 0 before but non-0 after.
  // This was tested and found to work, then #ifdefed out because it is confusing (says it's an error when it's CORRECT
  // for it to be an error.
  resetStatus();
  s_banner = "TRY macro with throw";
  TRY(throw std::logic_error("this exception does not truly indicate failure");, s_banner);
  if (0 == getStatus()) {
    teststatus = 1;
    AH_ERR << "After \"" << s_banner << "\", status was unexpectedly 0" << std::endl;
  }
#endif

  // Test TRYEXCEPTION macro with a success case (exception thrown). Confirm status is 0 before and after.
  resetStatus();
  s_banner = "TRYEXCEPTION macro with throw";
  TRYEXCEPTION(throw std::logic_error("this exception shows successful test of a failure case");, s_banner);
  if (0 != getStatus()) {
    teststatus = 1;
    AH_ERR << "After \"" << s_banner << "\", status was " << getStatus() << ", not 0" << std::endl;
  }

#ifdef YOU_WANT_TO_SEE_TRYEXCEPTION_MACRO_NOT_CATCH_AN_EXCEPTION
  // Test TRYEXCEPTION macro with a failure case (no exception thrown). Confirm status is 0 before but non-0 after.
  // This was tested and found to work, then #ifdefed out because it is confusing (says it's an error when it's CORRECT
  // for it to be an error.
  resetStatus();
  s_banner = "TRYEXCEPTION macro with no code";
  TRYEXCEPTION(, s_banner);
  if (0 == getStatus()) {
    teststatus = 1;
    AH_ERR << "After \"" << s_banner << "\", status was unexpectedly 0" << std::endl;
  }
#endif

  // Final reset of status prior to recording final result from *this* test.
  resetStatus();

  // Set status in case this test had any problems.
  setStatus(teststatus);

  // Restore status (unless a failure in this test set it above).
  setStatus(status);
}

static void testStartUp(void) {
  // Note this must come before any calls to Ape, which reads this variable once and caches it.
  setenv("PFILES", ";./input", 1);

  // Test error case: trying to start the tool with a non-existent parameter file.
  s_banner = "invalid tool name (no par file)";
  TRYEXCEPTION( \
    char * argv[] = { "non-existent-tool" }; \
    int argc = sizeof(argv)/sizeof(argv[0]); \
    startUp(argc, argv); \
  , s_banner);

  // Test success case: start the tool with a findable parameter file.
  s_banner = "valid tool name (par file ./input/testahgen.par)";
  TRY( \
    char * argv[] = { "testahgen" }; \
    int argc = sizeof(argv)/sizeof(argv[0]); \
    startUp(argc, argv); \
  , s_banner);
}

static void testShutDown(void) {
  // The shutDown function should never throw an exception. Test this by calling it twice (not much of a test).
  s_banner = "call shutDown";
  TRY(shutDown();, s_banner);

  s_banner = "call shutDown a second time";
  TRY(shutDown();, s_banner);
}

static void testClobber(void) {
#if 0
  // Weird, this does not work -- the comma in the line "char *..." gets interpreted as a third argument.
  // Some sort of preprocessor madness. Tried backslashes around the comma and curly braces,
  // and trigraph ??< and ??> for the curly braces, but to no avail.
  TRY( \
    char * argv[] = { "testahgen", "clobber=yes" }; \
    int argc = sizeof(argv)/sizeof(argv[0]); \
    startUp(argc, argv); \
    if (!getClobber()) throw std::runtime_error("after initializing testahgen, clobber was false, not true as expected"); \
  , s_banner);
#endif
  s_banner = "clobber=yes for par file ./input/testahgen.par";
  TRY( \
    char * argv[2]; /* See note above about this less-than-elegant way to set this up. */ \
    argv[0] = "testahgen"; \
    argv[1] = "clobber=yes"; \
    int argc = sizeof(argv)/sizeof(argv[0]); \
    startUp(argc, argv); \
    if (!getClobber()) throw std::runtime_error("after initializing testahgen, clobber was false, not true as expected"); \
  , s_banner);

  s_banner = "shutdown after test of clobber=yes";
  TRY(shutDown();, s_banner);

  s_banner = "clobber=no for par file ./input/testahgen.par";
  TRY( \
    char * argv[2]; \
    argv[0] = "testahgen"; \
    argv[1] = "clobber=no"; \
    int argc = sizeof(argv)/sizeof(argv[0]); \
    startUp(argc, argv); \
    if (getClobber()) throw std::runtime_error("after initializing testahgen, clobber was true, not false as expected"); \
  , s_banner);

  s_banner = "shutdown after test of clobber=no";
  TRY(shutDown();, s_banner);
}

/// \brief test instrument enumeration and conversion functions
static void testInst(void) {
  // check string-to-enumeration conversion function for instruments
  std::cout << std::endl;
  std::cout << "Test conversion functions for instruments" << std::endl;
  std::cout << "Check SXS_PIXEL: ";
  if (ahgen::atoinst((char*)"SXS_PIXEL") == ahgen::INST_SXS_PIXEL) {
    std::cout << "Passed!" << std::endl;
  } else {
    std::cout << "Failed!" << std::endl;
  }
  std::cout << "Check SXS_ANTICO: ";
  if (ahgen::atoinst((char*)"SXS_ANTICO") == ahgen::INST_SXS_ANTICO) {
    std::cout << "Passed!" << std::endl;
  } else {
    std::cout << "Failed!" << std::endl;
  }
  std::cout << "Check SXI: ";
  if (ahgen::atoinst((char*)"SXI") == ahgen::INST_SXI) {
    std::cout << "Passed!" << std::endl;
  } else {
    std::cout << "Failed!" << std::endl;
  }
  std::cout << "Check HXI: ";
  if (ahgen::atoinst((char*)"HXI") == ahgen::INST_HXI) {
    std::cout << "Passed!" << std::endl;
  } else {
    std::cout << "Failed!" << std::endl;
  }
  std::cout << "Check SGD_CC: ";
  if (ahgen::atoinst((char*)"SGD_CC") == ahgen::INST_SGD_CC) {
    std::cout << "Passed!" << std::endl;
  } else {
    std::cout << "Failed!" << std::endl;
  }
  std::cout << "Check SGD_WAM: ";
  if (ahgen::atoinst((char*)"SGD_WAM") == ahgen::INST_SGD_WAM) {
    std::cout << "Passed!" << std::endl;
  } else {
    std::cout << "Failed!" << std::endl;
  }
  std::cout << "Check for failure: ";
  if (ahgen::atoinst((char*)"nonsense") == ahgen::INST_BAD) {
    std::cout << "Passed!" << std::endl;
  } else {
    std::cout << "Failed!" << std::endl;
  }
  std::cout << std::endl;
}

/* Revision Log
 $Log: testahgen.cxx,v $
 Revision 1.4  2012/04/24 15:26:07  mwitthoe
 add instrument enumeration and conversion function to ahgen

 Revision 1.3  2012/04/13 17:30:30  peachey
 Use new ahlog macros (that use st_stream.

 Revision 1.2  2012/02/03 15:30:45  peachey
 Add and test getClobber facility, from parameter file.

 Revision 1.1  2012/01/31 22:23:33  peachey
 Add first version of general support library.

*/
