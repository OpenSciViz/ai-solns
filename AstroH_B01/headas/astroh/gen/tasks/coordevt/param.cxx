#ifdef __cplusplus
extern "C"
{
#endif

#include "param.h"
#include "param_wrappers.h"
#include "random.h"
#include "caldbquery.h"

#ifdef __cplusplus
}
#endif

#include <string.h>
#include <stdlib.h>
#include "headas.h"

#include "attfile.h"


/**************************************************************************
**************************************************************************
* read the input parameters 
**************************************************************************/
PARAM* getPar(void) 
{
PARAM* param;
char att_interpolation[FLEN_VALUE];
param = (PARAM*) calloc(1, sizeof(PARAM));


read_string_param("infile",param->in_file     ,FILENAME_LENGTH);
read_string_param("outfile",param->out_file     ,FILENAME_LENGTH);
read_string_param("teldeffile"   ,param->teldef_file       ,FILENAME_LENGTH);

read_string_param("attfile"  ,param->att_file       ,FILENAME_LENGTH);
read_string_param("dattfile"  ,param->datt_file       ,FILENAME_LENGTH);

read_string_param("startsys" ,param->startsysname, FLEN_VALUE     );
read_string_param("stopsys" ,param->stopsysname, FLEN_VALUE     );


read_string_param("eventext" ,param->event_extension,FLEN_VALUE     );

read_string_param("timecol"  ,param->time_col_name  ,FLEN_VALUE     );



/****************************************************************
* the user can turn off aspecting by giving "none" or a blank 
* string for the attitude file name 
****************************************************************/
if(strncasecmp(param->att_file,"none",FILENAME_LENGTH) || 
   param->att_file[0]=='\0' ) {
    /***************************************************
    * valid attfile name given so we will do aspecting *
    ***************************************************/
    param->do_sky=1;
} else {
    /**********************************************************
    * null attfile name given, so we won't be doing aspecting *
    **********************************************************/
    param->do_sky=0;
}

 param->do_sky = 0; /* temp! */
/****************************************************************
* the user can pass CALDB for the TELDEF file name
****************************************************************/
if(!strncasecmp(param->teldef_file, "CALDB", 5)) {
	CALDBQuery query = { 0 };
	sprintf(query.codename, "TELDEF");
	query.infile = param->in_file;
	set_caldb_query_qualifiers(&query, param->teldef_file);
	if (simple_caldb_query(&query, 0, param->teldef_file)) {
		headas_chat(0, "unable to resolve teldef=CALDB\n");
		free(param);
		return 0;
	}
	HDpar_note("teldef", param->teldef_file);
}

    param->null_value = read_int_param("nullvalue");

/*******************************
* aspecting-related parameters *
*******************************/
if(param->do_sky) {
   
    /******************************
    * should we apply aberration? *
    ******************************/
    param->do_aberration=read_boolean_param("aberration");
    if(param->do_aberration) param->follow_sun=read_boolean_param("follow_sun");
    else                     param->follow_sun=0;

    /*******************************************************
    * nominal pointing entered in decimal R.A. and Dec but
    * converted immediately to Euler angles
    ******************************************************/
    param->ra =read_double_param("ra");
    param->dec=read_double_param("dec");
    param->roll=read_double_param("roll");


    read_string_param("attinterpolation", att_interpolation ,FLEN_VALUE);
    if (!strcasecmp(att_interpolation, "CONSTANT")) {
        param->att_interpolation = ATTFILE_CONSTANT;
        headas_chat(2, "using CONSTANT interpolation for aspecting\n");
    }
    else {
        param->att_interpolation = ATTFILE_LINEAR;
        headas_chat(2, "using LINEAR interpolation for aspecting\n");
    }

} else {
   /*************************************************
   * we will not be doing DET -> SKY transformation *
   *************************************************/
   param->do_aberration=0;
   param->follow_sun=0;
}

/*************************************
* random number generator parameters *
*************************************/
param->randomize=read_boolean_param("randomize");
if(param->randomize) {
    param->seed=read_int_param("seed");
    seed_random(param->seed);
    read_string_param("randsys", param->randsysname, COORDSYSNAME_LENGTH);
}

/********************************************************
* how far outside the attitude file can we extrapolate? *
********************************************************/
param->att_time_margin=read_double_param("atttimemargin");
param->datt_time_margin=read_double_param("datttimemargin");


return(param);
}

/****************************************************************************
*****************************************************************************
* free all the memory associated with a param structure
****************************************************************************/
void destroyParam(PARAM* param) {

free(param);

} /* end of destroyParam function */

