#ifndef ABERRATOR_H
#define ABERRATOR_H

/*
 * $Source: /headas/headas/attitude/tasks/aberrator/aberrator.h,v $
 * $Revision: 1.1 $
 * $Date: 2005/01/29 23:55:11 $
 *
 *
 * $Log: aberrator.h,v $
 * Revision 1.1  2005/01/29 23:55:11  rwiegand
 * Tool to adjust an attitude file for velocity aberration.
 *
 */


#include "fitsio.h"
#include "orbfile.h"
#include "align.h"
#include "quat.h"



typedef struct
{
	fitsfile * fptr;

	ALIGN * align;
	ORBFILE * orbfile;
	QUAT * quat;

	double mjdref;

	int move_earth;
	int move_satellite;

} Aberrator;



typedef struct
{
	double time;    /* MET [s] */

	double * inertial; /* RA,DEC,ROLL vectors [deg] */
	double * apparent;

	double * quaternion;

} AttitudeRecord;



int correct_attitude_record (Aberrator * tool, AttitudeRecord * record);


#endif
