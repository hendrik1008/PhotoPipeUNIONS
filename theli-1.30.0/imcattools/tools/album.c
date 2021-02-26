/*----------------------------------------------------------------------------

   File name    :   album.c
   Author       :   Nick Kaiser, Thomas Erben & Mischa Schirmer
   Created on   :   24.01.2005
   Description  :   paste fits images into a larger image

 ---------------------------------------------------------------------------*/

/*
	$Id: album.c,v 1.16 2018/03/28 12:06:14 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:14 $
	$Revision: 1.16 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
   24.01.2005:
   I changed exit(0) to return 0 in the main program
   to avoid compiler warnings

   28.01.2005:
   I added HISTORY information to the produced FITS
   images

   08.02.2005:
   - I added memory tracing capabilities via xmemory.
   - I removed several memory leaks.

   11.04.2005:
   - I introduced a binning of input images
    (option '-b'. The former '-b' was renamed to '-l')
   - The BITPIX of the output image can be chosen between
    16 and -32 now (option '-p')
   - I made the command line reading more stable

   03.06.2005:
   I improved the description output.

   13.09.05:
   I included the possibility to use an old header for
   the output. In this case only BITPIX and image sizes
   are adapted. New option '-h'.

   09.03.2006:
   I removed compiler warnings (-W -Wall -pedantic compiler flags)

   09.02.2010:
   The THELI pipeline version number is included in the HISTORY
   comments of result images.

   30.03.2010:
   Included an optional outlier rejection during the binning such that
   hot and cold pixels do not show up (Mischa)

   03.07.2010:
   I changed the reject option so that the low and high percentage
   of pixels to be rejected need to be provided as arguments to the
   '-r' command line option. Before this was hardwired to a rejection
   of the 10% lowest and higest pixels.

   24.08.2010:
   I removed some remarks raised by the Intel 'icc' compiler.
   (removal of never used variables; correction of unclean float
   comparisons)

   03.04.2013:
   xmemory is no longer used by default but only if the variable
   __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
   etc. calls are used for memory management.
 */

/*----------------------------------------------------------------------------
                                Includes and Defines
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "fits.h"
#include "error.h"
#include "arrays.h"
#include "global_vers.h"
#include "t_iso8601.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

/* uncomment the following line to obtain a status on allocated/freed
   memory at the end of the program. Only available if also
   __XMEM_DEBUG__ is defined at compile time.
 */

/* #define MEMORY_DEBUG */

/*----------------------------------------------------------------------------
                           Function declarations
 ---------------------------------------------------------------------------*/

void printUsage(void);
float **bin2Dfloatimage(float **fin, int N1, int N2, int binning,
                        int rejectmin, int rejectmax);
int floatcompare(const void *a, const void *b);
float calcfloatmean(float *data, long start, long end);

/*----------------------------------------------------------------------------
                                Main Function
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  int         arg = 1, nimages, im, i, j, ii, jj, dx, dy, nx, ny;
  FILE        *ipf, *offsetf;
  int         N1, N2, *ioffset, *joffset, drawbox, offsetsarg;
  int         useoffsetsfile, hasgrid;
  fitsheader  *fits = NULL, *fitsout = NULL;
  char        errorstring[1024], line[1024], tmpstring[256];
  float       **fin, **finbin, **fout, xoff, yoff;
  int         Nin1, Nin2, magicinit = 0;
  int         binning    = 1;    /* the binning factor for the input images */
  int         outbitpix  = -32;  /* the BITPIX value for the output image
                                    (16 or -32) */
  int         useoldhead = 0;    /* use header from one of the input images
                                    for the result (0=no; otherwise this
                                    variable gives the input image
                                    number from which to use the header) */
  int         rejectmin, rejectmax; /* percentage of low/high pixels to reject
                                       in the binning process */

  if (argc < 3) {
    printUsage();
    exit(1);
  }
  nimages = argc - 3;

  /* defaults */
  useoffsetsfile = 0;
  drawbox        = 0;
  offsetsarg     = 0;
  hasgrid        = 0;
  rejectmin      = 0;
  rejectmax      = 0;

  /* process arguments */
  while (arg < argc) {
    if (argv[arg][0] == '-') {
      switch (argv[arg++][1]) {
      case 'b':
        if (argc >= arg + 1) {
          if (1 != sscanf(argv[arg++], "%d", &binning)) {
            printUsage();
            exit(1);
          }
          nimages -= 2;
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 'd':
        if (argc >= arg + 2) {
          if (1 != sscanf(argv[arg++], "%d", &dx)) {
            printUsage();
            exit(1);
          }
          if (1 != sscanf(argv[arg++], "%d", &dy)) {
            printUsage();
            exit(1);
          }
          hasgrid  = 1;
          nimages -= 3;
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 'f':
        if (argc >= arg + 1) {
          offsetsarg     = arg++;
          nimages       -= 2;
          useoffsetsfile = 1;
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 'h':
        if (argc >= arg + 1) {
          if (1 != sscanf(argv[arg++], "%d", &useoldhead)) {
            printUsage();
            exit(1);
          }
          nimages -= 2;
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 'l':
        drawbox  = 1;
        nimages -= 1;
        break;
      case 'M':
        magicinit = 1;
        nimages  -= 1;
        break;
      case 'p':
        if (argc >= arg + 1) {
          if (1 != sscanf(argv[arg++], "%d", &outbitpix)) {
            printUsage();
            exit(1);
          }
          nimages -= 2;
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 'r':
        if (argc >= arg + 2) {
          if (1 != sscanf(argv[arg++], "%d", &rejectmin)) {
            printUsage();
            exit(1);
          }
          if (1 != sscanf(argv[arg++], "%d", &rejectmax)) {
            printUsage();
            exit(1);
          }
          nimages -= 3;
        } else {
          printUsage();
          exit(1);
        }
	break;
      default:
        printUsage();
        exit(1);
        break;
      }
    } else {
      break;
    }
  }
  if ((argc - arg) < 3) {
    printUsage();
    exit(1);
  }

  /* sanity checks */
  if (useoldhead < 0 || useoldhead > nimages) {
    error_exit("album: invalid index for '-h' command line argument\n");
  }

  if ((rejectmin + rejectmax) >= 100) {
    error_exit("album: You request to reject more than 100% of the pixels!\n");
  }
  if (rejectmin < 0 || rejectmax < 0) {
    error_exit("album: reject percentages must be positive!\n");
  }

  /* print information */
  fprintf(stderr, "album: combining %d images with binning %d\n",
          nimages, binning);
  fprintf(stderr, "Output BITPIX is %d\n\n", outbitpix);

  if (useoffsetsfile) {
    sscanf(argv[arg++], "%d", &N1);
    sscanf(argv[arg++], "%d", &N2);
    N1 = N1 / binning;
    N2 = N2 / binning;
    IMCAT_CALLOC(ioffset, int, nimages);
    IMCAT_CALLOC(joffset, int, nimages);
    offsetf = fopen(argv[offsetsarg], "r");
    if (!offsetf) {
      error_exit("album: unable to open offsetsfile for input\n");
    }
    fgets(line, 1024, offsetf);
    for (im = 0; im < nimages; im++) {
      if (!fgets(line, 1024, offsetf)) {
        snprintf(errorstring, 1023,
                 "album: problem reading from %s\n",
                 argv[offsetsarg]);
        error_exit(errorstring);
      }
      if (2 != sscanf(line, "%f %f", &xoff, &yoff)) {
        snprintf(errorstring, 1023,
                 "album: problem reading from %s\n",
                 argv[offsetsarg]);
        error_exit(errorstring);
      }
      joffset[im] = xoff / binning;
      ioffset[im] = yoff / binning;
    }
  } else {
    sscanf(argv[arg++], "%d", &nx);
    sscanf(argv[arg++], "%d", &ny);
    IMCAT_CALLOC(ioffset, int, nx * ny);
    IMCAT_CALLOC(joffset, int, nx * ny);
    if (!hasgrid) {
      ipf = fopen(argv[argc - nimages], "r");
      if (!ipf) {
        sprintf(errorstring, "album: failed to open %s\n",
                argv[argc - nimages]);
        error_exit(errorstring);
      }
      fits = readfitsheader(ipf);
      fclose(ipf);
      dx = fits->n[0];
      dy = fits->n[1];
      delfitsheader(fits);
    }
    N1 = (nx * dx) / binning;
    N2 = (ny * dy) / binning;
    for (i = 0; i < ny; i++) {
      for (j = 0; j < nx; j++) {
        ioffset[nx * i + j] = (i * dy) / binning;
        joffset[nx * i + j] = (j * dx) / binning;
      }
    }
  }

  allocFloatArray(&fout, N1, N2);

  if (magicinit) {
    for (i = 0; i < N2; i++) {
      for (j = 0; j < N1; j++) {
        fout[i][j] = FLOAT_MAGIC;
      }
    }
  }

  /* process files */
  for (im = 0; im < nimages; im++) {
    ipf = fopen(argv[argc - nimages + im], "r");
    if (!ipf) {
      sprintf(errorstring, "album: failed to open %s\n",
              argv[argc - nimages + im]);
      error_exit(errorstring);
    }
    fprintf(stderr, "working on image: %s\n", argv[argc - nimages + im]);
    /* check if we need the header of that image */
    if ((useoldhead - 1) == im) {
      fitsout = readfitsheader(ipf);
    }

    read2Dfloatimage(&fin, &Nin1, &Nin2, &fits, ipf);
    /* bin image */
    finbin = bin2Dfloatimage(fin, Nin1, Nin2, binning, rejectmin, rejectmax);
    fclose(ipf);
    for (i = 0; i < Nin2 / binning; i++) {
      for (j = 0; j < Nin1 / binning; j++) {
        ii = ioffset[im] + i;
        jj = joffset[im] + j;
        if (ii >= 0 && ii < N2 && jj >= 0 && jj < N1) {
          if (fabs(finbin[i][j] - FLOAT_MAGIC) > (0.05 * fabs(FLOAT_MAGIC))) {
            fout[ii][jj] = finbin[i][j];
          }
          if (drawbox) {
            if (i == 0 || i == ((Nin2 / binning) - 1) ||
                j == 0 || j == ((Nin1 / binning) - 1)) {
              fout[ii][jj] = SHRT_MAX;
            }
          }
        }
      }
    }
    freeFloatArray(fin);
    freeFloatArray(finbin);
    delfitsheader(fits);
  }

  /* create FITS header for output image */
  switch (outbitpix) {
  case 16:
    if (useoldhead == 0) {
      fitsout = new2Dfitsheader(N1, N2, SHORT_PIXTYPE);
    } else {
      set2Dimagesize(fitsout, N1, N2);
    }
    fitsout->bscaling   = 1;
    fitsout->bzero      = 32768.0;
    fitsout->bscale     = 1.0;
    fitsout->extpixtype = SHORT_PIXTYPE;
    break;
  default:
    if (useoldhead == 0) {
      fitsout = new2Dfitsheader(N1, N2, FLOAT_PIXTYPE);
    } else {
      set2Dimagesize(fitsout, N1, N2);
    }
    fitsout->extpixtype = FLOAT_PIXTYPE;
    break;
  }
  appendcomment(newtextcomment("HISTORY", "", NULL), fitsout);
  sprintf(tmpstring, "album: %s", __PIPEVERS__);
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  sprintf(tmpstring, "album: called at %s", get_datetime_iso8601());
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);

  add_comment(argc, argv, fitsout);
  write2Dfloatimage(fout, fitsout);

  /* clean memory and bye */
  freeFloatArray(fout);
  delfitsheader(fitsout);
  IMCAT_FREE(ioffset);
  IMCAT_FREE(joffset);

#ifdef MEMORY_DEBUG
  xmemory_status();
#endif

  return(0);
}

/*-------------------------------------------------------------------------*/

/**
   @brief    Perform a binning of a 2D float FITSimage by a given factor
   @param    fin         Pixel array of Input image.
   @param    N1          x-dimension of Input image.
   @param    N2          y-dimension of Input image.
   @param    binning     the scrunching factor
   @param    rejectmin   rejection of 'rejectmin' percent low values
   @param    rejectmax   rejection of 'rejectmax' percent high values
   @return   1 newly allocated 2D float pixel array.

   The binning is done by calculating the mean of a bin x bin square
   right and up from the point under consideration. If 'reject' equals
   1, the 10% lowest and 10% highest pixel values are excluded from
   the binning process. The function returns a newly allocated
   2-dimensional float array that has to be freed by freeFloatArray.
   The user has to remember the dimensions of the binned image.
 */
/*--------------------------------------------------------------------------*/

float **bin2Dfloatimage(float **fin, int N1, int N2, int binning,
                        int rejectmin, int rejectmax)
{
  int     i, j, k, l, t;        /* loop variables */
  int     N1bin = N1 / binning; /* x-dimension of binned image */
  int     N2bin = N2 / binning; /* y-dimension of binned image */
  float **fitsbin;              /* Array of binned image */
  float  *chunk;                /* array containing the pixels that is
                                   going to be binned */
  int     bsq;                  /* Number of pixels being binned */
  int     minpix, maxpix;       /* minimum and maximum pixel (exclusive)
                                   for mean-bin estimation */

  if (binning < 1) {
    error_exit("wrong binning factor\n");
  }
  allocFloatArray(&fitsbin, N1bin, N2bin);

  bsq = binning * binning;

  IMCAT_CALLOC(chunk, float, bsq);

  for (i = 0; i < N2bin; i++) {
    for (j = 0; j < N1bin; j++) {
      t = 0;
      /* bin lines */
      for (k = 0; k < binning; k++) {
        for (l = 0; l < binning; l++) {
	    chunk[t] = fin[i * binning + k][j * binning + l];
	    t++;
        }
      }
      qsort(chunk, bsq, sizeof(float), floatcompare);
      minpix = (int)((rejectmin / 100.) * bsq + 0.5);
      maxpix = (int)((1. - (rejectmax / 100.)) * bsq + 0.5);
      fitsbin[i][j] = calcfloatmean(chunk, minpix, maxpix);
    }
  }

  IMCAT_FREE(chunk);

  return(fitsbin);
}

/*-------------------------------------------------------------------------*/

/**
   @brief    calculate the mean from values in a float array
   @param    data        float array
   @param    start       first array index for mean calculation
   @param    end         last array index (exclusive) for mean calculation
   @return   the calculated mean

   The user is responsible that the array 'data' and the indices
   'start' and 'end' are valid.
 */
/*--------------------------------------------------------------------------*/
float calcfloatmean(float *data, long start, long end)
{
    long i, numpixels;
    float mean;

    mean = 0.;
    numpixels = 0;

    for (i = start; i < end; i++)  {
      mean += data[i];
      numpixels++;
    }

    if (numpixels != 0) {
      mean = mean / (float) numpixels;
    } else {
      mean = 0.;
    }

    return (mean);
}


/* helper function for providing a comparison of
   floats for qsort*/
int floatcompare(const void *a, const void *b)
{
  float a1 = *((float*)a);
  float b1 = *((float*)b);

  if (a1 < b1) {
      return -1;
  } else {
    if (a1 > b1) {
      return 1;
    } else {
      return 0;
    }
  }
}


/*
 * This function only gives the usage message for the program
 */

void printUsage(void)
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "	album - paste fits images into a larger image\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "	album [options... ] nx ny fits1 fits2 ...\n");
  fprintf(stdout, "	album -f offsetsfile [options... ] N1 N2 fits1 fits2 ...\n");
  fprintf(stdout, "               -d dx dy	  # grid spacing\n");
  fprintf(stdout, "	       -l	  # draw a line around each chip\n");
  fprintf(stdout, "	       -h n	  # use header from input image n for output image\n");
  fprintf(stdout, "			  # (default is 0 which creates a new, minumum \n");
  fprintf(stdout, "                          # header)\n");
  fprintf(stdout, "	       -b	  # binning factor\n");
  fprintf(stdout, "               -r min max # reject outliers (default: no rejection)\n");
  fprintf(stdout, "	       -p	  # BITPIX of output image (16 or -32) \n");
  fprintf(stdout, "	       -f file	  # file for offsets\n");
  fprintf(stdout, "	       -M	  # initialise to MAGIC.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION \n");
  fprintf(stdout, "        'album' combines a set of images into an album.  With -f\n");
  fprintf(stdout, "	option the layout of the image is determined from\n");
  fprintf(stdout, "	the'offsetsfile', and the size of the output image is\n");
  fprintf(stdout, "	specified after the options.  The format of this file should\n");
  fprintf(stdout, "	be a single comment line followed by x,y pairs to define\n");
  fprintf(stdout, "	location of bottom left corners of the images to be pasted.\n");
  fprintf(stdout, "	Without the -f option, the grid spacing will be equal to the\n");
  fprintf(stdout, "	dimensions of the first image (unless you override this with\n");
  fprintf(stdout, "	-d option) and the images will be placed on a nx by ny\n");
  fprintf(stdout, "	grid. First, the lowest row is filled from left to right, then\n");
  fprintf(stdout, "	the second row from left to right and so on.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	The output image is initialised to zero, images are painted on\n");
  fprintf(stdout, "	sequentially, erasing any previously painted values, except\n");
  fprintf(stdout, "	that MAGIC values are not painted.  Us the -M option to\n");
  fprintf(stdout, "	intialise to MAGIC instead. With '-b' the input images are\n");
  fprintf(stdout, "	binned by the given factor. In this case all quantities to\n");
  fprintf(stdout, "	provide (i. e. values in the offsetsfile or the size of the\n");
  fprintf(stdout, "	output image) have to be in 'unbinned' coordinates\n");
  fprintf(stdout, "	nevertheless. If you use the '-r' option, then the lowest \n");
  fprintf(stdout, "        'min'%% and highest 'max'%% of the pixels to be binned are \n");
  fprintf(stdout, "        discared.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR\n");
  fprintf(stdout, "        Original version:\n");
  fprintf(stdout, "	Nick Kaiser:  kaiser@cita.utoronto.ca\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        THELI version maintained by\n");
  fprintf(stdout, "        Thomas Erben: terben@astro.uni-bonn.de\n");
}


