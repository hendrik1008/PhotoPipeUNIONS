/*
 * fits.h
 *
 * functions to implement FITS I/O
 */

/*
   16.01.2005:
   I removed compiler warnings (gcc with -W -pedantic -Wall)

   28.01.2005:
   - I added a new function 'get_basename' to extract the
     basename from a path.
   - change of argument list from argsToString to match
     modifications described in fits.c

   07.02.2005:
   I introduced a new function 'delfitsheader' to cleanly
   free memory associated with fitsheaders allocated in
   'readfitsheader' and 'new2Dfitsheader'.

   04.06.2007:
   Changes to include Large File Support to the LDAC tools:
   On 32-bit systems calls to fseek are changed to fseeko
   which takes an argument of type 'off_t' as offset argument.
   This argument can be turned into a 64 bit type with compiler
   flags (_FILE_OFFSET_BITS). Similarily the command fseek is
   changed to fseeko on 32-bit machines. The types of concerned
   variables are changed accordingly.

   24.08.2010:
   I test for _FILE_OFFSET_BITS being defined now tih '#ifdef' instead
   of '#if'. The last gived compiler remarks with the Intel 'icc'
   compiler.
*/

/* sys/types.h is needed for the definition of 'off_t'
   (needed for Large File Support)
*/

#include <sys/types.h>

#define MAX_FITS_DIM	7
#define COM_LENGTH      80
#define COM_LENGTH1     81
#define NAME_LENGTH	8
#define NAME_LENGTH1	9
#define VALUE_LENGTH	70
#define VALUE_LENGTH1	71

#define FITS_REC_SIZE	2880

#define UCHAR_PIXTYPE	8
#define SHORT_PIXTYPE   16
#define INT_PIXTYPE     32
#define FLOAT_PIXTYPE   -32
#define DBL_PIXTYPE     -64

#define UCHAR_MAGIC   UCHAR_MAX
#define SHORT_MAGIC   SHRT_MIN
#define INT_MAGIC     INT_MIN
#define FLOAT_MAGIC   (-((float)  0x80000000) * ((float)  0x80000000))
#define DBL_MAGIC     (-((double) 0x80000000) * ((double) 0x80000000))

#define BIG_ENDIAN_BYTE_ORDER		0
#define LITTLE_ENDIAN_BYTE_ORDER	1
#ifdef LITTLEENDIAN
#define	NATIVE_BYTE_ORDER	LITTLE_ENDIAN_BYTE_ORDER
#define	NON_NATIVE_BYTE_ORDER	BIG_ENDIAN_BYTE_ORDER
#else
#define	NATIVE_BYTE_ORDER	BIG_ENDIAN_BYTE_ORDER
#define	NON_NATIVE_BYTE_ORDER	LITTLE_ENDIAN_BYTE_ORDER
#endif

/* definitions for Large File Support */
#ifdef _LARGEFILE_SOURCE
#define	FSEEKO	fseeko
#define	FTELLO	ftello
#else
#define	FSEEKO	fseek
#define	FTELLO	ftell
#endif

#ifdef _FILE_OFFSET_BITS
#define OFF_T	off_t
#else
#define OFF_T	long
#endif


/* We define the following structures for headers
 *
 * null terminated doubly linked lists of comments
 *
 */

typedef struct fitscomment {
	char			name[NAME_LENGTH1];
    /* The '+2' comes from COMMENTS or HISTORY keywords */
	char			value[VALUE_LENGTH1+2];
	struct fitscomment 	*next, *prev;
} fitscomment;

/* the header, containing the information needed to convert from
 * disk to internal format and vice versa, and the comments.
 */

typedef struct fitsheader {
	int			extpixtype;
	int			intpixtype;		/* defaults to FLOAT_PIXTYPE */
	int			ndim;
	int			n[MAX_FITS_DIM];
	int			bscaling;
	double			bscale,  bzero;
	FILE			*ipstream, *opstream;	/* default to stdin, stdout */
	char			*linebuffer;
	int			linebuffersize;	
	int			ipbyteorder, opbyteorder;
	int			convertnans;
	struct fitscomment 	*basecomment;
	int			hasextensions, isextension, nextensions, pcount, gcount;
} fitsheader;



fitsheader 	*readfitsheader(FILE *stream);
fitsheader 	*copyfitsheader(fitsheader *theheader);
int		writefitsheader(fitsheader *theheader);
int		writefitstail(fitsheader *theheader);
int		copycommenttostring(fitscomment *thecomment, char *string);
fitscomment	*newtextcomment(char *name, char *val, char *comment);
fitscomment	*newnumericcomment(char *name, double value, char *comment);
fitscomment	*getcommentbyname(char *name, fitsheader *theheader);
double		getnumericvalue(fitscomment *thecomment);
char		*gettextvalue(fitscomment *thecomment);
void		readfitsline(void *f, fitsheader *theheader);
void		writefitsline(void *f, fitsheader *theheader);
void		readfitsplane(void **f, fitsheader *theheader);
void		writefitsplane(void **f, fitsheader *theheader);
void		readfitscube(void ***f, fitsheader *theheader);
void		writefitscube(void ***f, fitsheader *theheader);
void		removecomment(fitscomment *thecomment, fitsheader *fitshead);
void		removenamedcomments(char *name, fitsheader *fitshead);
int		pixsize(int pixtype);

int		read2Dfloatimage(float ***f, int *N1, int *N2, fitsheader **fits, FILE *stream);
/* int		read2Dfloatimage_shm(float ***f, int *N1, int *N2, fitsheader **fits, FILE *stream); */
int		write2Dfloatimage(float **f, fitsheader *fits);
fitsheader 	*newfitsheader(int ndim, int *dim, int extpixtype);
fitsheader 	*new2Dfitsheader(int N1, int N2, int extpixtype);
void            delfitsheader(fitsheader *aheader);

char		*argsToString(int argc, char **argv, int basename);
void		add_comment(int argc, char **argv, fitsheader *fitshead);
void		appendcomment(fitscomment *newcomment, fitsheader *fitshead);
void		prependcomment(fitscomment *newcomment, fitsheader *fitshead);
void		setextpixtype(fitsheader *fits, int pixtype);
void		set2Dimagesize(fitsheader *fits, int N1, int N2);
OFF_T		skiplines(fitsheader *fits, int nlines);
int		updatelinebuffer(fitsheader *theheader);

int		byteswapline(void *data, int nel, int pixsize);
int		convertmagictonans(void	*data, int nel, int pixsize);
int		convertnanstomagic(void *data, int nel, int pixsize);
/* Find out the base name of a file (i.e. without prefix path) */
char            *get_basename(const char *filename);
