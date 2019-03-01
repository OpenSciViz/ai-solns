/** \file Implementaion of a prototype Astro-H tool that demonstrates the basic
      style and some of the planned features supported by the infrastructure.
    \author James Peachey
    \date 2012-01-31
*/

/* Function of this application:
   1. Read rows from FFF file. Each row contains one "occurrence", which is some arbitrary number of "hits".
   2. Unpack each row (including variable-size array) into its "hits".
   3. Calibrate the hits to produce some number of single-photon events.
   4. Write each single-photon event to one row of the output (SFF) file, which thus has one photon event per row.

   Assumptions for purposes of this mock-up:
   A. The FFF file contains two columns:
        "SEGMENT", an integer that tells the number of hits in the current row, and
        "ASIC_MOCKUP", a variable-size column that is interpreted as data.
   B. The maximum possible number of hits in one row of the FFF is known to be MAXHIT. Each hit requires HITSIZE
      double values to encode its information. These hits will be converted to at most MAXEVENT events.
   C. The SFF file contains one fixed-length vector column named "EVENT". There is exactly one photon event per row. Each
      photon event requires EVENTSIZE double values to encode all its information.
   D. The output FITS file is created from a template file, by default input/sff-template.tpl.
      (Obviously this would need to look in some other standard place for this in a real tool.)
   E. There is a library libahgen that contains general purpose support functions.
   F. There is a library libahfits that works as follows:
      a. It can open or create files, and handles the cfitsio buffers.
      b. Using a function, the client can "connect" local variables to the struct that manages the FITS file and buffers.
   G. There is a library libahlog that has logging functions.
   H. Certain checks (for null pointers and other validations) have been omitted for now to improve readability.

   C++ exception assumptions:
   I. There is an exception class AhException defined in libahgen that has desired error-handling properties.
*/

// Local includes.
#include "ahfits/ahfits.h"
#include "ahgen/ahgen.h"
#include "ahlog/ahlog.h"

// Regional includes.
#include "ape/ape_error.h"
#include "ape/ape_trad.h"

// ISO includes.
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace ahfits;
using namespace ahgen;
using namespace ahlog;
using namespace std;

// Some local definitions.
#define MAXHIT (3)
#define HITSIZE (3)
#define MAXEVENT MAXHIT
#define EVENTSIZE (3)

// Local function declarations.
void getPar(char ** infile_name, char ** outfile_name, char ** template_name);
void initialize(const char *, const char *, const char *, AhFitsFilePtr *, AhFitsFilePtr *);
void doWork(AhFitsFilePtr, AhFitsFilePtr);
void asic2events(int, double *, int *, double *);
void finalize(AhFitsFilePtr infile, AhFitsFilePtr outfile);

/* All applications would need to include this standardized main function. Developers would be required
   to follow this basic structure, including certain required function calls as noted below.
*/
int main(int argc, char ** argv) {
  /* Names of input and output files. */
  char * infile_name = 0;
  char * outfile_name = 0;
  char * template_name = 0;

  /* AhFitsFilePtr is a typedef struct from libahfits that handles FITS file access
     (fitsfile pointers, buffers, current row, etc.). */
  /* Input file structure. */
  AhFitsFilePtr infile = 0;

  /* Output file structure. */
  AhFitsFilePtr outfile = 0;

  try {
    /* Global initializations. Implemented in libahgen. Set up logging streams, open parameter file,
       handle universal parameters like "debug" and "chatter", etc. REQUIRED IN ALL APPLICATIONS. */
    startUp(argc, argv);

    /* Get tool-specific parameters. Implemented in this application with support from libahgen.  */
    getPar(&infile_name, &outfile_name, &template_name);

    /* Perform application-specific initializations based on parameters. Implemented in application with
       support from libahgen. Open the input file(s). Create the output file. Position input and output
       file streams appropriately. */
    initialize(infile_name, outfile_name, template_name, &infile, &outfile);

    /* Do the work of this application, which processes the input file line-by-line to unpack the ASIC
       data as it is encoded in FFF, repack it as it is encoded in SFF, and write it to the output file. */
    doWork(infile, outfile);
  } catch (const exception & x) {
    // C++ exception: this is a standard C++ exception. It includes a function "what()" that returns a
    // string describing the error. For now, just flag the error, and write that messsage to the error stream.
    setStatus(1);
    AH_ERR << "ahdemo-exception: caught error: " << x.what() << endl;
  }

  /* Perform finalizations in parallel to initialize function. Implemented in application with support
     from libahgen.  Close/flush files, etc. Call this regardless of whether an error occurred above. */
  // C++: this function handles its own exceptions.
  finalize(infile, outfile);

  /* Clean up local variables. */
  free(template_name); /* This were allocated by C, so use free, not delete []. */
  free(outfile_name); /* This were allocated by C, so use free, not delete []. */
  free(infile_name); /* This were allocated by C, so use free, not delete []. */

  /* Perform global clean-up in parallel to startUp function. Implemented in libahgen. Close parameter file,
     close logging streams, etc. REQUIRED IN ALL APPLICATIONS. */
  // C++: this function handles its own exceptions.
  shutDown();

  /* Return the global status. Passing a 0 will not affect the status if it was already set to 1. */
  return getStatus();
}

void getPar(char ** infile_name, char ** outfile_name, char ** template_name) {
  // C++: this status is needed because Ape uses int status for errors.
  int status = eOK; /* Ape defines this status code. */
  /* Get the input file name parameter. */
  status = ape_trad_query_file_name("infile", infile_name);

  if (eOK == status) {
    /* Get the output file name parameter. */
    status = ape_trad_query_file_name("outfile", outfile_name);
  }

  if (eOK == status) {
    /* Get the template file name parameter. */
    status = ape_trad_query_file_name("templatefile", template_name);
  }

  if (eOK != status) {
    setStatus(status);
    // TODO: make this message more specific.
    throw runtime_error("getPar: problem with parameters.");
  }
}

void initialize(const char * infile_name, const char * outfile_name, const char * template_name,
  AhFitsFilePtr * infile, AhFitsFilePtr * outfile) {
  /* Open events extension in the input file. ahFitsOpen is a function in libahfits that opens the file
     (using the @filename convention to open multiple input files). It also sets up the optimal buffering
     for the file(s). */
  // C++ version: this will throw an exception if it fails to open the file.
  ahFitsOpen(infile_name, "EVENTS", infile);

  /* Open output file. ahFitsCreate is a function in libahfits that creates the file using
     a FITS standard template It also sets up the optimal buffering for the file(s). */
  // C++ version: this will throw an exception if it fails to create the file.
  ahFitsCreate(outfile_name, template_name, outfile, getClobber());

  // Go to the events extension in the output file.
  ahFitsMove(*outfile, "EVENTS");
}

void doWork(AhFitsFilePtr infile, AhFitsFilePtr outfile) {
  /* Local variable to store the number of hits in each row of the input file. */
  int numhits = 0;

  /* Local variable to store the number of events in each row of the input file. */
  int event_per_row = 1;

  /* Local variable to store the hit data read from the input file. */
  long long hits_count = 0;
  double hits[MAXHIT * HITSIZE];

  /* Local variable to store the photon event data for *ONE EVENT* in the output file. */
  double event[EVENTSIZE];
  long long event_count = sizeof(event)/sizeof(event[0]);

  /* Connect local variable numhits to the "SEGMENT" column in the infile.
     After this, calls to AhFitsReadRow(infile) will read the next value for that column into
     the local variable numhits. */
  // C++ version: this will throw an exception if it fails to create the file.
  ahFitsConnect(infile, "SEGMENT", &numhits, 0);

  /* Connect local variable hits to the "HITS" column in the infile.
     After this, calls to AhFitsReadRow(infile) will read the next (variable vector) value for that column into
     the local variable hits. */
  // C++ version: this will throw an exception if it fails to create the file.
  ahFitsConnect(infile, "ASIC_MOCKUP", &hits, &hits_count);

  /* Connect local variable numhits to the "SEGMENT" column in the infile.
     After this, calls to AhFitsReadRow(infile) will read the next value for that column into
     the local variable numhits. */
  // C++ version: this will throw an exception if it fails to create the file.
  ahFitsConnect(outfile, "SEGMENT", &event_per_row, 0);

  /* Connect local variable event to the "EVENT" column in the outfile.
     After this, calls to AhFitsWriteRow(outfile) will write the current local (vector-valued) variable
     into the next row in the outfile. */
  // C++ version: this will throw an exception if it fails to create the file.
  ahFitsConnect(outfile, "ASIC_MOCKUP", &event, &event_count);

  /* Iterate over the input file row-by-row. AhFitsStatus is a function in libahfits that returns:
       AHFOK if a) there are more rows in the file and b) no errors were encountered;
       AHFEOF if a) at the end of the file and  b) no errors were encountered;
       Some other code if an error was encountered on the given file structure. */
  for (ahFitsFirstRow(infile), ahFitsFirstRow(outfile); ahFitsReadOK(infile); ahFitsNextRow(infile)) {
    int numevents = 0;
    double events[MAXEVENT * EVENTSIZE];

    ahFitsReadRow(infile);

    /* Make sure numhits is 1 to 3. */
    numhits %= 3; ++numhits;

    asic2events(numhits, hits, &numevents, events);

    for (int ii = 0; ii != numevents; ++ii) {
      memcpy(event, events + ii * EVENTSIZE, sizeof(event));
      ahFitsWriteRow(outfile);
      ahFitsNextRow(outfile);
    }
  }

  /* Break all connections from local variables to the files. This is not so critical for this example,
     but if the file pointers were passed to other functions, they could still try to access local variables
     from *this* function, which could be out-of-scope. */
  ahFitsDisconnectAll(outfile);
  ahFitsDisconnectAll(infile);

  // C++: the error logging statements at this spot in the C version are not necessary because exceptions would be thrown
  // and these are handled at the top level.
}

/** \brief Convert input "hits" into output "events". See assumptions at the top of this file for how "hits"
    and "events" are assumed to be structured. In the FFF and SFF FITS files, the data are packed into vector-valued
    columns. Cfitsio reads these as 1d arrays regardless of their underlying structure. This function thus
    "reinterprets" the arguments as 3d arrays for the hitsm and 2d arrays for the events.
    \param numhits The number of input hits in this hits array.
    \param hits The hit input data, passed as a 1d array, but handled internally as a 3d array (each hit is a 2d array).
    \param numevents The number of output events.
    \param events The output event data, passed as a 1d array, but handled internally as a 2d array (each event is a 1d array).
*/
void asic2events(int numhits, double * hits, int * numevents, double * events) {
  /* Here assume that each hit (passed as a 1d array) is really structured as an 2 x 3 array.
     Define a type for "hits" that is consistent with all assumptions. */
  typedef double hit_type[HITSIZE];

  /* Create a local pointer that will reinterpret the input 1d "hits" array using the "hit_type" (2 x 3 array) just defined. */
  hit_type * hitptr = (hit_type *) hits; /* Note this is a 3d array (pointers to 2d arrays). */

  /* Here assume that each event (passed as a 1d array) is really structured as an array of arrays of size EVENTSIZE.
     Define a type for "events" that is consistent with all assumptions. */
  typedef double event_type[EVENTSIZE];

  /* Create a local pointer that will reinterpret the input 1d "events" array using the "event_type" (array) just defined. */
  event_type * eventptr = (event_type *) events; /* Note this is a 2d array (pointers to 1d arrays). */

  /* Placeholder for code that would use the calibration to reinterpret the "hits" as photon events. Just for
     purposes of this mock-up, assume:
       a) each 2d vector hit corresponds to one 1d vector event
       b) some arbitrary formula is used to convert the 2 x 3 hit data into the 1 x 4 event data.
  */

  /* Assumption a: number of events == number of hits. */
  *numevents = numhits;

  /* Output events are simply copied from the hits for now. */
  for (int ii = 0; ii != *numevents; ++ii) {
    for (int jj = 0; jj != EVENTSIZE; ++jj) {
      eventptr[ii][jj] = hitptr[ii][jj];
    }
  }

}

void finalize(AhFitsFilePtr infile, AhFitsFilePtr outfile) {
  /* ahFitsClose is a function in ahfits that flushes output, updates the checksum and closes the file. */
  /* Close the output file first. */
  // C++: handle exceptions here because the clean-up code needs to continue even if there are errors.
  try {
    ahFitsClose(outfile);
  } catch (const exception & x) {
    setStatus(1);
    AH_ERR << "finalize: error while closing output file: " << x.what() << endl;
  }

  /* Close infile regardless of whether an error occurred above. */
  // C++: handle exceptions here because the clean-up code needs to continue even if there are errors.
  try {
    ahFitsClose(infile);
  } catch (const exception & x) {
    setStatus(1);
    AH_ERR << "finalize: error while closing input file(s): " << x.what() << endl;
  }
}

/* Revision Log
   $Log: ahdemo.cxx,v $
   Revision 1.5  2012/04/13 17:31:07  peachey
   Use new ahlog macros (that use st_stream.

   Revision 1.4  2012/02/03 17:59:07  peachey
   Use templatefile parameter instead of hard-wired template file name.

   Revision 1.3  2012/02/03 15:36:00  peachey
   1. Refine code to use latest ahfits, including changes to ahFitsNextRow
      that allow ahdemo to add rows to the output file.
   2. Change some variable names and use macros for EVENTSIZE and HITSIZE
      etc. consistently, to clarify code.
   3. Various tweaks to the logic to make the code do what it intends:
      read multiple hits from each row of input, and convert those to
      multiple event output, but write those output events one per row.

   Revision 1.2  2012/02/02 20:56:50  peachey
   1. Changes to use the new functions ahFitsFirstRow, ahFitsNextRow, and
      changes to ahFitsRead/WriteRow.
   2. Use new signature for ahFitsConnect that includes the "count"
      variable.

   Revision 1.1  2012/01/31 22:25:47  peachey
   Add demonstration/prototpye tool using design agreed upon 2012-01-27.

*/
