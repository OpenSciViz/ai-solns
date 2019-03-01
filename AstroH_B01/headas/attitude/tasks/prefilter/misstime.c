/*
 * $Source: /headas/headas/attitude/tasks/prefilter/misstime.c,v $
 * $Revision: 1.3 $
 * $Date: 2005/09/14 21:41:12 $
 *
 * Modified from ascalib/src/general/ascatime.c for Swift
 *
 * $Log: misstime.c,v $
 * Revision 1.3  2005/09/14 21:41:12  rwiegand
 * Deleted local copy of report.[ch] and updated report calling sequence.
 * Pruned unused mean alignment structure.
 *
 * Revision 1.2  2002/12/06 20:14:21  rwiegand
 * Added Filter object to pass function pointers for iteration, initialization,
 * status checking.  Broke derive.c into separate files for FORTRAN calling
 * routines, iteration, initialization.  Made compare mode less XTE-centric.
 * Added parameters for pointing axis and boresight.  Allow loading two line
 * elements from FITS or text files.
 *
 * Revision 1.1  2002/05/14 14:51:57  rwiegand
 * Reworked parameter interface with Ed Pier's suggestions.
 *
 * Revision 1.1  2002/01/28 16:08:22  rwiegand
 * Initial revision
 *
 */

/************************************************************************
 *  misstime
 *    [English translation] of the original Japanese comments added 
 *    by Ken Ebisawa on 1999-11-05
 *    
 *    AtTime�ȥߥå���󥿥�����Ѵ�����롼����
 *    [Routine to convert between the AtTime and Mission time]
 *					by R.Fujimoto
 *
 *
 *  93/03/06 V1.0	���ä��θ����褦�ˤ��� [take into account of leap seconds]
 *     03/16 V1.1	�ǡ�����������¸����褦�ˤ��� [store data in memory]
 *     03/28 V1.2       ���ä��������Ѵ������褦�ˤ��� [correct leap second conversion]
 *     06/10 V1.3       ���äΥơ��֥��Ķ��ѿ��ǻ��� [leap second table is specfied by
 *                      an environmental variable]
 *     06/12 V1.4       leapflag��NO�˽�������� [leapflag is initialized as NO]
 *
 *  95/10/14 V2.0	bug fix�ʤ�Ӥ�mjd2asca��asca2mjd�Ȥ����ؿ����ɲá�
 *                      [bug fixed, mjd2asca, asca2mjd added]
 *     10/15 V2.1	minor bug fix
 *     10/16 V2.3	reformAtTime�ΰ����˱��ä�ä��롣 [leap second is added to the
 *                      arguments of reformAtTime]
 *			asca2attime�Ǳ��ä����ä��ִ֤��ɤ�����Ƚ�����ɡ�
 *                      [in asca2attitime, algorithm of judgement of the the moment 
 *                       when a leap second is inserted is improved]
 *     10/16 V2.4	asca2mjdtmp���ߡ�asca2attime��asca2mjd������
 *			���������mjd��Ϣ³�ˤʤ�褦�������
 *                      [asca2mjdtmp added. asca2attime, asca2mjd modified.
 *                       mjd is so defined as to be continuous before and after
 *                       the leap seconds]
 *  98/07/22 v2.5       1) switched to leapsec.fits from leapsec.dat
 *                         This is now similar to ascatime.c in mkfilter2,
 *                         but it has two more functions.
 *                      2) No longer using ENV variable to access leapsec file.
 *                         As a result, a few functions were removed.
 *  99/10/21 v2.6       Changed the attime0 from 93 to 1993.
 *                      
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "atFunctions.h"
#include "fitsio.h"
#include "report.h"
#include "misstime.h"

#define EPSILON_T	0.00005

typedef struct
{
	AtTime epoch;
	double mjd;
	double step;
} Leap;

static int leapSpace;
static int leapCount;
static Leap *LEAPS;
static int readDataFlag;

static AtTime missionEpoch = { 2000, 1, 1, 0, 0, 0, 0.0 };
static double missionMJD;



/**************************************************************************
 * this variable is used only if the calling function failed to 
 * initialize the leapsec table first using the leapsec.fits file
 * specified in parameter file;
 * If this variable is used, a warning message will be reported
 **************************************************************************/
#define LEAPTABLE "./refdata/leapsec.fits"


/************************************************************************
 *  x_round
 *    �������ʲ���ͼθ�������ؿ� [round decimals]
 *
 *  95/10/14
 *  ��static double�����ѹ�
 ************************************************************************/
static double x_round(double x)
{
	double n;

	n = (double)(int)x;
	if (x > 0.0) {
		if ((x - n) < 0.5)
			return n;
		else
			return (n + 1.0);
	} else {
		if ((n - x) < 0.5)
			return n;
		else
			return (n - 1.0);
	}
}


/************************************************************************
 *	reformAtTime
 *		����夬����θ����AtTime����������ؿ� 
 *		[reform AtTime taking into account of raising decimals]
 *		Input&Output:
 *			*attime: �������������� [time to be reformed]
 *		Input:
 *			extrasec: ����夬�꤬60�ä��Ĺ�����ˤϤ��ο�����Ϳ���롣
 *								60�äǷ���夬����ˤ�0.0���Ϥ����ȡ��������������Ǥ�
 *								������ʬ�������Ƥ��ʤ���
 *								[When the amount of raising is longer than 60 sec,
 *								 give that number.	If being raised at 60 sec, give 0.0.
 *								 However, only the integer part is checed currently.]
 *
 *	95/10/14
 *	��attime->mn = month;�ȤʤäƤ����Х���fix��
 *		[attime->mn = month; bug fix]
 *	��static void�����ѹ���[convert to static void]
 *	��̾�Τ�reform����reformAtTime���ѹ��� [rename from reform to reformt AtTime]
 *
 *	95/10/16
 *	��������extrasec���ɲá� [add extrasec to the argument]
 ************************************************************************/
static void reformAtTime(AtTime *attime, double extrasec)
{
	int year, month, day, hour, minute, sec;
	int fullsec;
	float msec;

	year    = attime->yr;
	month   = attime->mo;
	day     = attime->dy;
	hour    = attime->hr;
	minute  = attime->mn;
	sec     = attime->sc;

	/* �ߥ��ä�10�ܤ��ƾ������ʲ���ͼθ��������줬10000��ۤ������
		 ����夬��׻���Ԥʤ� 
		 [multiply miliseconds by 10,	and round the decimals. If this execced
		 100000, calculate to raise.]*/
	msec = (x_round((double)(attime->ms)*10.0));
	if (msec >= 10000.0) {
		msec -= 10000.0;
		msec /= 10.0;
		sec++;
	} else {
		msec /= 10.0;
	}

	/* ���ä���������Ƥ����硢60�äǤϤʤ�60+extrasec�äǷ���夬�� 
		 [When leapseconds are inserted, raise at 60+extrasec, not 60 sec.]*/
	fullsec = 60 + (int)extrasec;
	sec += (int)extrasec;
	if (sec >= fullsec) {
		sec -= fullsec;
		minute += 1;
	}
	else if (sec < 0) {
		sec += 60;
		--minute;
	}

	if (minute >= 60) {
		minute -= 60;
		hour++;
	}
	else if (minute < 0) {
		minute += 60;
		--hour;
	}

	if (hour >= 24) {
		hour -= 24;
		day++;
	}
	else if (hour < 0) {
		hour += 24;
		--day;
	}

	if (day > 28) {
		if ((day == 29) && (month == 2) && (year%4 != 0)) {
			day = 1;
			month = 3;
		} else if ((day == 30) && (month == 2) && (year%4 == 0)) {
			day = 1;
			month = 3;
		} else if ((day == 31) && ((month == 4) || (month == 6) || (month == 9) || (month == 11))) {
			day = 1;
			month++;
		} else if (day == 32) {
			day = 1;
			month++;
		}
	}
	else if (day < 1) {
		day += 31;
		--month;
/* REW: what if month does not have 31 days? */
	}

	if (month == 13) {
		month = 1;
		year++;
	}
	else if (month < 1) {
		month += 12;
		--year;
	}

	attime->yr = year;
	attime->mo = month;
	attime->dy = day;
	attime->hr = hour;
	attime->mn = minute;
	attime->sc = sec;
	attime->ms = msec;
}


int setMissionEpoch (const AtTime *epoch)
{
	int code = 1;

	if (epoch)
		{
			const AtTime *t = epoch;

			missionEpoch = *epoch;
			code = 0;

			/* initialize missionMJD from missionEpoch */
			atMJulian(&missionEpoch, &missionMJD);

			report_verbose("mission epoch set to %d/%02d/%02d+%02d:%02d:%02d %f [mjd %f]\n",
					t->yr, t->mo, t->dy, t->hr, t->mn, t->sc, t->ms, missionMJD);
		}
	else
			report_warning("null epoch\n");

	return code;
}


/************************************************************************
 * Using leapsec.fits in FTOOLS' refdate area instead of leadsec.dat
 * Leap seconds before mjd0 will be ignored.
 ************************************************************************/
int readLeapTable (const char *path)
{
	int code = 0;
	fitsfile *fp = 0;
	int i, hdutype, status=0, anynul=0;
	long NAXIS2;
	char comm[73];
#define DEFAULT_BUFFER 64
	double buffer0[DEFAULT_BUFFER];
	double buffer1[DEFAULT_BUFFER];
	double *d0, *d1;
	int firstSignificant;

	/* initialize missionMJD from missionEpoch (in the event we are using
			 the default epoch) */
	atMJulian(&missionEpoch, &missionMJD);

	if (!path)
		{
			path = LEAPTABLE;
			report_warning("using default leap second table %s\n",
					path);
		}

	if (fits_open_file(&fp, path, READONLY, &status))
		{
			report_error("unable to open leapsec table %s [%d]\n",
					path, status);
			return 1;
		}

	if (fits_movabs_hdu(fp, 2, &hdutype, &status))
		fits_report_error(stderr, status);

	if (fits_read_key_lng(fp, "NAXIS2", &NAXIS2, comm, &status))
		fits_report_error(stderr, status);

	d0 = &buffer0[0];
	d1 = &buffer1[0];
	if (NAXIS2 > DEFAULT_BUFFER)
		{
			d0 = (double*) malloc(NAXIS2 * sizeof(double));
			d1 = (double*) malloc(NAXIS2 * sizeof(double));
		}

	/***************************************************************** 
	 * READ MJD and determine when mjd > lower limit
	 ******************************************************************/
	/* now read mjd time from col 3 in leapsec.fits */
	if (fits_read_col_dbl(fp, 3, 1, 1, NAXIS2, 0.0,
				d0, &anynul, &status))
		fits_report_error(stderr, status);

	/* read column 5/LEAPSECS:	dbl*/
	if (fits_read_col_dbl(fp, 5, 1, 1, NAXIS2, 0.0,
				d1, &anynul, &status))
		fits_report_error(stderr, status);

	/* find rows after mission epoch */
	for (i = 0; i < NAXIS2; ++i)
		if (d0[i] >= missionMJD)
			break;

	firstSignificant = i;

	if (NAXIS2 - firstSignificant > leapCount)
		{
			if (LEAPS)
				free(LEAPS);
			LEAPS = 0;
			leapSpace = 0;
		}

	leapCount = NAXIS2 - firstSignificant;
	if (!LEAPS && leapCount > 0)
		{
			LEAPS = malloc(leapCount * sizeof(Leap));
			leapSpace = leapCount;
		}

	for (i = 0; i < leapCount; ++i)
		{
			Leap *leap = LEAPS + i;
			double mjd = d0[i + firstSignificant];
			leap->mjd = mjd;
			leap->step = d1[i + firstSignificant];
			atMJDate(mjd, &leap->epoch);
		}

	if (fits_close_file(fp, &status))
		{
			report_warning("unable to close %s [%d]\n",
					path, status);
			code = 1;
		}

	readDataFlag = 1;

	if (d0 != &buffer0[0])
		free(d0);
	if (d1 != &buffer1[0])
		free(d1);

	if (!code)
		report_verbose("leapsec FITS data was successfully read\n");

#if 1 /* DEBUG */
	report_debug("leapCount %d [epoch mjd step]\n", leapCount);
	for (i = 0; i < leapCount; ++i)
		{
			const Leap *leap = LEAPS + i;
			const AtTime *t = &leap->epoch;
			report_debug("\t%d/%02d/%02d+%02d:%02d:%02d %f %f %f\n",
					t->yr, t->mo, t->dy, t->hr, t->mn, t->sc, t->ms,
					leap->mjd, leap->step);
		}
#endif

	return code;
}


/************************************************************************
 *	AtTime_to_missionTime
 *		������AtTime���ˤ�ߥå���󥿥�����Ѵ����� [convert date and time
 *									 (AtTime type) to the mission time.]
 *
 *	95/10/14	
 *	��������mjd0�ȡ����ä��������줿����mjd_leap��readLeapTable�Ƿ׻�����
 *		�褦�ˤ�����
 *		[Standard mjd0 and the mjd_leap when leap seconds are inserted are
 *		calculated by readLeapTable.]
 ************************************************************************/
double AtTime_to_missionTime (const AtTime *atin)
{
	double mjd;
	double missionTime;
	int i;
AtTime attime = *atin;

	if (!readDataFlag)
		readLeapTable(0);

	/* ������mjd���Ѵ����� [convert date and time to MJD]*/
	atMJulian(&attime, &mjd);

	/* �ߥå���󥿥����׻����� [calculate the mission time]*/
	missionTime = (mjd - missionMJD) * 86400.0;

	for (i = 0; i < leapCount; i++) {
		/* ��������������ä�����������ʤ顢���λ��֤��θ���� 
			 The date and time considering is after insertion of the leapseconds,
			 take into account that time*/
		Leap *leap = LEAPS + i;
		if (mjd >= leap->mjd) {
			const AtTime *t = &leap->epoch;
			missionTime += leap->step;
			if ((mjd < leap->mjd + leap->step/86400.0)
				&& ((attime.mo != t->mo)
					|| (attime.dy != t->dy)
					|| (attime.hr != t->hr)
					|| (attime.mn != t->mn)))
				missionTime -= leap->step;

		} else
			break;
	}

	return missionTime;
}	


/************************************************************************
 *	missionTime_to_MJD_aux
 *		�ߥå���󥿥�����������Ѵ����뤿��δ���Ū�ʥ롼����
 *		 [basic routine to convert mission time to date and time]
 *		Input:
 *			missionTime: �ߥå���󥿥��� [mission time]
 *		Output:
 *			*mjd: �ƥ�ݥ��mjd [temporary mjd]
 *			*leapflag: ���ä��������줿�ִ֤��ݤ��򼨤��ե饰 [flag to tell if this is
 *			 the memoent leapsecond is inserted]
 *			*currentleap: �������줿���� [inserted leap seconds]
 *			*mjd_leap: ���ä��������줿������mjd�� [mjd when leap seconds are inserted]
 *
 *	95/10/16 
 *	��EPSILON_T��Ƴ���������ä��������줿���νִ֤�Ƚ����ˡ�����������
 *		[Introduce EPSILON_T, improve the algorithm to judge the moment when
 *		 a leap second is introduced.]
 *	��missionTime_to_AtTime����Ⱦ��ʬ����Ω�������� [the first half of
 *		 missionTime_to_AtTime is made independent.]
 ************************************************************************/
static void missionTime_to_MJD_aux(double missionTime,
		double *mjd, int *leapflag, double *currentleap, double *mjdLeap)
{
	double missionLeap;
	double totalLeap = 0;
	int i;

	/* �ݥ����ѿ��ν���� [initialize pointer variable]*/
	*currentleap = 0.0;
	*leapflag = 0;

	/* ���äΥơ��֥�򳫤� [open the leapsecond table]*/
	if (!readDataFlag)
		readLeapTable(0);

#if 0
	/* REW: this is never used */
	/* �ߥå���󥿥������������mjd�򻻽Ф��� [calcluate mjd corresponding to
		 the mission time]]*/
	*mjd = missionMJD + missionTime / 86400.0;
#endif

	for (i = 0; i < leapCount; i++) {

		Leap *leap = LEAPS + i;
		/* ���ä��������줿��������������ߥå���󥿥���򻻽Ф��� 
			 [calculate the mission time corresponding to the date ane time when
			 leap seconds are inserted]*/
		missionLeap = AtTime_to_missionTime(&leap->epoch);

		if (missionTime >= missionLeap) {
			/* ���ä���������Ƥ����� [in the case leapseconds are inserted] */
			totalLeap += leap->step;
		}
		else if (missionTime > missionLeap - leap->step - EPSILON_T) {
			/* ���ä��������줿���νִ֤ǡ��б�����mjd��¸�ߤ��ʤ���� 
				 [in the case leap seconds are inserted and the corresponding mjd does
					not exist]r*/
			*leapflag = 1;
			totalLeap += leap->step;
			*mjdLeap = missionMJD + (missionLeap - totalLeap) / 86400.0;
			*currentleap = leap->step;
			break;
		} else {
			/* ���ä���������Ƥ��ʤ���� [when leap seconds are not inserted]*/
			break;
		}
	}

	/* �������줿���ä�ʬ�����ߥå���󥿥��फ�麹�ð��� 
	 [subtract the inserted leap seconds from the mission time]*/
	missionTime -= totalLeap;
	*mjd = missionMJD + missionTime / 86400.0;

}


/************************************************************************
 *	missionTime_to_AtTime
 *		�ߥå���󥿥�����������Ѵ����롣[convert mission time to date and time]
 *		���ߡ���ñ�̤α��äˤ����б����Ƥ��ޤ���[only can handle leap seconds
 *		in the unit of second]
 *
 *	95/10/14 
 *	��mjd0�����ѿ��Ȥ���readLeapTable���ɤޤ���褦�ˤ�����[mjd0 is an external
 *		variable, and read from readLeapTable]
 *	95/10/16
 *	�����ä����ä����η���夲������reformAtTime�ǹԤʤ��褦�ˤ�����
 *		[reformAtTime takes care of raising decimals when leap seconds are inserted]
 *	����Ⱦ��ʬ��missionTime_to_MJD�Ȥ����ؿ��Ȥ�����Ω��������
 *		[The first half is made independent as missionTime_to_MJD_aux]
 ************************************************************************/
int missionTime_to_AtTime(double missionTime, AtTime *attime)
{
	double mjd, currentleap = 0.0;
	double dummy;
	int leapflag = 0;

	/* �ߥå���󥿥��फ��MJD�ȡʱ��ä��������줿�ִ֤ʤ�Сˤ��α��ä�׻� */
	/* calculated MJD from mission time. At the moment leapsecons are
	 inserted, calculate that leap seconds */
	missionTime_to_MJD_aux(missionTime, &mjd, &leapflag, &currentleap, &dummy);

	/* MJD��AtTime���Ѵ� [convert MJD to AtTime ]*/
	atMJDate(mjd, attime);

	/* ����夬����θ����AtTime���������� [reform AtTime taking account of
	 raising decimals] */
	reformAtTime(attime, currentleap);
	return 0;
}


/************************************************************************
 *	mjd_to_missionTime
 *		mjd��missionTime���Ѵ����롣[convert mjd to missionTime]
 ************************************************************************/
double mjd_to_missionTime(double mjd)
{
	AtTime attime;

	atMJDate(mjd, &attime);
	return AtTime_to_missionTime(&attime);
}


/************************************************************************
 *	missionTime_to_MJD
 *		missionTime_to_MJD��mjd���Ѵ����롣[covert missionTime to mjd]
 *
 *	95/10/16
 *	��missionTime_to_AtTime���ͳ���ʤ��褦�ˤ����� [not through
 *		missionTime_to_AtTime]
 *	�����äλ��ˤ�Ϣ³�ˤʤ�褦�ˤ�����	[continuous even at leap seconds]
 *	1998-07-22: changed mjd_leap to mjdLeap to avoid confusion with mjd_leap[]
 ************************************************************************/
double missionTime_to_MJD (double missionTime)
{
	double mjd, mjdLeap, dummy;
	int leapflag = 0;

	/* �ߥå���󥿥��फ��MJD��׻����� [calculate MJD from mission time]*/
	missionTime_to_MJD_aux(missionTime, &mjd, &leapflag, &dummy, &mjdLeap);

	/* ���ä���������Ƥ������ʤ�С��б�����MJD���ʤ��Τǡ��������줿����
		 MJD���֤� [if this is during insertion of leap seconds, there will be no
		 corresponding MJD, so MJD at the insertion is returned*/
	if (leapflag)
		return mjdLeap;
	else 
		return mjd;
}
