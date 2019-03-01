#ifndef COORDEVT_PARAM_INCLUDED

#include "fitsio.h"
#include "coord.h"
#define FILENAME_LENGTH FLEN_FILENAME
#define COORDSYSNAME_LENGTH 10

/****************************
* input paramater structure 
****************************/
typedef struct {
  
  char in_file[FILENAME_LENGTH];
  char out_file[FILENAME_LENGTH];
  char teldef_file[FILENAME_LENGTH];
  char att_file[FILENAME_LENGTH];
  char datt_file[FILENAME_LENGTH];
  char startsysname[COORDSYSNAME_LENGTH];
  char stopsysname[COORDSYSNAME_LENGTH];
  int startsys;
  int stopsys;

  int do_aberration;
  int follow_sun;
  int do_sky;
  
  int randomize;
  int seed;
  int att_interpolation;
  int datt_interpolation;

  char event_extension[FLEN_VALUE]; /* name of event file events extension */
  char time_col_name[FLEN_VALUE]; /* name of time column */
  char** seg_col_name; /* names of segment column s*/
  char randsysname[COORDSYSNAME_LENGTH];
  int includeintcols;
  int includefloatcols;

  int blank_col;
  int null_value;

  double ra;
  double dec;
  double roll;
  double euler1;
  double euler2;
  double euler3;

  double att_time_margin;
  double datt_time_margin;

  int do_history;
  int chatter;

} PARAM;


/**************************************************************************
**************************************************************************
* read the input parameters 
**************************************************************************/
PARAM* getPar(void);

/****************************************************************************
*****************************************************************************
* free all the memory associated with a param structure
****************************************************************************/
void destroyParam(PARAM* param);

#define COORDEVT_PARAM_INCLUDED
#endif /* COORDEVT_PARAM_INCLUDED */
