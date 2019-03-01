#ifndef TR_RAWTODET_INCLUDED

#include "fitsio.h"

/* Raw method type - which Raw-To-Det transformation is used. */
enum RM_TYPE {RM_UNKNOWN, RM_CORNER_LIST, RM_LINEAR_COEFF, RM_ASCA_GIS};

/****************************************************************************
* TR_RAWTODET structure holds the parameters needed for a variety of
* transformations from a multi-segment coordinate system to a
* single-segment system.cients for the transformation.  The
* transformation parameters in common with the TR_BASIC transformation
* are held in the TR_BASIC structure and are not repeated here.
*****************************************************************************/
typedef struct {

  char lowcoordsysname[FLEN_VALUE];
  char highcoordsysname[FLEN_VALUE];
  int low_sys;
  int high_sys;

  /* Internal coordinate system center */
  double int_cen_x;
  double int_cen_y;
  int int_cen_found;

  /* Transformation method */
  enum RM_TYPE rawmethod;
  
  /* Number of segments in lowcoordsys. */
  int n_segs;
  int min_seg;
  char segcolname[FLEN_VALUE];

  /* Pixel corner map columns */

  int* corner_seg;
  double** corner_x;
  double** corner_y;

  /* Linear Coefficients */

  double coeff_x_a[10];
  double coeff_x_b[10];
  double coeff_x_c[10];
  double coeff_y_a[10];
  double coeff_y_b[10];
  double coeff_y_c[10];

} TR_RAWTODET;


/*****************************************************************************
* create a new TR_RAWTODET structure and read its contents from a TelDef file.
****************************************************************************/
TR_RAWTODET* readTrRawtodet (fitsfile* fp, 
			     char* lowcoordsysname, char* highcoordsysname,
			     int low_sys, char* filename);

enum RM_TYPE readPixelCornerMapInTelDef2(fitsfile* fp, TR_RAWTODET* rawtodetparam, char* filename);

enum RM_TYPE readLinearCoeffInTelDef2(fitsfile* fp, TR_RAWTODET* rawtodetparam, char* filename);

void printTrRawtodet (TR_RAWTODET* rawtodetparam);

/************************************************************************
 * Destroy a TR_RAWTODET structure.
 ***********************************************************************/

void destroyTrRawtodet (TR_RAWTODET* rawtodetparam);
 
#define TR_RAWTODET_INCLUDED
#endif  /* TR_RAWTODET_INCLUDED */
