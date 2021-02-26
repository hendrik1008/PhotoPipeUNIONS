/* 30.01.2005:
 * I changed the arguments for fdo_stats: instead of the margin
 * we now have to give the full rectangle in x and y and also
 * thressholds which pixel values are to be considered.
 *
 * 14.02.2005:
 * I changed the arguments of findmode to take a one dimensional
 * pixel array instead of a two-dimensional one.
 */

#ifndef __STATS_STUFF_H__
#define __STATS_STUFF_H__

typedef struct fstatsrec {
	float 	fmin;
	float 	fmax;
	float 	fmean;
	float 	fmedian;
	float 	fupperquartile;
	float 	flowerquartile;
	float 	fmode;
	float 	sigma;
	long	badpix;
	long	goodpix;
	long	samplesize;
} fstatsrec;

/*
void		do_stats(short **f, int N1, int N2, int margin, statsrec *srec);
*/
void		fdo_stats(float **f, int N1, int N2, int xmin, int xmax,
                          int ymin, int ymax, float lo_rej, float hi_rej,
                          fstatsrec *fsrec);

int		liststats(	float 	*fsample,
				int	samplesize,
				float	*median,
				float	*lquart,
				float	*uquart,
				float	*sigma);
int		findmode(float *f, int nf, float lquart, float uquart, float *mode, float*flquart);

#endif

