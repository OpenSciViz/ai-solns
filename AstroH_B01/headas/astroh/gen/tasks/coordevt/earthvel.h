#ifndef EARTHVEL_H
#define EARTHVEL_H

/*
 * $Source: /astroh/astroh/gen/tasks/coordevt/earthvel.h,v $
 * $Revision: 1.1 $
 * $Date: 2012/07/24 15:04:38 $
 *
 *
 * $Log: earthvel.h,v $
 * Revision 1.1  2012/07/24 15:04:38  treichar
 * New coordevt tool, which converts coordinates in an event file according
 * to parameters specified in a TelDef file.
 *
 * Revision 1.1  2005/02/01 17:02:59  rwiegand
 * Updated the earth about sun velocity model.
 *
 */


/*
 * returns negative of earth velocity with respect to sun at mjd
 * in units of C
 */
double compat_earthvel_at_mjd (double vhat[3], double mjd);


#endif
