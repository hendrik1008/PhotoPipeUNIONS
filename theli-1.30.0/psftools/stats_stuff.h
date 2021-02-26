typedef struct statsrec {
	short 	fmin;
	short 	fmax;
	short 	fmedian;
	short 	fupperquartile;
	short 	flowerquartile;
	float 	fmode;
	float 	sigma;
	long	badpix;
} statsrec;

typedef struct fstatsrec {
	float 	fmin;
	float 	fmax;
	float 	fmedian;
	float 	fupperquartile;
	float 	flowerquartile;
	float 	fmode;
	float 	sigma;
	long	badpix;
} fstatsrec;

void		do_stats(short **f, int N1, int N2, int margin, statsrec *srec);
void		fdo_stats(float **f, int N1, int N2, int margin, fstatsrec *fsrec);
int		liststats(	float 	*fsample,
				int	samplesize,
				float	*mode,
				float	*median,
				float	*lquart,
				float	*uquart,
				float	*sigma);
int		findmode(float *f, int nel, float lquart, float uquart, float *mode);



