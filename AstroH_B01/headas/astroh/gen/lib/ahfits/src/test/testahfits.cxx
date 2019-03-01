/** \brief Unit test of ahfits library.
    \author James Peachey
    \date 2012-01-31
*/
#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahgen/ahtest.h"
#include "ahlog/ahlog.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;

static void testAhFitsOpenClose(void);
static void testAhFitsCreate(void);
static void testAhFitsConnect(void);
static void testAhFitsRead(void);
static void testAhFitsWrite(void);

// Variable to hold banner (reused throughout the test).
static std::string s_banner;

int main(int argc, char ** argv) {
  // Set up logging.
  ahlog::setup(argv[0], std::string(argv[0])+".log", 3, true);

  // Test opening and closing FITS files.
  testAhFitsOpenClose();

  // Test creating FITS files.
  testAhFitsCreate();

  // Test connecting and disconnecting local data elements.
  if (0 == getStatus()) testAhFitsConnect(); else AH_WARN(4) << "Problems upstream: skipping testAhFitsConnect" << std::endl;

  // Test reading local data elements.
  if (0 == getStatus()) testAhFitsRead(); else AH_WARN(4) << "Problems upstream: skipping testAhFitsRead" << std::endl;

  // Test writing local data elements.
  if (0 == getStatus()) testAhFitsWrite(); else AH_WARN(4) << "Problems upstream: skipping testAhFitsWrite" << std::endl;

  // Shut down logging.
  ahlog::shutdown();

  return (0 != getStatus()) ? 1 : 0;
}

static void testAhFitsOpenClose(void) {
  // Try to open a file but pass null pointer.
  s_banner = "open file will null pointer for name";
  TRYEXCEPTION( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsOpen(0, "EVENTS", &ahffp); \
  , s_banner);

  // Try to open a file that isn't there.
  s_banner = "open non-existent-file";
  TRYEXCEPTION( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsOpen("non-existent.fits", "EVENTS", &ahffp); \
  , s_banner);

  // Try to open a file that is there, but use a non-existent extension.
  s_banner = "open existent file, non-existent extension";
  TRYEXCEPTION( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsOpen("./input/event_file_1.fits", "SPUD", &ahffp); \
  , s_banner);

  // Try to open a file that is valid and has the given extension.
  // This should work, so also close the file. This also tests the close function
  // in a non-error case.
  s_banner = "open existent file with EVENTS extension";
  TRY( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsOpen("./input/event_file_1.fits", "EVENTS", &ahffp); \
    if (0 == ahffp) throw std::runtime_error("after ahFitsOpen, ahffp is null"); \
    ahFitsClose(ahffp); \
  , s_banner);

  // It should be safe to close an unopened (0) file.
  s_banner = "close file (null pointer)";
  TRY( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsClose(ahffp); \
  , s_banner);
} 

static void testAhFitsCreate() {
  // Try to create a file but pass null pointer.
  s_banner = "create file will null pointer for template";
  TRYEXCEPTION( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsCreate("./output/testahfits.fits", 0, &ahffp); \
  , s_banner);

  // Try to create a file in a place probably the code can't write.
  s_banner = "create a file in /, which should not work";
  TRYEXCEPTION( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsCreate("/testahfits.fits", "infile/event_file_1.fits", &ahffp); \
  , s_banner);

  // Try to create a file in a place probably the code can write, but use
  // a non-existent template file.
  s_banner = "create a file in ./, using a non-existent template file, which should not work";
  TRYEXCEPTION( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsCreate("./output/testahfits.fits", "non-existent.fits", &ahffp); \
  , s_banner);

  // Try to create a file in a place probably the code can write,
  // using a valid template file.
  s_banner = "create a file in ./, using a valid template file, which should work";
  TRY( \
    AhFitsFilePtr ahffp = 0; \
    ahFitsCreate("./output/testahfits.fits", "./input/event_file_1.fits", &ahffp, true); \
    ahFitsClose(ahffp); \
    if (0 == ahffp) throw std::runtime_error("after ahFitsCreate, ahffp is null"); \
  , s_banner);
}

// This tests both ahFitsConnect and ahFitsDisconnectAll
static void testAhFitsConnect(void) {
  // Open a file to use for this test.
  AhFitsFilePtr ahffp = 0;
  s_banner = "set up to test connect";
  TRY( \
    ahFitsOpen("./input/event_file_1.fits", "EVENTS", &ahffp); \
    if (0 == ahffp) throw std::runtime_error("after ahFitsOpen, ahffp is null"); \
  , s_banner);

  if (0 == ahffp) {
    setStatus(1);
    AH_ERR << "FAILED: " << __func__ << ": set-up for test failed, test skipped!" << std::endl;
  } else {
    // Try to connect a null column name.
    s_banner = "connect a non-existent column";
    TRYEXCEPTION( \
      long long data_count = 0; \
      int data = 0; \
      ahFitsConnect(ahffp, 0, &data, &data_count); \
    , s_banner);

    // Try to connect a non-existent column.
    s_banner = "connect a non-existent column";
    TRYEXCEPTION( \
      long long data_count = 0; \
      int data = 0; \
      ahFitsConnect(ahffp, "NONEXIST", &data, &data_count); \
    , s_banner);

    // Try to connect a column that does exist.
    s_banner = "connect the SEGMENT column to data";
    TRY( \
      long long data_count = 0; \
      int data = 0; \
      ahFitsConnect(ahffp, "SEGMENT", &data, &data_count); \
    , s_banner);

    // Try to connect a column that does exist, using a null pointer for the count. This should be OK for
    // non-variable columns.
    s_banner = "connect the SEGMENT column to data";
    TRY( \
      int data = 0; \
      ahFitsConnect(ahffp, "SEGMENT", &data, 0); \
    , s_banner);

    // Try to connect a variable-size column, using a null pointer for the count. This should fail.
    s_banner = "connect the ASIC_MOCKUP column to data with null count variable pointer";
    TRYEXCEPTION( \
      double data[9]; \
      ahFitsConnect(ahffp, "ASIC_MOCKUP", &data, 0); \
    , s_banner);

    // Try to connect a variable-size column, using a valid count pointe. This should succeed.
    s_banner = "connect the ASIC_MOCKUP column to data";
    TRY( \
      long long data_count = 0; \
      double data[9]; \
      ahFitsConnect(ahffp, "ASIC_MOCKUP", &data, &data_count); \
    , s_banner);
  }

  // Clean up. Call ahFitsDisconnectAll explicitly to test it.
  s_banner = "end of connect test: disconnect all columns from data";
  TRY(ahFitsDisconnectAll(ahffp);, s_banner); 

  ahFitsClose(ahffp);
}

#define MAXEVENT 3u
#define EVENTSIZE 3u
#define ASCII_COL_WIDTH 32u
static void testAhFitsRead(void) {
  // Open a file to use for this test.
  AhFitsFilePtr ahffp = 0;
  s_banner = "set up to test read";
  TRY( \
    ahFitsOpen("./input/event_file_1.fits", "EVENTS", &ahffp); \
    if (0 == ahffp) throw std::runtime_error("after ahFitsOpen, ahffp is null"); \
  , s_banner);

  if (0 == ahffp) {
    setStatus(1);
    AH_ERR << "FAILED: " << __func__ << ": set-up for test failed, test skipped!" << std::endl;
  } else {
    // Try to connect a column that does exist to local variable segment. This should have no effect on segment's value.
    s_banner = "connect the SEGMENT column to local variable \"segment\"";
    long long segment_count = 0;
    int segment = 0;
    TRY( \
      ahFitsConnect(ahffp, "SEGMENT", &segment, &segment_count); \
      if (0 != segment_count) throw std::logic_error("connecting SEGMENT column to local variable (segment) changed count"); \
      if (0 != segment) throw std::logic_error("connecting SEGMENT column to local variable (segment) changed variable"); \
    , s_banner);

    // Try to connect a column that does exist to local variable segment. This should have no effect on segment's value.
    s_banner = "connect the ASIC_MOCKUP column to local variable \"asic_mockup\"";
    long long asic_count = 0;
    double asic_mockup[MAXEVENT * EVENTSIZE];
    std::memset(asic_mockup, '\0', sizeof(asic_mockup));
    TRY( \
      ahFitsConnect(ahffp, "ASIC_MOCKUP", asic_mockup, &asic_count); \
      if (0 != asic_count) \
        throw std::logic_error("connecting ASIC_MOCKUP column to local variable (asic_mockup) changed count"); \
      if (0 != asic_mockup[0]) \
        throw std::logic_error("connecting ASIC_MOCKUP column to local variable (asic_mockup) changed variable"); \
    , s_banner);

    // Try to connect an ASCII column that does exist to local variable ascii_col.
    s_banner = "connect the ASCII_COL column to local variable \"ascii_col\"";
    long long ascii_col_count = 0;
    char ascii_col[ASCII_COL_WIDTH + 1] = "";
    char * ascii_col_ptr = ascii_col;
    
    TRY( \
      ahFitsConnect(ahffp, "ASCII_COL", &ascii_col_ptr, &ascii_col_count); \
      if (0 != ascii_col_count) \
        throw std::logic_error("connecting ASCII_COL column to local variable (ascii_col) changed count"); \
      if (0 != ascii_col[0]) \
        throw std::logic_error("connecting ASCII_COL column to local variable (ascii_col) changed variable"); \
    , s_banner);

    // Try to read the next (first) row. Confirm content of all columns is correct.
    s_banner = "read the next (first) row of the data";
    TRY( \
      ahFitsFirstRow(ahffp); \
      ahFitsReadRow(ahffp); \
      if (6 != asic_count) throw std::logic_error("asic_count was not 3 after ahFitsReadRow"); \
      if (100. != asic_mockup[0]) throw std::logic_error("asic_mockup[0] was not 100. after ahFitsReadRow"); \
      if (1 != segment) throw std::logic_error("segment variable was not 1 after ahFitsReadRow"); \
      if (1 != segment_count) throw std::logic_error("segment_count variable was not 1 after ahFitsReadRow"); \
      if (1 != segment) throw std::logic_error("segment variable was not 1 after ahFitsReadRow"); \
      if (1 != ascii_col_count) \
        throw std::logic_error("ascii_col_count variable had wrong value after ahFitsReadRow"); \
      if (std::string("String number 1") != ascii_col) \
        throw std::logic_error(std::string("ascii_col variable was \"") + ascii_col + \
          "\", not \"String number 1\" as expected after ahFitsReadRow"); \
    , s_banner);
  }

  // Clean up. Note that ahFitsClose calls ahFitsDisconnectAll.
  ahFitsClose(ahffp);
}

#define MAXHIT 3u
#define HITSIZE 3u
static void testAhFitsWrite(void) {
  // Open a file to use for this test.
  AhFitsFilePtr ahffp = 0;
  s_banner = "set up to test write";
  TRY( \
    ahFitsOpen("./input/event_file_1.fits", "EVENTS", &ahffp); \
    if (0 == ahffp) throw std::runtime_error("after ahFitsOpen, ahffp is null"); \
  , s_banner);

  if (0 == ahffp) {
    setStatus(1);
    AH_ERR << "FAILED: " << __func__ << ": set-up for test failed, test skipped!" << std::endl;
  } else {
    // Try to connect segment column.
    s_banner = "connect the SEGMENT column to local variable \"segment\"";
    int segment = 0;
    TRY( \
      ahFitsConnect(ahffp, "SEGMENT", &segment, 0); \
    , s_banner);

    // Try to connect (variable size) asic column.
    s_banner = "connect the ASIC_MOCKUP column to local variable \"asic_mockup\"";
    long long asic_count = 0;
    double asic_mockup[MAXHIT][HITSIZE];
    std::memset(asic_mockup, '\0', sizeof(asic_mockup));
    TRY( \
      ahFitsConnect(ahffp, "ASIC_MOCKUP", asic_mockup, &asic_count); \
    , s_banner);

    // Try to connect an ASCII column that does exist to local variable ascii_col.
    s_banner = "connect the ASCII_COL column to local variable \"ascii_col\"";
    long long ascii_col_count = 0;
    char ascii_col[ASCII_COL_WIDTH + 1] = "";
    char * ascii_col_ptr = ascii_col;
    
    TRY( \
      ahFitsConnect(ahffp, "ASCII_COL", &ascii_col_ptr, &ascii_col_count); \
      if (0 != ascii_col_count) \
        throw std::logic_error("connecting ASCII_COL column to local variable (ascii_col) changed count"); \
      if (0 != ascii_col[0]) \
        throw std::logic_error("connecting ASCII_COL column to local variable (ascii_col) changed variable"); \
    , s_banner);

    // Iterate over the file. Read each row, then use segment to compute data for asic_mockup.
    s_banner = "iterate over file, reading and (over)writing columns";
    int rownum = 1;
    TRY( \
      for (ahFitsFirstRow(ahffp); ahFitsReadOK(ahffp); ahFitsNextRow(ahffp)) { \
        ahFitsReadRow(ahffp); \
        int numhit = (segment % MAXHIT) + 1; \
        for (int ii = 0; ii != MAXHIT; ++ii) { \
          for (int jj = 0; jj != HITSIZE; ++jj) { \
            asic_mockup[ii][jj] = rownum * 100 + ii * 10 + jj; \
          } \
        } \
        asic_count = numhit * HITSIZE; \
        std::ostringstream os; os << "String number " << rownum; \
        std::strncpy(ascii_col, os.str().c_str(), sizeof(ascii_col) - 1); \
        ahFitsWriteRow(ahffp); \
        ++rownum; \
      } \
    , s_banner);

    // Disconnect local variables before they go out of scope.
    s_banner = "disconnect all columns in write exercise";
    TRY(ahFitsDisconnectAll(ahffp);, s_banner); 
  }

  // Clean up. Note that ahFitsClose calls ahFitsDisconnectAll.
  ahFitsClose(ahffp);
}

/* Revision Log
   $Log: testahfits.cxx,v $
   Revision 1.5  2012/05/17 19:35:11  peachey
   Add test code for reading and writing ascii columns.

   Revision 1.4  2012/04/13 17:30:29  peachey
   Use new ahlog macros (that use st_stream.

   Revision 1.3  2012/02/07 18:31:19  dhon
   checkin

   Revision 1.2  2012/02/02 20:54:55  peachey
   Use and test latest changes to ahfits, including tests for writing
   FITS files with ahFitsWriteRow.

   Revision 1.1  2012/01/31 22:21:29  peachey
   Add first version of fits support code.

*/
