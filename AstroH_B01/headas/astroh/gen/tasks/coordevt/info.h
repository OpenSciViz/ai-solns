#ifndef COORDEVT_INFO_INCLUDED
#include "coordfits2.h"
#include "param.h"

/**********************************************************************
* a structure of information useful to the update_coordinate function *
**********************************************************************/
typedef struct {

PARAM* param;

ATTFILE* att;
ATTFILE* datt;
TELDEF2* teldef;


/*************************************************************
* these things do not transport information. They are
* just in here so that we don't have to create and destroy
* these structures withing the iterated function 
*************************************************************/
QUAT* q; /* current pointing */


/************************************************************
* abberation information used only when param->follow_sun=0 *
************************************************************/
double v;       /* magnitude of earth's velocity */
double vhat[3]; /* direction of earths velocity (normalized to unity)  */

double mjdref; /* value of MJDREF keyword in event file */


/********************
* FITS column stuff *
********************/
  iteratorCol* time_col; /* For SKYATT transformations */
  iteratorCol** seg_cols; /* For RAWTODET transformations. */
  iteratorCol*** prop_cols; /* For MULTISEG transformations. */
  iteratorCol** intx_cols; /* For all transformations. */
  iteratorCol** inty_cols; /* For all transformations. */
  iteratorCol** floatx_cols; /* For all transformations. */
  iteratorCol** floaty_cols; /* For all transformations. */

  /* Coordinate value arrays */
 
  double* time;
  int** intx;  /* intx[sys][row] */
  int** inty;  /* inty[sys][row] */
  double** floatx; /* coordx[sys][row] */
  double** floaty; /* coordy[sys][row] */
  int** segs; /* segs[sys][row] */
  int*** props; /* props[sys][prop][row] */
  
  long missing_att_count; /* number of events not covered by attitude file */
  long missing_datt_count; /* number of events not covered by attitude file */

} INFO;


/*****************************************************
* set up misc info structure to hand to the iterator 
* the fitsfile pointer argument is for the event file
*****************************************************/
INFO* createInfo(PARAM* param, fitsfile* fp);


/**************************************************************************
***************************************************************************
* function to set iterator FITS column strucutres
**************************************************************************/
iteratorCol* setIteratorColumns(INFO* info, fitsfile* fp, int* n_cols);


/******************************************************************************
*******************************************************************************
* free all the memory associated with an INFO structure
* Note this even destroys the associated PARAM structure even though
* that structure was created separately.
******************************************************************************/
void destroyInfo(INFO* info);

#define COORDEVT_INFO_INCLUDED
#endif /* COORDEVT_INFO_INCLUDED */
