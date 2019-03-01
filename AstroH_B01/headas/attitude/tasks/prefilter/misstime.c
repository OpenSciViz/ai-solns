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
 *    AtTimeとミッションタイムを変換するルーチン
 *    [Routine to convert between the AtTime and Mission time]
 *					by R.Fujimoto
 *
 *
 *  93/03/06 V1.0	閏秒を考慮するようにした [take into account of leap seconds]
 *     03/16 V1.1	データをメモリに保存するようにした [store data in memory]
 *     03/28 V1.2       閏秒が正しく変換されるようにした [correct leap second conversion]
 *     06/10 V1.3       閏秒のテーブルを環境変数で指定 [leap second table is specfied by
 *                      an environmental variable]
 *     06/12 V1.4       leapflagをNOに初期化した [leapflag is initialized as NO]
 *
 *  95/10/14 V2.0	bug fixならびにmjd2asca、asca2mjdという関数を追加。
 *                      [bug fixed, mjd2asca, asca2mjd added]
 *     10/15 V2.1	minor bug fix
 *     10/16 V2.3	reformAtTimeの引数に閏秒を加える。 [leap second is added to the
 *                      arguments of reformAtTime]
 *			asca2attimeで閏秒が入った瞬間かどうかの判定を改良。
 *                      [in asca2attitime, algorithm of judgement of the the moment 
 *                       when a leap second is inserted is improved]
 *     10/16 V2.4	asca2mjdtmpを新設。asca2attimeとasca2mjdを修正。
 *			閏秒前後でmjdが連続になるように定義。
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
 *    小数点以下を四捨五入する関数 [round decimals]
 *
 *  95/10/14
 *  ・static double型に変更
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
 *		繰り上がりを考慮してAtTimeを整形する関数 
 *		[reform AtTime taking into account of raising decimals]
 *		Input&Output:
 *			*attime: 整形したい時刻 [time to be reformed]
 *		Input:
 *			extrasec: 繰り上がりが60秒より長い場合にはその数字を与える。
 *								60秒で繰り上がる場合には0.0を渡すこと。ただし、現状では
 *								整数部分しか見ていない。
 *								[When the amount of raising is longer than 60 sec,
 *								 give that number.	If being raised at 60 sec, give 0.0.
 *								 However, only the integer part is checed currently.]
 *
 *	95/10/14
 *	・attime->mn = month;となっていたバグをfix。
 *		[attime->mn = month; bug fix]
 *	・static void型に変更。[convert to static void]
 *	・名称をreformからreformAtTimeに変更。 [rename from reform to reformt AtTime]
 *
 *	95/10/16
 *	・引数にextrasecを追加。 [add extrasec to the argument]
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

	/* ミリ秒を10倍して小数点以下を四捨五入、これが10000を越える場合は
		 繰り上がり計算を行なう 
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

	/* 閏秒が挿入されている場合、60秒ではなく60+extrasec秒で繰り上がる 
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
 *		日時（AtTime型）をミッションタイムに変換する [convert date and time
 *									 (AtTime type) to the mission time.]
 *
 *	95/10/14	
 *	・基準時のmjd0と、閏秒が挿入された時のmjd_leapをreadLeapTableで計算する
 *		ようにした。
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

	/* 日時をmjdに変換する [convert date and time to MJD]*/
	atMJulian(&attime, &mjd);

	/* ミッションタイムを計算する [calculate the mission time]*/
	missionTime = (mjd - missionMJD) * 86400.0;

	for (i = 0; i < leapCount; i++) {
		/* 問題の日時が閏秒を挿入した後なら、その時間を考慮する 
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
 *		ミッションタイムを日時に変換するための基本的なルーチン。
 *		 [basic routine to convert mission time to date and time]
 *		Input:
 *			missionTime: ミッションタイム [mission time]
 *		Output:
 *			*mjd: テンポラリmjd [temporary mjd]
 *			*leapflag: 閏秒が挿入された瞬間か否かを示すフラグ [flag to tell if this is
 *			 the memoent leapsecond is inserted]
 *			*currentleap: 挿入された閏秒 [inserted leap seconds]
 *			*mjd_leap: 閏秒が挿入された時点のmjd。 [mjd when leap seconds are inserted]
 *
 *	95/10/16 
 *	・EPSILON_Tを導入し、閏秒が挿入されたその瞬間の判定方法を改善した。
 *		[Introduce EPSILON_T, improve the algorithm to judge the moment when
 *		 a leap second is introduced.]
 *	・missionTime_to_AtTimeの前半部分を独立させた。 [the first half of
 *		 missionTime_to_AtTime is made independent.]
 ************************************************************************/
static void missionTime_to_MJD_aux(double missionTime,
		double *mjd, int *leapflag, double *currentleap, double *mjdLeap)
{
	double missionLeap;
	double totalLeap = 0;
	int i;

	/* ポインタ変数の初期化 [initialize pointer variable]*/
	*currentleap = 0.0;
	*leapflag = 0;

	/* 閏秒のテーブルを開く [open the leapsecond table]*/
	if (!readDataFlag)
		readLeapTable(0);

#if 0
	/* REW: this is never used */
	/* ミッションタイムに相当するmjdを算出する [calcluate mjd corresponding to
		 the mission time]]*/
	*mjd = missionMJD + missionTime / 86400.0;
#endif

	for (i = 0; i < leapCount; i++) {

		Leap *leap = LEAPS + i;
		/* 閏秒が挿入された日時に相当するミッションタイムを算出する 
			 [calculate the mission time corresponding to the date ane time when
			 leap seconds are inserted]*/
		missionLeap = AtTime_to_missionTime(&leap->epoch);

		if (missionTime >= missionLeap) {
			/* 閏秒が挿入されている場合 [in the case leapseconds are inserted] */
			totalLeap += leap->step;
		}
		else if (missionTime > missionLeap - leap->step - EPSILON_T) {
			/* 閏秒が挿入されたその瞬間で、対応するmjdが存在しない場合 
				 [in the case leap seconds are inserted and the corresponding mjd does
					not exist]r*/
			*leapflag = 1;
			totalLeap += leap->step;
			*mjdLeap = missionMJD + (missionLeap - totalLeap) / 86400.0;
			*currentleap = leap->step;
			break;
		} else {
			/* 閏秒が挿入されていない場合 [when leap seconds are not inserted]*/
			break;
		}
	}

	/* 挿入された閏秒の分だけミッションタイムから差っ引く 
	 [subtract the inserted leap seconds from the mission time]*/
	missionTime -= totalLeap;
	*mjd = missionMJD + missionTime / 86400.0;

}


/************************************************************************
 *	missionTime_to_AtTime
 *		ミッションタイムを日時に変換する。[convert mission time to date and time]
 *		現在、秒単位の閏秒にしか対応していません。[only can handle leap seconds
 *		in the unit of second]
 *
 *	95/10/14 
 *	・mjd0を外部変数とし、readLeapTableで読ませるようにした。[mjd0 is an external
 *		variable, and read from readLeapTable]
 *	95/10/16
 *	・閏秒が入った時の繰り上げ処理をreformAtTimeで行なうようにした。
 *		[reformAtTime takes care of raising decimals when leap seconds are inserted]
 *	・前半部分をmissionTime_to_MJDという関数として独立させた。
 *		[The first half is made independent as missionTime_to_MJD_aux]
 ************************************************************************/
int missionTime_to_AtTime(double missionTime, AtTime *attime)
{
	double mjd, currentleap = 0.0;
	double dummy;
	int leapflag = 0;

	/* ミッションタイムからMJDと（閏秒が挿入された瞬間ならば）その閏秒を計算 */
	/* calculated MJD from mission time. At the moment leapsecons are
	 inserted, calculate that leap seconds */
	missionTime_to_MJD_aux(missionTime, &mjd, &leapflag, &currentleap, &dummy);

	/* MJDをAtTimeに変換 [convert MJD to AtTime ]*/
	atMJDate(mjd, attime);

	/* 繰り上がりを考慮してAtTimeを整形する [reform AtTime taking account of
	 raising decimals] */
	reformAtTime(attime, currentleap);
	return 0;
}


/************************************************************************
 *	mjd_to_missionTime
 *		mjdをmissionTimeに変換する。[convert mjd to missionTime]
 ************************************************************************/
double mjd_to_missionTime(double mjd)
{
	AtTime attime;

	atMJDate(mjd, &attime);
	return AtTime_to_missionTime(&attime);
}


/************************************************************************
 *	missionTime_to_MJD
 *		missionTime_to_MJDをmjdに変換する。[covert missionTime to mjd]
 *
 *	95/10/16
 *	・missionTime_to_AtTimeを経由しないようにした。 [not through
 *		missionTime_to_AtTime]
 *	・閏秒の時にも連続になるようにした。	[continuous even at leap seconds]
 *	1998-07-22: changed mjd_leap to mjdLeap to avoid confusion with mjd_leap[]
 ************************************************************************/
double missionTime_to_MJD (double missionTime)
{
	double mjd, mjdLeap, dummy;
	int leapflag = 0;

	/* ミッションタイムからMJDを計算する [calculate MJD from mission time]*/
	missionTime_to_MJD_aux(missionTime, &mjd, &leapflag, &dummy, &mjdLeap);

	/* 閏秒が挿入されている最中ならば、対応するMJDがないので、挿入された時の
		 MJDを返す [if this is during insertion of leap seconds, there will be no
		 corresponding MJD, so MJD at the insertion is returned*/
	if (leapflag)
		return mjdLeap;
	else 
		return mjd;
}
