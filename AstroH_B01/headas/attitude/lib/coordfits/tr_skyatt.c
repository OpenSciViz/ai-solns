#include <math.h>
#include <string.h>
#include "fitsio.h"
#include "longnam.h"
#include "teldef2.h"

TR_SKYATT* readTrSkyAtt(fitsfile* fp, 
		      char* lowcoordsysname, char* highcoordsysname,
		      int low_sys, char* filename)
{
  TR_SKYATT* skyattparam = (TR_SKYATT*) malloc(sizeof(TR_SKYATT));
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  int status = 0;
  int hdutype;

  strcpy(skyattparam->lowcoordsysname, lowcoordsysname);
  strcpy(skyattparam->highcoordsysname, highcoordsysname);
  skyattparam->low_sys = low_sys;
  skyattparam->high_sys = low_sys + 1;
  skyattparam->sky_pix_per_radian = 0; /* Initial invalid value. */

  /* Move to primary extension. */

  fits_movabs_hdu(fp, 1, &hdutype, &status);
  checkFITSerrors(status, "moving to primary extension of", filename); 

 /* Read the focal length from the FOCALLEN keyword. */

  strcpy(keyname, "FOCALLEN");
  fits_read_key_dbl(fp, keyname, &skyattparam->focal_length, NULL, &status);
  if(status == KEY_NO_EXIST) /* Keyword not found */
    {
      skyattparam->focal_length = 1.0;
      status = 0;
    }
  sprintf(tempstring, "reading %s from", keyname);
  checkFITSerrors(status, tempstring, filename);

  /* Allocate the alignment matrix. */

  skyattparam->alignment = readTelDef2Alignment(fp, skyattparam->lowcoordsysname, filename);

  return skyattparam;
}

ALIGN* readTelDef2Alignment(fitsfile* fp, char* lowcoordsysname, char* filename)
{
  ALIGN* align;
  ROTMATRIX* rot;
  int status = 0;

  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  int i, j;
  long long_value;

  /* Allocate ALIGN structure. */

  align = allocateDefaultAlign();

  /* Allocate ROTMATRIX and open file */

  rot = allocateRotMatrix();

  /* Read alignment matrix from TelDef keywords. */

  for(j = 0; j < 3; j++)
    {
      for(i = 0; i < 3; i++)
	{
	  /* Read matrix element from xxx_Mij keyword. */
	  
	  sprintf(keyname, "%s_M%d%d", lowcoordsysname, i + 1, j + 1);
	  fits_read_key_dbl(fp, keyname, &(rot->m[j][i]), NULL, &status);

	  /* Try ALIGNMij keyword if xxx_Mij wasn't found. */

	  if(status == KEY_NO_EXIST)
	    {
	      status = 0;
	      sprintf(keyname, "ALIGNM%d%d", i + 1, j + 1);
	      fits_read_key_dbl(fp, keyname, &(rot->m[j][i]), NULL, &status);
	      sprintf(tempstring, "reading %s from", keyname);
	      checkFITSerrors(status, tempstring, filename);
	    }
	  else
	    {
	      sprintf(tempstring, "reading %s from", keyname);
	      checkFITSerrors(status, tempstring, filename);
	    }
	}
    }

  /* Convert rotation matrix to quaternion. */

  convertRotMatrixToQuat(align->q, rot);
  
  if(fabs(normOfQuat(align->q) - 1.0) > ALIGN_ROUNDOFF_ERROR) 
    {
      fprintf(stderr, "Ill-formed alignment matrix for transforming from %s.\n", lowcoordsysname);
      return NULL;
    }
  
  /* Calculate inverse quaternion. */

  invertQuat(align->q_inverse, align->q);

  /* Free rotation matrix. */

  destroyRotMatrix(rot);

  /* Read the roll angle keywords */

  fits_read_key_dbl(fp, "ROLLOFF", &align->roll_offset, NULL, &status);
  if(status == KEY_NO_EXIST) 
    {
      align->roll_offset = 0.;
      status=0;
    }
  
  fits_read_key_lng(fp, "ROLLSIGN", &long_value, NULL, &status);
  if(status == KEY_NO_EXIST) 
    {
      long_value = 1;
      status=0;
    }
  align->roll_sign = long_value;
  if(abs(align->roll_sign) != 1)
    {
      fprintf(stderr, "Invalid ROLLSIGN=%d\n", align->roll_sign);
      return NULL;
    }
  
  /* Check for stray errors. */

  checkFITSerrors(status, "reading alignment structure from", filename);

  /* No problems were encountered, so return the structure pointer. */

  return align;
}



void printTrSkyAtt(TR_SKYATT* skyattparam)
{
  printf("    Tr_SkyAtt structure contents:\n");
  printf("      lowcoordsysname: %s\n", skyattparam->lowcoordsysname);
  printf("      highcoordsysname: %s\n", skyattparam->highcoordsysname);
  printf("      low_sys: %d\n", skyattparam->low_sys);
  printf("      high_sys: %d\n", skyattparam->high_sys);
  printf("      focal_length: %f\n", skyattparam->focal_length);
  printf("      sky_pix_per_radian: %f\n", skyattparam->sky_pix_per_radian);
  printf("      alignment:\n");
  printAlign(skyattparam->alignment);
}


void printAlign(ALIGN* align)
{
  printf("        ALIGN structure contents:\n");
  printf("          q: ");
  printQuat(align->q);
  printf("\n          q_inverse: ");
  printQuat(align->q_inverse);
  printf("\n          roll_sign: %d\n", align->roll_sign);
  printf("          roll_offset: %f\n", align->roll_offset);
  
}

void printQuat(QUAT* q)
{
  printf("%g %g %g %g", q->p[0], q->p[1], q->p[2], q->p[3]);
}


void destroyTrSkyAtt(TR_SKYATT* skyattparam)
{
  int low_sys = skyattparam->low_sys;
  
#ifdef DEBUG
  printf("Freeing skyattparam[%d]\n", low_sys);
#endif

  destroyAlign(skyattparam->alignment);
  free(skyattparam);

#ifdef DEBUG
  printf("Freed skyattparam[%d]\n", low_sys);
#endif
}

