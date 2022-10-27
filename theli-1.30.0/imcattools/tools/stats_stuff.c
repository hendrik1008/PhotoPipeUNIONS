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
**  calculates sigma from width of inner 25 percent range
** Fri Aug  7 09:29:37 EDT 1992
**		do_stats also converted to sampling technique
**		mode determined by smoothed histogram method
**		incorporates findmode() - formerly in analyse.c
**		general purpose routine liststats() to return
**		mode, medians, quartiles from an unsorted list.
**
**/

/*
 * 16.01.2005:
 * - I removed the not needed ran1 function definition.
 * - I substituted HUGE_VAL by FLT_MAX
 *
 * 31.01.2005:
 * I changed the arguments for fdo_stats: instead of the margin
 * we now have to give the full rectangle in x and y and also
 * thressholds which pixel values are to be considered.
 *
 * 14.02.2005:
 * - function fdo_stats:
 *   The N1, N2 values used in this function were replaced by
 *   the dimesnions of the image really used for statistics.
 * - function findmode:
 *   I changed the arguments so that the pixel sample collected in
 *   fdo_stats is used instead of the 'original' pixel data
 *
 * 17.04.2010:
 * - function fdo_stats:
 *   the mean of images is estimated with double arithemtics. Float
 *   turned out to be too inaccurate for already moderately large images
 *   (2k x 4k) (Note: the mean is estimated with pixels of the whole
 *   image; median, mode etc. are only estimated on a random subsample)
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#include        "stats_stuff.h"
#include        "error.h"
#include        "fits.h"

#define good(x)    (x != SHRT_MIN && x != SHRT_MAX && x != SHORT_MAGIC)

#define DEFSAMPLEFRAC      0.1
#define MAX_SAMPLE_SIZE    10000

void fdo_stats(float **f, int N1, int N2, int xmin, int xmax,
               int ymin, int ymax, float lo_rej, float hi_rej,
               fstatsrec *srec)
{
  int    i, j, ngood, badpix, step;
  int    actualsize, samplesize;
  float  fmin, fmax, samplefrac;
  double fbar; /* a float oerflows for large images */
  float  mode, median, lquart, uquart, sigma;
  float *fsample, flquart;
  int    N1real, N2real;

  fmin   = FLT_MAX;                     /* find max & min */
  fmax   = -FLT_MAX;
  fbar   = 0.;
  ngood  = 0;
  badpix = 0;

  /* sanity checks for boundaries */
  if (xmin < 0 || ymin < 0 || xmax > N1 || ymax > N2 ||
      xmax < xmin || ymax < ymin) {
    error_exit("Error in range parameters xmin, xmax, ymin, ymax: aborting");
  }

  N1real = (xmax - xmin);
  N2real = (ymax - ymin);

  for (i = ymin; i < ymax; i++) {
    for (j = xmin; j < xmax; j++) {
      if ((f[i][j] > lo_rej) && (f[i][j] < hi_rej)) {
        fmax = (f[i][j] > fmax ? f[i][j] : fmax);
        fmin = (f[i][j] < fmin ? f[i][j] : fmin);
        ngood++;
        fbar += f[i][j];
      } else {
        badpix++;
      }
    }
  }

  srec->badpix  = badpix;
  srec->goodpix = ngood;

  if (ngood) {
    srec->fmin  = fmin;
    srec->fmax  = fmax;
    fbar       /= (double)ngood;
    srec->fmean = (float)fbar;
  } else {
    srec->fmin           = 0.0;
    srec->fmax           = 0.0;
    srec->fmean          = 0.0;
    srec->flowerquartile = 0.0;
    srec->fupperquartile = 0.0;
    srec->fmedian        = 0.0;
    srec->fmode          = 0.0;
    srec->sigma          = 0.0;
    srec->samplesize     = 0;
    return;
  }
  samplefrac = DEFSAMPLEFRAC;                   /* take a random sample */
  samplesize = (int)ceil(samplefrac * N1real * N2real);
  samplesize = (samplesize > MAX_SAMPLE_SIZE ? MAX_SAMPLE_SIZE : samplesize);

  if (samplesize > ngood) {
    step = 1;
  } else {
    step = (int)floor(sqrt(ngood / (double)samplesize));
  }
  samplesize = (int)(2 + N1real / step) * (2 + N2real / step);

  fsample    = (float *)calloc(samplesize, sizeof(float));
  actualsize = 0;

  for (i = ymin; i < ymax; i += step) {
    for (j = xmin; j < xmax; j += step) {
      if ((f[i][j] > lo_rej) && (f[i][j] < hi_rej)) {
        fsample[actualsize++] = f[i][j];
      }
      if (actualsize >= samplesize) {
        break;
      }
    }
    if (actualsize >= samplesize) {
      break;
    }
  }

  srec->samplesize = samplesize = actualsize;

  if (liststats(fsample, samplesize, &median, &lquart, &uquart, &sigma)) {
    fprintf(stderr, "fdo_stats: warning: problem with stats from sample\n");
  }
  findmode(fsample, samplesize, lquart, uquart, &mode, &flquart);
  srec->flowerquartile = lquart;
  srec->fupperquartile = uquart;
  srec->fmedian        = median;
  srec->fmode          = mode;
  /* empirical fudge */
  srec->sigma = 1.49 * (mode - flquart);
  free(fsample);
}

int liststats(float *fsample,
              int samplesize,
              float *medianptr,
              float *lquartptr,
              float *uquartptr,
              float *sigmaptr)
{
  int     floatcmp();

  int   uppi, lowi;
  float lquart, uquart, median;
  int   returnvalue = 0;

  lowi = floor(0.5 + 0.25 * samplesize);
  uppi = floor(0.5 + 0.75 * samplesize);
  qsort(fsample, samplesize, sizeof(float), floatcmp);
  *lquartptr = lquart = fsample[lowi];
  if (samplesize % 2) {
    *medianptr = median = fsample[samplesize / 2];
  } else {
    *medianptr = median = 0.5 *
      (fsample[samplesize / 2] + fsample[samplesize / 2 - 1]);
  }
  *uquartptr = uquart = fsample[uppi];

  /* crude estimate of sigma from quartiles */
  *sigmaptr = (uquart - lquart) / (2 * 0.67);
  return(returnvalue);
}

int floatcmp(float *f1, float *f2)
{
  return(*f1 > *f2 ? 1 : (*f1 == *f2 ? 0 : -1));        /* ascending order */
}

#define BINS          1000
#define SIG_FACTOR    1.0
#define BIG_FACTOR    4

/*
 * - estimate the mode given sorted array of values f with median, quartiles.
 * - works by making a histogram of counts over range BIG_FACTOR * central
 *	50 percential range.
 *	Smooths with gaussian width ca SIG_FACTOR * sigma (where sigma is
 *	estimated from the quartiles.
 *
 * we also return 'flquart' which is the value such that equally many
 * pixels lie in f < flquart and flquart < f < mode, which is used to give a better sigma..
 *
 */
int findmode(float *f, int nf, float lquart, float uquart,
             float *mode, float *flquart)
{
  float  fmin, fmax;
  long   count[BINS];
  int    index, i, b, db, bmax, dbmax, submodecount = 0, tempcount = 0;
  float  smoothcount[BINS], smoothcountmax, df, fs, crudesigma;
  float *expon;
  int    returnvalue;

  if (uquart <= lquart) {
    *mode    = 0.5 * (uquart + lquart);
    *flquart = *mode;
    return(1);
  }
  crudesigma = (uquart - lquart) / (2 * 0.67);
  fs         = SIG_FACTOR * crudesigma;
  fmin       = 0.5 * (uquart + lquart) - BIG_FACTOR * crudesigma;
  fmax       = 0.5 * (uquart + lquart) + BIG_FACTOR * crudesigma;
  df         = (fmax - fmin) / BINS;
  dbmax      = 3 * fs / df;     /* smoothing window extends to 3-sigma */

  expon = (float *)calloc(2 * dbmax + 1, sizeof(float)) + dbmax;
  for (db = -dbmax; db <= dbmax; db++) {
    expon[db] = exp(-0.5 * df * df * db * db / (fs * fs));
  }

  for (b = 0; b < BINS; b++) {                  /* initialise arrays */
    smoothcount[b] = count[b] = 0;
  }

  /* make counts */
  for (i = 0; i < nf; i++) {
    index = floor(0.5 + (f[i] - fmin) / df);
    if (index >= 0 && index < BINS) {
      count[index]++;
    }
  }

  /* smooth counts */
  for (b = 0; b < BINS; b++) {
    for (db = -dbmax; db <= dbmax; db++) {
      if (b + db >= 0 && b + db < BINS) {
        smoothcount[b] += count[b + db] * expon[db];
      }
    }
  }

  smoothcountmax = bmax = 0;  /* find peak of smoothed counts */
  for (b = 0; b < BINS; b++) {
    if (smoothcount[b] > smoothcountmax) {
      smoothcountmax = smoothcount[b];
      bmax           = b;
    }
  }
  if (bmax > 0 && bmax < BINS - 1) { /* seems to have worked */
    *mode = fmin + df * bmax;
    /* compute flquart */
    for (b = 0; b < bmax; b++) {
      submodecount += count[b];
    }
    submodecount += count[bmax] / 2;
    for (b = 0; b < bmax; b++) {
      tempcount += count[b];
      if (tempcount > 0.5 * submodecount) {
        break;
      }
    }
    *flquart    = fmin + df * b;
    returnvalue = 0;
  } else {
    *mode       = 0.5 * (uquart + lquart);
    *flquart    = lquart;
    returnvalue = 1;
  }

  free(expon - dbmax);
  return(returnvalue);
}

#undef BINS
#undef SIG_FACTOR
#undef BIG_FACTOR
#undef DEFSAMPLESIZE
