#include <math.h>
#include <string.h>
#include "fitsio.h"
#include "longnam.h"
#include "teldef2.h"

TR_RAWTODET* readTrRawtodet(fitsfile* fp, 
		      char* lowcoordsysname, char* highcoordsysname,
		      int low_sys, char* filename)
{
  
  TR_RAWTODET* rawtodetparam = (TR_RAWTODET*) malloc(sizeof(TR_RAWTODET));
  int status = 0;
  int hdutype;
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  

  strcpy(rawtodetparam->lowcoordsysname, lowcoordsysname);
  strcpy(rawtodetparam->highcoordsysname, highcoordsysname);
  rawtodetparam->low_sys = low_sys;
  rawtodetparam->high_sys = low_sys + 1;
  rawtodetparam->min_seg = 0; /* Default value */
  rawtodetparam->n_segs = 1; /* Default value */

  /* Move to primary extension. */

  fits_movabs_hdu(fp, 1, &hdutype, &status);
  checkFITSerrors(status, "moving to primary extension of", filename); 

  
  /* Need to add ASCA routine. */

  /* Read INn_[XY]CEN keywords for center of internal coordinate system. */
  
  sprintf(keyname, "IN%d_XCEN", low_sys);
  fits_read_key_dbl(fp, keyname, &rawtodetparam->int_cen_x, NULL, &status);
  if(status == KEY_NO_EXIST)
    {
      status = 0;

      sprintf(keyname, "INT_XCEN");
      fits_read_key_dbl(fp, keyname, &rawtodetparam->int_cen_x, NULL, &status);
      if(status == KEY_NO_EXIST)
	{
	  rawtodetparam->int_cen_x = 0.;
	  rawtodetparam->int_cen_y = 0.;
	  rawtodetparam->int_cen_found = 0;
	}
      else
	{
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  
	  sprintf(keyname, "INT_YCEN");
	  fits_read_key_dbl(fp, keyname, &rawtodetparam->int_cen_y, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->int_cen_found = 1;
	}
    }
  else
    {
      sprintf(tempstring, "reading %s from", keyname);
      checkFITSerrors(status, tempstring, filename);

      sprintf(keyname, "IN%d_YCEN", low_sys);
      fits_read_key_dbl(fp, keyname, &rawtodetparam->int_cen_y, NULL, &status);
      sprintf(tempstring, "reading %s from", keyname);
      checkFITSerrors(status, tempstring, filename);
      rawtodetparam->int_cen_found = 1;
    }

  /* Read segment column name from xxx_SCOL keyword. */

  sprintf(keyname, "%s_SCOL", rawtodetparam->lowcoordsysname);
  fits_read_key_str(fp, keyname, rawtodetparam->segcolname, NULL, &status);

  if(status == KEY_NO_EXIST)
    {
      /* If xxx_SCOL keyword isn't found, try SEG_COL. */
      status = 0;
      strcpy(keyname, "SEG_COL");
      fits_read_key_str(fp, keyname, rawtodetparam->segcolname, NULL, &status);
      sprintf(tempstring, "reading %s from primary extension", keyname);
      checkFITSerrors(status, tempstring, filename);
    }
  else {
    sprintf(tempstring, "reading %s from primary extension", keyname);
    checkFITSerrors(status, tempstring, filename);
  }

  /* Determine transformation method. */

  rawtodetparam->rawmethod = RM_UNKNOWN;

  if(rawtodetparam->rawmethod == RM_UNKNOWN)
    rawtodetparam->rawmethod = readPixelCornerMapInTelDef2(fp, rawtodetparam, filename);
  if(rawtodetparam->rawmethod == RM_UNKNOWN)
    rawtodetparam->rawmethod = readLinearCoeffInTelDef2(fp, rawtodetparam, filename);


  return rawtodetparam;
}

enum RM_TYPE readPixelCornerMapInTelDef2(fitsfile* fp, TR_RAWTODET* rawtodetparam, char* filename)
{
  enum RM_TYPE rawmethod = RM_UNKNOWN;
  int orig_hdu;
  int hdutype;
  int anynul;
  int status = 0;

  long row;
  long n_rows;
  char tempstring[1000];

  char xcolname[] = "PIXELX";
  char ycolname[] = "PIXELY";
  char extname[] = "PIXEL_MAP";
  int segcolnumber, xcolnumber, ycolnumber;
  
  fits_get_hdu_num(fp, &orig_hdu);
  
  /* Try to find PIXEL_MAP extension. */

  fits_movnam_hdu(fp, ANY_HDU, extname, 0/* any version*/, &status);

  if(status==BAD_HDU_NUM) 
    return (RM_UNKNOWN);
  sprintf(tempstring, "finding %s extension in", extname);
  checkFITSerrors(status, tempstring, filename);

  /* Since PIXEL_MAP extension is found, the rawmethod is RM_CORNER_LIST. */

  rawmethod = RM_CORNER_LIST;

  /* Read the number of PIXEL_MAP table rows.  This value is the
   * number of segments. */

  fits_read_key_lng(fp,"NAXIS2", &n_rows, NULL, &status);
  sprintf(tempstring, "reading NAXIS2 from %s extension in", extname);
  checkFITSerrors(status, tempstring, filename);

  rawtodetparam->n_segs = n_rows;

  /* Allocate the arrays for storing the table.*/

  rawtodetparam->corner_seg = calloc(n_rows, sizeof(double));
  rawtodetparam->corner_x = calloc(n_rows, sizeof(double*));
  rawtodetparam->corner_y = calloc(n_rows, sizeof(double*));

  for(row = 0; row < n_rows; row++)
    {
      rawtodetparam->corner_x[row] = calloc(4, sizeof(double*));
      rawtodetparam->corner_y[row] = calloc(4, sizeof(double*));
    }


  /* Return to primary extension. */
  
  fits_movabs_hdu(fp,1,&hdutype,&status);
  checkFITSerrors(status,"moving to primary extension in", filename);


  /* Return to PIXEL_MAP extension. */

  fits_movnam_hdu(fp, ANY_HDU, extname, 0/* any version*/, &status);

  if(status==BAD_HDU_NUM) 
    return (RM_UNKNOWN);
  sprintf(tempstring, "finding %s extension in", extname);
  checkFITSerrors(status, tempstring, filename);

  /* Find column names in PIXEL_MAP table. */

  fits_get_colnum(fp, CASESEN, rawtodetparam->segcolname, &segcolnumber, &status);
  sprintf(tempstring, "finding %s column in %s extension", rawtodetparam->segcolname, extname);
  checkFITSerrors(status, tempstring, filename);

  fits_get_colnum(fp, CASESEN, xcolname, &xcolnumber, &status);
  sprintf(tempstring, "finding %s column in %s extension", xcolname, extname);
  checkFITSerrors(status, tempstring, filename);

  fits_get_colnum(fp, CASESEN, ycolname, &ycolnumber, &status);
  sprintf(tempstring, "finding %s column in %s extension", ycolname, extname);
  checkFITSerrors(status, tempstring, filename);

  /* Read the PIXEL_MAP table data. */

  for(row = 0; row < n_rows; row++)
    {
      fits_read_col_int(fp, segcolnumber, row + 1, 1l, 1l, 0  , &rawtodetparam->corner_seg[row],    &anynul, &status);
      fits_read_col_dbl(fp,   xcolnumber, row + 1, 1l, 4l, 0.0, rawtodetparam->corner_x[row], &anynul, &status);
      fits_read_col_dbl(fp,   ycolnumber, row + 1, 1l, 4l, 0.0, rawtodetparam->corner_y[row], &anynul, &status);

    }

  sprintf(tempstring, "reading %s, %s, %s columns in %s extension", rawtodetparam->segcolname, xcolname, ycolname, extname);
  checkFITSerrors(status, tempstring, filename);



  return rawmethod;
}


enum RM_TYPE readLinearCoeffInTelDef2(fitsfile* fp, TR_RAWTODET* rawtodetparam, char* filename)
{
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  double keyvalue = 0.0;
  int status = 0;
  int coeff_found = 0;
  int seg = 0;

  /* Look first for COE_[XY]_[ABC] keywords.  If found, there is only one segment. */

  fits_read_key_dbl(fp, "COE_X_A", &keyvalue, NULL, &status);
  if(status != KEY_NO_EXIST)
    {
      /* Since COE_X_A was found, there is only one segment. */
      rawtodetparam->n_segs = 1;
      rawtodetparam->min_seg = 0;
      
      coeff_found = 1;
      checkFITSerrors(status,"reading COE_X_A from", filename);

      rawtodetparam->coeff_x_a[0] = keyvalue;

      /* Read the remaining keywords. */

      fits_read_key_dbl(fp, "COE_X_B", &keyvalue, NULL, &status);
      checkFITSerrors(status,"reading COE_X_B from", filename);
      rawtodetparam->coeff_x_b[0] = keyvalue;

      fits_read_key_dbl(fp, "COE_X_C", &keyvalue, NULL, &status);
      checkFITSerrors(status,"reading COE_X_C from", filename);
      rawtodetparam->coeff_x_c[0] = keyvalue;

      fits_read_key_dbl(fp, "COE_Y_A", &keyvalue, NULL, &status);
      checkFITSerrors(status,"reading COE_Y_A from", filename);
      rawtodetparam->coeff_y_a[0] = keyvalue;

      fits_read_key_dbl(fp, "COE_Y_B", &keyvalue, NULL, &status);
      checkFITSerrors(status,"reading COE_Y_B from", filename);
      rawtodetparam->coeff_y_b[0] = keyvalue;

      fits_read_key_dbl(fp, "COE_Y_C", &keyvalue, NULL, &status);
      checkFITSerrors(status,"reading COE_Y_C from", filename);
      rawtodetparam->coeff_y_c[0] = keyvalue;

      return RM_LINEAR_COEFF;
    }


  /* The COE_X_A keyword was not found.  Look next for
   * COc_[XY]n_[ABC] keywords.  Here c = coord sys number of
   * lower-level system, and n = segment number. Search
   * single-digit n values. */
  
  status = 0;
  
  for(seg = 0; seg < 10; seg++)
    {
      sprintf(keyname, "CO%d_X%d_A", rawtodetparam->low_sys, seg);
      fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
      if(status == KEY_NO_EXIST)
	{
	  status = 0;

	  if(coeff_found == 1)
	    {
	      rawtodetparam->n_segs = seg - rawtodetparam->min_seg;
	      break;
	    }
	  continue;
	}
      else
	{
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_x_a[seg] = keyvalue;
	  if(!coeff_found)
	    {
	      rawtodetparam->min_seg = seg;
	    }
	  coeff_found = 1;
	  
	  
	  sprintf(keyname, "CO%d_X%d_B", rawtodetparam->low_sys, seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_x_b[seg] = keyvalue;

	  sprintf(keyname, "CO%d_X%d_C", rawtodetparam->low_sys, seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_x_c[seg] = keyvalue;


	  sprintf(keyname, "CO%d_Y%d_A", rawtodetparam->low_sys, seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_y_a[seg] = keyvalue;

	  sprintf(keyname, "CO%d_Y%d_B", rawtodetparam->low_sys, seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_y_b[seg] = keyvalue;

	  sprintf(keyname, "CO%d_Y%d_C", rawtodetparam->low_sys, seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_y_c[seg] = keyvalue;

	}
    }

  if(coeff_found)
    {
      return RM_LINEAR_COEFF;
    }
  


  /* The COc_Xn_A keywords were not found either.  Look next for
   * COE_[XY]n_[ABC] keywords.  
   * Here n = segment number. Search single-digit n values. */
  
  status = 0;
  
  for(seg = 0; seg < 10; seg++)
    {
      sprintf(keyname, "COE_X%d_A", seg);
      fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
      if(status == KEY_NO_EXIST)
	{
	  status = 0;

	  if(coeff_found == 1)
	    {
	      rawtodetparam->n_segs = seg - rawtodetparam->min_seg;
	      break;
	    }
	  continue;
	}
      else
	{
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_x_a[seg] = keyvalue;
	  if(!coeff_found)
	    {
	      rawtodetparam->min_seg = seg;
	    }
	  coeff_found = 1;
	  
	  
	  sprintf(keyname, "COE_X%d_B", seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_x_b[seg] = keyvalue;

	  sprintf(keyname, "COE_X%d_C", seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_x_c[seg] = keyvalue;


	  sprintf(keyname, "COE_Y%d_A", seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_y_a[seg] = keyvalue;

	  sprintf(keyname, "COE_Y%d_B", seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_y_b[seg] = keyvalue;

	  sprintf(keyname, "COE_Y%d_C", seg);
	  fits_read_key_dbl(fp, keyname, &keyvalue, NULL, &status);
	  sprintf(tempstring, "reading %s from", keyname);
	  checkFITSerrors(status, tempstring, filename);
	  rawtodetparam->coeff_y_c[seg] = keyvalue;

	}
    }

  if(coeff_found)
    {
      return RM_LINEAR_COEFF;
    }

  return RM_UNKNOWN;  
}





void printTrRawtodet(TR_RAWTODET* rawtodetparam)
{
  int row;
  printf("    Tr_Rawtodet structure contents:\n");
  
  printf("      lowcoordsysname: %s\n", rawtodetparam->lowcoordsysname);
  printf("      highcoordsysname: %s\n", rawtodetparam->highcoordsysname);
  printf("      low_sys: %d\n", rawtodetparam->low_sys);
  printf("      high_sys: %d\n", rawtodetparam->high_sys);
  printf("      n_segs: %d\n", rawtodetparam->n_segs);
  printf("      min_seg: %d\n", rawtodetparam->min_seg);
  printf("      segcolname: %s\n", rawtodetparam->segcolname);
  printf("      int_cen_x: %g\n", rawtodetparam->int_cen_x);
  printf("      int_cen_y: %g\n", rawtodetparam->int_cen_y);
  printf("      int_cen_found: %d\n", rawtodetparam->int_cen_found);
  printf("      rawmethod: %d\n", rawtodetparam->rawmethod);
  
  if(rawtodetparam->rawmethod == RM_CORNER_LIST)
    {
      printf("      pixel map:\n");
      printf("         %4s  %7s %7s %7s %7s   %7s %7s %7s %7s\n", "SEG", "X0", "X1", "X2", "X3", "Y0", "Y1", "Y2", "Y3");

      for(row = 0; row < rawtodetparam->n_segs; row++)
	{
	  printf("         %4d  %7g %7g %7g %7g   %7g %7g %7g %7g\n", rawtodetparam->corner_seg[row], rawtodetparam->corner_x[row][0], rawtodetparam->corner_x[row][1],  rawtodetparam->corner_x[row][2],  rawtodetparam->corner_x[row][3],  rawtodetparam->corner_y[row][0],rawtodetparam->corner_y[row][1],  rawtodetparam->corner_y[row][2],  rawtodetparam->corner_y[row][3]);
	}

    }

  if(rawtodetparam->rawmethod == RM_LINEAR_COEFF)
    {
      printf("      linear coefficients:\n");
      printf("         %4s  %7s %7s %7s   %7s %7s %7s\n", "SEG", "X_A", "X_B", "X_C", "Y_A", "Y_B", "Y_C");

      for(row = rawtodetparam->min_seg; row < rawtodetparam->min_seg + rawtodetparam->n_segs; row++)
	{
	  printf("         %4d  %7g %7g %7g   %7g %7g %7g\n", row, rawtodetparam->coeff_x_a[row], rawtodetparam->coeff_x_b[row], rawtodetparam->coeff_x_c[row], rawtodetparam->coeff_y_a[row], rawtodetparam->coeff_y_b[row], rawtodetparam->coeff_y_c[row]);
	}

      
    }

}


void destroyTrRawtodet(TR_RAWTODET* rawtodetparam)
{
  int low_sys =  rawtodetparam->low_sys;
  int row;

#ifdef DEBUG
  printf("Freeing rawtodetparam[%d]\n", low_sys);
#endif
  

  if(rawtodetparam->rawmethod == RM_CORNER_LIST)
    {
      for(row = 0; row < rawtodetparam->n_segs; row++)
	{
	  free(rawtodetparam->corner_x[row]);
	  free(rawtodetparam->corner_y[row]);
	}

      free(rawtodetparam->corner_seg);
      free(rawtodetparam->corner_x);
      free(rawtodetparam->corner_y);
    }

  free(rawtodetparam);

#ifdef DEBUG
  printf("Freed rawtodetparam[%d]\n", low_sys);
#endif
}

