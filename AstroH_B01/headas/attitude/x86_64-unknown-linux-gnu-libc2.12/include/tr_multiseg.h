#ifndef TR_MULTISEG_INCLUDED

#include "fitsio.h"


/****************************************************************************
* TR_MULTISEG structure holds the parameters needed for a transformation
* between multi-segment coordinate systems using coefficients for the
* transformation.  The transformation parameters in common with the TR_BASIC
* transformation are held in the TR_BASIC structure and are not repeated
* here.
*****************************************************************************/
typedef struct {

  char lowcoordsysname[FLEN_VALUE];
  char highcoordsysname[FLEN_VALUE];
  int low_sys;
  int high_sys;

  /* column arrays for the MULTISEGn_COEFF table */
  long n_properties;     /* Stores the number of property columns 
			   from the NPROP keyword */
  char** propertynames; /* Stores values of PROPp keyword names, which give
			   the corresponding event file column names */
  long n_rows;          /* Stores the number of table rows */
  long n_cols;          /* Stores the number of table columns */
  long** properties;    /* Stores property columns */
  double* coeff_x_a;    /* Stores column COEFF_X_A */
  double* coeff_x_b;    /* Stores column COEFF_X_B */
  double* coeff_x_c;    /* Stores column COEFF_X_C */
  double* coeff_y_a;    /* Stores column COEFF_Y_A */
  double* coeff_y_b;    /* Stores column COEFF_Y_B */
  double* coeff_y_c;    /* Stores column COEFF_Y_C */
  double* coeffarrays[6]; /* Stores pointers for the same coeff arrays. */
		    
} TR_MULTISEG;


/*****************************************************************************
* create a new TR_MULTISEG structure and read its contents from a TelDef file.
****************************************************************************/
TR_MULTISEG* readTrMultiseg (fitsfile* fp, 
			     char* lowcoordsysname, char* highcoordsysname,
			     int low_sys, char* filename);

void printTrMultiseg(TR_MULTISEG* multisegparam);

/************************************************************************
 * Destroy a TR_MULTISEG structure.
 ***********************************************************************/

void destroyTrMultiseg (TR_MULTISEG* multisegparam);
 
#define TR_MULTISEG_INCLUDED
#endif  /* TR_MULTISEG_INCLUDED */
