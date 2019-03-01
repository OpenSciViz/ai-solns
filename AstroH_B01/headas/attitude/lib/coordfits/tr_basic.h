#ifndef TR_BASIC_INCLUDED

#include "fitsio.h"

/****************************************************************************
* TR_BASIC structure holds the parameters needed for a basic linear
* transformation.
*****************************************************************************/
typedef struct {

  char lowcoordsysname[FLEN_VALUE];
  char highcoordsysname[FLEN_VALUE];
  int low_sys;
  int high_sys;

  double xoffset; /* Value of xxx_XOFF x offset keyword. */
  double yoffset; /* Value of xxx_YOFF y offset keyword. */
  double scale;   /* Value of xxx_SCAL scaling keyword. */
  long xflip;      /* Value of xxxFLIPX x flip keyword. */
  long yflip;      /* Value of xxxFLIPY y flip keyword. */
  double rotangle;    /* Value of xxx_ROTD rotation (deg.) keyword. */
		    
} TR_BASIC;


/*****************************************************************************
* create a new TR_BASIC structure and read its contents from a TelDef file.
****************************************************************************/
TR_BASIC* readTrBasic (fitsfile* fp, int is_rawtodet, 
		       char* lowcoordsysname, char* highcoordsysname, 
		       int low_sys, char* teldef_file);

void printTrBasic(TR_BASIC* basicparam);

/************************************************************************
 * Destroy a TR_BASIC structure.
 ***********************************************************************/

void destroyTrBasic (TR_BASIC* basicparam);

#define TR_BASIC_INCLUDED
#endif  /* TR_BASIC_INCLUDED */
