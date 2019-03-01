#ifndef MISSTIME_H
#define MISSTIME_H

/*
 * $Source: /headas/headas/attitude/tasks/prefilter/include/misstime.h,v $
 * $Revision: 1.1 $
 * $Date: 2002/06/05 14:23:52 $
 *
 * Modified from ascalib/src/general/ascatime.h for Swift
 *
 *
 * $Log: misstime.h,v $
 * Revision 1.1  2002/06/05 14:23:52  rwiegand
 * Updated time interface for mission independence.
 *
 * Revision 1.1  2002/02/20 14:39:17  miket
 * Merging Bob's original RCS files with HEAdas revisions
 *
 * Revision 1.1  2002/01/28 19:23:26  rwiegand
 * Initial revision
 *
 */


#include "atFunctions.h"         /* AtTime */


int setMissionEpoch (const AtTime *epoch);
int readLeapTable (const char *leaptable);

double AtTime_to_missionTime (const AtTime *attime);
int missionTime_to_AtTime (double swifttime, AtTime *attime);


#endif

