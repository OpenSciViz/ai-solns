#include <math.h>
#include <string.h>
#include "fitsio.h"
#include "longnam.h"

#include "teldef2.h"

/*#define DEBUG 1*/

/****************************************************************************
*****************************************************************************
* create a new TELDEF2 structure and read its contents from a file.
* The actual reading is usually done by other mission-dependant routines
****************************************************************************/
TELDEF2* readTelDef2(char* filename) {

  TELDEF2* teldef;
  int length;
  fitsfile *fp;
  int hdutype;
  int status=0;
  long sys; /* Index or counter variable for coordinate system number */
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  int seg;

  /* Allocate space for the teldef structure. */

  teldef = (TELDEF2*) malloc(sizeof(TELDEF2));

  /* Set the filename in the structure. */

  length = strlen(filename) + 1;
  strncpy(teldef->filename, filename, length);

  /* Open the teldef file and move to the primary extension. */
  
  fits_open_file(&fp, filename, READONLY, &status);
  checkFITSerrors(status, "opening", teldef->filename); 

  fits_movabs_hdu(fp, 1, &hdutype, &status);
  checkFITSerrors(status, "moving to primary extension of", teldef->filename); 


  /* Read the mission from the TELESCOP keyword. */

  fits_read_key_str(fp, "TELESCOP", teldef->mission, NULL, &status);
  checkFITSerrors(status, "reading TELESCOP from", teldef->filename);


  /* Read the instrument from the INSTRUME keyword. */

  fits_read_key_str(fp, "INSTRUME", teldef->instrument, NULL, &status);
  checkFITSerrors(status, "reading INSTRUME from", teldef->filename);

  /* Read the Teldef file format version number from TD_VERS keyword. 
   *  If the keyword is not found, then the version is 0.1. */

  fits_read_key_dbl(fp, "TD_VERS", &teldef->td_version, NULL, &status);
  if(status == 202) /* Keyword not found */
    {
      teldef->td_version = 0.1;
      status = 0;
    }
  checkFITSerrors(status, "reading TD_VERS from", teldef->filename);

  /* Read the number of coordinate systems from the NCOORDS keyword. */

  fits_read_key_lng(fp, "NCOORDS", &teldef->n_coordsys, NULL, &status);
  checkFITSerrors(status, "reading NCOORDS from", teldef->filename);
  if(teldef->n_coordsys <= 0)
    {
      fprintf(stderr, "NCOORDS keyword value is %ld, but must be nonnegative. Quitting.\n", teldef->n_coordsys);
      exit(status);
    }
  
  /* Allocate several arrays. 
   * - coordsys, rawtodetparam, multisegparam, skyattparam are only
   *   partially allocated. More info is needed to finish allocating them. 
   * - basicparam is completely allocated.  It is needed for all systems. 
   */

  teldef->coordsysnames = calloc(teldef->n_coordsys,  sizeof(char*));
  teldef->trtypes = calloc(teldef->n_coordsys - 1,  sizeof(char*));
  teldef->coordsys = calloc(teldef->n_coordsys,  sizeof(COORDDEF**));
  teldef->n_segments = calloc(teldef->n_coordsys,  sizeof(int));
  teldef->min_segment = calloc(teldef->n_coordsys,  sizeof(int));
  teldef->basicparam = calloc(teldef->n_coordsys - 1,  sizeof(TR_BASIC*));
  teldef->rawtodetparam = calloc(teldef->n_coordsys - 1,  sizeof(TR_RAWTODET*));
  teldef->multisegparam = calloc(teldef->n_coordsys - 1,  sizeof(TR_MULTISEG*));
  teldef->skyattparam = calloc(teldef->n_coordsys - 1,  sizeof(TR_SKYATT*));

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      teldef->coordsysnames[sys] = calloc(FLEN_VALUE, sizeof(char));

      if(sys < teldef->n_coordsys - 1)
	{
	  teldef->trtypes[sys] = calloc(FLEN_VALUE, sizeof(char));
	}

    }


  /* Read the names of the coordinate systems from the COORDn keywords. */

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      sprintf(keyname, "COORD%ld", sys);
      fits_read_key_str(fp, keyname, tempstring, NULL, &status);
      strcpy(teldef->coordsysnames[sys], tempstring);

      sprintf(tempstring, "reading %s from", keyname);
      checkFITSerrors(status, tempstring, teldef->filename);
    }


  /* Read the names of the transformation types from the TRTYPEn keywords. 
   * These are required if TD_VERS >= 0.1. These are determined if TD_VERS < 0.1. */

  readTransformationTypesFromTelDef2(fp, teldef);

  /* Allocate and fill the transformation parameter structures. */

  setTransformationParametersFromTelDef2(fp, teldef);

  /* Determine the number of segments in each coordinate system. */

  setSegmentCountInTelDef2(fp, teldef);


  /* Allocate COORDDEF structures for each segment of each coordinate system.  */

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      teldef->coordsys[sys] = (COORDDEF**) malloc((teldef->n_segments[sys] + teldef->min_segment[sys])* sizeof(COORDDEF*));
				     
      for(seg = teldef->min_segment[sys]; 
	  seg < teldef->min_segment[sys] + teldef->n_segments[sys]; seg++)
	{
	  teldef->coordsys[sys][seg] = allocateCoordDef();

	  /* Initialize the transformation with some dummy values for now. */
	  
	  teldef->coordsys[sys][seg]->trans->rot[0][0] = 1;
	  teldef->coordsys[sys][seg]->trans->rot[0][1] = 0;
	  teldef->coordsys[sys][seg]->trans->rot[1][0] = 0;
	  teldef->coordsys[sys][seg]->trans->rot[1][1] = 1;
	  teldef->coordsys[sys][seg]->trans->xshift = 0.;
	  teldef->coordsys[sys][seg]->trans->yshift = 0.;
	  teldef->coordsys[sys][seg]->min_x = 0.;
	  teldef->coordsys[sys][seg]->max_x = 1.;
	  teldef->coordsys[sys][seg]->min_y = 0.;
	  teldef->coordsys[sys][seg]->max_y = 1.;
	  setCoordDefTransformInfo(teldef->coordsys[sys][seg]);

	  /* Set the coord. sys. name and other coordinate properties. */

	  teldef->coordsys[sys][seg]->name = teldef->coordsysnames[sys];
	  setCoordinatesFromKeywordsInTeldef2(teldef, teldef->coordsys[sys][seg], fp, sys);
	}
    }


  /* For RAWTODET transformations, fix the internal coordinate system
   * center if the relevant keywords were not found. */

  for(sys = 0; sys < teldef->n_coordsys - 1; sys++)
    {
      if(!strcasecmp(teldef->trtypes[sys], "RAWTODET"))
	{
	  if(!teldef->rawtodetparam[sys]->int_cen_found)
	    {
	      teldef->rawtodetparam[sys]->int_cen_x = teldef->coordsys[sys+1][0]->center_x;
	      teldef->rawtodetparam[sys]->int_cen_y = teldef->coordsys[sys+1][0]->center_y;
	      teldef->rawtodetparam[sys]->int_cen_found = 1;
	    }

	}

    }

  /* Fill in coordinate system keyword values and set 2D transformations. */

  for(sys = 0; sys < teldef->n_coordsys - 2; sys++)
    {
      for(seg = teldef->min_segment[sys]; 
	  seg < teldef->min_segment[sys] + teldef->n_segments[sys]; seg++)
	{
	  /*teldef->coordsys[sys][seg]->trans->rot[0][0] = 1;
	  teldef->coordsys[sys][seg]->trans->rot[1][0] = 0;
	  teldef->coordsys[sys][seg]->trans->rot[0][1] = 0;
	  teldef->coordsys[sys][seg]->trans->rot[1][1] = 1;
	  teldef->coordsys[sys][seg]->trans->xshift = 0;
	  teldef->coordsys[sys][seg]->trans->yshift = 0;*/

	  setXform2dFromTransformationParameters(teldef, teldef->coordsys[sys][seg], 
						 teldef->coordsys[sys + 1][0], sys, seg);
	  setCoordDefTransformInfo(teldef->coordsys[sys][seg]);
	}
    }

  /* Read intrapixel event location randomization parameters. */

  readRandomizationFromTeldef2(fp, teldef);

  /* Clean up. */

  fits_close_file(fp, &status);
  checkFITSerrors(status, "closing", teldef->filename);

  return teldef;

}

/* Read transformation types. */
void readTransformationTypesFromTelDef2(fitsfile* fp, TELDEF2* teldef)
{
  int sys;
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  int status = 0;
  
  for(sys = 0; sys < teldef->n_coordsys - 1; sys++)
    {
      sprintf(keyname, "TRTYPE%d", sys);
      fits_read_key_str(fp, keyname, tempstring, NULL, &status);
      
      /* If the TelDef version is earlier than 0.2, the TRTYPEn 
       * keywords are absent and must be inferred. */
      
      if(teldef->td_version < 0.2 && status == KEY_NO_EXIST)
	{
	  status = 0;
	  if(sys == 0)
	    {
	      strcpy(tempstring, "RAWTODET");
	    }
	  else if(sys == teldef->n_coordsys - 2)
	    {
	      strcpy(tempstring, "SKYATT");
	    }
	  else
	    {
	      strcpy(tempstring, "BASIC");
	    }
	}

      strcpy(teldef->trtypes[sys], tempstring);

      sprintf(tempstring, "reading %s from", keyname);
      checkFITSerrors(status, tempstring, teldef->filename);
    }
  
  /* Suzaku XIS0 - XIS3 are exceptions without the TRTYPE keywords. */

  if(!strcasecmp(teldef->mission, "SUZAKU") && !strncmp(teldef->instrument,"XIS", 3))
    {
      strcpy(teldef->trtypes[0], "BASIC");
      strcpy(teldef->trtypes[1], "RAWTODET");
    }
	  

}

/* Allocate and fill the transformation parameter structures. */
void setTransformationParametersFromTelDef2(fitsfile* fp, TELDEF2* teldef)
{
  int sys;
    
  for(sys = 0; sys < teldef->n_coordsys - 1; sys++)
    {
      if(sys < teldef->n_coordsys - 2)
	{
	  teldef->basicparam[sys] = readTrBasic(fp, 
						!strcasecmp(teldef->trtypes[sys], "RAWTODET"),
						teldef->coordsysnames[sys], 
						teldef->coordsysnames[sys + 1], 
						sys, teldef->filename);
	}
      else
	{
	  teldef->basicparam[sys] = NULL;
	}
	
      if(!strcasecmp(teldef->trtypes[sys], "SKYATT"))
	{
	  teldef->skyattparam[sys] = readTrSkyAtt(fp, teldef->coordsysnames[sys], 
						  teldef->coordsysnames[sys + 1], 
						  sys, teldef->filename);
	}
      else
	{
	  teldef->skyattparam[sys] = NULL;
	}
	
      if(!strcasecmp(teldef->trtypes[sys], "MULTISEG"))
	{
	  teldef->multisegparam[sys] = readTrMultiseg(fp, teldef->coordsysnames[sys], 
						      teldef->coordsysnames[sys + 1], 
						      sys, teldef->filename);
	}
      else
	{
	  teldef->multisegparam[sys] = NULL;
	}
	
      if(!strcasecmp(teldef->trtypes[sys], "RAWTODET"))
	{
	  teldef->rawtodetparam[sys] = readTrRawtodet(fp, teldef->coordsysnames[sys], 
						      teldef->coordsysnames[sys + 1], 
						      sys, teldef->filename);
	  /*printTrRawtodet(teldef->rawtodetparam[sys]);*/
	}
      else
	{
	  teldef->rawtodetparam[sys] = NULL;
	}
    }
    
  return;
}

/* Read randomization parameters from TelDef keywords. */

void readRandomizationFromTeldef2
(
 fitsfile* fp, /* TelDef file pointer */
 TELDEF2* teldef /* teldef structure */
)
{
  int sysnamematches = 0;
  int sys;
  int status = 0;

  /* Read the randomization coordinate system from the RANCOORD keyword. */
  fits_read_key_str(fp, "RANCOORD", teldef->ransysname, NULL, &status);

  if(status == KEY_NO_EXIST)
    {
      status = 0;
      strcpy(teldef->ransysname, "RAW");
    }
  else
    {
      checkFITSerrors(status, "reading RANCOORD from", teldef->filename);
    }

  /* Make sure the RANCOORD value is a real coordinate system or NONE. 
   * If it not a real system, set to NONE. */
  
  if(!strcasecmp(teldef->ransysname, "NONE"))
    {
      sysnamematches = 1;
      teldef->ransys = -1;
    }
  else
    {
      
      for(sys = 0; sys < teldef->n_coordsys; sys++)
	{
	  if(!strcasecmp(teldef->ransysname, teldef->coordsysnames[sys]))
	    {
	      sysnamematches = 1;
	      teldef->ransys = sys;
	      break;
	    }
	}
    }

  if(!sysnamematches)
    {
      fprintf(stdout, "Keyword RANCOORD = %s is not a recognized coordinate system for %s %s. Setting RANCOORD = NONE to suppress any recommendation for event location randomization.\n", teldef->ransysname, teldef->mission, teldef->instrument);
      strcpy(teldef->ransysname, "NONE");
      teldef->ransys = -1;
    }

  /* If RANCOORD is not NONE (ransys >= 0) at this point, read the
     RAN_SCAL keyword to get the name of the system whose pixel size
     is to be used for the event location randomization. If it is not
     found, set it to the value of the RANCOORD keyword. */
  
  sysnamematches = 0;
  
  if(teldef->ransys >= 0)
    {
      fits_read_key_str(fp, "RAN_SCAL", teldef->ranscalesysname, NULL, &status);
      if(status == KEY_NO_EXIST)
	{
	  status = 0;
	  strcpy(teldef->ranscalesysname, teldef->ransysname);
	}
      else
	{
	  checkFITSerrors(status, "reading RAN_SCAL from", teldef->filename);
	}
      
      for(sys = 0; sys < teldef->n_coordsys; sys++)
	{
	  if(!strcasecmp(teldef->ranscalesysname, teldef->coordsysnames[sys]))
	    {
	      sysnamematches = 1;
	      teldef->ranscalesys = sys;
	      break;
	    }
	}
      
      if(!sysnamematches)
	{
	  fprintf(stdout, "Keyword RAN_SCAL = %s is not a recognized coordinate system for %s %s. Setting RAN_SCAL = %s.\n", teldef->ranscalesysname, teldef->mission, teldef->instrument, teldef->ransysname);
	  strcpy(teldef->ransysname, teldef->ransysname);
	  teldef->ranscalesys = teldef->ransys;
	}
    } 
  else /* ransysname == NONE, so set ranscalesysname = NONE showing it won't be used. */
    {
      strcpy(teldef->ranscalesysname, "NONE");
      teldef->ranscalesys = -1;
    }

  return;
}

/* Retrieve the coordinate system number from the name.  Returns -1 if
   coordinate system name is not found in teldef->coordsysnames. */

int getCoordSystemNumberFromName(TELDEF2* teldef, char* name)
{
  int sys = 0;
  
  while(sys < teldef->n_coordsys)
    {
      /*printf("sys=%d name=%s\n", sys, name);*/
      if(!strcasecmp(teldef->coordsysnames[sys], name))
	{
	  return sys;
	}
	
      sys++;
    }
  
  return -1;
}



void setXform2dFromTransformationParameters(TELDEF2* teldef, 
					    COORDDEF* origcoord, COORDDEF* destcoord,
					    int sys, int seg)
{
  XFORM2D* basictrans;
  XFORM2D* fulltrans;

  /* Get the basic transformation. */

  basictrans = getXform2dFromTrBasicParameters(teldef, origcoord, destcoord, 
					       teldef->basicparam[sys], sys);
  
#ifdef DEBUG
  printf("sys=%d basictrans:\n", sys);
  printXform2d(basictrans, stdout);
#endif
  
  if(!strcasecmp(teldef->trtypes[sys], "BASIC"))
    {
      copyXform2d(origcoord->trans, basictrans);
    }
  else if(!strcasecmp(teldef->trtypes[sys], "RAWTODET") && 
	  teldef->rawtodetparam[sys]->rawmethod == RM_LINEAR_COEFF)
    {
      fulltrans = getXform2dFromTrRawtodetLinearCoeffParameters(teldef, basictrans,
								teldef->rawtodetparam[sys], 
								sys, seg);
      copyXform2d(origcoord->trans, fulltrans);

      destroyXform2d(fulltrans);
    }
  else if(!strcasecmp(teldef->trtypes[sys], "RAWTODET") && 
	  teldef->rawtodetparam[sys]->rawmethod == RM_CORNER_LIST)
    {
      fulltrans = getXform2dFromTrRawtodetCornerMapParameters(teldef, basictrans,
							      origcoord,
							      teldef->rawtodetparam[sys], 
							      sys, seg);
      copyXform2d(origcoord->trans, fulltrans);

#ifdef DEBUG
      printf("sys=%d seg=%d fulltrans:\n", sys, seg);
      printXform2d(origcoord->trans, stdout);
#endif

      destroyXform2d(fulltrans);
    }
  else if(!strcasecmp(teldef->trtypes[sys], "MULTISEG"))
    {
      fulltrans = getXform2dFromTrMultisegLinearCoeffParameters(teldef, basictrans,
								teldef->multisegparam[sys], 
								sys, seg);
      copyXform2d(origcoord->trans, fulltrans);

      destroyXform2d(fulltrans);
    }

  /* Clean up. */

  destroyXform2d(basictrans);
  
  return;
}


XFORM2D* getXform2dFromTrBasicParameters(TELDEF2* teldef, 
					 COORDDEF* origcoord, COORDDEF* destcoord,
					 TR_BASIC* basicparam, int sys)
{
  XFORM2D* translation;
  XFORM2D* scaling;
  XFORM2D* rotation;
  XFORM2D* temp;
  XFORM2D* trans;
  int is_rawtodet = !strcasecmp(teldef->trtypes[sys], "RAWTODET");

  /* Translate the center of the orig coordinates (0., 0.), and then apply an offset. 
   *
   * If the corresponding transformation type is RAWTODET, then the internal coordinate
   * system center should be used instead of that of the lower-level center. */

  translation = allocateXform2d();

  if(is_rawtodet)
    {
      setXform2dToTranslation(translation, 
			      -teldef->rawtodetparam[sys]->int_cen_x - basicparam->xoffset,
			      -teldef->rawtodetparam[sys]->int_cen_y - basicparam->yoffset
			      );
      
    }
  else
    {
      setXform2dToTranslation(translation, 
			      -origcoord->center_x - basicparam->xoffset,
			      -origcoord->center_y - basicparam->yoffset
			      );
    }

  /* Set the transformation for scaling and inversion. */

  scaling = allocateXform2d();
  setXform2dToScaling(scaling, 
		      basicparam->xflip/basicparam->scale, 
		      basicparam->yflip/basicparam->scale, 0., 0.
		      );

  /* Set the transformation for rotation. */

  rotation = allocateXform2d();
  setXform2dToRotation(rotation, 
		       sin(basicparam->rotangle * M_PI / 180.), 
		       cos(basicparam->rotangle * M_PI / 180.), 0., 0.
		       );

  /* Combine the translation, scaling, and rotation in this order. */
  temp = allocateXform2d();
  trans = allocateXform2d();
  combineXform2ds(temp, translation, scaling);
  combineXform2ds(trans, temp, rotation);

#ifdef DEBUG
 printf("translation:\n");
 printXform2d(translation, stdout);
 printf("scaling:\n");
 printXform2d(scaling, stdout);
 printf("rotation:\n");
 printXform2d(rotation, stdout);
 printf("trans:\n");
 printXform2d(trans, stdout);
#endif

  /* Translate the origin of the center of the destination coordinate system. 
     Store the result in trans. */

  applyTranslationToXform2d(trans, destcoord->center_x, destcoord->center_y);

#ifdef DEBUG
  printf("dest-transl trans:\n");
  printXform2d(trans, stdout);
#endif

  /* Clean up. */

  destroyXform2d(temp);
  destroyXform2d(translation);
  destroyXform2d(scaling);
  destroyXform2d(rotation);

  return trans;
}


XFORM2D* getXform2dFromTrRawtodetLinearCoeffParameters(TELDEF2* teldef, 
						       XFORM2D* int2dettrans, 
						       TR_RAWTODET* rawtodetparam, 
						       int sys, int seg)
{
  XFORM2D* raw2inttrans = allocateXform2d();
  XFORM2D* fulltrans = allocateXform2d();
  
  /* Set the RAW-to-INT transformation from the linear coefficients. */

  raw2inttrans->xshift = rawtodetparam->coeff_x_a[seg];
  raw2inttrans->rot[0][0] = rawtodetparam->coeff_x_b[seg];
  raw2inttrans->rot[0][1] = rawtodetparam->coeff_x_c[seg];
  
  raw2inttrans->yshift = rawtodetparam->coeff_y_a[seg];
  raw2inttrans->rot[1][0] = rawtodetparam->coeff_y_b[seg];
  raw2inttrans->rot[1][1] = rawtodetparam->coeff_y_c[seg];

  /* Combine RAW-to-INT and INT-to-DET transformations for full RAW-to-DET 
   * transformation. */

  combineXform2ds(fulltrans, raw2inttrans, int2dettrans);

  /* Clean up. */

  destroyXform2d(raw2inttrans);
  
  return fulltrans;
}


XFORM2D* getXform2dFromTrRawtodetCornerMapParameters(TELDEF2* teldef, 
						     XFORM2D* basictrans, 
						     COORDDEF* origcoord,
						     TR_RAWTODET* rawtodetparam, 
						     int sys, int seg)
{
  XFORM2D* fulltrans = allocateXform2d();
  XFORM2D* pretrans = allocateXform2d();
  XFORM2D* middletrans = allocateXform2d();
  XFORM2D* temptrans = allocateXform2d();
  
  /* Set a tranlation from the raw address space specified in the
   * basic transformation parameters to coordinates with a single
   * pixel whose center is at (0.5, 0.5). This is because
   * setXform2dFromCornerPixels returns a transform from the latter
   * coordinates. */

  setXform2dToScaling(pretrans, 1./(double)origcoord->npixels_x,
		      1./(double)origcoord->npixels_y,
		      origcoord->center_x, origcoord->center_y);

  applyTranslationToXform2d(pretrans, 0.5 - origcoord->center_x,
			    0.5 - origcoord->center_y);

#ifdef DEBUG
  printf("sys=%d seg=%d pretrans:\n", sys, seg);
  printXform2d(pretrans, stdout);
#endif

  /* Get the transformation from the pixel corner map. */

  setXform2dFromCornerPixels(middletrans, 
			     rawtodetparam->corner_x[seg], rawtodetparam->corner_y[seg]);
  combineXform2ds(temptrans, pretrans, middletrans);
  combineXform2ds(fulltrans, temptrans, basictrans);

  /* Clean up. */

  destroyXform2d(pretrans);
  destroyXform2d(temptrans);
  destroyXform2d(middletrans);
  
  return fulltrans;
}



XFORM2D* getXform2dFromTrMultisegLinearCoeffParameters(TELDEF2* teldef, 
						       XFORM2D* int2hightrans, 
						       TR_MULTISEG* multisegparam, 
						       int sys, int seg)
{
  XFORM2D* low2inttrans = allocateXform2d();
  XFORM2D* fulltrans = allocateXform2d();
  
  /* Set the Low-to-INT transformation from the linear coefficients. */

  low2inttrans->xshift = multisegparam->coeff_x_a[seg];
  low2inttrans->rot[0][0] = multisegparam->coeff_x_b[seg];
  low2inttrans->rot[0][1] = multisegparam->coeff_x_c[seg];
  
  low2inttrans->yshift = multisegparam->coeff_y_a[seg];
  low2inttrans->rot[1][0] = multisegparam->coeff_y_b[seg];
  low2inttrans->rot[1][1] = multisegparam->coeff_y_c[seg];

  /* Combine RAW-to-INT and INT-to-DET transformations for full RAW-to-DET 
   * transformation. */

  combineXform2ds(fulltrans, low2inttrans, int2hightrans);

  /* Clean up. */

  destroyXform2d(low2inttrans);
  
  return fulltrans;
}



/* Determine the number of segments in each coordinate system. */
void setSegmentCountInTelDef2(fitsfile* fp, TELDEF2* teldef)
{
  int sys;
  long n_segs;
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  int status = 0;

  /* Get the number of segments in each coord sys.  The number of
   * segments is 1 for systems undergoing BASIC and SKYATT
   * transformation types. The number of segments for systems
   * undergoing MULTISEG transformations is determined by the number
   * of coefficients found in the MULTISEGn_COEFF table.  The number
   * of segments for systems undergoing a RAWTODET transformation is
   * already given by the number of coefficient keywords in a
   * coefficients RAWTODET transformation, or by the number of rows in
   * a PIXEL_MAP table of a corners RAWTODET transformation.  The
   * following is a check against existing keywords specifically
   * giving the number of keywords. */

  /* !!!This check might not be needed at all!!! */

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      teldef->n_segments[sys] = 1;
      teldef->min_segment[sys] = 0;

      if(sys < teldef->n_coordsys - 1 && 
	 !strcasecmp(teldef->trtypes[sys], "RAWTODET")
	 )
	{
	  sprintf(keyname, "%s_NSEG", teldef->coordsysnames[sys]);
	  fits_read_key_lng(fp, keyname, &n_segs, NULL, &status);
	  if(status == KEY_NO_EXIST && teldef->td_version < 0.2)
	    {
	      status = 0;

	      strcpy(keyname, "SEG_NUM");
	      fits_read_key_lng(fp, keyname, &n_segs, NULL, &status);
	      if(status == KEY_NO_EXIST)
		{
		  status = 0;
		  n_segs = 1;
		}
	      else
		{
		  sprintf(tempstring, "reading %s from", keyname);
		  checkFITSerrors(status, tempstring, teldef->filename);
		}
	    }
	  else
	    {
	      sprintf(tempstring, "reading %s from", keyname);
	      checkFITSerrors(status, tempstring, teldef->filename);
	    }

	  /* At this point, n_seg is the value of the keyword
	   * explicitly giving the number of segments or is 1 if no
	   * such keyword was found.  The number of segments from the
	   * other methods of getting this value is given in the
	   * multisegparam->n_rows to rawtodetparam->n_segs
	   * variables.  These must match. */

	  teldef->n_segments[sys] = teldef->rawtodetparam[sys]->n_segs;
	  
	  
	  if(n_segs != teldef->n_segments[sys])
	    {
	      fprintf(stderr, "Mismatch in number of segments in %s system between keyword %s = %ld and number of segments (%d) deduced from transformation parameters read from the TelDef file %s. The TelDef file likely has an error.\n", teldef->coordsysnames[sys], keyname, n_segs, teldef->n_segments[sys], teldef->filename);
	      exit(0);
	    }
	  
	  /* Set minimum segment number for RAWTODET systems. */
	  
	  teldef->min_segment[sys] = teldef->rawtodetparam[sys]->min_seg;
	}

      if(sys < teldef->n_coordsys - 1 &&
	 !strcasecmp(teldef->trtypes[sys], "MULTISEG"))
	{
	  teldef->n_segments[sys] = teldef->multisegparam[sys]->n_rows;
	}
    }

  return;
}

void setCoordinatesFromKeywordsInTeldef2(TELDEF2* teldef, COORDDEF* coord,
					 fitsfile* fp, long sys)
{
  int status=0;
  
  char key[FLEN_KEYWORD];
  char* key_end;
  
  double first_pixel_x, first_pixel_y;
  long npixels_x, npixels_y;
  
  char units[FLEN_VALUE];
  char colname[FLEN_VALUE];
  char tempstring[1000];

  /**************************************************************
   * copy the root name into the beginning of the keyword string *
   **************************************************************/
  strncpy(key,coord->name,FLEN_KEYWORD);
  key_end=key+strlen(coord->name);
  
  
  /*****************************************
   * read the address space limits keywords *
   *****************************************/
  strcpy(key_end,"XPIX1");
  fits_read_key_dbl(fp,key,&first_pixel_x,NULL,&status);
  
  strcpy(key_end,"YPIX1");
  fits_read_key_dbl(fp,key,&first_pixel_y,NULL,&status);
  
  strcpy(key_end,"_XSIZ");
  fits_read_key_lng(fp,key,&npixels_x,NULL,&status);
  
  strcpy(key_end,"_YSIZ");
  fits_read_key_lng(fp,key,&npixels_y,NULL,&status);
  
  if(status==KEY_NO_EXIST && !strcasecmp(coord->name,"RAW") &&   
     !strcasecmp(teldef->mission,"ASCA") ) {
    /**************************************************************
     * we have some special defaults because the original ASCA 
     * teldef files omitted these keywords for the raw coordinates
     **************************************************************/
    status=0;
    
    if(!strcasecmp(teldef->instrument,"SIS0") ||
       !strcasecmp(teldef->instrument,"SIS1")) {
      /***********
       * ASCA SIS *
       ***********/
      first_pixel_x=6.;
      first_pixel_y=2.;
      
      npixels_x=419;
      npixels_y=416;
      
    } else if(!strcasecmp(teldef->instrument,"GIS2") ||
              !strcasecmp(teldef->instrument,"GIS3")) {
      /***********
       * ASCA GIS *
       ***********/
    } else {
      /**********
       * unknown *
       **********/
      fprintf(stderr,"Unknown ASCA instrument %s\n",teldef->instrument);
      exit(1);
    }
    
  } /*end of ASCA defaults */
  
  if(status==KEY_NO_EXIST && sys == teldef->n_coordsys - 1 ) {
    /*******************************************************
     * sky coordinate dimensions default to the originating
     * detector coordinate dimensions
     *******************************************************/
    status=0;
    
    first_pixel_x = teldef->coordsys[sys - 1][0]->first_pixel_x;
    first_pixel_y = teldef->coordsys[sys - 1][0]->first_pixel_y;
    npixels_x     = teldef->coordsys[sys - 1][0]->npixels_x;
    npixels_y     = teldef->coordsys[sys - 1][0]->npixels_y;
  }

  sprintf(tempstring, "reading %sXPIX1, %sYPIX1, %s_XSIZ, and %s_YSIZ keywords from", coord->name, coord->name, coord->name, coord->name);
  checkFITSerrors(status, tempstring, teldef->filename);


  /*******************************************
   * set the values in the COORDDEF structure *
   *******************************************/
  setCoordDefLimits(coord,first_pixel_x,(int)npixels_x,
		    first_pixel_y,(int)npixels_y  );
  
  /*************************
   * determine scale values *
   *************************/
  if(sys == teldef->n_coordsys - 1) {
    /**********************************************************************
     * SKY coordinates have their scale determined from the focal length
     * and the scale of the originating detector coordinates
     * note that the physical pixel size of the sky coordinates must
     * be the same as the size of the originating detector coordinates
     * Note that the FOCALLEN value must be in the same units as
     * the scale of the originating detector coordinates. 
     **********************************************************************/
    double from_radians;
    COORDDEF* det;

    /*************************************
     * determine preferred physical units *
     *************************************/
    strcpy(key_end,"_UNIT");
    fits_read_key_str(fp,key,units,NULL,&status);
    if(status==KEY_NO_EXIST) {
      /***********************************
       * sky units default to arc minutes *
       ***********************************/
      status=0;
      strcpy(units,"arcmin");
    }
    
    setStringInCoordDef(&(coord->scale_unit),units);
    
    /**********************************
     * interpret the name of the units *
     **********************************/
    if(     !strcasecmp(units,"arcmin")     ) from_radians=180.*60./M_PI;
    else if(!strcasecmp(units,"arc minutes")) from_radians=180.*60./M_PI;
    else if(!strcasecmp(units,"arcsec")     ) from_radians=180.*3600./M_PI;
    else if(!strcasecmp(units,"arc seconds")) from_radians=180.*3600./M_PI;
    else if(!strcasecmp(units,"deg")        ) from_radians=180./M_PI;
    else if(!strcasecmp(units,"degrees")    ) from_radians=180./M_PI;
    else if(!strcasecmp(units,"rad"    )    ) from_radians=1.;
    else if(!strcasecmp(units,"radians")    ) from_radians=1.;
    else {
      fprintf(stderr,"Unsupported SKY units %s\n",units);
      exit(1);
    }
    
    
    /**************************************************************
     * the X and Y scales of the originating detector coordinates
     * must be the same - perhaps some day we will lift this
     * restriction
     **************************************************************/
    det=teldef->coordsys[teldef->n_coordsys - 2][0];
    if(det->scale_x != det->scale_y) {
      fprintf(stderr,"Sky pixels are not square since:\n");
      fprintf(stderr,"%s scale=%g %s/pixel and %s scale=%g %s/pixel\n",
	      det->name_x, det->scale_x, det->scale_unit,
	      det->name_y, det->scale_y, det->scale_unit );
      exit(1);
    }

    /****************
     * set the scale *
     ****************/
    coord->scale_x=det->scale_x * from_radians/teldef->skyattparam[sys - 1]->focal_length;
    coord->scale_y=det->scale_y * from_radians/teldef->skyattparam[sys - 1]->focal_length;

    /**************************************************************
     * det->sky transformations require the pixel scale in radians.
     * Note that the X and Y scales are required to be the same
     * so we arbitrarily pick the X scale here.
     **************************************************************/
    teldef->skyattparam[sys - 1]->sky_pix_per_radian = teldef->skyattparam[sys - 1]->focal_length / det->scale_x;


  } else {
    /**************************************************************************
     * for raw or detector coordinates we try to read the scale keywords
     * though in the usual case the raw coordinates default to being unscaled
     **************************************************************************/
    strcpy(key_end,"_XSCL");
    fits_read_key_dbl(fp,key,&(coord->scale_x),NULL,&status);
    
    strcpy(key_end,"_YSCL");
    fits_read_key_dbl(fp,key,&(coord->scale_y),NULL,&status);

    if(status==KEY_NO_EXIST && !strcasecmp(coord->name,"RAW") ) {
      /*************************************
       * by default raw pixels are unscaled *
       *************************************/
      status=0;

      coord->scale_x=1.;
      coord->scale_y=1.;

      setStringInCoordDef(&(coord->scale_unit),"pixels"  );
    } else {
      /*************************************************
       * This is not the RAW scale default case, so
       * determine the scale unit.
       *************************************************/
    
      strcpy(key_end,"_UNIT");
      fits_read_key_str(fp,key,units,NULL,&status);

      if(status==KEY_NO_EXIST) {
	/************************************************
	 * reset status and set default value milimeters *
	 ************************************************/
	status=0;
	strcpy(units,"mm");
      }

      setStringInCoordDef(&(coord->scale_unit),units);

    } /* end if not raw default scale */

  } /* end if setting raw or detector scales */


  /***************
   * column names *
   ***************/
    

  /****************
   * X column name *
   ****************/
  strcpy(key_end,"_XCOL");
  fits_read_key_str(fp,key,colname,NULL,&status);
  if(status==KEY_NO_EXIST) {
    /***********************
     * use the default name *
     ***********************/
    status=0;
    if(sys == teldef->n_coordsys - 1) {
  /***********************************************
       * default name for sky coordinates is just "X" *
       ***********************************************/
      strcpy(colname,"X");

    } else {
  
      /*****************************************
       * for everything else default is "nameX" *
       *****************************************/
      strcpy(colname,coord->name);
      strcat(colname,"X");

    }

  } 
  setStringInCoordDef(&(coord->name_x),colname);


  /****************
   * Y column name *
   ****************/
  strcpy(key_end,"_YCOL");
  fits_read_key_str(fp,key,colname,NULL,&status);
  if(status==KEY_NO_EXIST) {
    /***********************
     * use the default name *
     ***********************/
    status=0;
    if(sys == teldef->n_coordsys - 1) {
      /***********************************************
       * default name for sky coordinates is just "Y" *
       ***********************************************/
      strcpy(colname,"Y");
    } else {
      /*****************************************
       * for everything else default is "nameY" *
       *****************************************/
      strcpy(colname,coord->name);
      strcat(colname,"Y");
    }
   }
  setStringInCoordDef(&(coord->name_y),colname);


  /******************************
   * check for stray FITS errors *
   ******************************/
  sprintf(tempstring, "reading %s_XCOL, %s_YCOL, %s_UNIT, %s_XSCL, and %s_YSCL keywords from", coord->name, coord->name, coord->name, coord->name, coord->name);
  checkFITSerrors(status, tempstring, teldef->filename);

}


/* Convert coordinates from lower- to higher-level coord. system using the RAWTODET transformation. */

int convertToHigherCoordRawtodet
(
 TELDEF2* teldef, /* teldef structure */
 double lowx, /* x coordinate of lower-level system */
 double lowy, /* y coordinate of lower-level system */
 double* highx, /* x coordinate of higher-level system */
 double* highy, /* y coordinate of higher-level system */
 int lowsys, /* number of lower-level system */
 int lowseg  /* segment number of lower-level system */
)
{
  /*printf("min_segment=%d  n_segments=%d lowsys=%d lowseg=%d\n", teldef->min_segment[lowsys], teldef->n_segments[lowseg], lowsys, lowseg);*/
  /* Check the segment number for validity. */
  if(lowseg < teldef->min_segment[lowsys] || lowseg >= teldef->min_segment[lowsys] + teldef->n_segments[lowsys])
    {
      fprintf(stderr, "Invalid segment number %d in %s coordinate system, which has valid segment numbers %d to %d\n", lowseg, teldef->coordsysnames[lowsys], teldef->min_segment[lowsys], teldef->min_segment[lowsys] + teldef->n_segments[lowsys]);
      return 1;
    }

  /* Apply transformation to coordinates. */

  applyXform2dToContinuousCoords(teldef->coordsys[lowsys][lowseg]->trans, 
				 highx, highy, lowx, lowy);

  return 0;
}




/* Convert coordinates from lower- to higher-level coord. system using the BASIC transformation. */

int convertToHigherCoordBasic
(
 TELDEF2* teldef, /* teldef structure */
 double lowx, /* x coordinate of lower-level system */
 double lowy, /* y coordinate of lower-level system */
 double* highx, /* x coordinate of higher-level system */
 double* highy, /* y coordinate of higher-level system */
 int lowsys /* number of lower-level system */
)
{
  /* Apply transformation to coordinates. */

  applyXform2dToContinuousCoords(teldef->coordsys[lowsys][0]->trans, 
				 highx, highy, lowx, lowy);

  return 0;
}



int convertToHigherCoordMultiseg
(
 TELDEF2* teldef, /* teldef structure */
 double lowx, /* x coordinate of lower-level system */
 double lowy, /* y coordinate of lower-level system */
 double* highx, /* x coordinate of higher-level system */
 double* highy, /* y coordinate of higher-level system */
 int lowsys, /* number of lower-level system */
 int* lowprops  /* segment properties of lower-level system */
)
{
  /* Check for correct set of segment properties. */

  int propsfound = 0;
  int usethisrow = 1;
  int prop = 0;
  int coeffrow = 0;

  for(coeffrow = 0; coeffrow < teldef->multisegparam[lowsys]->n_rows; coeffrow++)
    {
      usethisrow = 1;
      for(prop = 0; prop < teldef->multisegparam[lowsys]->n_properties; prop++)
	{
	  if(lowprops[prop] != teldef->multisegparam[lowsys]->properties[coeffrow][prop])
	    {
	      usethisrow = 0;
	    }
	  
	  if(!usethisrow)
	    break;
	}
      
      if(usethisrow)
	{
	  propsfound = 1;
	  break;
	}
    }

  if(!propsfound)
    return 1;

  /* The correct set of properties were found, so convert the
     coordinates with its transformation. */

  applyXform2dToContinuousCoords(teldef->coordsys[lowsys][coeffrow]->trans, 
				 highx, highy, lowx, lowy);


  return 0;
}


void printTelDef2(TELDEF2* teldef)
{
  long sys, seg;

  printf("\nTelDef2 structure contents:\n");
  printf("  filename: %s\n", teldef->filename);
  printf("  mission: %s\n", teldef->mission);
  printf("  instrument: %s\n", teldef->instrument);
  printf("  td_version: %f\n", teldef->td_version);
  printf("  n_coordsys: %ld\n", teldef->n_coordsys);
  printf("  ransysname: %s\n", teldef->ransysname);
  printf("  ransys: %d\n", teldef->ransys);
  printf("  ranscalesysname: %s\n", teldef->ranscalesysname);
  printf("  ranscalesys: %d\n", teldef->ranscalesys);

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      printf("  coordsysnames[%ld]: %s\n", sys, teldef->coordsysnames[sys]);
    }

  for(sys = 0; sys < teldef->n_coordsys - 1; sys++)
    {
      printf("  trtypes[%ld]: %s\n", sys, teldef->trtypes[sys]);
    }

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      printf("  n_segments[%ld]: %d\n", sys, teldef->n_segments[sys]);
    }

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      printf("  min_segment[%ld]: %d\n", sys, teldef->min_segment[sys]);
    }

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      if(sys < teldef->n_coordsys - 2)
	{
	  printf("  basicparam[%ld]:\n", sys);
	  printTrBasic(teldef->basicparam[sys]);
	}

      if(sys < teldef->n_coordsys - 1 && teldef->multisegparam[sys] != NULL)
	{
	  printf("  multisegparam[%ld]:\n", sys);
	  printTrMultiseg(teldef->multisegparam[sys]);
	}

      if(sys < teldef->n_coordsys - 1 && teldef->skyattparam[sys] != NULL)
	{
	  printf("  skyattparam[%ld]:\n", sys);
	  printTrSkyAtt(teldef->skyattparam[sys]);
	}

      if(sys < teldef->n_coordsys - 1 && teldef->rawtodetparam[sys] != NULL)
	{
	  printf("  rawtodetparam[%ld]:\n", sys);
	  printTrRawtodet(teldef->rawtodetparam[sys]);
	}

      for(seg = teldef->min_segment[sys]; seg < teldef->min_segment[sys] + teldef->n_segments[sys]; seg++)
	{
	  printf("  coordsys[%ld][%ld]:\n", sys, seg);
	  printCoordDef(teldef->coordsys[sys][seg]);
	}
    }

}



/***********************************************************************
*************************************************************************
* handle FITSIO errors while reading teldef files
* prints error messages and exits if there is an error
************************************************************************/
void checkFITSerrors(int status, char* doing, char* filename) 
{
#ifdef DEBUG
  if(!status) 
    fprintf(stderr,"FITSIO success while %s file %s:\n",doing,filename);
#endif

  if(!status) return;
  
  fprintf(stderr,"FITSIO error while %s file %s:\n",doing,filename);
  fits_report_error(stderr,status);
  
  exit(status);
}


void printCoordDef(COORDDEF* coord)
{
  printf("      first_pixel_x: %g\n", coord->first_pixel_x);
  printf("      first_pixel_y: %g\n", coord->first_pixel_y);
  printf("      npixels_x: %d\n", coord->npixels_x);
  printf("      npixels_y: %d\n", coord->npixels_y);
  printf("      last_pixel_x: %g\n", coord->last_pixel_x);
  printf("      last_pixel_y: %g\n", coord->last_pixel_y);
  printf("      min_x: %g\n", coord->min_x);
  printf("      max_x: %g\n", coord->max_x);
  printf("      min_y: %g\n", coord->min_y);
  printf("      max_y: %g\n", coord->max_y);
  printf("      center_x: %g\n", coord->center_x);
  printf("      center_y: %g\n", coord->center_y);
  printf("      scale_x: %g\n", coord->scale_x);
  printf("      scale_y: %g\n", coord->scale_y);
  printf("      scale_unit: %s\n", coord->scale_unit);
  printf("      name_x: %s\n", coord->name_x);
  printf("      name_y: %s\n", coord->name_y);
  printf("      name: %s\n", coord->name);
  printf("      trans:\n");
  printXform2d(coord->trans, stdout);
  printf("      inverse_trans:\n");
  printXform2d(coord->inverse_trans, stdout);
  printf("      trans_min_x: %g\n", coord->trans_min_x);
  printf("      trans_max_x: %g\n", coord->trans_max_x);
  printf("      trans_min_y: %g\n", coord->trans_min_y);
  printf("      trans_max_y: %g\n", coord->trans_max_y);
    
}

void destroyTelDef2(TELDEF2* teldef)
{
  int sys, seg;

#ifdef DEBUG
  printf("Freeing teldef\n");
#endif

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      for(seg = teldef->min_segment[sys]; 
	  seg < teldef->min_segment[sys] + teldef->n_segments[sys]; seg++)
	{
	  destroyCoordDef(teldef->coordsys[sys][seg]);
	}

      free(teldef->coordsys[sys]);
    }

  for(sys = 0; sys < teldef->n_coordsys; sys++)
    {
      free(teldef->coordsysnames[sys]);

      if(sys < teldef->n_coordsys - 1)
	{
	  free(teldef->trtypes[sys]);

	  if(teldef->basicparam[sys] != NULL)
	    {
	      destroyTrBasic(teldef->basicparam[sys]);
	    }
	  if(teldef->skyattparam[sys] != NULL)
	    {
	      destroyTrSkyAtt(teldef->skyattparam[sys]);
	    }
	  if(teldef->multisegparam[sys] != NULL)
	    {
	      destroyTrMultiseg(teldef->multisegparam[sys]);
	    }
	  if(teldef->rawtodetparam[sys] != NULL)
	    {
	      destroyTrRawtodet(teldef->rawtodetparam[sys]);
	    }
	}
    }

  free(teldef->basicparam);
  free(teldef->rawtodetparam);
  free(teldef->skyattparam);
  free(teldef->multisegparam);
  free(teldef->coordsysnames);
  free(teldef->trtypes);
  free(teldef->coordsys);
  free(teldef->n_segments);
  free(teldef->min_segment);

  free(teldef);

#ifdef DEBUG
  printf("Freed teldef\n");
#endif
}

