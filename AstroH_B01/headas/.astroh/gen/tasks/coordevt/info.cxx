#ifdef __cplusplus
extern "C"
{
#endif

#include "info.h"
#define new i_cannot_believe_that_new_is_used_as_a_variable_name
#include "headas_gti.h"
#undef new

#ifdef __cplusplus
}
#endif

#include <stdlib.h>
#include <string.h>

#include "headas.h"

#include "fitsio.h"
#include "longnam.h"
#include "earthvel.h"

#include "ahlog/ahlog.h"

using namespace std;
using namespace ahlog;

/******************************************************************************
*******************************************************************************
* set up misc info structure to hand to the iterator 
* the fitsfile pointer argument is for the event file
******************************************************************************/
INFO* createInfo(PARAM* param, fitsfile* fp) {

INFO* info;

double mjd;

int status=0;
char comment[FLEN_COMMENT]="";
char mission[FLEN_VALUE]="UNKNOWN";
char instrument[FLEN_VALUE]="UNKNOWN";

/********************************
* allocate memory for structure *
********************************/
info = (INFO*) calloc(1, sizeof(INFO));

/***********************************
* remember the parameter structure *
***********************************/
info->param=param;

/**********************************************
* read mission and instrument from event file *
**********************************************/
fits_read_key_str(fp,"TELESCOP",mission   ,comment,&status);
fits_read_key_str(fp,"INSTRUME",instrument,comment,&status);
if(status) {
  AH_ERR << "Cannot read TELESCOP and INSTRUME keywords from " << param->in_file << endl;
    status=0;
}

/******************************************************
* open attitude file and make some consistency checks *
******************************************************/
if(param->do_sky) {
    info->att=openAttFile(param->att_file);
    resetAttFileExtrapolationLimits(info->att,param->att_time_margin);

    if(strncmp(mission,info->att->mission, FLEN_VALUE)) {
	AH_ERR << "Keyword TELESCOP=" << mission << " in " << param->in_file << " but TELESCOP=" << info->att->mission << " in " << info->att->name << "." << endl;
    }

    setAttFileInterpolation(info->att, param->att_interpolation);
}

/****************************************************
* open teldef file and make some consistency checks *
****************************************************/
 info->teldef=readTelDef2(param->teldef_file);

 info->seg_cols = (iteratorCol**) calloc(info->teldef->n_coordsys, sizeof(iteratorCol*));

if(strncmp(mission, info->teldef->mission, FLEN_VALUE)) {
    AH_ERR << "Keyword TELESCOP=" << mission << " in " << param->in_file << " but TELESCOP=" << info->teldef->mission << " in " << info->teldef->filename << "." << endl;
}

if(strncmp(instrument, info->teldef->instrument, FLEN_VALUE)) {
    AH_ERR << "Keyword INSTRUME=" << instrument << " in " << param->in_file << " but INSTRUME=" << info->teldef->instrument << " in " << info->teldef->filename << "." << endl;
}


/*******************************************************
* allocate storage for the current pointing quaternion *
*******************************************************/
info->q=allocateQuat();

/********************************************
* set the tangent plane coordinate system  
* corresponding to the nominal pointing
********************************************/
/*setSkyCoordCenterInTeldef(info->teldef, param->ra,param->dec);*/



if(param->do_aberration) {
    /********************************************
    * set up for applying aberration correction *
    ********************************************/
    if(param->follow_sun) {
        /**************************************************************
        * we will be recalculating the earth's velocity for
        * each event. All we need to do here is read the
        * MJD reference time used to calculate MJD from mission time
        **************************************************************/
        info->mjdref = HDget_frac_time(fp, "MJDREF", 0, 0, &status);
        /*
        fits_read_key_dbl(fp,"MJDREF",&(info->mjdref),comment,&status);
        */
        if(status) {
            fprintf(stderr,"Can't read MJDREF from %s\n",param->in_file);
            fits_report_error(stderr,status);
            exit(status);
        }

    } else {
        /******************************************************
        * we won't be recalculating the earth's velocity for
        * each event, so get the earth's velocity at MJD-OBS
        * and use that throughout the observation 
        ******************************************************/
        fits_read_key_dbl(fp,"MJD-OBS",&mjd,comment,&status);
        if(status) {
            fprintf(stderr,"Can't read MJD-OBS from %s\n",param->in_file);
            exit(status);
        }
        info->v = compat_earthvel_at_mjd(info->vhat, mjd);

    } /* end if we will not be recalculating sun posistion for each event */
} else {
    /***********************************************
    * no aberation - indicate this by setting v=0. 
    * note in this case follow_sun will have been  
    * set to "NO" in readParam
    ***********************************************/
    info->v=0.;
}



return info;

} /* end of createInfo function */


/* Set up column structures for the CFITSIO iterator. */

iteratorCol* setIteratorColumns(INFO* info, fitsfile* fp, int* n_cols)
{
  iteratorCol* fits_col;
  iteratorCol* col;
  int fits_col_number = 0;
  PARAM* param = info->param;
  TELDEF2* teldef = info->teldef;
  char tempstring[1000] = "";
  char floatcolsuffix[FLEN_VALUE] = "_FLOAT";
  int iotype;
  int iotype_float;
  int includeintcols = info->param->includeintcols;
  int includefloatcols = info->param->includefloatcols;
  
  int sys;
  int prop;
  
  int min_sys = info->param->startsys; /* temp */
  int max_sys = 0*info->param->stopsys + teldef->n_coordsys - 1; /* temp */
  
  *n_cols = 0;

  if(!includeintcols && !includefloatcols)
    {
      AH_ERR << "Both integer and floating-point coordinate columns are set to be suppressed. Quitting." << endl;
      exit(1);
    }
  
  /* Get count of columns. Begin by adding a time column if
     necessary. */
  
  if(param->do_sky) /* need to add similar condition for d-att files. */
    (*n_cols)++; /* TIME */
  
  /* Add columns for each system based on the transformation type and
     its details. */

  for(sys = min_sys; sys <= max_sys; sys++)
    {
      if(sys < teldef->n_coordsys - 1)
	{
	  if(!strcasecmp(teldef->trtypes[sys], "RAWTODET"))
	    {
	      if(teldef->rawtodetparam[sys]->rawmethod != RM_CORNER_LIST)
		{
		  /* RAWTODET methods besides the pixel corner map
		     have two coordinate columns. */
		  
		  *n_cols += 2 * (includeintcols + includefloatcols);  
		}
	      
	      if(teldef->n_segments[sys] > 1)
		{
		  /* If there are multiple segments, then there is a
		     segment column in addition to the
		     coordinate columns. */
		  
		  (*n_cols)++;
		}
	    }
	  else if(!strcasecmp(teldef->trtypes[sys], "MULTISEG"))
	    {
	      /* The MULTISEG transformation needs two coordinate
		 columns and a number of property columns (like
		 segment, window mode, etc). */
	      
	      *n_cols += teldef->multisegparam[sys]->n_properties + 
		2 * (includeintcols + includefloatcols);
	    }
	  else /* Other transformations */
	    {
	      /* Other transformation types have only two coordinate
		 columns without any other columns like one for
		 segments. */
	      
	      *n_cols += 2 * (includeintcols + includefloatcols);
	    }

	}
      else /* highest coordinate system */
	{
	  /* The last coordinate system always results in two
	     coordinate columns without any extra columns. */

	  *n_cols += 2 * (includeintcols + includefloatcols); 
	}
      
    } /* end for(sys) */
  

  /* Allocate space for the iterator columns and set col to first column. */
  
  fits_col = (iteratorCol*) malloc(sizeof(iteratorCol) * (*n_cols));
  col = fits_col;

  info->seg_cols = (iteratorCol**) malloc(sizeof(iteratorCol*) * teldef->n_coordsys);
  info->intx_cols = (iteratorCol**) malloc(sizeof(iteratorCol*) * teldef->n_coordsys);
  info->inty_cols = (iteratorCol**) malloc(sizeof(iteratorCol*) * teldef->n_coordsys);
  info->floatx_cols = (iteratorCol**) malloc(sizeof(iteratorCol*) * teldef->n_coordsys);
  info->floaty_cols = (iteratorCol**) malloc(sizeof(iteratorCol*) * teldef->n_coordsys);
  info->prop_cols = (iteratorCol***) malloc(sizeof(iteratorCol**) * teldef->n_coordsys);

  /* Allocate space for the coordinate, segment, and property values. */


  info->intx = (int**) malloc(sizeof(int*) * teldef->n_coordsys);
  info->inty = (int**) malloc(sizeof(int*) * teldef->n_coordsys);
  info->floatx = (double**) malloc(sizeof(double*) * teldef->n_coordsys);
  info->floaty = (double**) malloc(sizeof(double*) * teldef->n_coordsys);
  info->segs = (int**) malloc(sizeof(int*) * teldef->n_coordsys);
  info->props = (int***) malloc(sizeof(int**) * teldef->n_coordsys);
  
  for(sys = min_sys; sys <= max_sys && sys < teldef->n_coordsys - 1; sys++)
    {
      /* Set iterator column pointers to zero initially. */

      info->seg_cols[sys] = NULL;
      info->intx_cols[sys] = NULL;
      info->inty_cols[sys] = NULL;
      info->floatx_cols[sys] = NULL;
      info->floaty_cols[sys] = NULL;
      
      /* Now set the needed iterator columns to the allocated fits_col
	 columns. */

      /*if(!strcasecmp(teldef->trtypes[sys], "MULTISEG"))
	{
	  info->prop_cols[sys] = (iteratorCol**) malloc(sizeof(iteratorCol*) * teldef->multisegparam[sys]->n_properties);
	  }*/
    }

  /* Set time column if needed. */

  if(param->do_sky) /* need to add similar condition for d-att files. */
    {
      fits_iter_set_by_name(col, fp, param->time_col_name, TDOUBLE, InputCol);
      /*printf("Set column #%d %s.\n", fits_col_number, param->time_col_name);*/
      info->time_col = col;
      col++;
      fits_col_number++;
    }
  else
    info->time_col = NULL;

  /* Set segment, property, and coordinate columns where needed. */

  for(sys = min_sys; sys <= max_sys; sys++)
    {
      iotype = (sys == min_sys ? InputCol : OutputCol);
      iotype_float = (sys == min_sys ? InputOutputCol : OutputCol);
      
      if(sys < teldef->n_coordsys - 1) 
	{
	  if(!strcasecmp(teldef->trtypes[sys], "RAWTODET"))
	    {
	      if(teldef->rawtodetparam[sys]->rawmethod != RM_CORNER_LIST)
		{
		  /* Set the first coordinate column. */
		  
		  if(includeintcols)
		    {
		      fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_x, TINT, iotype);
		      /*		      printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_x);*/
		      info->intx_cols[sys] = col;
		      col++;
		      fits_col_number++;
		      
		    }
		  
		  if(includefloatcols)
		    {
		      sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_x, floatcolsuffix);
		      fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
		      /*		      printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
		      info->floatx_cols[sys] = col;
		      col++;
		      fits_col_number++;
		      
		    }
		  
		  
		  /* Set the second coordinate column. */
		  if(includeintcols)
		    {
		      fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_y, TINT, iotype);
		      /*		      printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_y);*/
		      info->inty_cols[sys] = col;
		      col++;
		      fits_col_number++;

		    }
		  
		  if(includefloatcols)
		    {
		      sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_y, floatcolsuffix);
		      fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
		      /*		      printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
		      info->floaty_cols[sys] = col;
		      col++;      
		      fits_col_number++;

		    }
		}
	      
	      
	      if(teldef->n_segments[sys] > 1)
		{
		  /* Set the segment column. */
	      
		  fits_iter_set_by_name(col, fp, teldef->rawtodetparam[sys]->segcolname, TINT, InputCol);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, teldef->rawtodetparam[sys]->segcolname);*/
		  info->seg_cols[sys] = col;
		  col++;     
		  fits_col_number++;

		}
	    } /* end if RAWTODET */
	  
	  else if(!strcasecmp(teldef->trtypes[sys], "MULTISEG"))
	    {
	      info->prop_cols[sys] = (iteratorCol**) malloc(sizeof(iteratorCol*) * teldef->multisegparam[sys]->n_properties);
	      info->props[sys] = (int**) malloc(sizeof(int*) * teldef->multisegparam[sys]->n_properties);

	      /* Set the coordinate columns. */
	      if(includeintcols)
		{
		  fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_x, TINT, iotype);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_x);*/
		  info->intx_cols[sys] = col;
		  col++;
		  fits_col_number++;
      

		  fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_y, TINT, iotype);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_y);*/
		  info->inty_cols[sys] = col;
		  col++;
		  fits_col_number++;

		}
	      
	      if(includefloatcols)
		{
		  sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_x, floatcolsuffix);
		  fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
		  info->floatx_cols[sys] = col;
		  col++;
		  fits_col_number++;
      

		  sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_y, floatcolsuffix);
		  fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
		  info->floaty_cols[sys] = col;
		  col++;
		  fits_col_number++;

		}

	      /* Set property columns. */

	      for(prop = 0; prop < teldef->multisegparam[sys]->n_properties; prop++)
		{
		  fits_iter_set_by_name(col, fp, teldef->multisegparam[sys]->propertynames[prop], TINT, InputCol);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, teldef->multisegparam[sys]->propertynames[prop]);*/
		  info->prop_cols[sys][prop] = col;
		  col++;
		  fits_col_number++;

		}
	      
	    } /* end if MULTISEG */
	  else /* other transformations */
	    {
	      /* Set the coordinate columns. */
	      if(includeintcols)
		{
		  fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_x, TINT, iotype);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_x);*/
		  info->intx_cols[sys] = col;
		  col++;
		  fits_col_number++;


		  fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_y, TINT, iotype);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_y);*/
		  info->inty_cols[sys] = col;
		  col++;
		  fits_col_number++;

		}
	      
	      if(includefloatcols)
		{
		  sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_x, floatcolsuffix);
		  fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
		  info->floatx_cols[sys] = col;
		  col++;      
		  fits_col_number++;

		  
		  sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_y, floatcolsuffix);
		  fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
		  /*		  printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
		  info->floaty_cols[sys] = col;
		  col++;
		  fits_col_number++;

		}
	    } /* end else (other transformations) */
	} /* end if not last system */
      else /* last system */
	{
	  /* Set x and y coordinate columns. */
	  
	  if(includeintcols)
	    {
	      fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_x, TINT, iotype);
	      /*	      printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_x);*/
	      info->intx_cols[sys] = col;
	      col++;
	      fits_col_number++;

	      
	      fits_iter_set_by_name(col, fp, teldef->coordsys[sys][0]->name_y, TINT, iotype);
	      /*	      printf("Set column #%d %s.\n", fits_col_number, teldef->coordsys[sys][0]->name_y);*/
	      info->inty_cols[sys] = col;
	      col++;
	      fits_col_number++;

	    }
	  
	  if(includefloatcols)
	    {
	      sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_x, floatcolsuffix);
	      fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
	      /*	      printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
	      info->floatx_cols[sys] = col;
	      col++;
	      fits_col_number++;
      
      
	      sprintf(tempstring, "%s%s", teldef->coordsys[sys][0]->name_y, floatcolsuffix);
	      fits_iter_set_by_name(col, fp, tempstring, TDOUBLE, iotype_float);
	      /*	      printf("Set column #%d %s.\n", fits_col_number, tempstring);*/
	      info->floaty_cols[sys] = col;
	      col++;
	      fits_col_number++;
      
	    }
	} /* end else last system */
    } /* end for sys loop */


  return fits_col;
}




/******************************************************************************
*******************************************************************************
* free all the memory associated with an INFO structure
* Note this even destroys the associated PARAM structure even though
* that structure was created separately.
******************************************************************************/
void destroyInfo(INFO* info) {

  int sys; 

  if(info->param->do_sky) closeAttFile(info->att);
  
  free(info->intx);
  free(info->inty);
  free(info->floatx);
  free(info->floaty);
  free(info->segs);

  for(sys = info->param->startsys; sys <= info->param->stopsys && sys < info->teldef->n_coordsys - 1; sys++)
    {
      if(!strcasecmp(info->teldef->trtypes[sys], "MULTISEG"))
	{
	  free(info->prop_cols[sys]);
	  free(info->props[sys]);
	}
    }

  free(info->props);
  free(info->seg_cols);
  free(info->intx_cols);
  free(info->inty_cols);
  free(info->floatx_cols);
  free(info->floaty_cols);
  free(info->prop_cols);

  destroyTelDef2(info->teldef);
  destroyQuat(info->q);
  destroyParam(info->param); 
  
  free(info); 
}
