#ifndef TELDEF2_INCLUDED

#define TELDEF_UPDATED_OCT_2003

#include "fitsio.h"
#include "coord.h"
#include "align.h"
#include "tr_basic.h"
#include "tr_skyatt.h"
#include "tr_multiseg.h"
#include "tr_rawtodet.h"


/*char trtypeslist[4][TRTYPE_STRING_LENGTH] = {"BASIC", "SKYATT", "RAWTODET", "MULTISEG"};*/

/*****************************************************************************
******************************************************************************
* A TELDEF2 (Telescope Definition 2) structure defines the coordinate
* systems used for a given instrument. We make the following
* assumptions:
*
* The coordinates are generalized instead of being fixed in position and
* raw/detector/sky classes as they were in the original TELDEF structure.
*
* This new teldef library supports v0.2 of the Teldef File Format
* Specification.
*
*****************************************************************************/
typedef struct {

/*******************************************
* input file, mission and instrument names *
*******************************************/
  char filename[FLEN_FILENAME];  /* TelDef file name */
  char mission[FLEN_VALUE];      /* Mission name */
  char instrument[FLEN_VALUE];   /* Instrument abbreviation */
  double td_version;             /* Teldef version of Teldef file spec. */

  /***************************
   * generalized coordinates *
   ***************************/
  long n_coordsys;   /* Number of coordinate systems */

  char** coordsysnames; /* Array of 3-letter names of coordinate systems 
			 * coordsysnames[sys] */

  COORDDEF*** coordsys;  /* Array of coordinate system structures
			  * coordsys[sys][seg] */

  int* n_segments;   /* Number of segments in each coordinate system level
			n_segments[sys] */

  int* min_segment;  /* Minumum segment number in each coord. sys.
		      * min_segments[sys] */
  
  char** trtypes;    /* Transformation types, such as MULTISEG or RAWTODET
		      * for each system 
		      * trtypes[sys] */

  /* Optical axis coordinates */

  char* optcoord; 
  double optaxisx;
  double optaxisy;

  /* Parameters for randomization of intrapixel event locations. */

  int ransys;   /* System number of lower-level system where 
			       randomization is applied */
  char ransysname[FLEN_VALUE];   /* System name of lower-level system where
				    randomization is applied */
  int ranscalesys;  /* Number of system swhose pixel size is used
		     * to set the amount of randomizatoin. */
  char ranscalesysname[FLEN_VALUE];  /* Name of system whose pixel size is used 
					to set the amount of randomization */

  /*******************************************************
   * Arrays of structures for transformation parameters. *
   * TR_BASIC is used for all transformations.   *
   * Only one of the other structures is used for each 
   * coordinate system level.
   *******************************************************/

  TR_BASIC**    basicparam;  /* Structure holding basic transformation
			      * parameters, such as flip, offset, and
			      * rotation */
 
  TR_RAWTODET** rawtodetparam;  /* Structure holding RAWTODET
				   transformation parameters */
  TR_MULTISEG** multisegparam;  /* Structure holding MULTISEG
				   transformation parameters (linear
				   coefficients for each set of
				   segment properties */
  TR_SKYATT**   skyattparam;    /* Structure holding SKYATT
				   parameters, such as the alignment
				   matrix and focal length. */
		    
} TELDEF2;


/******************************************************
 * Functions to create or destroy a TELDEF2 structure.
 ******************************************************/

/* Create a new TELDEF2 structure and read its contents from a file. */

TELDEF2* readTelDef2
(
 char* filename /* TelDef filename */
 );


/* Free all memory associated with a TELDEF2 structure. */

void destroyTelDef2
(
 TELDEF2* teldef /* TELDEF2 structure to destroy */
 );


/*********************************************************************
 * Functions to load transformation parameters in a TELDEF2 structure.
 *********************************************************************/

/* Read the transformation types into the teldef structure from the
   TRTYPE keywords in the header of the TelDef file. */

void readTransformationTypesFromTelDef2
(
 fitsfile* fp, /* TelDef file pointer */
 TELDEF2* teldef /* teldef structure */
 );

/* Read the transformation parameters of all the transformations from
   the TelDef file and store them in the teldef structure. */

void setTransformationParametersFromTelDef2
(
 fitsfile* fp, /* TelDef file pointer */
 TELDEF2* teldef /* teldef structure */
 );

/* Determine the number of segments in each coordinate system and
   store them in teldef->n_segments. */

void setSegmentCountInTelDef2
(
 fitsfile* fp, /* TelDef file pointer */
 TELDEF2* teldef /* teldef structure */
 );

/* Read the coordinate system properties from various TelDef keywords
   and store them in the coord structure. */

void setCoordinatesFromKeywordsInTeldef2
(
 TELDEF2* teldef, /* teldef structure */ 
 COORDDEF* coord, /* coord structure */
 fitsfile* fp,  /* TelDef file pointer */
 long sys /* system number of lower-level system */
 );

/* Read randomization parameters from TelDef keywords. */

void readRandomizationFromTeldef2
(
 fitsfile* fp, /* TelDef file pointer */
 TELDEF2* teldef /* teldef structure */
);

/* Retrieve the coordinate system number from the name. */

int getCoordSystemNumberFromName(TELDEF2* teldef, char* name);

/****************************************************************************
 * Functions for calculating the 2D transformations for the coord. systems.
 ****************************************************************************/

/* Call the various routines for setting the various types of 2D
   transformations for all the coordinate systems and segments. */

void setXform2dFromTransformationParameters
(
 TELDEF2* teldef, /* teldef structure */
 COORDDEF* origcoord, /* originating coordinate system structure */
 COORDDEF* destcoord, /* destination coordinate system structure */
 int sys, /* system number of originating coordinate system */
 int seg  /* segment number */
 );

/* Set the basic transformation parameters. 
 * Return the 2D transformation for the segment */

XFORM2D* getXform2dFromTrBasicParameters
(
 TELDEF2* teldef, /* teldef structure */
 COORDDEF* origcoord, /* originating coordinate system structure */
 COORDDEF* destcoord, /* destination coordinate system structure */ 
 TR_BASIC* basicparam, /* basic transformation structure */
 int sys               /* system number of originating corodinate system */
 );

/* Set the parameters for a RAWTODET linear coefficients transformation. 
 * Return the 2D transformation for the segment */

XFORM2D* getXform2dFromTrRawtodetLinearCoeffParameters
(
 TELDEF2* teldef, /* teldef structure */
 XFORM2D* int2dettrans,  /* 2D transformation structure for the 
			    internal-to-upper coordinate system */
 TR_RAWTODET* rawtodetparam, /* Structure holding transformation parameters */
 int sys, /* system number of originating coordinate structure */
 int seg /* segment number of originating coordinate system */
 );

/* Set the parameters for a RAWTODET pixel corner map transformation.
 * Return the 2D transformation for the segment */

XFORM2D* getXform2dFromTrRawtodetCornerMapParameters
(
 TELDEF2* teldef, /* teldef structure */
 XFORM2D* int2hightrans, /* 2D transformation structure for the 
			    internal-to-upper coordinate system */
 COORDDEF* origcoord, /* originating coordinate system structure */
 TR_RAWTODET* rawtodetparam, /* structure holding transformation parameters */
 int sys, /* system number of originating coordinate structure */
 int seg  /* segment number of originating coordinate system */
);

/* Set the parameters for a MULTISEG linear coefficients transformation.
 * Return the 2D transformation for the segment. */
				  
XFORM2D* getXform2dFromTrMultisegLinearCoeffParameters
(
 TELDEF2* teldef, /* teldef structure */
 XFORM2D* int2hightrans, /* 2D transformation structure for the 
			 internal-to-upper coordinate system */
 TR_MULTISEG* multisegparam, /* structure holding transformation parameters */
 int sys, /* system number of originating coordinate structure */
 int seg  /* segment number of originating coordinate system */
 );


/****************************************************************************
 * Functions for applying the transformations to coordinate values
 ****************************************************************************/

/* Convert coordinates from lower- to higher-level coord. system. */

int convertToHigherCoordRawtodet
(
 TELDEF2* teldef, /* teldef structure */
 double lowx, /* x coordinate of lower-level system */
 double lowy, /* y coordinate of lower-level system */
 double* highx, /* x coordinate of higher-level system */
 double* highy, /* y coordinate of higher-level system */
 int lowsys, /* number of lower-level system */
 int lowseg  /* segment number of lower-level system */
);


int convertToHigherCoordBasic
(
 TELDEF2* teldef, /* teldef structure */
 double lowx, /* x coordinate of lower-level system */
 double lowy, /* y coordinate of lower-level system */
 double* highx, /* x coordinate of higher-level system */
 double* highy, /* y coordinate of higher-level system */
 int lowsys /* number of lower-level system */
);

int convertToHigherCoordMultiseg
(
 TELDEF2* teldef, /* teldef structure */
 double lowx, /* x coordinate of lower-level system */
 double lowy, /* y coordinate of lower-level system */
 double* highx, /* x coordinate of higher-level system */
 double* highy, /* y coordinate of higher-level system */
 int lowsys, /* number of lower-level system */
 int* lowprops  /* segment properties of lower-level system */
 );


/************************************************************************
* Functions to handle FITSIO errors while reading TelDef files.
************************************************************************/

/* Print error message and exit if there is a CFITSIO error. */

void checkFITSerrors(int status, char* doing, char* teldef_file);


/****************************************************************************
 * Functions for print a TELDEF2 structure (for debugging)
 ****************************************************************************/

/* Print a whole TELDEF2 structure to stdout. */

void printTelDef2(TELDEF2* teldef);

/* Print a whole COORDDEF structure to stdout. */

void printCoordDef(COORDDEF* coord);





#define TELDEF2_INCLUDED
#endif  /* TELDEF2_INCLUDED */
