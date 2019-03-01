#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdlib.h>
#include <math.h>
#include <string.h>


#define TOOLSUB coordevt
/* att_fatal.h / headas_main() require that TOOLSUB be defined first */

/*#include "headas.h"*/
#include "coordfits2.h"
#include "param.h"
#include "info.h"
#include "earthvel.h"
#include "keywords.h"
#include "version.h"
#include "random.h"

#ifdef __cplusplus
}
#endif

#include <ahlog/ahlog.h>
#include <ahgen/ahgen.h>
#include <iostream>

/*#include "headas_main.c"
  #include "att_fatal.h" */

using namespace ahlog;
using namespace ahgen;
using namespace std;


#define DEBUG


void checkCoordinateColumns(fitsfile* fp, iteratorCol* fits_col, int n_cols)
{
  int iotype;
  int datatype;
  char colname[FLEN_VALUE];
  int col;
  int colnumber;
  int n_eventcol = 0;
  int status = 0;
  int errorcount = 0;
  char tempstring[1000];
  char tformat[100];

  fits_get_num_cols(fp, &n_eventcol, &status);

  if(status)
    {
      fits_report_error(stderr, status);
      AH_ERR << "Could not read number of columns in event extension of event file." << endl;
    }

  for(col = 0; col < n_cols; col++)
    {
      strcpy(colname, fits_iter_get_colname(&fits_col[col]));
      iotype = fits_iter_get_iotype(&fits_col[col]);
      datatype = fits_iter_get_datatype(&fits_col[col]);
      AH_DEBUG << "Searching for column " << colname << " #" << col << "." << endl;

      fits_get_colnum(fp, CASEINSEN, colname, &colnumber, &status);

      if(status == COL_NOT_FOUND && iotype == InputCol)
	{
	  AH_ERR << "Input column " << colname << " not found in event extension." << endl;
	  errorcount++;
	  status = 0;
	}
      else if(status == COL_NOT_FOUND && (iotype == OutputCol || iotype == InputOutputCol))
	{
	  status = 0;

	  if(datatype == TINT)
	    strcpy(tformat, "1I");
	  else
	    strcpy(tformat, "1D");

	  fits_get_num_cols(fp, &n_eventcol, &status);
  
	  fits_insert_col(fp, n_eventcol+1, colname, tformat, &status);
	  sprintf(tempstring, "Output column %s not found in event extension. Adding that column.", colname);
	  AH_INFO(LOW) << tempstring << endl;
	  if(status)
	    {
	      fits_report_error(stderr, status);
	      sprintf(tempstring, "Could not create output column %s in event extension.", colname);
	      AH_ERR << tempstring << endl;
	      errorcount++;
	    }
	  else
	    {
	      AH_INFO(LOW) << "Created output column " << colname << " in event extension." << endl;
	    }
	  status = 0;
	}
      else if(status)
	{
	  sprintf(tempstring, "Could not search for column %s in event extension.\n", colname);
	  AH_ERR << tempstring << endl;
	  fits_report_error(stderr, status);
	  errorcount++;
	}
      
    }

  if(errorcount > 0)
    {
      AH_ERR << "There were problems finding or creating the necessary columns in the EVENTS extension." << endl;
      exit(1);
    }

}

#ifdef __cplusplus
extern "C" {
#endif

int updateCoordinates(long total_rows, long offset, long first_row, long n_rows,
               int ncols, iteratorCol *fits_col,  void* void_info ) 
{
  INFO* info = (INFO*) void_info;
  PARAM* param = info->param;
  TELDEF2* teldef = info->teldef;
  /*  ATTFILE* att = info->att;*/

  int sys;
  int prop;
  int row;
  double percent_done = 0.;
  
  double* time = info->time;       /* time[row] */
  int** segs = info->segs;         /* segs[sys][row] */
  int** intx = info->intx;         /* intx[sys][row] */
  int** inty = info->inty;         /* inty[sys][row] */
  double** floatx = info->floatx;  /* floatx[sys][row] */
  double** floaty = info->floaty;  /* floaty[sys][row] */
  int*** props = info->props;      /* props[sys][prop][row] */
  int** rowprops;                  /* rowprops[sys][prop] = props for a single row */

  double x, y, prev_x, prev_y;
  int px_x, px_y;

  int startwithfloat = 0;
  int convertstatus = 0; /* 0 means the conversion succeeded, 1 means it failed. */
  /*  char colname[FLEN_VALUE];*/

  /* Get the column arrays from the column structures. */
  if(info->time_col != NULL)
    time = (double*) fits_iter_get_array(info->time_col);

  for(sys = param->startsys; sys <= param->stopsys; sys++)
    {
      if(sys < teldef->n_coordsys - 1 && info->seg_cols[sys] != NULL)
	segs[sys] = (int*) fits_iter_get_array(info->seg_cols[sys]);

      if(info->intx_cols[sys] != NULL)
	{
	  intx[sys] = (int*) fits_iter_get_array(info->intx_cols[sys]);
	  intx[sys][0] = param->null_value;
	}

      if(info->inty_cols[sys] != NULL)
	{
	  inty[sys] = (int*) fits_iter_get_array(info->inty_cols[sys]);
	  inty[sys][0] = param->null_value;
	}
      
      if(info->floatx_cols[sys] != NULL)
	{
	  floatx[sys] = (double*) fits_iter_get_array(info->floatx_cols[sys]);
	  floatx[sys][0] = DOUBLENULLVALUE;
	}

      if(info->floaty_cols[sys] != NULL)
	{
	  floaty[sys] = (double*) fits_iter_get_array(info->floaty_cols[sys]);
	  floaty[sys][0] = DOUBLENULLVALUE;
	}

      if(sys < teldef->n_coordsys - 1 && !strcasecmp(teldef->trtypes[sys], "MULTISEG"))
	{
	  for(prop = 0; prop < teldef->multisegparam[sys]->n_properties; prop++)
	    {
	      props[sys][prop] = (int*) fits_iter_get_array(info->prop_cols[sys][prop]);
	    }

	}

    }

  /* Allocate and rowprops for the MULTISEG segment properties. */

  rowprops = (int**) malloc((teldef->n_coordsys - 1) * sizeof(int*));
  for(sys = 0; sys < teldef->n_coordsys - 1; sys++)
    {
      if(strcasecmp(teldef->trtypes[sys], "MULTISEG")) /* not a MULTISEG transformation */
	continue;
      
      rowprops[sys] = (int*) malloc(teldef->multisegparam[sys]->n_properties * sizeof(int));
    }
			    

  
  /* Work on each row and do the coordinate conversions. */

  for(row = 1; row <= n_rows; row++)
    {
      /* Get initial coordinates for the startsys system. For each of
	 the x and y coordinates, look first for a floating point
	 value to start with.  If there isn't one, use the integer
	 coordinate.  If that also does not exist, then this must be a
	 pixel map RAW system, where each segment has only one pixel,
	 and that value is given by first_pixel_x or _y. */

      prev_x = 0;
      prev_y = 0;
      
      if(startwithfloat && info->floatx_cols[param->startsys] != NULL)
	prev_x = floatx[param->startsys][row];
      else if(info->intx_cols[param->startsys] != NULL)
	prev_x = (double) intx[param->startsys][row];
      else
	prev_x = teldef->coordsys[param->startsys][teldef->min_segment[param->startsys]]->first_pixel_x;
      if(startwithfloat && info->floaty_cols[param->startsys] != NULL)
	prev_y = floaty[param->startsys][row];
      else if(info->inty_cols[param->startsys] != NULL)
	prev_y = (double) inty[param->startsys][row];
      else
	prev_y = teldef->coordsys[param->startsys][teldef->min_segment[param->startsys]]->first_pixel_y;
      

      for(sys = param->startsys; sys < teldef->n_coordsys - 1; sys++)
	{
	  /* Don't change any coordinate column values if this system
	     is outside the range of conversions and blank_col is
	     false. */

	  if(!param->blank_col && sys >= param->stopsys)
	    continue;

	  /* If this system is outside the range of conversions and
	     blank_col is true, then fill in the coordinate columns
	     with null values. */

	  if(param->blank_col && sys >= param->stopsys)
	    {
	      if(param->includeintcols)
		{
		  intx[sys + 1][row] = param->null_value;
		  inty[sys + 1][row] = param->null_value;
		}
	      if(param->includefloatcols)
		{
		  floatx[sys + 1][row] = DOUBLENULLVALUE;
		  floaty[sys + 1][row] = DOUBLENULLVALUE;
		}
	      continue;
	    }
	  
	  /* If this system is within the range of conversions, then do the conversions. */

	  convertstatus = 0;


	  if(prev_x == DOUBLENULLVALUE || prev_y == DOUBLENULLVALUE 
	     || (param->blank_col && sys >= param->stopsys) )
	    {
	      x = DOUBLENULLVALUE;
	      y = DOUBLENULLVALUE;
	      convertstatus = 1;
	    }
	  
	  
	  if(!convertstatus && !strcasecmp(teldef->trtypes[sys], "RAWTODET"))
	    {
	      convertstatus = convertToHigherCoordRawtodet(teldef, prev_x, prev_y, &x, &y, sys, segs[sys][row]);
	    }
	  else if(!convertstatus && !strcasecmp(teldef->trtypes[sys], "BASIC"))
	    {
	      convertstatus = convertToHigherCoordBasic(teldef, prev_x, prev_y, &x, &y, sys);
	    }
	  else if(!convertstatus && !strcasecmp(teldef->trtypes[sys], "MULTISEG"))
	    {
	      for(prop = 0; prop < teldef->multisegparam[sys]->n_properties; prop++)
		{
		  rowprops[sys][prop] = props[sys][prop][row];
		}
	      
	      convertstatus = convertToHigherCoordMultiseg(teldef, prev_x, prev_y, &x, &y, sys, rowprops[sys]);
	    }
	  
	  

	  if(convertstatus)
	    {
	      x = DOUBLENULLVALUE;
	      y = DOUBLENULLVALUE;
	      px_x = param->null_value;
	      px_y = param->null_value;
	    }
	  else
	    {
	      px_x = (int) (x + 0.5);
	      px_y = (int) (y + 0.5);
	    }


	  if(param->includeintcols)
	    {
	      intx[sys + 1][row] = px_x;
	      inty[sys + 1][row] = px_y;
	    }
	  if(param->includefloatcols)
	    {
	      floatx[sys + 1][row] = x;
	      floaty[sys + 1][row] = y;
	    }

	  prev_x = x;
	  prev_y = y;

	}

    }

  
  /* Free rowprops. */

  for(sys = 0; sys < teldef->n_coordsys - 1; sys++)
    {
      if(strcasecmp(teldef->trtypes[sys], "MULTISEG")) /* not a MULTISEG transformation */
	continue;
      
      free(rowprops[sys]);
    }
  free(rowprops);


  percent_done= 100* ((double)(first_row-offset+n_rows-1)/ 
			     (double)(total_rows-offset)      );
  AH_INFO(LOW) << percent_done << "% done." << endl;

  return 0;
}


#ifdef __cplusplus
} /* end extern */
#endif


INFO* initialize(fitsfile* fp, PARAM* param, INFO* info, int& n_cols, iteratorCol* fits_col, int& status)
{

  int sys;
  string tempstr = "";

  info = createInfo(param, fp);
    info->param->includeintcols = 1;
    info->param->includefloatcols = 0;
    
    /* Check startsys and stopsys parameters and get system numbers
       matching the system names. */

    if(!strcasecmp(param->startsysname, "LOWEST"))
    param->startsys = 0;
    else
      param->startsys = getCoordSystemNumberFromName(info->teldef, param->startsysname);
    
    if(!strcasecmp(param->stopsysname, "HIGHEST"))
      param->stopsys = info->teldef->n_coordsys - 1;
    else
      param->stopsys = getCoordSystemNumberFromName(info->teldef, param->stopsysname);
    
    if(param->startsys < 0)
      {
	tempstr = "startsys '" + string(param->startsysname) + "' not recognized.";
	AH_ERR << tempstr << endl;
      }
    if(param->stopsys < 0)
      {
	tempstr = "stopsys '" + string(param->stopsysname) + "' not recognized.";
	AH_ERR << tempstr << endl;
      }

    if(param->startsys < 0 || param->stopsys < 0)
      {
	tempstr = "Valid coordinate system names for " + string(info->teldef->mission) + " " + string(info->teldef->instrument) + ": LOWEST HIGHEST";
	for(sys = 0; sys < info->teldef->n_coordsys; sys++)
	  {
	    tempstr += " " + string(info->teldef->coordsysnames[sys]);
	  }
	AH_ERR << tempstr << endl;
	exit(1);
      }
    if(param->startsys >= param->stopsys)
      {
	AH_ERR << "Cannot convert from " << param->startsysname << " to " << param->stopsysname << " coordinate system." << endl;
	exit(1);
      }


#ifdef DEBUG
    /*    printTelDef2(info->teldef);*/
#endif /* DEBUG */

    return info;
}


void doWork(INFO* info, int n_cols, iteratorCol fits_col[], int& status)
{
  AH_DEBUG << "About to iterate." << endl;
  fits_iterate_data(n_cols, fits_col, 0L, 0L, updateCoordinates, info, &status);
  AH_DEBUG << "Finished iterating. status=" << status << endl;

  if(status)
    {
      fits_report_error(stderr, status);
    }
}


void finalize(INFO* info, iteratorCol* fits_col, fitsfile* fp, int& status)
{
  free(fits_col);
  destroyInfo(info);
  fits_close_file(fp, &status);
}

int main(int argc, char** argv)
{
  char toolname[] = "coordevt";
  fitsfile* fp;
  PARAM* param;
  INFO* info;

  int n_cols;
  iteratorCol* fits_col = NULL;
  int status=0;

  int out;
  char outfile[FLEN_FILENAME];
  char cmd[10000];

  string logfile = "coordevt.log";


  try {
    
    
    startUp(argc, argv);
    
    AH_OUT << "Starting." << endl;

    /****************************
     * read the input parameters *
     ****************************/
    param = getPar();
    if (!param) {
      AH_ERR << "Unable to load parameters." << endl;
      exit(1);
    }
    
    /* Copy input FITS file to make the output file. */
    if(!strcmp(param->out_file, param->in_file))
      sprintf(outfile, "tempevt%d.fits", (int)(1000000*rand()));
    else
      strcpy(outfile, param->out_file);
    
    sprintf(cmd, "ftcopy infile=%s outfile=%s clobber=yes", param->in_file, outfile);
    out = system(cmd);
    /*printf("ftcopy returns %d\n", out);*/
    if(out)
      {
	AH_ERR << "ftcopy error in copying " << param->in_file << " to " << outfile << "." << endl;
	exit(out);
      }
    
    /* Open output FITS file and move to event extension. */
    
    fits_open_file(&fp,param->out_file,READWRITE,&status);
    fits_movnam_hdu(fp,BINARY_TBL,param->event_extension,0/* any version*/,&status);
    if(status) {
      AH_ERR << "Error while opening event file " << param->in_file << " and event extension " << param->event_extension << "." << endl;
      fits_report_error(stderr,status);
      exit(status);
    }
    
    info = initialize(fp, param, info, n_cols, fits_col, status);

    fits_col = setIteratorColumns(info, fp, &n_cols);

    /* Check that needed columns exist.  Create any output columns that
       do not yet exist. */
    
    checkCoordinateColumns(fp, fits_col, n_cols);

    
    doWork(info, n_cols, fits_col, status);

    finalize(info, fits_col, fp, status);
  } //end try
  catch (const exception& x)
    {
      setStatus(1);
      AH_ERR << toolname << "-exception: caught error: " << x.what() << endl;
    }
  
  AH_OUT << "Finished." << endl;

  shutDown();

  return getStatus();
}


