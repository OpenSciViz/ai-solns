#include <math.h>
#include <string.h>
#include "fitsio.h"
#include "longnam.h"
#include "teldef2.h"

TR_MULTISEG* readTrMultiseg(fitsfile* fp, 
			    char* lowcoordsysname, char* highcoordsysname,
			    int low_sys, char* filename)
{
  TR_MULTISEG* multisegparam = (TR_MULTISEG*) malloc(sizeof(TR_MULTISEG));
  char keyname[FLEN_KEYWORD];
  char tempstring[1000];
  int status = 0;
  int hdutype = ANY_HDU;
  char extname[20];
  int col;
  char colname[FLEN_VALUE];
  int colnum;
  int anynul;
  char* coeffcolnames[6] = {"COEFF_X_A", "COEFF_X_B", "COEFF_X_C",
			    "COEFF_Y_A", "COEFF_Y_B", "COEFF_Y_C"};

  strcpy(multisegparam->lowcoordsysname, lowcoordsysname);
  strcpy(multisegparam->highcoordsysname, highcoordsysname);
  multisegparam->low_sys = low_sys;
  multisegparam->high_sys = low_sys + 1;

  /* Move to MULTISEGn_COEFF extension. */

  sprintf(extname, "MULTISEG%d_COEFF", low_sys);
  fits_movnam_hdu(fp, hdutype, extname, 0, &status);
  sprintf(tempstring, "moving to %s extension of", extname);
  checkFITSerrors(status, tempstring, filename); 

 /* Read the number of segment properties from the NPROP keyword. */

  fits_read_key_lng(fp, "NPROP", &multisegparam->n_properties, NULL, &status);
  checkFITSerrors(status, "reading NPROP from", filename);

  /* Allocate propertynames array. These names are assumed to 
   * match the event file column names. */

  multisegparam->propertynames = calloc(multisegparam->n_properties, sizeof(char*));

  /* Read the property names. */

  for(col = 0; col < multisegparam->n_properties; col++)
    {
      multisegparam->propertynames[col] = calloc(FLEN_KEYWORD, sizeof(char));

      sprintf(keyname, "PROP%d", col);
      fits_read_key_str(fp, keyname, tempstring, NULL, &status);
      strcpy(multisegparam->propertynames[col], tempstring);
      sprintf(tempstring, "reading %s from", keyname);
      checkFITSerrors(status, tempstring, filename);
    }


  /* Read the number of table rows from the NAXIS2 keyword. */

  fits_read_key_lng(fp, "NAXIS2", &multisegparam->n_rows, NULL, &status);
  checkFITSerrors(status, "reading NAXIS2 from", filename);

  /* Read the number of table rows from the NAXIS2 keyword. */

  fits_read_key_lng(fp, "TFIELDS", &multisegparam->n_cols, NULL, &status);
  checkFITSerrors(status, "reading TFIELDS from", filename);

  if(multisegparam->n_properties + 6 > multisegparam->n_cols)
    {
      fprintf(stderr, "Teldef error: Mismatch in the number of columns of extension %s of teldef file %s.\nNPROP = %ld and TFIELDS = %ld, but TFIELDS must be at least NPROP + 6.\n", extname, filename, multisegparam->n_properties, multisegparam->n_cols);
      exit(1);
    }
  
  /* Allocate properties and coeff arrays to store the MULTISEGn_COEFF table. */
  
  multisegparam->properties = calloc(multisegparam->n_properties, sizeof(long*));
  multisegparam->coeff_x_a = calloc(multisegparam->n_rows, sizeof(double));
  multisegparam->coeff_x_b = calloc(multisegparam->n_rows, sizeof(double));
  multisegparam->coeff_x_c = calloc(multisegparam->n_rows, sizeof(double));
  multisegparam->coeff_y_a = calloc(multisegparam->n_rows, sizeof(double));
  multisegparam->coeff_y_b = calloc(multisegparam->n_rows, sizeof(double));
  multisegparam->coeff_y_c = calloc(multisegparam->n_rows, sizeof(double));
  multisegparam->coeffarrays[0] = multisegparam->coeff_x_a;
  multisegparam->coeffarrays[1] = multisegparam->coeff_x_b;
  multisegparam->coeffarrays[2] = multisegparam->coeff_x_c;
  multisegparam->coeffarrays[3] = multisegparam->coeff_y_a;
  multisegparam->coeffarrays[4] = multisegparam->coeff_y_b;
  multisegparam->coeffarrays[5] = multisegparam->coeff_y_c;

  for(col = 0; col < multisegparam->n_properties; col++)
    {
      multisegparam->properties[col] = calloc(multisegparam->n_rows, sizeof(long));
    }


  /* Read PROPERTYn columns of MULTISEGn_COEFF table. */

  for(col = 0; col < multisegparam->n_properties; col++)
    {
      sprintf(colname, "PROPERTY%d", col);
      fits_get_colnum(fp, CASEINSEN, colname, &colnum, &status);
      sprintf(tempstring, "finding column %s in %s extension of", colname, extname);
      checkFITSerrors(status, tempstring, filename);
      
      fits_read_col_lng(fp, colnum, 1, 1, multisegparam->n_rows, 
			-999, multisegparam->properties[col], &anynul, &status);
      sprintf(tempstring, "reading column %s in %s extension of", colname, extname);
      checkFITSerrors(status, tempstring, filename);
    }


  /* Read PROPERTYn columns of MULTISEGn_COEFF table. */

  for(col = 0; col < 6; col++)
    {
      strcpy(colname, coeffcolnames[col]);
      fits_get_colnum(fp, CASEINSEN, colname, &colnum, &status);
      sprintf(tempstring, "finding column %s in %s extension of", colname, extname);
      checkFITSerrors(status, tempstring, filename);
      
      fits_read_col_dbl(fp, colnum, 1, 1, multisegparam->n_rows, 
			-999, multisegparam->coeffarrays[col], &anynul, &status);
      sprintf(tempstring, "reading column %s in %s extension of", colname, extname);
      checkFITSerrors(status, tempstring, filename);
    }


  return multisegparam;
}


void printTrMultiseg(TR_MULTISEG* multisegparam)
{
  int col;
  int row;

  printf("    Tr_Multiseg structure contents:\n");
  printf("      lowcoordsysname: %s\n", multisegparam->lowcoordsysname);
  printf("      highcoordsysname: %s\n", multisegparam->highcoordsysname);
  printf("      low_sys: %d\n", multisegparam->low_sys);
  printf("      high_sys: %d\n", multisegparam->high_sys);
  printf("      n_properties: %ld\n", multisegparam->n_properties);

  for(col = 0; col < multisegparam->n_properties; col++)
    {
      printf("      propertyname[%d]: %s\n", col, multisegparam->propertynames[col]);

    }
  printf("      n_rows: %ld\n", multisegparam->n_rows);
  printf("      n_cols: %ld\n", multisegparam->n_cols);
  printf("      MULTISEG#_COEFF table:\n");
  printf("      ");
  
  for(col = 0; col < multisegparam->n_properties; col++)
    {
      printf("  P%d " , col);
    }
  printf("     X_A      X_B      X_C      Y_A      Y_B      Y_C\n");


  for(row = 0; row < multisegparam->n_rows; row++)
    {
      printf("      ");
      for(col = 0; col < multisegparam->n_cols; col++)
	{
	  if(col < multisegparam->n_properties)
	    {
	      printf("%4ld ", multisegparam->properties[col][row]);
	    }
	  else
	    {
	      printf("%8g ", multisegparam->coeffarrays[col - multisegparam->n_properties][row]);
	    }
	}
      printf("\n");
    }
}



void destroyTrMultiseg(TR_MULTISEG* multisegparam)
{
  int col = 0;
  int low_sys = multisegparam->low_sys;

#ifdef DEBUG
  printf("Freeing multisegparam[%d]\n", low_sys);
#endif

  free(multisegparam->coeff_x_a);
  free(multisegparam->coeff_x_b);
  free(multisegparam->coeff_x_c);
  free(multisegparam->coeff_y_a);
  free(multisegparam->coeff_y_b);
  free(multisegparam->coeff_y_c);

  for(col = 0; col < multisegparam->n_properties; col++)
    {
      free(multisegparam->propertynames[col]);
      free(multisegparam->properties[col]);
    }

  free(multisegparam->properties);
  free(multisegparam->propertynames);

  free(multisegparam);

#ifdef DEBUG
  printf("Freed multisegparam[%d]\n", low_sys);
#endif
}

