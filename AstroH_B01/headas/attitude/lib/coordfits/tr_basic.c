#include <math.h>
#include <string.h>
#include "fitsio.h"
#include "longnam.h"
#include "teldef2.h"

TR_BASIC* readTrBasic(fitsfile* fp, int is_rawtodet, 
		      char* lowcoordsysname, char* highcoordsysname,
		      int low_sys, char* filename)
{
  
  TR_BASIC* basicparam = (TR_BASIC*) malloc(sizeof(TR_BASIC));
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  int status = 0;
  int hdutype;

  strcpy(basicparam->lowcoordsysname, lowcoordsysname);
  strcpy(basicparam->highcoordsysname, highcoordsysname);
  basicparam->low_sys = low_sys;
  basicparam->high_sys = low_sys + 1;

  /* Move to primary extension. */

  fits_movabs_hdu(fp, 1, &hdutype, &status);
  checkFITSerrors(status, "moving to primary extension of", filename); 


  /* Read the x offset from the xxx_XOFF keyword. */

  strcpy(keyname, highcoordsysname);
  strcat(keyname, "_XOFF");
  fits_read_key_dbl(fp, keyname, &basicparam->xoffset, NULL, &status);
  if(status == KEY_NO_EXIST) /* Keyword not found */
    {
      basicparam->xoffset = 0.0;
      status = 0;
    }
  sprintf(tempstring, "reading %s from", keyname);
  checkFITSerrors(status, tempstring, filename);

  /* Read the y offset from the xxx_YOFF keyword. */

  strcpy(keyname, highcoordsysname);
  strcat(keyname, "_YOFF");
  fits_read_key_dbl(fp, keyname, &basicparam->yoffset, NULL, &status);
  if(status == KEY_NO_EXIST) /* Keyword not found */
    {
      basicparam->yoffset = 0.0;
      status = 0;
    }
  sprintf(tempstring, "reading %s from", keyname);
  checkFITSerrors(status, tempstring, filename);

  /* Read the scaling factor from the xxx_SCAL keyword. */

  strcpy(keyname, highcoordsysname);
  strcat(keyname, "_SCAL");
  fits_read_key_dbl(fp, keyname, &basicparam->scale, NULL, &status);
  if(status == KEY_NO_EXIST) /* Keyword not found */
    {
      basicparam->scale = 1.0;
      status = 0;
    }
  sprintf(tempstring, "reading %s from", keyname);
  checkFITSerrors(status, tempstring, filename);

  /* Read the X flip factor from the xxxFLIPX keyword. */

  strcpy(keyname, highcoordsysname);
  strcat(keyname, "XFLIP");
  fits_read_key_lng(fp, keyname, &basicparam->xflip, NULL, &status);
  if(status == KEY_NO_EXIST) /* Keyword not found */
    {
      basicparam->xflip = 1;
      status = 0;
    }
  sprintf(tempstring, "reading %s from", keyname);
  checkFITSerrors(status, tempstring, filename);

  /* Read the Y flip factor from the xxxFLIPY keyword. */

  strcpy(keyname, highcoordsysname);
  strcat(keyname, "YFLIP");
  fits_read_key_lng(fp, keyname, &basicparam->yflip, NULL, &status);
  if(status == KEY_NO_EXIST) /* Keyword not found */
    {
      basicparam->yflip = 1;
      if(is_rawtodet)
	{
	  basicparam->yflip = -1;
	}
      status = 0;
    }
  sprintf(tempstring, "reading %s from", keyname);
  checkFITSerrors(status, tempstring, filename);

  /* Read the rotation angle (degrees) from the xxx_ROTD keyword. */

  strcpy(keyname, highcoordsysname);
  strcat(keyname, "_ROTD");
  fits_read_key_dbl(fp, keyname, &basicparam->rotangle, NULL, &status);
  if(status == KEY_NO_EXIST) /* Keyword not found */
    {
      basicparam->rotangle = 0.0;
      status = 0;
    }
  sprintf(tempstring, "reading %s from", keyname);
  checkFITSerrors(status, tempstring, filename);


  return basicparam;
}


void printTrBasic(TR_BASIC* basicparam)
{
  printf("    Tr_Basic structure contents:\n");
  
  printf("      lowcoordsysname: %s\n", basicparam->lowcoordsysname);
  printf("      highcoordsysname: %s\n", basicparam->highcoordsysname);
  printf("      low_sys: %d\n", basicparam->low_sys);
  printf("      high_sys: %d\n", basicparam->high_sys);
  printf("      xoffset: %f\n", basicparam->xoffset);
  printf("      yoffset: %f\n", basicparam->yoffset);
  printf("      scale: %f\n", basicparam->scale);
  printf("      xflip: %ld\n", basicparam->xflip);
  printf("      yflip: %ld\n", basicparam->yflip);
  printf("      rotangle: %f\n", basicparam->rotangle);
}


void destroyTrBasic(TR_BASIC* basicparam)
{
  int low_sys = basicparam->low_sys;

#ifdef DEBUG
  printf("Freeing basicparam[%d]\n", low_sys);
#endif

  free(basicparam);

#ifdef DEBUG
  printf("Freed basicparam[%d]\n", low_sys);
#endif
}

