#ifndef TR_SKYATT_INCLUDED

#include "fitsio.h"
#include "align.h"

/****************************************************************************
* TR_SKYATT structure holds the parameters needed for a transformation
* that includes an attitude adjustment, such as converting to sky
* coordinates.
* The transformation parameters in common with the TR_BASIC
* transformation are held in the TR_BASIC structure and are not repeated
* here.
*****************************************************************************/
typedef struct {

  char lowcoordsysname[FLEN_VALUE];
  char highcoordsysname[FLEN_VALUE];
  int low_sys;
  int high_sys;

  ALIGN* alignment; /* Stores the alignment matrix */
  
  double focal_length;
  double sky_pix_per_radian; /* plate scale used to det -> sky conversions */
  
  /*************************************************************************
   * the following items are not read from the teldef file, but
   * can be set "manually" after the teldef file has been read.
   * q0 is the quaternion describing the nominal pointing - i.e. it gives the
   * the orientation of the sky coordinate plane with respect to the celestial
   * fcoordinate axies.
   * rot0 is a 3x3 rotation matrix equivalent to q0. This is needed to
   * do the aberration calculation efficiently.
   * delta_q, xrt, and det2sky are temporary storage for 
   * convertDetectorToSkyUsingTeldef
   * they record the last rotation between q0 and the current pointing,
   * xrt pointing and detector to sky transform calculated.
   **************************************************************************/
  QUAT* q0; 
  ROTMATRIX* rot0;
  
  QUAT* delta_q;
  QUAT*    xrt;
  XFORM2D* det2sky;
		    
} TR_SKYATT;


/*****************************************************************************
* create a new TR_SKYATT structure and read its contents from a TelDef file.
****************************************************************************/
TR_SKYATT* readTrSkyAtt (fitsfile* fp,
			 char* lowcoordsysname, char* highcoordsysname,
			 int low_sys, char* filename);

ALIGN* readTelDef2Alignment(fitsfile* fp, char* lowcoordsysname, char* filename);

void printTrSkyAtt(TR_SKYATT* skyattparam);

void printAlign(ALIGN* align);

void printQuat(QUAT* q);

/************************************************************************
 * Destroy a TR_SKYATT structure.
 ***********************************************************************/

void destroyTrSkyAtt (TR_SKYATT* skyattparam);
 
#define TR_SKYATT_INCLUDED
#endif  /* TR_SKYATT_INCLUDED */
