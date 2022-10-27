/*----------------------------------------------------------------------------

   File name    :   imcombflat.c
   Author       :   Thomas Erben
   Created on   :   01.02.2005
   Description  :   stacks FITS images

   ---------------------------------------------------------------------------*/

/*
        $Id: imcombflat.c,v 1.13 2014/06/26 14:04:43 thomas Exp $
        $Author: thomas $
        $Date: 2014/06/26 14:04:43 $
        $Revision: 1.13 $
 */

/*----------------------------------------------------------------------------
                                History Coments
   ---------------------------------------------------------------------------*/

/*
   01.02.2005:
   project started.

   09.04.2005:
   A warning message is printed if only 1 or zero images effectively
   enter the coaddition. This is the case if many low and high ranks
   are rejected.

   28.06.2005:
   I fixed a major bug. Sorting of the input pixel array (before being coadded)
   had to be at a different place.

   06.12.2005:
   I introduced a clipmean combination mode (parameter -c CLIPMEAN)).
   It estimates mean and sigma
   of the pixel samples and rejects all pixels lying below and above user
   defined sigma values (parameter -l lo_clip hi_clip). This procedure
   is repeated until the array is not changed anymore (after each iteration
   the complete initial array is considered. i.e. also old points are
   allowed to return to the pixel sample) or until a maximum number
   of iterations is reached (20; variable maxiterations in the new
   function meanclip).

   23.03.2006:
   I substituted the quicksort algorithm by a faster one.

   16.01.2008:
   I corrected a bug in the definition of the two-dimensional
   imagenames array.

   03.04.2013:
   xmemory is no longer used by default but only if the variable
   __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
   etc. calls are used for memory management.

   18.06.2014:
   I introduced the possibility to treat problematic CCDs with the help
   of a BADCCD header keyword:
   - CCDs having this key set (unequal to zero) are not considered
    in stacking procedures.
   - The output image gets this flag set if only one or no images
    contribute to it.

   24.06.2014:
   If no images contribute to the stack the output mode was 'nan'
   (in the case of requested rescaling). I consider this case now and
   images get an output value of zero in this case.

   25.06.2014:
   I refined the treatment for co-adds to which 'no images' contribute.
   They now can get a value provided on the command line (-f: faulty image
   value). To always give zero is problematic if these images are later
   used as 'flats' and hence it is divided by them.

   26.06.2014:
   Bug fix for the enhancement from 25.06.2014.
 */

/*----------------------------------------------------------------------------
                                Includes and Defines
   ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

#include "error.h"
#include "arrays.h"
#include "fits.h"
#include "global_vers.h"
#include "t_iso8601.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

#ifndef MAXCHAR
#define MAXCHAR     255
#endif

#define MAXIMAGE    1000 /* the maximum number of input images allowed */

/* uncomment the following line to obtain a status on allocated/freed
   memory at the end of the program. Only available if also __XMEM_DEBUG__
   is defined at compile time.
 */

/* #define MEMORY_DEBUG */

/*----------------------------------------------------------------------------
                           Function declarations
   ---------------------------------------------------------------------------*/

/* print usage message for this program */
void printUsage(void);

/* sort a float array by increasing number */
void float_qsort(float *arr, int l, int r);

/* return the median of an array */
float median(float *, int);

/* return the mean of an array */
float mean(float *, int);

/* return the mean of an array after iertatively clipping */
float meanclip(float *, int);

/* global variables for efficiency reasons !! */
float Glo_clip, Ghi_clip; /* low and high sigma clip values (CLIPMEAN
                             coaddition method) */

/*----------------------------------------------------------------------------
                            Main code
   ---------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  int a, i, j;                         /* loop variables */
  int N1first         = 0, N2first = 0, N1 = 0, N2 = 0;
  int ninputimages    = 0;             /* the total number of given input
                                          images */
  int nimages         = 0;             /* the total number of input
                                          images. Tis is ninputimages minus
                                          those which have a BADCCD keyword
                                          set */
  fitsheader **fitsin = NULL, *fitsout = NULL;
  FILE *       ilist;
  FILE **      ipf, *opf;              /* File handles for input and
                                          result image data */
  float lo_rej = -60000.0;             /* low pixel reject. threshhold for
                                          statistics */
  float hi_rej = 60000.0;              /* high pixel reject. threshhold for
                                          statistics */
  int lo_rank  = 0;                    /* low pixels to be rejected
                                          before coaddition */
  int hi_rank  = 0;                    /* high pixels to be rejected
                                          before coaddition */
  int rescale  = 0;                    /* should images be rescaled
                                          before coaddition (1=YES) */
  char         inputlist[MAXCHAR] = "";        /* list with input images */
  char         imagenames[MAXIMAGE][MAXCHAR];  /* names of input images */
  float        inputmodes[MAXIMAGE];
  float        resultmode              = 0.0;
  float        badresultmode           = 0.0; /* mode of 'stacks' to which no
                                                 images contribute */
  char         outfilename[MAXCHAR]    = "imcombflat.fits";
  char         badccdkey[MAXCHAR]      = "";
  int          imagebad                = 0;
  fitscomment *badccdcomment           = NULL;
  int          outbitpix               = -32; /* bitpix of result image */
  char         coadditionmode[MAXCHAR] = "MEDIAN";
  float ***    inputline               = NULL; /* 'batch' lines of all input
                                          images */
  float *outputline = NULL;            /* the currently considered
                                          line from the result */
  float *pixstack;                     /* contains pixel values from
                                          the input images that are to
                                          be stacked by median or
                                          mean */
  int batchsize = 64;                  /* the number of lines to be
                                          read from the input images
                                          in one go (to save I/O) */
  int origbatchsize;                   /* contains the same value as
                                          batchsize; as batchsize is
                                          changed we need a copy to
                                          cleanly free memory at the
                                          end */
  int nlinesprocessed = 0;
  int done            = 0;
  int nactupix        = 0;             /* the number of remaining
                                          pixels (after rejections due
                                          to thresshholds) at the
                                          current stack position */
  float (*coaddfunc)(float *, int);         /* pointer to the
                                               coaddition function:
                                               mean or median */
  char tmpstring[MAXCHAR];             /* temporary strings used for
                                          output or out header
                                          construction */

  /* set further default values */
  origbatchsize = batchsize;
  Glo_clip      = Ghi_clip = 3.0;

  /* read comand line arguments */
  if (argc < 2) {
    printUsage();
  }

  for (a = 1; a < argc; a++) {
    if (argv[a][0] == '-') {
      switch ((int)tolower((int)argv[a][1])) {
      /*-------- Config filename */
      case 'h':    printUsage();
        break;
      case 'i':     if (a < (argc - 1)) {
          strncpy(inputlist, argv[++a], MAXCHAR - 1);
      } else {
          printUsage();
      }
        break;
      case 'o':     if (a < (argc - 1)) {
          strncpy(outfilename, argv[++a], MAXCHAR - 1);
      } else {
          printUsage();
      }
        break;
      case 'e':     if (a < (argc - 2)) {
          lo_rank = atoi(argv[++a]);
          hi_rank = atoi(argv[++a]);
      } else {
          printUsage();
      }
        break;
      case 't':     if (a < (argc - 2)) {
          lo_rej = atof(argv[++a]);
          hi_rej = atof(argv[++a]);
      } else {
          printUsage();
      }
        break;
      case 'l':     if (a < (argc - 2)) {
          Glo_clip = atof(argv[++a]);
          Ghi_clip = atof(argv[++a]);
      } else {
          printUsage();
      }
        break;
      case 'c':     if (a < (argc - 1)) {
          strncpy(coadditionmode, argv[++a], MAXCHAR - 1);
      } else {
          printUsage();
      }
        break;
      case 'f':     if (a < (argc - 1)) {
          badresultmode = atof(argv[++a]);
      } else {
          printUsage();
      }
        break;
      case 'm':     if (a < (argc - 1)) {
          resultmode = atof(argv[++a]);
      } else {
          printUsage();
      }
        break;
      case 's':     if (a < (argc - 1)) {
          rescale = atoi(argv[++a]);
      } else {
          printUsage();
      }
        break;
      case 'b':     if (a < (argc - 1)) {
          outbitpix = atoi(argv[++a]);
      } else {
          printUsage();
      }
        break;
      case 'k':     if (a < (argc - 1)) {
          strncpy(badccdkey, argv[++a], MAXCHAR - 1);
      } else {
          printUsage();
      }
        break;
      default:     printUsage();
        break;
      }
    }
  }

  /* Print welocme message */
  fprintf(stdout, "imcombflat: %s\n\n", __PIPEVERS__);

  /* check if an input list was given */
  if (strcmp(inputlist, "") == 0) {
    printUsage();
  }

  /* read input images */
  if ((ilist = fopen(inputlist, "r")) == NULL) {
    fprintf(stderr, "Could not read input list %s!\n", inputlist);
    exit(1);
  }

  while (fgets(tmpstring, MAXCHAR - 1, ilist) && ninputimages < MAXIMAGE) {
    i = sscanf(tmpstring, "%s %f", imagenames[ninputimages],
               &(inputmodes[ninputimages]));

    /* do not consider comment lines; see if the first character
       in imagesnames is '#' */
    j = 0;
    while (imagenames[j][0] == ' ') {
      j++;
    }

    if (imagenames[j][0] != '#') {
      if (i < 2) {
        if (i == 0) {
          error_exit("Errorneous input list: aborting!!");
        } else {
          /* no mode is found: disable rescaling !! */
          if (rescale == 1) {
            rescale = 0;
            fprintf(stderr,
                    "Warning: no modes in input file; rescaling disabled !!\n");
          }
        }
      }
      ninputimages++;
    }
  }

  /* allocate input stuff */
  IMCAT_CALLOC(fitsin, fitsheader *, ninputimages);
  IMCAT_CALLOC(ipf, FILE *, ninputimages);
  IMCAT_CALLOC(inputline, float **, ninputimages);

  for (i = 0; i < ninputimages; i++) {
    IMCAT_CALLOC(inputline[i], float *, batchsize);
  }

  /* open input images and reject those which have the BADCCD keyword
     set */
  for (i = 0; i < ninputimages; i++) {
    if ((ipf[i] = fopen(imagenames[i], "r")) == NULL) {
      fprintf(stderr, "Cannot read input image %s: aborting !!\n",
              imagenames[i]);
      exit(1);
    }
    fitsin[i] = readfitsheader(ipf[i]);
    N1        = fitsin[i]->n[0];
    N2        = fitsin[i]->n[1];

    if (i == 0) {
      N1first = N1;
      N2first = N2;
    }
    if ((N1 != N1first) || (N2 != N2first)) {
      error_exit("Inconsistent image sizes in input images\n");
    }

    imagebad = 0;    /* is the currently examined image bad ? */
    if (strcmp(badccdkey, "") != 0) {
      badccdcomment = getcommentbyname(badccdkey, fitsin[i]);

      if (badccdcomment == NULL) {
        fprintf(stderr, "imcombflat: No %s keyword in image %s\n",
                badccdkey, imagenames[i]);
      } else {
        if ((int)getnumericvalue(badccdcomment) != 0) {
          fprintf(stderr,
                  "imcomblfat: Image %s has the %s key set! I reject it!\n",
                  imagenames[i], badccdkey);
          imagebad = 1;
        }
      }
    }

    if (imagebad == 0) {
      inputmodes[nimages] = inputmodes[i];

      /* allocate memory for input lines */
      for (j = 0; j < batchsize; j++) {
        IMCAT_CALLOC(inputline[nimages][j], float, N1first);
      }
      nimages++;
    }
  }

  /* output result image */
  if ((opf = fopen(outfilename, "w")) == NULL) {
    fprintf(stderr, "error opening output image %s: aborting!\n",
            outfilename);
    exit(1);
  }

  switch (outbitpix) {
  case 16:
    fitsout           = new2Dfitsheader(N1, N2, SHORT_PIXTYPE);
    fitsout->bscaling = 1;
    fitsout->bzero    = 32768.0;
    fitsout->bscale   = 1.0;
    break;
  default:
    fitsout = new2Dfitsheader(N1, N2, FLOAT_PIXTYPE);
  }
  fitsout->opstream = opf;

  /* determine scaling factors for images; in case where no rescaling
     is applied this is set to 1 */
  if (rescale == 1) {
    if (nimages > 0) {
      if (resultmode < 1.0e-24) {
        for (i = 0; i < nimages; i++) {
          resultmode += inputmodes[i];
        }
        resultmode /= nimages;
      }
    }

    /* save the scaling factors in the input mode variables */
    for (i = 0; i < nimages; i++) {
      inputmodes[i] = resultmode / inputmodes[i];
    }
  } else {
    for (i = 0; i < nimages; i++) {
      inputmodes[i] = 1.0;
    }
  }

  /* write comments into the new FITS header and on screen */

  /* First see whether the outut image will be 'bad' and set
     the BADCCD keyword if necessary */
  if (strcmp(badccdkey, "") != 0) {
    if (nimages - lo_rank - hi_rank <= 1) {
      appendcomment(newnumericcomment(badccdkey, (double)1,
                                      "Is_CCD_Bad_(1=Yes)"), fitsout);
    } else {
      appendcomment(newnumericcomment(badccdkey, (double)0,
                                      "Is_CCD_Bad_(1=Yes)"), fitsout);
    }
  }

  appendcomment(newtextcomment("HISTORY", "", NULL), fitsout);
  sprintf(tmpstring, "imcombflat: %s", __PIPEVERS__);
  /* fprintf(stdout, "\n%s\n", tmpstring); */
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  sprintf(tmpstring, "imcombflat: called at %s", get_datetime_iso8601());
  /* fprintf(stdout, "\n%s\n", tmpstring); */
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  add_comment(argc, argv, fitsout);

  if (strncmp(coadditionmode, "MEAN", 4) == 0) {
    sprintf(tmpstring, "imcombflat: mean of %d images", nimages);
  } else {
    if (strncmp(coadditionmode, "CLIPMEAN", 8) == 0) {
      sprintf(tmpstring, "imcombflat: meanclip of %d images", nimages);
    } else {
      sprintf(tmpstring, "imcombflat: median of %d images", nimages);
    }
  }
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  fprintf(stdout, "%s\n", tmpstring);

  sprintf(tmpstring, "imcombflat: rejecting low/high pixels: %d/%d",
          lo_rank, hi_rank);
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  fprintf(stdout, "%s\n", tmpstring);
  sprintf(tmpstring, "imcombflat: Threshholding: [%f; %f]", lo_rej, hi_rej);
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  fprintf(stdout, "%s\n", tmpstring);

  if (rescale == 1) {
    if (nimages - lo_rank - hi_rank > 0) {
      sprintf(tmpstring, "imcombflat: rescaling with resultmode %f",
              resultmode);
    } else {
      sprintf(tmpstring,
              "imcombflat: no images contribute to stack! Resultmode set to %f",
              badresultmode);
    }
  } else {
    sprintf(tmpstring, "imcombflat: no rescaling");
  }
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  fprintf(stdout, "%s\n", tmpstring);

  for (i = 0; i < nimages; i++) {
    if (rescale == 1) {
      sprintf(tmpstring, "imcombflat: %s %f",
              get_basename(imagenames[i]), resultmode / inputmodes[i]);
    } else {
      sprintf(tmpstring, "imcombflat: %s", get_basename(imagenames[i]));
    }
    appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
    fprintf(stdout, "%s\n", tmpstring);
  }

  writefitsheader(fitsout);

  /* some allocations for the coaddition process */
  IMCAT_CALLOC(outputline, float, N1first);
  IMCAT_CALLOC(pixstack, float, nimages);

  /* set the coaddition fuction pointer */
  if (strncmp(coadditionmode, "MEAN", 4) == 0) {
    coaddfunc = mean;
  } else {
    if (strncmp(coadditionmode, "CLIPMEAN", 8) == 0) {
      coaddfunc = meanclip;
    } else {
      coaddfunc = median;
    }
  }

  /* first give a warning if the result only consists of one value */
  if (nimages - lo_rank - hi_rank <= 1) {
    fprintf(stdout,
            "Only 0 or 1 images effectively enter the coaddition !!!!\n");
  }

  /* main loop for image coaddition */
  while (done != 1) {
    if (nlinesprocessed + batchsize > N2first) {
      batchsize = N2first - nlinesprocessed;
      done      = 1;
    }
    for (i = 0; i < nimages; i++) {
      for (j = 0; j < batchsize; j++) {
        readfitsline(inputline[i][j], fitsin[i]);
      }
    }
    for (i = 0; i < batchsize; i++) {
      for (j = 0; j < N1first; j++) {
        nactupix = 0;
        for (a = 0; a < nimages; a++) {
          pixstack[nactupix] = inputline[a][i][j];
          if (pixstack[nactupix] > lo_rej &&
              pixstack[nactupix] < hi_rej) {
            pixstack[nactupix] *= inputmodes[a];
            nactupix++;
          }
        }
        if ((nactupix - lo_rank - hi_rank) <= 0) {
          outputline[j] = badresultmode;
        } else {
          /* sort array */
          float_qsort(pixstack, 0, nactupix - 1);
          outputline[j] = coaddfunc(&(pixstack[lo_rank]),
                                    nactupix - lo_rank - hi_rank);
        }
      }
      writefitsline(outputline, fitsout);
    }
    nlinesprocessed += batchsize;
    fprintf(stderr, " %d lines done\r", nlinesprocessed);
  }

  /* close files and release memory */
  for (i = 0; i < nimages; i++) {
    if (0 != fclose(ipf[i])) {
      error_exit("error closing input files!");
    }
  }

  writefitstail(fitsout);

  if (0 != fclose(opf)) {
    error_exit("error closing output file!");
  }

  IMCAT_FREE(ipf);
  IMCAT_FREE(pixstack);
  IMCAT_FREE(outputline);
  delfitsheader(fitsout);

  for (i = 0; i < nimages; i++) {
    for (j = 0; j < origbatchsize; j++) {
      IMCAT_FREE(inputline[i][j]);
    }
    IMCAT_FREE(inputline[i]);
    delfitsheader(fitsin[i]);
  }
  IMCAT_FREE(fitsin);
  IMCAT_FREE(inputline);

#ifdef MEMORY_DEBUG
  xmemory_status();
#endif

  return(0);
}

/*
 * This function only gives the usage for the program
 */

void printUsage()
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "        imcombflat - stack FITS images on a pixel basis\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "        imcomblfat [options...]\n");
  fprintf(stdout, "              -h  print this and leave\n");
  fprintf(stdout, "              -i  file listing input images\n");
  fprintf(stdout, "                  and their modes\n");
  fprintf(stdout, "            [ -o  output result file                               ]\n");
  fprintf(stdout, "            [     (default: imcombflat.fits)                       ]\n");
  fprintf(stdout, "            [ -b  output bitpix (16 or -32)                        ]\n");
  fprintf(stdout, "            [     (default: -32)                                   ]\n");
  fprintf(stdout, "            [ -e  lo_rank hi_rank                                  ]\n");
  fprintf(stdout, "            [     low and high ranks to exclude during coaddition  ]\n");
  fprintf(stdout, "            [     (default: to 0 0)                                ]\n");
  fprintf(stdout, "            [ -t  lo_rej hi_rej                                    ]\n");
  fprintf(stdout, "            [     define low/high threshholds for pixels to        ]\n");
  fprintf(stdout, "            [     be considered in the coaddition process          ]\n");
  fprintf(stdout, "            [ -l  lo_clip hi_clip                                  ]\n");
  fprintf(stdout, "            [     define low/high sigma clip values                ]\n");
  fprintf(stdout, "            [     for pixels to be considered                      ]\n");
  fprintf(stdout, "            [     in the CLIPMEAN coaddition.                      ]\n");
  fprintf(stdout, "            [     (default: 3.0 3.0)                               ]\n");
  fprintf(stdout, "            [ -c  coaddition method                                ]\n");
  fprintf(stdout, "            [     ('MEAN', 'CLIPMEAN' or 'MEDIAN')                 ]\n");
  fprintf(stdout, "            [     (default: MEDIAN)                                ]\n");
  fprintf(stdout, "            [ -s  rescale input images before coaddition           ]\n");
  fprintf(stdout, "            [     (1=YES, 0=NO)                                    ]\n");
  fprintf(stdout, "            [     (default: 0)                                     ]\n");
  fprintf(stdout, "            [ -m  define mode of result image in case of rescaling ]\n");
  fprintf(stdout, "            [     turned on                                        ]\n");
  fprintf(stdout, "            [     (default: 0; mean of modes from input data)      ]\n");
  fprintf(stdout, "            [ -f  define mode of result image in case no images    ]\n");
  fprintf(stdout, "            [     contribute (faulty and bad images)               ]\n");
  fprintf(stdout, "            [     (default: 0.0)                                   ]\n");
  fprintf(stdout, "            [ -k  header keyword indcating a bad CCD               ]\n");
  fprintf(stdout, "            [     (default: NONE)                                  ]\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "     The program stacks a set of 2d FITS images of equal size.  The\n");
  fprintf(stdout, "     stacking is done by collecting all input pixels on a given array\n");
  fprintf(stdout, "     position, rejecting the 'low_rank' lowest and 'high_rank' highest\n");
  fprintf(stdout, "     values. In this process, only pixels with values between 'lo_rej'\n");
  fprintf(stdout, "     and 'hi_rej' are at all considered. From the remaining pixel\n");
  fprintf(stdout, "     values, a straight median, mean or a sigma clipped mean is given\n");
  fprintf(stdout, "     as result. In the latter case mean and sigma are estimated,\n");
  fprintf(stdout, "     pixels below and above defined thresholds are rejected and the\n");
  fprintf(stdout, "     process is repeated until the array does not change anymore or\n");
  fprintf(stdout, "     until 20 iterations are reached. In the rejection process all\n");
  fprintf(stdout, "     points entering are again considered, i.e. previously rejected\n");
  fprintf(stdout, "     points are again allowed to reenter the process.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "     If a BADCCD header keyword is considered (command line option '-k'),\n");
  fprintf(stdout, "     images having a BADCCD value unequal to zero are not considered\n");
  fprintf(stdout, "     in the stacking process. An output image gets a BADDCCD keyword\n");
  fprintf(stdout, "     of '1' if no useful stacking could be performed (no images contribute\n");
  fprintf(stdout, "     to the output etc.). In the case that no (valid) images\n");
  fprintf(stdout, "     contribute to the output stack, an image with a single value (-f\n");
  fprintf(stdout, "     command line option) is created. \n");
  fprintf(stdout, "\n");
  fprintf(stdout, "     The input images are given in a file containing on each line one\n");
  fprintf(stdout, "     input image and optionally its mode. Such a file can directly be\n");
  fprintf(stdout, "     produced with the 'imstats' program. An example file could look like: \n");
  fprintf(stdout, "\n");
  fprintf(stdout, "     # imstats: Pipeline Global Version: 0.9\n");
  fprintf(stdout, "     # imstats: called at 2005-02-07T00:21:24\n");
  fprintf(stdout, "     # imstats: estimating statistics in\n");
  fprintf(stdout, "     # the whole frames within -60000.000000 < pixvalue < 60000.000000\n");
  fprintf(stdout, "     #\n");
  fprintf(stdout, "     # imstats: filename     mode    median  mean    sigma\n");
  fprintf(stdout, "     test_1.fits       1.01    1.06    1.02    2.00\n");
  fprintf(stdout, "     test_2.fits       1.99    2.02    1.99    2.00\n");
  fprintf(stdout, "     test_3.fits       3.00    2.99    3.01    2.01\n");
  fprintf(stdout, "     test_4.fits       4.00    3.94    4.01    2.01\n");
  fprintf(stdout, "     test_5.fits       5.00    5.03    5.01    1.90\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "     The input images can be rescaled to a provided level (options -s and\n");
  fprintf(stdout, "     -m) if desired.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR:\n");
  fprintf(stdout, "   Thomas Erben (terben@astro.uni-bonn.de)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   The code is heavily based on Nick Kaiser's 'combineimages'\n");
  fprintf(stdout, "   routine within imcat.\n");

  exit(1);
}

/*-------------------------------------------------------------------------*/

/**
   @brief	Estimate the median of an array.
   @param	array	Array of floats to consider
   @param	n	Number of floats in the array.
   @return	the median of 'array'

   Calculate the median of an array of float numbers. The median is
   estimated by sorting the array and returning the 'middle' element.
   If the number of elements is even, the mean of the two 'middle'
   elements is returned.
 */
/*--------------------------------------------------------------------------*/

float median(float *array, int n)
{
  float result;

  int middle;

  middle = n / 2;

  if (n % 2 == 0) {
    result = (array[middle] + array[middle - 1]) / 2.;
  } else {
    result = array[middle];
  }

  return(result);
}

/*-------------------------------------------------------------------------*/

/**
   @brief	Calculate the mean of an array.
   @param	array	Array of floats to consider
   @param	n	Number of floats in the array.
   @return	the mean of 'array'

   Calculate the mean of an array of float numbers.
 */
/*--------------------------------------------------------------------------*/

float mean(float *array, int n)
{
  float result = 0.0;

  int i;

  for (i = 0; i < n; i++) {
    result += array[i];
  }

  result /= n;
  return(result);
}

/*-------------------------------------------------------------------------*/

/**
   @brief	Calculate the mean of an array after sigma clipping.
   @param	array	 Array of floats to consider
   @param	n	 Number of floats in the array.
   @param	lowclip	 sigma for clipping on the low end of the array.
   @param	highclip sigma for clipping at the high end of the arraz.
   @return	the mean of 'array'

   Calculate the mean of an array of float numbers after ierative sigma
   clipping. From the initial array, mean and sigma is estimated. Points
   lying lowclip*sigma below or highclip*sigma above the mean are rejected
   and a new mean and sigma are estimated. This process is repeated until
   no more points are rejected or added to the array or until 20 iertations
   are done.
 */
/*--------------------------------------------------------------------------*/

float meanclip(float *array, int n)
{
  /* the variables Glo_clip and Ghi_clip are global !! */
  int   cont   = 1;
  int   nstart = 0, nend = n;
  int   i;
  int   nactu;
  int   nstartnew, nendnew;
  float meanactu, sigmaactu;
  int   iterations    = 0;
  int   maxiterations = 20;

  /* in case that less than 4 elements are in the array
     we return a straight mean */

  if (n <= 3) {
    return(mean(array, n));
  }

  nactu = nend - nstart;

  while (cont == 1) {
    meanactu = sigmaactu = 0.0;
    iterations++;

    for (i = 0; i < nactu; i++) {
      meanactu  += array[nstart + i];
      sigmaactu += array[nstart + i] * array[nstart + i];
    }

    meanactu /= nactu;

    /* sanity check if all array elements are the same */
    if (sigmaactu < 1.0e-06) {
      sigmaactu = 0.0;
    } else {
      sigmaactu = sqrt((1. / (nactu - 1)) * (sigmaactu - nactu * meanactu * meanactu));
    }


    /* determine new nstart and nend values;
       note that the array is sorted !!
     */
    nstartnew = nendnew = 0;
    i         = 0;
    while ((array[i] < meanactu + Ghi_clip * sigmaactu) && (i < n)) {
      if (array[i] < meanactu - Glo_clip * sigmaactu) {
        nstartnew++;
      }
      i++;
    }

    nendnew = i;

    /* leave the loop if either:
       - if max iterations is reached
       - if no more points are rejected or added to the array
       - if 2 points or less are left in the process
     */
    if ((nactu = (nendnew - nstartnew)) < 3 ||
        (iterations >= maxiterations) ||
        (nstartnew == nstart && nendnew == nend)) {
      cont = 0;
    }
    nstart = nstartnew;
    nend   = nendnew;
  }

  return(meanactu);
}

/*-------------------------------------------------------------------------*/

/**
   @brief	Sort an array of floats by increasing value.
   @param	a   Array of floats to consider
   @param	l	lower array index
   @param	r	higher array index
   @return	void

   Fast sort on an array of floats. The array is sorted between the
   indices l and r. The alorithm is a modified quicksort which
   uses insertion sort for subfields with less than 'M' elements.
   The pivot element for each subdivision is chosen with a 'median
   of three' strategy.
 */
/*--------------------------------------------------------------------------*/

#define M    10
void float_qsort(float *a, int l, int r)
{
  int   i, j, m;
  float tmp;       /* change the type of this variable if you
                      rewrite the sort for another variable type */
  if (r - l > M) {
    /* quicksort with a median of three strategy
       to choose the pivot element
     */
    i = l - 1; j = r;
    m = l + (r - l) / 2;
    if (a[l] > a[m]) {
      tmp = a[l]; a[l] = a[m]; a[m] = tmp;
    }
    if (a[l] > a[r]) {
      tmp = a[l]; a[l] = a[r]; a[r] = tmp;
    } else if (a[r] > a[m]) {
      tmp = a[r]; a[r] = a[m]; a[m] = tmp;
    }

    for (;;) {
      while (a[++i] < a[r]) {
        ;
      }
      while (a[--j] > a[r]) {
        ;
      }
      if (i >= j) {
        break;
      }
      tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
    tmp = a[i]; a[i] = a[r]; a[r] = tmp;

    float_qsort(a, l, i - 1);
    float_qsort(a, i + 1, r);
  } else {
    /* insertion sort */
    for (i = l + 1; i <= r; ++i) {
      tmp = a[i];
      for (j = i - 1; j >= l && tmp < a[j]; --j) {
        a[j + 1] = a[j];
      }
      a[j + 1] = tmp;
    }
  }
}

#undef M
