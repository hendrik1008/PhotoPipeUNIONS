/**
 **
 ** calculate simple statistics for an image
 **
 **/

/*
   06.06.03:
   I removed the not needed 3D image stuff

   24.01.2005:
   I changed exit(0) to return 0 in the main program
   to avoid compiler warnings

   31.01.2005:
   I adapted the call of fdo_stats to the new syntax
   of this function. The functionality of this program
   is not changed.

   07.02.2005:
   - I included memory tracing capabilities from xmemory.
   - I cleanly free memory at the end of the program.

   24.09.2007:
   A new command line option '-t' is added. It provides
   threshold values for pixels entering statistics.

   24.08.2010:
   Removal of remarks raised by the Intel icc compiler;
   (removal of never-used variables)

   03.04.2013:
   xmemory is no longer used by default but only if the variable
   __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
   etc. calls are used for memory management.
 */

/* uncomment the following line to obtain a status on allocated/freed
   memory at the end of the program. Only available if also
   __XMEM_DEBUG__ is defined at compile time.
 */

/* #define MEMORY_DEBUG */

#define usage    "\n\n\n\
NAME\n\
	stats -- calculate simple statistics for an image\n\
\n\
SYNOPSIS\n\
	stats   [option...]\n\
		-m n	    # ignore outer n pixel margin (def = 0)\n\
		-t min max  # low and high threshholds of pixels to\n\
                            # be considerd (def: all pixels)\n\
		-v stat	    # just output value for 'stat', which\n\
			      can be one of:\n\
			      N1 N2 pixtype min max mean mode median\n\
			      lquart uquart sigma\n\
\n\
DESCRIPTION\n\
	\"stats\" reads a 2-D fits file from stdin\n\
	and writes the descriptive statistics listed above to stdout.\n\
\n\
AUTHOR\n\
	Nick Kaiser:  kaiser@cita.utoronto.ca\n\
        Thomas Erben: terben@astro.uni-bonn.de\n\
\n\n\n"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#include "stats_stuff.h"
#include "error.h"
#include "arrays.h"
#include "fits.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

int main(int argc, char *argv[])
{
  int         i, nplanes, arg = 1, N1, N2, margin = 0, statarg = 0;
  float **    f;
  fstatsrec   srec;
  fitsheader *fits;
  FILE *      opf;
  float       lo_reject, hi_reject;

  /* default variable values */
  opf       = stdout;
  lo_reject = -FLT_MAX;
  hi_reject = FLT_MAX;

  /* read command line arguments */
  while (arg < argc) {
    if (*argv[arg] != '-') {
      error_exit(usage);
    }
    switch (*(argv[arg++] + 1)) {
    case 'm':
      if (1 != sscanf(argv[arg++], "%d", &margin)) {
        error_exit(usage);
      }
      break;
    case 't':
      if (1 != sscanf(argv[arg++], "%f", &lo_reject)) {
        error_exit(usage);
      }
      if (1 != sscanf(argv[arg++], "%f", &hi_reject)) {
        error_exit(usage);
      }
      break;
    case 'u':
      error_exit(usage);
      break;
    case 'v':
      statarg = arg++;
      break;
    default:
      return(0);

      break;
    }
  }

  fits = readfitsheader(stdin);
  N1   = fits->n[0];
  N2   = fits->n[1];
  allocFloatArray(&f, N1, N2);


  nplanes = 1;
  for (i = 2; i < fits->ndim; i++) {
    nplanes *= fits->n[i];
  }

  if (statarg) {
    if (!strcmp(argv[statarg], "N1")) {
      fprintf(opf, "%d\n", N1);
      exit(0);
    }
    if (!strcmp(argv[statarg], "N2")) {
      fprintf(opf, "%d\n", N2);
      exit(0);
    }
    if (!strcmp(argv[statarg], "pixtype")) {
      fprintf(opf, "%d\n", fits->extpixtype);
      exit(0);
    }
  }

  for (i = 0; i < nplanes; i++) {
    readfitsplane((void **)f, fits);
    fdo_stats(f, N1, N2, margin, N1 - margin, margin, N2 - margin,
              lo_reject, hi_reject, &srec);

    if (statarg) {
      if (!strcmp(argv[statarg], "min")) {
        fprintf(opf, "%g\n", srec.fmin);
      }
      if (!strcmp(argv[statarg], "max")) {
        fprintf(opf, "%g\n", srec.fmax);
      }
      if (!strcmp(argv[statarg], "mean")) {
        fprintf(opf, "%g\n", srec.fmean);
      }
      if (!strcmp(argv[statarg], "mode")) {
        fprintf(opf, "%g\n", srec.fmode);
      }
      if (!strcmp(argv[statarg], "median")) {
        fprintf(opf, "%g\n", srec.fmedian);
      }
      if (!strcmp(argv[statarg], "lquart")) {
        fprintf(opf, "%g\n", srec.flowerquartile);
      }
      if (!strcmp(argv[statarg], "uquart")) {
        fprintf(opf, "%g\n", srec.fupperquartile);
      }
      if (!strcmp(argv[statarg], "sigma")) {
        fprintf(opf, "%g\n", srec.sigma);
      }
    } else {
      if (nplanes == 1) {
        fprintf(opf, "%4d x %4d image\n", N1, N2);
        fprintf(opf, "BITPIX = %4d\n", fits->extpixtype);
        fprintf(opf, "min = %g; max = %g\n", srec.fmin,
                srec.fmax);
        fprintf(opf, "mean       = %g\n", srec.fmean);
        fprintf(opf, "mode       = %g\n", srec.fmode);
        fprintf(opf, "median     = %g\n", srec.fmedian);
        fprintf(opf, "quartiles  = %g %g\n", srec.flowerquartile,
                srec.fupperquartile);
        fprintf(opf, "sigma      = %g\n", srec.sigma);
        fprintf(opf, "goodpix    = %ld\n", srec.goodpix);
        fprintf(opf, "samplesize = %ld\n", srec.samplesize);
        fprintf(opf, "badpix     = %ld\n", srec.badpix);
      }
    }
  }

  /* free memory and bye */
  freeFloatArray(f);
  delfitsheader(fits);

#ifdef MEMORY_DEBUG
  xmemory_status();
#endif

  return(0);
}
