/**
 **
 ** routines for calulating statistics of fields:
 **
 ** mostly straightforward 'cept for the mode & sigma where we first
 ** make a crude estimate of the mode as the most frequently
 ** occuring value and estimate sigma from sub-mode f values. We then
 ** refine this by making a least squares fit to the mode ± sigma / 2 region
 **
 ** aug 4th '92:
 ** revised fdo_stats
 **	now estimates quartiles and median from a random sample
 ** 	calculates sigma from width of inner 25 percent range
 ** Fri Aug  7 09:29:37 EDT 1992
 **		do_stats also converted to sampling technique
 **		mode determined by smoothed histogram method
 **		incorporates findmode() - formerly in analyse.c
 **		general purpose routine liststats() to return
 **		mode, medians, quartiles from an unsorted list.
 **
 **/

/* 15.03.1999:
   substituted drand48 with ran1 as the former did not work
   on SunOS

   28.03.1999:
   changed
   #define	good(x)		(x != SHRT_MIN && x != SHRT_MAX && x != MAGIC)
   to
   #define	good(x)		(x != SHORT_MAGIC)

   22.01.01:
   fixed a bug in liststats. The mode variable was used in a comparison
   although it may not have been defined. fixed by substitution with the well
   defined var. *modeptr

   23.01.01: removed unused variables

   01.02.2010:
   I substituted the ran1 function with the C standardlibrary 'rand'
   equivalent. We do not need a very sophisticated random number generator
   here.
*/

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>

#include "stats_stuff.h"
#include "magic.h"
#include "error.h"

#define	good(x)		(x != SHORT_MAGIC)
#define	fgood(x)	(x != (float) MAGIC)


#define DEFSAMPLESIZE 30000

#ifndef HUGE_VAL
#define HUGE_VAL	1.0e20
#endif

#ifndef HUGE_VAL_FLOAT
#define HUGE_VAL_FLOAT	1.0e20
#endif

void		do_stats(short **f, int N1, int N2, int margin, statsrec *srec)
{
	int			i, j, isample, ngood, badpix, samplesize, gotagoodone;
	short		fmin, fmax;
	float		fbar;
	float		mode, median, lquart, uquart, sigma;
	float		*fsample;
	
	fmin = SHRT_MAX; 		/* find max & min */
	fmax = SHRT_MIN;
	fbar = 0;
	ngood = 0;
	badpix = 0;
	for (i = margin; i < N2 - margin; i++)							
		for (j = margin; j < N1 - margin; j++)	{
			if (good(f[i][j])) {
				fmax = (f[i][j] > fmax ? f[i][j] : fmax);
				fmin = (f[i][j] < fmin ? f[i][j] : fmin);
				ngood++;
				fbar += f[i][j];
			} else {
			badpix++;
			}
		}
	srec->fmin = fmin;
	srec->fmax = fmax;
	srec->badpix = badpix;
	if (ngood)
		fbar /= (float) ngood;

	samplesize = DEFSAMPLESIZE;		/* take a random sample */
	if (samplesize > 0.1 * N1 * N2)
		samplesize = floor(0.1 * N1 * N2);
	fsample = (float *) calloc(samplesize, sizeof(float));
	for (isample = 0; isample < samplesize; isample++) {
		gotagoodone = 0;
		while (!gotagoodone) {
			i = margin + floor((N2 - 2 * margin - 1) *
                                           rand()/((double)RAND_MAX + 1));
			j = margin + floor((N1 - 2 * margin - 1) *
                                           rand()/((double)RAND_MAX + 1));
			if (i < margin || i >= N2 - margin || j < margin || j >= N1 - margin) {
				fprintf(stderr, "%d %d\n", i, j);
				error_exit("do_stats: this cannot happen 1\n");
			}
			if (good(f[i][j])) {
				fsample[isample] = (float) f[i][j];
				gotagoodone = 1;
			}
		}
	}
	if (liststats(fsample, samplesize, &mode, &median, &lquart, &uquart, &sigma))
	fprintf(stderr, "do_stats: warning: possible problems with mode\n");
	srec->flowerquartile = floor(0.5 + lquart);
	srec->fupperquartile = floor(0.5 + uquart);;
	srec->fmedian = floor(0.5 + median);
	srec->fmode = mode;
	srec->sigma = sigma;
	free(fsample);
}


void		fdo_stats(float **f, int N1, int N2, int margin, fstatsrec *srec)
{
	int			i, j, isample, ngood, badpix, samplesize, gotagoodone;
	float		fmin, fmax, fbar;
	float		mode, median, lquart, uquart, sigma;
	float		*fsample;
	
	fmin = HUGE_VAL_FLOAT; 		/* find max & min */
	fmax = -HUGE_VAL_FLOAT;
	fbar = 0;
	ngood = 0;
	badpix = 0;
	for (i = margin; i < N2 - margin; i++)							
		for (j = margin; j < N1 - margin; j++)	{
			if (fgood(f[i][j])) {
				fmax = (f[i][j] > fmax ? f[i][j] : fmax);
				fmin = (f[i][j] < fmin ? f[i][j] : fmin);
				ngood++;
				fbar += f[i][j];
			} else {
			badpix++;
			}
		}
	srec->fmin = fmin;
	srec->fmax = fmax;
	srec->badpix = badpix;
	if (ngood)
		fbar /= (float) ngood;

	samplesize = DEFSAMPLESIZE;		/* take a random sample */
	if (samplesize > 0.1 * N1 * N2) {
		samplesize = floor(0.1 * N1 * N2);
        }
	fsample = (float *) calloc(samplesize, sizeof(float));
	for (isample = 0; isample < samplesize; isample++) {
		gotagoodone = 0;
		while (!gotagoodone) {
			i = margin + floor((N2 - 2 * margin - 1) *
                                           rand()/((double)RAND_MAX + 1));
			j = margin + floor((N1 - 2 * margin - 1) *
                                           rand()/((double)RAND_MAX + 1));
			if (i < margin || i >= N2 - margin || j < margin || j >= N1 - margin) {
				error_exit("fdo_stats: this cannot happen 1\n");
			}
			if (fgood(f[i][j])) {
				fsample[isample] = f[i][j];
				gotagoodone = 1;
			}
		}
	}
	if (liststats(fsample, samplesize, &mode, &median, &lquart, &uquart, &sigma))
		fprintf(stderr, "fdo_stats:warning: possible problems with mode\n");
	srec->flowerquartile = lquart;
	srec->fupperquartile = uquart;
	srec->fmedian = median;
	srec->fmode = mode;
	srec->sigma = sigma;
	free(fsample);
}


int			liststats(	float 	*fsample,
						int		samplesize,
						float	*modeptr,
						float	*medianptr,
						float	*lquartptr,
						float	*uquartptr,
						float	*sigmaptr)
{
	int 	floatcmp();
	int		iplus, iminus, uppi, lowi, medi, imod;
	float	mode, lquart, uquart, median;
	int	returnvalue;
	
	lowi = floor(0.5 + 0.25 * samplesize);
	medi = floor(0.5 + 0.50 * samplesize);
	uppi = floor(0.5 + 0.75 * samplesize);
	qsort(fsample, samplesize, sizeof(float), floatcmp);
	*lquartptr = lquart = fsample[lowi];
	*medianptr = median = fsample[medi];
	*uquartptr = uquart = fsample[uppi];

	if (uquart != lquart) {
		returnvalue = findmode(fsample, samplesize, lquart, uquart, &mode);
		*modeptr = mode;
	} else {
		*modeptr = median;
		returnvalue = 1;
	}
	
	/* now estimate sigma from the range about mode which contains 25 percent */
	for (imod = 0; imod < samplesize; imod++)
		if (fsample[imod] > *modeptr)
			break;
	imod--;
	iplus = imod + floor(0.5 + 0.125 * samplesize);
	iminus = imod - floor(0.5 + 0.125 * samplesize);
	if (iplus >= 0 && iplus < samplesize && iminus >= 0
		&& iminus < samplesize && iplus != iminus)
		*sigmaptr = (fsample[iplus] - fsample[iminus]) / (2 * 0.32);
	else
		*sigmaptr = (uquart - lquart) / (2 * 0.67);
	return (returnvalue);
}




int		floatcmp(float *f1, float *f2)
{
	return (*f1 > *f2 ? 1 : (*f1 == *f2 ? 0 : -1));	/* ascending order */
}

#define	BINS 		100
#define SIG_FACTOR 	1.0
#define BIG_FACTOR	4

/*
 * - estimate the mode given sorted array of values f with median, quartiles.
 * - works by making a histogram of counts over range BIG_FACTOR * central
 *	50 percential range.
 *	Smooths with gaussian width ca SIG_FACTOR * sigma (where sigma is
 *	estimated from the quartiles.
 */
int		findmode(float *f, int nel, float lquart, float uquart, float *mode)
{
	float 	fmin, fmax;
	long	count[BINS];
	int		index, i, b, db, bmax, dbmax;
	float	smoothcount[BINS], smoothcountmax, df, fs, crudesigma;
	float	*expon;
	int	returnvalue;

	if (uquart <= lquart) {
		*mode = 0.5 * (uquart + lquart);
		return (1);
	}
	crudesigma = (uquart - lquart) / (2 * 0.67);
	fs = SIG_FACTOR * crudesigma;
	fmin = 0.5 * (uquart + lquart) - BIG_FACTOR * crudesigma;
	fmax = 0.5 * (uquart + lquart) + BIG_FACTOR * crudesigma;
	df = (fmax - fmin) / BINS;
	dbmax = 3 * fs / df;	/* smoothing window extends to 3-sigma */
	
	expon = (float *) calloc(2 * dbmax + 1, sizeof(float)) + dbmax;
	for (db = - dbmax ; db <= dbmax; db++) {
		expon[db] = exp (-0.5 * df * df * db * db / (fs * fs));
	}	
	
	for (b = 0; b < BINS; b++) 		/* initialise arrays */
		smoothcount[b] = count[b] = 0;
	
	for (i = 0; i < nel; i++) {			/* make counts */
		index = floor(0.5 + (f[i] - fmin) / df);
		if (index >= 0 && index < BINS)
			count[index]++;
	}
	
	for (b = 0; b < BINS; b++) {		/* smooth counts */
		for (db = - dbmax ; db <= dbmax; db++) {
			if (b + db >= 0 && b + db < BINS)
				smoothcount[b] += count[b + db] * expon[db];
		}	
	}

	smoothcountmax = bmax = 0;				/* find peak of smoothed counts */
	for (b = 0; b < BINS; b++) {
		if (smoothcount[b] > smoothcountmax) {
			smoothcountmax = smoothcount[b];
			bmax = b;
		}
	}
	if (bmax > 0 && bmax < BINS - 1) {		/* seems to have worked */
		*mode = fmin + df * bmax;
		returnvalue = 0;
	} else {
		*mode = 0.5 * (uquart + lquart);
		returnvalue = 1;
	}
	free(expon - dbmax);
	return (returnvalue);
}


#undef BINS
#undef SIG_FACTOR
#undef BIG_FACTOR
#undef DEFSAMPLESIZE
#undef HUGE_VAL



