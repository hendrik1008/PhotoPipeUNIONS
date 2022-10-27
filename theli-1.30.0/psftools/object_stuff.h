#define	GC_MAX		32
#define VERSION_NUMBER	1
#define MAX_CAT_COMMENTS 256
#define CAT_COM_LENGTH	80

#define	NORTH	0
#define SOUTH 1
#define	EAST	2
#define WEST	3

#define BAD		0
#define STAR	1
#define GAL		2

/* empirical factors to convert rg => rh, fs => mag */
#define	RFACTOR		0.66
#define FFACTOR 	15.41
#define EFACTOR 	1.50

#define	DEFAULT_ZEROPOINT 30

typedef struct skyquad {
	int	n[4];			/* count of pixels used */
	float	f[4];			/* median values */
	float	i[4];			/* centre of mass i */
	float	j[4];			/* centre of mass j */
} skyquad;


typedef struct object {
	int		i, j;			/* peak position */
	float   xbad[2];                        /* "first guess" centroid position from hfindpeaks */
        float   xgood[2];                       /* good centroid position from analyse */
	float	nu, fs;				/* peak parameters */
	float	rg;				/* radius assigned by hcat2cat */
	float	Psm12;				/* smear polarizability */
	int	rnumax;				/* radius of max significance */
	float	numax;				/* max significance */
	int	rmax;				/* radius for pixels used */
	float	rh;				/* half light radius */
	float	l;				/* luminosity */
	float	lg;				/* hfindpeaks luminosity */
	float	Psh11;				/* shear polarizability */
	float	Psh22;				/* shear polarizability */
	float   e[2];				/* ellipticity components */
	float   esigma[2];			/* ellipticity uncertainty */
	skyquad	sky;				/* NSEW sky parameters */
	int	nmagic;				/* number of MAGIC pixels */
	int	ncoinc;				/* number of coincidences */
	int	class;				/* classification */
	struct object *prev, *next;		/* used by aggregate.c */
	int status;				/* flag various bad things */
	int		ispare[4];		/* espansion slots */
	float	invr2;
	float	Psm11;				/* smear polarizability */
	float	Psm22;				/* smear polarizability */
	float	Psh12;				/* shear polarizability */
} object;

typedef struct cluster {
	int		nobjs, sumi, sumj;
	float	icentroid, jcentroid;
	struct cluster *prev, *next;
	struct object *baseobjectptr;
	object	*mainobjectptr;
} cluster;

typedef struct cathead {
	char		image[128];
	float		nu, rf, f_thresh;
	fstatsrec	srec;
	int		N;
	int		version_number;
	float		rpsf;
	int		comc;			/* comment count */
	char		com[MAX_CAT_COMMENTS][CAT_COM_LENGTH];
	int		hassky;			/* saves repeated sky determinations */
	int		ispare[4];		/* espansion slots */
	float		zeropoint;
	float		fspare[3];
} cathead;
	
	
	


#ifndef	PI
#define	PI 3.14159
#endif






