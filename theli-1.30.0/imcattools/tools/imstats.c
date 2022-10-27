/*----------------------------------------------------------------------------

   File name    :   imstats.c
   Author       :   Thomas Erben
   Created on   :   31.01.2005
   Description  :   estimates statistics for a set of FITS images

   ---------------------------------------------------------------------------*/

/*
        $Id: imstats.c,v 1.22 2018/03/28 12:06:14 thomas Exp $
        $Author: thomas $
        $Date: 2018/03/28 12:06:14 $
        $Revision: 1.22 $
 */

/*----------------------------------------------------------------------------
                                History Coments
   ---------------------------------------------------------------------------*/

/*
   31.01.2005:
   project started.

   01.02.2005:
   - I corrected an error in the reading of command line arguments
     (the index pointing to the argument to read was no longer correct
     when the filenames were not the last argument).
   - I corrected the form of CVS keywords for 'keyword substitution'.

   07.02.2005:
   - I added memory tracing capabilities by the inclusion
    of 'xmemory'.

   - I cleanly free memory at the end of the program.

   25.02.2005:
   I corrected a bug in the error message when a FITS file could not
   be opened.

   20.03.2005:
   I corrected a bug showing up when several images with different
   sizes were processed but no limits on the stats region were
   specified.

   12.09.2005:
   I corrected a bug in the definition of the imagenames array. The
   two dimensions giving the maximum number of images and the maximum
   number of string elements per entry were in the wrong order.

   28.03.2006:
   I included the lower and upper quartiles in the statistics output.

   25.04.2006:
   I corrected a bug in the 'usage' definition.

   27.04.2006:
   I corrected a bug in the scanning of input filenames.  A core dump
   could occur if a filename had more than MAXCHAR characters.

   17.05.2006:
   I deleted a never used parameter of the 'usage' function.

   22.03.2007:
   I included the output of number of 'good' pixels in absolute number
   ans as percent of the sample that is considered for
   statistics. Good pixels are those which liw within the user defined
   rejection criteria. It was included to be able to test how many
   percent of a FITS image have saturated pixels.

   10.09.2007:
   I corrected a bug when converting a command line argument to 'int'
   instead of to 'float'.

   18.09.2007:
   The number of decimal points output in the results can be given as
   command line option.

   16.12.2010:
   I introduced the possibility to list input images in a text file.
   With this we avoid too long command lines in case of many input
   images.

   11.08.2011:
   I included the minimum and maximum pixel values in the statistics.

   03.04.2013:
   xmemory is no longer used by default but only if the variable
   __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
   etc. calls are used for memory management.

   28.11.2014:
   I increased the maximum number of input images from 10000 to 30000.
   One DECam run had more than 1000 domeflat images which required to
   increase this number to be able to process them.
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

#include "stats_stuff.h"
#include "error.h"
#include "arrays.h"
#include "fits.h"
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


#ifndef MAXCHAR
#define MAXCHAR    255
#endif

#define MAXIMAGE       30000 /* the maximum number of input images allowed */

static const char notokstr[] = { " \t=,;\n\r\"" };
static void printUsage(void);

/*----------------------------------------------------------------------------
                            Main code
   ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  int a, i;
  int N1old   = 0, N2old = 0, N1, N2;
  int nimages = 0;
  float **f = NULL;         /* holds pixel data of input images */
  fstatsrec srec;
  fitsheader *fits;
  FILE *ipf, *opf = stdout;
  float lo_rej = -60000.0;      /* low pixel reject. threshhold for
                                   statistics */
  float hi_rej = 60000.0;       /* high pixel reject. threshhold for
                                   statistics */
  int ndecimals = 2;             /* number of decimal points in
                                    output numbers */
  int   xmin = 0, xmax = 0;   /* x-range for statistic estimation */
  int   ymin = 0, ymax = 0;   /* y-range for statistic estimation */
  int   rangespec = 0;        /* was a x and y-range specified ? */
  char  imagenames[MAXIMAGE][MAXCHAR];
  char  inputimagelist[MAXCHAR] = "";
  char  outfilename[MAXCHAR] = "";
  char  tmpstring[MAXCHAR];
  char  *tmpchar;
  char  formatstring[MAXCHAR];
  char *str;

  /* read comand line arguments */
  if (argc < 2) {
    printUsage();
    exit(0);
  }

  for (a = 1; a < argc; a++) {
    if (argv[a][0] == '-') {
      switch ((int)tolower((int)argv[a][1])) {
      /*-------- Config filename */
      case 'h':
        printUsage();
        exit(0);
        break;
      case 'd':
        if (a < (argc - 1)) {
          ndecimals = atoi(argv[++a]);
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 'i':
        if (a < (argc - 1)) {
          strncpy(inputimagelist, argv[++a], MAXCHAR - 1);
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 'o':
        if (a < (argc - 1)) {
          strncpy(outfilename, argv[++a], MAXCHAR - 1);
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 's':
        if (a < (argc - 4)) {
          xmin = atoi(argv[++a]);
          xmax = atoi(argv[++a]);
          ymin = atoi(argv[++a]);
          ymax = atoi(argv[++a]);
          rangespec = 1;
        } else {
          printUsage();
          exit(1);
        }
        break;
      case 't':
        if (a < (argc - 2)) {
          lo_rej = atof(argv[++a]);
          hi_rej = atof(argv[++a]);
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
      /*---- The input image filename(s) */
      for (; (a < argc) && (*argv[a] != '-'); a++) {
        for (str = NULL; (str = strtok(str ? NULL : argv[a], notokstr));
             nimages++) {
          if (nimages < MAXIMAGE) {
            strncpy(imagenames[nimages], str, MAXCHAR - 1);
          } else {
            snprintf(tmpstring, MAXCHAR - 1,
                     "*Error*: Too many input images (more than %d)\n",
                     MAXIMAGE);
            error_exit(tmpstring);
          }
        }
      }
      a--;
    }
  }

  /* test if we have a file containing input images
     if yes, read files and add them to the image list
  */
  if(strcmp(inputimagelist, "") != 0) {
    if ((ipf = fopen(inputimagelist, "r")) != NULL) {
      /* Todo: make the file reading more robust to filenames having spaces
         at the beginning and the end of their names
      */
      while((tmpchar = fgets(imagenames[nimages], MAXCHAR, ipf)) &&
            (nimages < MAXIMAGE)) {
        /* remove possible 'newline' in filenames */
        if((tmpchar = strchr(imagenames[nimages], '\n')) != NULL) {
          *tmpchar = '\0';
        }
        nimages++;
      }
      fclose(ipf);
      if(nimages == MAXIMAGE && tmpchar != NULL) {
        snprintf(tmpstring, MAXCHAR - 1,
                 "*Error*: Too many input images (more than %d)\n", MAXIMAGE);
        error_exit(tmpstring);
      }
    } else {
      snprintf(tmpstring, MAXCHAR - 1,
               "could not open file %s: aborting\n", inputimagelist);
      error_exit(tmpstring);
    }
  }

  if (strcmp(outfilename, "") != 0) {
    if ((opf = fopen(outfilename, "w")) == NULL) {
      snprintf(tmpstring, MAXCHAR - 1,
               "could not open %s for output: aborting\n", outfilename);
      error_exit(tmpstring);
    }
  }

  if (ndecimals < 0) {
    ndecimals = 2;
  }

  /* Welcome message */
  fprintf(opf, "# imstats: %s\n", __PIPEVERS__);
  fprintf(opf, "# imstats: called at %s\n", get_datetime_iso8601());
  fprintf(opf, "# imstats: estimating statistics in\n");
  if (xmax == 0 || ymax == 0) {
    fprintf(opf, "# the whole frames within %f < pixvalue < %f\n",
            lo_rej, hi_rej);
  } else {
    fprintf(opf, "# %d < x < %d, %d < y < %d within %f < pixvalue < %f\n",
            xmin, xmax, ymin, ymax, lo_rej, hi_rej);
  }
  fprintf(opf, "#\n");
  fprintf(opf, "# imstats: filename\tmode\tlquartile\tmedian\tuquartile\tmean\tsigma\tmin\tmax\tgoodpix\tgoodpix/percent\n");

  for (i = 0; i < nimages; i++) {
    if ((ipf = fopen(imagenames[i], "r")) == NULL) {
      snprintf(tmpstring, MAXCHAR - 1,
               "could not open file %s: aborting!!\n", imagenames[i]);
      error_exit(tmpstring);
    }

    /* read in FITS image header */
    fits = readfitsheader(ipf);
    N1   = fits->n[0];
    N2   = fits->n[1];

    if ((N1 != N1old) || (N2 != N2old)) {
      freeFloatArray(f);
      allocFloatArray(&f, N1, N2);
      N1old = N1;
      N2old = N2;
    }

    /* read in FITS image pixel data */
    readfitsplane((void **)f, fits);

    /* if limits have not been given we take the whole frame */
    if (0 == rangespec) {
      xmin = ymin = 0;
      xmax = N1;
      ymax = N2;
    }

    fdo_stats(f, N1, N2, xmin, xmax, ymin, ymax,
              lo_rej, hi_rej, &srec);

    snprintf(formatstring, MAXCHAR - 1,
             "%%s\t%%6.%df\t%%6.%df\t%%6.%df\t%%6.%df\t%%6.%df\t%%6.%df\t%%6.%df\t%%6.%df\t%%ld\t%%6.%df\n",
             ndecimals, ndecimals, ndecimals, ndecimals, ndecimals,
             ndecimals, ndecimals, ndecimals, ndecimals);

    fprintf(opf, formatstring,
            imagenames[i],
            srec.fmode, srec.flowerquartile, srec.fmedian,
            srec.fupperquartile, srec.fmean,
            srec.sigma, srec.fmin, srec.fmax, srec.goodpix,
            (float)srec.goodpix * 100. / ((xmax - xmin) * (ymax - ymin)));

    delfitsheader(fits);

    if (0 != fclose(ipf)) {
      snprintf(tmpstring, MAXCHAR - 1,
               "could not close %s: aborting", imagenames[i]);
      error_exit(tmpstring);
    }
  }

  freeFloatArray(f);

  if (strcmp(outfilename, "") != 0) {
    fclose(opf);
  }

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
  fprintf(stdout, "        imstats - estimate statistics from FITS images\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS:\n");
  fprintf(stdout, "        imstats images [options]\n");
  fprintf(stdout, "                   -h    # print this help and leave\n");
  fprintf(stdout, "                   -i    # file with one input image per line\n");
  fprintf(stdout, "                   -o    # output file (defaulted to stdout)\n");
  fprintf(stdout, "                   -s    # xmin xmax ymin ymax      \n");
  fprintf(stdout, "                         # define statistic region (defaulted to entire frame)\n");
  fprintf(stdout, "                   -t    # lo_rej hi_rej           \n");
  fprintf(stdout, "                         # define low/high threshholds for pixels \n");
  fprintf(stdout, "                         # to be considered (defaulted to -60000.0 60000.0)\n");
  fprintf(stdout, "                   -d    # number of decimal points in output numbers\n");
  fprintf(stdout, "                         # (defaulted to 2)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION:\n");
  fprintf(stdout, "        The program estimates statistics from the given FITS images.\n");
  fprintf(stdout, "        The program provides mode, lower quartile, median, upper quartile,\n");
  fprintf(stdout, "        mean, sigma, minimum, maximum and the number of good pixels in\n");
  fprintf(stdout, "        a tabulated form. Good pixels are defined as having values\n");
  fprintf(stdout, "        between 'lo_rej' and 'hi_rej' ('-t' command line option)\n");
  fprintf(stdout, "        Lower/upper quartile give the points where the integrated\n");
  fprintf(stdout, "        propability function of the sample points reaches 1/4 and 3/4\n");
  fprintf(stdout, "        respectively. Note that we want to estimate the 'sigma' (standard\n");
  fprintf(stdout, "        deviation) of the sky-background and we try to suppress the\n");
  fprintf(stdout, "        influence of astronomical objects on this quantity.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "EXAMPLE:\n");
  fprintf(stdout, "        imstats *.fits -s 500 1500 1500 2500 -t -50000.0 50000.0 -o stat.dat\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        This estimates from all FITS files in the current directory\n");
  fprintf(stdout, "        statistics for the given area within the specified pixel value\n");
  fprintf(stdout, "        region and prints the result to the file 'stat.dat'.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR:\n");
  fprintf(stdout, "        Thomas Erben: terben@astro.uni-bonn.de\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        The code is heavily based on Nick Kaiser's 'stats' routine within\n");
  fprintf(stdout, "        imcat.\n");
}
