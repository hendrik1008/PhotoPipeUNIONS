/*
 * fits.c
 *
 * functions to implement fits I/O
 */

/* 14.01.03:
   removed adding a BYTEORDER keyword to headers
   as files are ALWAYS written in big endian format.

   19.04.03:
   removed the BYTEORDER keyword required commenting
   a further line. Otherwise the last written keyword
   was written again to the header.

   14.01.04:
   removed compiler warnings under Linux

   02.07.2004:
   corrected an error in the padding of imaging data.
   The fill bytes there have to be zeros, not blanks.

   06.07.2004:
   I changed back the formula for the calculation of
   the pad length (I changed this together with the
   null byte correction on 02.07.).
   The new formula gave files that were too short
   for one element.

   16.01.2005:
   I removed compiler warnings (gcc with -W -pedantic -Wall)

   27.01.2005:
   If HISTORY or COMMENT keywords are written to a FITS Header,
   the '=' on the 9th position is no longer written.

   28.01.2005:
   function argsToString:
   - the function now returns a pointer to a new string
     that has to be freed manually.
   - the returned string is just a compilation of all the
     individual arguments given.
   - the function now has a flag 'basename'. If set to 1
     the basename from the first argument is determined.
     It is usually a program name.

   function add_comment:
   if the HISTORY string exceeds the allowed number for
   a single HISTORY line it is now split into several
   HISTORY lines instead of cut off.

   I added a new function 'get_basename' to extract the
   basename from a path.

   01.02.2005:
   I added a sanity check in readfitsheader to
   prevent strange behaviours with MEF files. We quit
   if the dimension of the given FITS file is not two.

   07.02.2005:
   - I added memory tracing capabilities by the inclusion
     of 'xmemory'.
   - I introduced a new function 'delfitsheader' to cleanly
     free memory associated with fitsheaders allocated in
     'readfitsheader' and 'new2Dfitsheader'.
   - I removed memory leaks at several places (mainly concerning
     'fitscomments)
   - I corrected an error in the freeing of comments in 'removecomment'.
     It was not done correctly if the base comment had to be removed
     from the linked list.

   13.09.05:
   In function readfitsheader I rewind the file before starting
   to read the header. There were problems when reading the
   header and calling read2Dfloatimage afterwards as this
   function again reads the header.

   04.06.2007:
   Changes to include Large File Support to the LDAC tools:
   On 32-bit systems calls to fseek are changed to fseeko
   which takes an argument of type 'off_t' as offset argument.
   This argument can be turned into a 64 bit type with compiler
   flags (_FILE_OFFSET_BITS). Similarily the command fseek is
   changed to fseeko on 32-bit machines. The types of concerned
   variables are changed accordingly.

   15.10.2007:
   I corrected the formula for calculating the number of bytes to pad
   to a new FITS image. The old formula was wrong if the image size
   just resulted in a complete multiple of FITS blocks (2880 bytes).

   24.08.2010:
   I removed several compiler remarks from the Intel icc compiler
   (mainly unclean float and double comparisons)

   03.04.2013:
   xmemory is no longer used by default but only if the variable
   __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
   etc. calls are used for memory management.

   28.05.2014:
   Output images now always contain BSCALE and BZERO header keywords.
   Up to now they were not added for float images or if these values were
   the default (BSCALE=1 and BZERO=0). The modification makes imcat
   consistent with the rest of the pipeline.
 */

/* TODOS AND NOTES:
   The function newnumericcomment to add numeric FITS header keywords is
   VERY UNCLEAN. It does not at all allow propert treatment and distiction
   of FLOAT and INT header keywords.
 */

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#include "fits.h"
#include "convertarray.h"
#include "error.h"
#include "arrays.h"
#include "nan.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

/* readfitsheader() reads a fits header from stream.
 *
 * intpixtype is set to FLOAT_PIXTYPE by default
 *
 */
fitsheader *readfitsheader(FILE *stream)
{
  fitsheader  *fitshead;
  fitscomment *newcomment, *thecomment, *copycomment;
  char         record[FITS_REC_SIZE], string[81], *textval;
  int          line, recorddone, alldone = 0, bad, dim, i;

  /* allocate space for the header structure */
  fitshead = (fitsheader *)calloc(1, sizeof(fitsheader));
  if (!fitshead) {
    error_exit("readfitsheader: failed to allocate space for header\n");
  }

  /* set default values for fitshead */
  fitshead->ipstream    = stream;
  fitshead->opstream    = stdout;
  fitshead->linebuffer  = NULL;
  fitshead->basecomment = NULL;
  copycomment           = NULL;

  /* read the comments into a null terminated, double linked list of
     fitscomments */
  thecomment = NULL;
  /* rewind the file; headers are always at the start !! */
  FSEEKO(stream, (OFF_T)0, SEEK_SET);

  while (!alldone) {
    if (FITS_REC_SIZE != fread(record, sizeof(char), FITS_REC_SIZE, stream)) {
      error_exit("readfitsheader: malformed header\n");
    }
    line = recorddone = 0;
    while (!recorddone) {
      newcomment = (fitscomment *)calloc(1, sizeof(fitscomment));
      if (!newcomment) {
        error_exit("readfitsheader: failed to allocate space for fits header comment\n");
      }
      if (!thecomment) {
        fitshead->basecomment = thecomment = newcomment;
        /* keep a copy for a later clean up */
        copycomment = fitshead->basecomment;
      } else {
        thecomment->next = newcomment;
        newcomment->prev = thecomment;
        thecomment       = newcomment;
      }
      strncpy(thecomment->name, record + line * COM_LENGTH, NAME_LENGTH);
      if (strncmp(thecomment->name, "HISTORY", 7) == 0 ||
          strncmp(thecomment->name, "COMMENT", 7) == 0) {
        strncpy(thecomment->value,
                record + line * COM_LENGTH + NAME_LENGTH, VALUE_LENGTH + 2);
      } else {
        strncpy(thecomment->value,
                record + line * COM_LENGTH + NAME_LENGTH + 2, VALUE_LENGTH);
      }
      if (!strncmp(thecomment->name, "END     ", NAME_LENGTH)) {
        alldone = recorddone = 1;
        (newcomment->prev)->next = NULL;
        free(thecomment);
      }
      line++;
      if (line * COM_LENGTH == FITS_REC_SIZE) {
        recorddone = 1;
      }
    }
  }

  /* check/get the required entries */

  /* SIMPLE/XTENSION line */
  bad = 0;
  if (!(thecomment = fitshead->basecomment)) {
    bad = 1;
  }

  if (!bad) {
    if (strncmp(thecomment->name, "SIMPLE  ", NAME_LENGTH)) {
      if (strncmp(thecomment->name, "XTENSION", NAME_LENGTH)) {
        bad = 1;
      } else {
        fitshead->isextension = 1;
      }
    }
  }
  if (!bad) {
    textval = gettextvalue(thecomment);
    if (fitshead->isextension) {
      if (strncmp(textval, "IMAGE", 5)) {
        bad = 1;
      }
    } else {
      if (strncmp(textval, "T", 1)) {
        bad = 1;
      }
    }
    free(textval);
  }
  if (bad) {
    error_exit("readfitsheader: bad SIMPLE/XTENSION line\n");
  }

  /* BITPIX line */
  if (!(thecomment = thecomment->next)) {
    bad = 1;
  }
  if (!bad) {
    if (strcmp(thecomment->name, "BITPIX  ")) {
      bad = 1;
    }
  }
  if (!bad) {
    if (1 != sscanf(thecomment->value, "%d", &(fitshead->extpixtype))) {
      bad = 1;
    }
    if (fitshead->extpixtype != UCHAR_PIXTYPE &&
        fitshead->extpixtype != SHORT_PIXTYPE &&
        fitshead->extpixtype != FLOAT_PIXTYPE &&
        fitshead->extpixtype != INT_PIXTYPE &&
        fitshead->extpixtype != DBL_PIXTYPE) {
      bad = 1;
    }
  }
  if (bad) {
    error_exit("readfitsheader: bad BITPIX line\n");
  }

  /* NAXIS line */
  if (!(thecomment = thecomment->next)) {
    bad = 1;
  }
  if (!bad) {
    if (strcmp(thecomment->name, "NAXIS   ")) {
      bad = 1;
    }
  }
  if (!bad) {
    if (1 != sscanf(thecomment->value, "%d", &(fitshead->ndim))) {
      bad = 1;
    }
    if (fitshead->ndim >= MAX_FITS_DIM) {
      bad = 1;
    }
  }
  if (bad) {
    error_exit("readfitsheader: bad NAXIS line\n");
  }

  /* sanity check on dimensions: we only support 2-d files
     for the moment
   */
  if (fitshead->ndim != 2) {
    error_exit("Only 2-dimensional FITS files with no extension are supported\n");
  }

  /* axis dimensions */
  for (dim = 0; dim < fitshead->ndim; dim++) {
    if (!(thecomment = thecomment->next)) {
      bad = 1;
    }
    sprintf(string, "NAXIS%-3d", dim + 1);
    if (strcmp(thecomment->name, string)) {
      bad = 1;
    }
    if (1 != sscanf(thecomment->value, "%d", fitshead->n + dim)) {
      bad = 1;
    }
    if (bad) {
      error_exit("readfitsheader: bad NAXIS? line\n");
    }
  }

  /* make fitshead->basecomment point at the next comment */
  fitshead->basecomment = thecomment->next;

  /* free all comments that we just skipped and do not reference any more */
  /* the 3 is for SIMPLE/XTENSION, BITPIX and NAXIS */
  for (i = 0; i < 3 + fitshead->ndim; i++) {
    copycomment = copycomment->next;
    free(copycomment->prev);
  }

  if (fitshead->basecomment) {
    (fitshead->basecomment)->prev = NULL;
  }

  /* deal with extensions */
  if (!(fitshead->isextension)) {
    if ((thecomment = getcommentbyname("EXTEND", fitshead))) {
      fitshead->hasextensions = 1;
      removecomment(thecomment, fitshead);
      if ((thecomment = getcommentbyname("NEXTEND", fitshead))) {
        fitshead->nextensions = (int)getnumericvalue(thecomment);
        removecomment(thecomment, fitshead);
      }
    }
  } else {
    if ((thecomment = getcommentbyname("PCOUNT", fitshead))) {
      fitshead->pcount = (int)getnumericvalue(thecomment);
      if (fitshead->pcount) {
        error_exit("readfitsheader: I can only deal with PCOUNT=0 FITS files\n");
      }
      removecomment(thecomment, fitshead);
    }
    if ((thecomment = getcommentbyname("GCOUNT", fitshead))) {
      fitshead->gcount = (int)getnumericvalue(thecomment);
      if (fitshead->gcount != 1) {
        error_exit("readfitsheader: I can only deal with GCOUNT=1 FITS files\n");
      }
      removecomment(thecomment, fitshead);
    }
  }

  /* look for BSCALE, BZERO */
  if ((thecomment = getcommentbyname("BSCALE", fitshead))) {
    fitshead->bscale = getnumericvalue(thecomment);
    removecomment(thecomment, fitshead);
    fitshead->bscaling = 1;
  } else {
    fitshead->bscale = 1.0;
  }
  if ((thecomment = getcommentbyname("BZERO", fitshead))) {
    fitshead->bzero = getnumericvalue(thecomment);
    removecomment(thecomment, fitshead);
    fitshead->bscaling = 1;
  } else {
    fitshead->bzero = 0.0;
  }

  /* default internal pixtype */
  fitshead->intpixtype = FLOAT_PIXTYPE;

  /* now we set the byte order */
  /* for old imcat format both ip/op controlled solely by IMCATSWAPFITSBYTES */
#ifdef BSWAP
  fitshead->ipbyteorder = NON_NATIVE_BYTE_ORDER;
  fitshead->opbyteorder = NON_NATIVE_BYTE_ORDER;
#else
  fitshead->ipbyteorder = NATIVE_BYTE_ORDER;
  fitshead->opbyteorder = NATIVE_BYTE_ORDER;
#endif

  if (getenv("IMCATCONVERTNANS")) {
    fitshead->convertnans = 1;
  }

  return(fitshead);
}

/*
 * copyfitsheader() makes copy of a header complete with copies
 * of the comment list
 */
fitsheader *copyfitsheader(fitsheader *srcfits)
{
  fitsheader *dstfits;
  fitscomment *srccom, *dstcom, *newcom;

  dstfits  = (fitsheader *)calloc(1, sizeof(fitsheader));
  *dstfits = *srcfits;
  srccom   = srcfits->basecomment;
  dstcom   = NULL;
  while (srccom) {
    newcom       = (fitscomment *)calloc(1, sizeof(fitscomment));
    *newcom      = *srccom;
    newcom->next = newcom->prev = NULL;
    if (!dstcom) {
      dstfits->basecomment = dstcom = newcom;
    } else {
      dstcom->next = newcom;
      newcom->prev = dstcom;
      dstcom       = newcom;
    }
    srccom = srccom->next;
  }
  return(dstfits);
}

/*
 * writefitsheader() writes header to the stream theheader->opstream
 */
int writefitsheader(fitsheader *theheader)
{
  char         record[FITS_REC_SIZE], string[COM_LENGTH1];
  int          line, dim;
  double       bscale, bzero; /* bscale and bzero values written to
                                 output image headers */
  fitscomment *thecomment;
  FILE *       stream;

  stream = theheader->opstream;

  /* write the required header items */
  line = 0;
  if (theheader->isextension) {
    thecomment = newtextcomment("XTENSION", "'IMAGE   '", "written by IMCAT");
  } else {
    thecomment = newtextcomment("SIMPLE", "T", "written by IMCAT");
  }
  copycommenttostring(thecomment, record + line++ * COM_LENGTH);
  free(thecomment);
  thecomment = newnumericcomment("BITPIX", (double)theheader->extpixtype, "");
  copycommenttostring(thecomment, record + line++ * COM_LENGTH);
  free(thecomment);
  thecomment = newnumericcomment("NAXIS", (double)theheader->ndim, "");
  copycommenttostring(thecomment, record + line++ * COM_LENGTH);
  free(thecomment);
  for (dim = 0; dim < theheader->ndim; dim++) {
    sprintf(string, "NAXIS%d", dim + 1);
    thecomment = newnumericcomment(string, (double)theheader->n[dim], "");
    copycommenttostring(thecomment, record + line++ * COM_LENGTH);
    free(thecomment);
  }

  /* EXTENSIONS */
  if (theheader->hasextensions) {
    thecomment = newtextcomment("EXTEND", "T", "file has extensions");
    copycommenttostring(thecomment, record + line++ * COM_LENGTH);
    free(thecomment);
    thecomment = newnumericcomment("NEXTEND",
                                   (double)theheader->nextensions,
                                   "number of extnesions");
    copycommenttostring(thecomment, record + line++ * COM_LENGTH);
    free(thecomment);
  }
  if (theheader->isextension) {
    thecomment = newnumericcomment("PCOUNT", 0.0, "no random parameters");
    copycommenttostring(thecomment, record + line++ * COM_LENGTH);
    free(thecomment);
    thecomment = newnumericcomment("GCOUNT", 1.0, "single group");
    copycommenttostring(thecomment, record + line++ * COM_LENGTH);
    free(thecomment);
  }


  /* BSCALE and BZERO */
  if (theheader->extpixtype == FLOAT_PIXTYPE ||
      theheader->extpixtype == DBL_PIXTYPE) {
    theheader->bscaling = 0;
  }
  if (theheader->bscaling) {
    bscale = theheader->bscale;
    bzero = theheader->bzero;
  } else {
    bscale = 1.0;
    bzero = 0.0;
  }

  thecomment = newnumericcomment("BSCALE", bscale, "pixel scale factor");
  copycommenttostring(thecomment, record + line++ * COM_LENGTH);
  free(thecomment);
  thecomment = newnumericcomment("BZERO", bzero, "pixel value offset");
  copycommenttostring(thecomment, record + line++ * COM_LENGTH);
  free(thecomment);


  /* output the linked list */
  thecomment = theheader->basecomment;
  while (thecomment) {
    copycommenttostring(thecomment, record + line++ * COM_LENGTH);
    thecomment = thecomment->next;
    if (line * COM_LENGTH == FITS_REC_SIZE) {
      if (FITS_REC_SIZE != fwrite(record, sizeof(char),
          FITS_REC_SIZE, stream)) {
        error_exit("writefitsheader: fwrite failed\n");
      }
      line = 0;
    }
  }

  /* output the END comment */
  sprintf(string, "%-80.80s", "END");
  strncpy(record + line++ * COM_LENGTH, string, COM_LENGTH);
  while (line * COM_LENGTH < FITS_REC_SIZE) {
    sprintf(string, "%-80.80s", "");
    strncpy(record + line++ * COM_LENGTH, string, COM_LENGTH);
  }
  if (FITS_REC_SIZE != fwrite(record, sizeof(char), FITS_REC_SIZE, stream)) {
    error_exit("writefitsheader: fwrite failed\n");
  }

  return(1);
}

/*
 * writefitstail() writes tail of last 2880 byte disk record
 */
int writefitstail(fitsheader *theheader)
{
  int   i, ic, pad;
  char  record[2880];
  long  length = 0;
  FILE *stream;

  stream = theheader->opstream;

  switch (theheader->extpixtype) {
  case UCHAR_PIXTYPE:
    length = (long)sizeof(unsigned char);
    break;
  case SHORT_PIXTYPE:
    length = (long)sizeof(short);
    break;
  case FLOAT_PIXTYPE:
    length = (long)sizeof(float);
    break;
  case INT_PIXTYPE:
    length = (long)sizeof(int);
    break;
  case DBL_PIXTYPE:
    length = (long)sizeof(double);
    break;
  default:
    error_exit("write_fits_tail: illegal pixtype\n");
    break;
  }
  for (i = 0; i < theheader->ndim; i++) {
    length *= (long)(theheader->n)[i];
  }
  pad = (int)((2880L - (length % 2880L)) % 2880L);

  for (ic = 0; ic < pad; ic++) {
    record[ic] = 0;
  }
  return(fwrite(record, sizeof(char), pad, stream));
}

int copycommenttostring(fitscomment *thecomment, char *string)
{
  char tempstring[COM_LENGTH1];

  if (strncmp(thecomment->name, "HISTORY", 7) == 0 ||
      strncmp(thecomment->name, "COMMENT", 7) == 0) {
    sprintf(tempstring, "%-8.8s%-72.72s", thecomment->name,
            thecomment->value);
  } else {
    sprintf(tempstring, "%-8.8s= %-70.70s", thecomment->name,
            thecomment->value);
  }
  strncpy(string, tempstring, COM_LENGTH);

  return(0);
}

fitscomment *newtextcomment(char *name, char *value, char *comment)
{
  fitscomment *thecomment;


  if (strlen(name) > NAME_LENGTH) {
    error_exit("newtextcomment: name too long\n");
  }
  if (!(thecomment = (fitscomment *)calloc(1, sizeof(fitscomment)))) {
    error_exit("newtextcomment: memory allocation failure\n");
  }
  sprintf(thecomment->name, "%-8.8s", name);
  if (comment) {
    sprintf(thecomment->value, "%20s / %-47.47s", value, comment);
  } else {
    sprintf(thecomment->value, "%-70.70s", value);
  }
  return(thecomment);
}

/* VERY UNCLEAN function to deal with INTS and FLOATS within FITS
   Headers!!
*/
fitscomment *newnumericcomment(char *name, double value, char *comment)
{
  fitscomment *thecomment;


  if (strlen(name) > NAME_LENGTH) {
    error_exit("newnumericcomment: name too long\n");
  }
  if (!(thecomment = (fitscomment *)calloc(1, sizeof(fitscomment)))) {
    error_exit("newnumericcomment: memory allocation failure\n");
  }
  sprintf(thecomment->name, "%-8.8s", name);
  if (comment) {
    /* The following, very nasty hack for BZERO and BSCALE is necessary
       because they always need to be float; also if they are 0 and 1!
    */
    if (((strcmp(name, "BSCALE") == 0) || strcmp(name, "BZERO") == 0) &&
        (fabs(value - (int)value) < 1.0e-06)) {
      sprintf(thecomment->value, "%20.8f / %-47.47s", value, comment);
    } else {
      sprintf(thecomment->value, "%20.8g / %-47.47s", value, comment);
    }
  } else {
    if (((strcmp(name, "BSCALE") == 0) || strcmp(name, "BZERO") == 0) &&
        (fabs(value - (int)value) < 1.0e-06)) {
      sprintf(thecomment->value, "%20.8f", value);
    } else {
      sprintf(thecomment->value, "%20.8g", value);
    }
  }
  return(thecomment);
}

fitscomment *getcommentbyname(char *name, fitsheader *theheader)
{
  fitscomment *thecomment;
  char         string[NAME_LENGTH1];

  if (strlen(name) > NAME_LENGTH) {
    error_exit("getcommentbyname: name too long\n");
  }
  sprintf(string, "%-8.8s", name);
  thecomment = theheader->basecomment;
  while (thecomment) {
    if (!strncmp(thecomment->name, string, NAME_LENGTH)) {
      return(thecomment);
    }
    thecomment = thecomment->next;
  }
  return(NULL);
}

double getnumericvalue(fitscomment *thecomment)
{
  double val;

  if (1 != sscanf(thecomment->value, "%lg", &val)) {
    error_exit("getnumericvalue: can't decipher value\n");
  }
  return(val);
}

char *gettextvalue(fitscomment *thecomment)
{
  char *val;
  char  tmpstr[VALUE_LENGTH1 + 2];
  int   i = 0;

  val = (char *)calloc(VALUE_LENGTH1 + 2, sizeof(char));
  strncpy(tmpstr, thecomment->value, VALUE_LENGTH);
  while (!strncmp(&(tmpstr[i]), " ", 1)) {
    i++;
  }
  if (strncmp(&(tmpstr[i]), "'", 1)) {
    strcpy(val, strtok(&(tmpstr[i]), " "));
  } else {
    strcpy(val, strtok(&(tmpstr[i]), "'"));
  }
  return(val);
}

void readfitsline(void *f, fitsheader *theheader)
{
  int   N1;
  FILE *theipf;

  N1     = theheader->n[0];
  theipf = theheader->ipstream;

  if (theheader->extpixtype == theheader->intpixtype &&
      theheader->ipbyteorder == NATIVE_BYTE_ORDER &&
      !(theheader->convertnans &&
      (theheader->extpixtype == FLOAT_PIXTYPE ||
       theheader->extpixtype == DBL_PIXTYPE))) {
    fread(f, pixsize(theheader->extpixtype), N1, theipf);
  } else {
    updatelinebuffer(theheader);
    fread(theheader->linebuffer, pixsize(theheader->extpixtype), N1, theipf);
    if (theheader->ipbyteorder == NON_NATIVE_BYTE_ORDER) {
      byteswapline((void *)(theheader->linebuffer), N1,
                   pixsize(theheader->extpixtype));
    }
    if (theheader->convertnans &&
        (theheader->extpixtype == FLOAT_PIXTYPE ||
         theheader->extpixtype == DBL_PIXTYPE)) {
      convertnanstomagic((void *)(theheader->linebuffer), N1,
                         pixsize(theheader->extpixtype));
    }
    convertarray((char *)theheader->linebuffer,
                 (char *)f, theheader->extpixtype, theheader->intpixtype, N1,
                 theheader->bscaling, theheader->bscale, theheader->bzero);
  }
}

void writefitsline(void *f, fitsheader *theheader)
{
  int    N1;
  FILE * theopf;
  double bscale = 1.0, bzero = 0.0;

  N1     = theheader->n[0];
  theopf = theheader->opstream;

  if (theheader->bscaling) {
    bscale = 1.0 / theheader->bscale;
    bzero  = -theheader->bzero / theheader->bscale;
  }

  if (theheader->extpixtype == theheader->intpixtype &&
      theheader->opbyteorder == NATIVE_BYTE_ORDER &&
      !(theheader->convertnans &&
      (theheader->extpixtype == FLOAT_PIXTYPE ||
       theheader->extpixtype == DBL_PIXTYPE))) {
    fwrite(f, pixsize(theheader->extpixtype), N1, theopf);
  } else {
    updatelinebuffer(theheader);
    convertarray((char *)f, (char *)theheader->linebuffer,
                 theheader->intpixtype, theheader->extpixtype, N1,
                 theheader->bscaling, bscale, bzero);
    if (theheader->convertnans &&
        (theheader->extpixtype == FLOAT_PIXTYPE ||
         theheader->extpixtype == DBL_PIXTYPE)) {
      convertmagictonans((void *)(theheader->linebuffer), N1,
                         pixsize(theheader->extpixtype));
    }
    if (theheader->opbyteorder == NON_NATIVE_BYTE_ORDER) {
      byteswapline((void *)(theheader->linebuffer), N1,
                   pixsize(theheader->extpixtype));
    }
    fwrite(theheader->linebuffer, pixsize(theheader->extpixtype), N1, theopf);
  }
}

void readfitsplane(void **f, fitsheader *theheader)
{
  int i;

  for (i = 0; i < theheader->n[1]; i++) {
    readfitsline(f[i], theheader);
  }
}

void writefitsplane(void **f, fitsheader *theheader)
{
  int i;

  for (i = 0; i < theheader->n[1]; i++) {
    writefitsline(f[i], theheader);
  }
}

void readfitscube(void ***f, fitsheader *theheader)
{
  int i;

  for (i = 0; i < theheader->n[2]; i++) {
    readfitsplane(f[i], theheader);
  }
}

void writefitscube(void ***f, fitsheader *theheader)
{
  int i;

  for (i = 0; i < theheader->n[2]; i++) {
    writefitsplane(f[i], theheader);
  }
}

void removecomment(fitscomment *thecomment, fitsheader *fitshead)
{
  fitscomment *com;

  if (fitshead->basecomment == thecomment) {
    fitshead->basecomment = thecomment->next;
    if (fitshead->basecomment != NULL) {
      fitshead->basecomment->prev = NULL;
    }
    free(thecomment);
    return;
  }
  com = fitshead->basecomment;
  while (com) {
    if (com == thecomment) {
      if (thecomment->prev) {
        (thecomment->prev)->next = thecomment->next;
      }
      if (thecomment->next) {
        (thecomment->next)->prev = thecomment->prev;
      }
      free(com);
      com = NULL;
      return;
    }
    com = com->next;
  }
}

void removenamedcomments(char *name, fitsheader *fitshead)
{
  fitscomment *com;

  while ((com = getcommentbyname(name, fitshead))) {
    removecomment(com, fitshead);
  }
}

int pixsize(int pixtype)
{
  switch (pixtype) {
  case UCHAR_PIXTYPE:
    return(sizeof(unsigned char));

    break;
  case SHORT_PIXTYPE:
    return(sizeof(short));

    break;
  case INT_PIXTYPE:
    return(sizeof(int));

    break;
  case FLOAT_PIXTYPE:
    return(sizeof(float));

    break;
  case DBL_PIXTYPE:
    return(sizeof(double));

    break;
  default:
    error_exit("pixsize: bad pixtype\n");
  }
  return(0);
}

/* higher level routines */

int read2Dfloatimage(float ***f, int *N1, int *N2,
                     fitsheader **fits, FILE *stream)
{
  int y;

  *fits = readfitsheader(stream);
  if ((*fits)->ndim != 2) {
    error_exit("read2Dfloatimage: image not 2 dimensional\n");
  }
  *N1 = (*fits)->n[0];
  *N2 = (*fits)->n[1];
  allocFloatArray(f, *N1, *N2);
  for (y = 0; y < *N2; y++) {
    readfitsline((void *)(*f)[y], *fits);
  }

  return(0);
}

int write2Dfloatimage(float **f, fitsheader *fits)
{
  int y;

  if (fits->ndim != 2) {
    error_exit("write2Dfloatimage: image not 2 dimensional\n");
  }
  writefitsheader(fits);
  for (y = 0; y < fits->n[1]; y++) {
    writefitsline((void *)f[y], fits);
  }
  writefitstail(fits);

  return(0);
}

fitsheader *newfitsheader(int ndim, int *dim, int extpixtype)
{
  fitsheader *fits;
  int         idim;

  fits             = (fitsheader *)calloc(1, sizeof(fitsheader));
  fits->extpixtype = extpixtype;
  fits->intpixtype = FLOAT_PIXTYPE;
  fits->ndim       = ndim;
  for (idim = 0; idim < ndim; idim++) {
    fits->n[idim] = dim[idim];
  }
  fits->bscaling    = 0;
  fits->ipstream    = stdin;
  fits->opstream    = stdout;
  fits->linebuffer  = NULL;
  fits->basecomment = NULL;
#ifdef BSWAP
  fits->opbyteorder = NON_NATIVE_BYTE_ORDER;
#else
  fits->opbyteorder = NATIVE_BYTE_ORDER;
#endif
  if (getenv("IMCATCONVERTNANS")) {
    fits->convertnans = 1;
  }
  return(fits);
}

fitsheader *new2Dfitsheader(int N1, int N2, int extpixtype)
{
  int dim[2];

  dim[0] = N1;
  dim[1] = N2;
  return(newfitsheader(2, dim, extpixtype));
}

/* this function deletes all structures connected to
   a FITS header
 */
void delfitsheader(fitsheader *aheader)
{
  fitscomment *thecomment, *copycomment;

  if (aheader != NULL) {
    /* delete the linked list */
    thecomment  = aheader->basecomment;
    copycomment = thecomment;

    while (thecomment != NULL) {
      copycomment = copycomment->next;
      free(thecomment);
      thecomment = NULL;
      thecomment = copycomment;
    }

    if (aheader->linebuffer != NULL) {
      free(aheader->linebuffer);
      aheader->linebuffer = NULL;
    }
    free(aheader);
    aheader = NULL;
  }
}

/* the function returns a new string that has to
   be freed manually. If the basename flag is set
   we take the 'basename' from the first argument
   that is usually the program name
 */
char *argsToString(int argc, char **argv, int basename)
{
  int   i, stringlen = 0;
  char *string = NULL;

  if (argc <= 0) {
    return(NULL);
  }

  for (i = 0; i < argc; i++) {
    /* the '+1' is for a space between the
       arguments */
    stringlen += (strlen(argv[i]) + 1);
  }

  string = (char *)calloc(stringlen, sizeof(char));

  if (basename == 1) {
    strcpy(string, get_basename(argv[0]));
  } else {
    strcpy(string, argv[0]);
  }

  for (i = 1; i < argc; i++) {
    strcat(string, " ");
    strcat(string, argv[i]);
  }
  return(string);
}

void add_comment(int argc, char **argv, fitsheader *fitshead)
{
  char *argstring;
  int   arglen;
  char  tmpstring[71]; /* '70' is the maximum number
                          of chars a HISTORY line can have.
                          One element more for the '\0'. */
  fitscomment *thecomment;
  int          i;

  argstring = argsToString(argc, argv, 1);
  arglen    = strlen(argstring);

  if (argstring != NULL) {
    i = 0;
    do {
      strncpy(tmpstring, &argstring[i * 70], 70);
      thecomment = newtextcomment("HISTORY", tmpstring, NULL);
      appendcomment(thecomment, fitshead);
      arglen -= 70;
      i++;
    } while (arglen > 0);
    free(argstring);
  }
}

void appendcomment(fitscomment *newcomment, fitsheader *fitshead)
{
  fitscomment *thecomment;

  if (!fitshead->basecomment) {
    fitshead->basecomment = newcomment;
    return;
  }
  thecomment = fitshead->basecomment;
  while (thecomment->next) {
    thecomment = thecomment->next;
  }
  thecomment->next = newcomment;
}

void prependcomment(fitscomment *newcomment, fitsheader *fitshead)
{
  if (fitshead->basecomment) {
    newcomment->next              = fitshead->basecomment;
    (fitshead->basecomment)->prev = newcomment;
  }
  fitshead->basecomment = newcomment;
}

void setextpixtype(fitsheader *fits, int pixtype)
{
  fits->extpixtype = pixtype;
  fits->linebuffer = NULL;
}

void set2Dimagesize(fitsheader *fits, int N1, int N2)
{
  fits->ndim       = 2;
  fits->n[0]       = N1;
  fits->n[1]       = N2;
  fits->linebuffer = NULL;
}

OFF_T skiplines(fitsheader *fits, int nlines)
{
  OFF_T        offset;
  struct  stat st;

  /* first we see if the file is regular */
  fstat(fileno(fits->ipstream), &st);
  if ((st.st_mode & S_IFMT) == S_IFREG) {
    offset = (OFF_T)(fits->n[0] * pixsize(fits->extpixtype) * nlines);
    return(FSEEKO(fits->ipstream, offset, SEEK_CUR));
  } else {
    updatelinebuffer(fits);
    while (nlines--) {
      fread((void *)fits->linebuffer,
            pixsize(fits->extpixtype), fits->n[0], fits->ipstream);
    }
  }

  return(0);
}

int updatelinebuffer(fitsheader *theheader)
{
  int N1;

  N1 = theheader->n[0];
  if (!theheader->linebuffer ||
      theheader->linebuffersize != N1 * pixsize(theheader->extpixtype)) {
    if (theheader->linebuffer) {
      free(theheader->linebuffer);
      theheader->linebuffer = NULL;
    }
    theheader->linebuffer = (char *)calloc(N1 * pixsize(theheader->extpixtype),
                                           sizeof(char));
    theheader->linebuffersize = N1 * pixsize(theheader->extpixtype);
  }

  return(0);
}

int byteswapline(void *data, int nel, int pixsize)
{
  static char tmpchar[8];
  int         i, b;
  char *      caddr;

  caddr = (char *)data;
  for (i = 0; i < nel; i++) {
    for (b = 0; b < pixsize; b++) {
      tmpchar[b] = caddr[b];
    }
    for (b = 0; b < pixsize; b++) {
      caddr[b] = tmpchar[pixsize - b - 1];
    }
    caddr += pixsize;
  }

  return(0);
}

int convertmagictonans(void *data, int nel, int pixsize)
{
  _Dconst _Nan = { { INIT(_DNAN) } };
  double  dnan = _Nan._D;
  float   fnan = (float)dnan;
  int     i;

  switch (pixsize) {
  case 4:
    for (i = 0; i < nel; i++) {
      if (fabs(FLOAT_MAGIC - ((float *)data)[i]) <
          (0.05 * fabs(FLOAT_MAGIC))) {
        ((float *)data)[i] = fnan;
      }
    }
    break;
  case 8:
    for (i = 0; i < nel; i++) {
      if (fabs(DBL_MAGIC - ((double *)data)[i]) <
          (0.05 * fabs(DBL_MAGIC))) {
        ((double *)data)[i] = dnan;
      }
    }
    break;
  default:
    error_exit("convertmagictonans: illegal pixsize\n");
  }

  return(0);
}

int convertnanstomagic(void *data, int nel, int pixsize)
{
  int i;

  switch (pixsize) {
  case 4:
    for (i = 0; i < nel; i++) {
      if (isnan((double)(((float *)data)[i]))) {
        ((float *)data)[i] = FLOAT_MAGIC;
      }
    }
    break;
  case 8:
    for (i = 0; i < nel; i++) {
      if (isnan(((double *)data)[i])) {
        ((double *)data)[i] = DBL_MAGIC;
      }
    }
    break;
  default:
    error_exit("convertnanstomagic: illegal pixsize\n");
  }

  return(0);
}

/*
 * Find out the base name of a file (i.e. without prefix path)
 */

char *get_basename(const char *filename)
{
  char *p;
  p = strrchr(filename, '/');
  return(p ? p + 1 : (char *)filename);
}
