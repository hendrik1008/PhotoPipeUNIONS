/*----------------------------------------------------------------------------

   File name    :   transformimage.c
   Author       :   Nick Kaiser & Thomas Erben
   Created on   :   05.06.2009
   Description  :   apply linear transformations to FITS images

 ---------------------------------------------------------------------------*/

/*
	$Id: transformimage.c,v 1.4 2018/03/28 12:06:15 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:15 $
	$Revision: 1.4 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
  05.06.2009:
  Nick Kaisers imcat tool 'transformimage' integrated to THELI

  03.04.2013:
  xmemory is no longer used by default but only if the variable
  __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
  etc. calls are used for memory management.
 */

/*----------------------------------------------------------------------------
                                TODOS
 ---------------------------------------------------------------------------*/

/*
 05.06.2009:
 !!understand and fix memory (de)allocation problems with image headers!!

 */

/*----------------------------------------------------------------------------
                                Includes and Defines
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "arrays.h"
#include "error.h"
#include "fits.h"
#include "map.h"

#include "global_vers.h"
#include "t_iso8601.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

/* making psi and t global is necessary because function deflection is
   passed as pointer to mapping functions.  Hence, there is no freedom
   to change the argument list of that function. deflection, on the
   other hand, needs direct access to psi and t. */
float psi[2][2], t[2];

int deflection(float ri, float rj,
               float *di, float *dj);

void printUsage(void);

#ifndef TINY
#define TINY    1.e-50
#endif

#ifndef MAXCHAR
#define MAXCHAR 256
#endif

/* uncomment the following line to obtain a status on allocated/freed
   memory at the end of the program. Only available if also __XMEM_DEBUG__
   is defined at compile time.
*/

/* #define MEMORY_DEBUG */

/*----------------------------------------------------------------------------
                                Main Function
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
  int         arg = 1;
  int         N1, N2, M1, M2, mapmode, centrefixed, inverse;
  float       **fsource = NULL, **ftarget = NULL;
  float       a, b, c, d, t0, t1, det;
  FILE        *targetf = NULL;
  fitsheader  *fitsin = NULL, *fitsout = NULL;
  char        tmpstring[MAXCHAR];


  /* defaults */
  psi[0][0]   = psi[1][1] = 1.0;
  psi[1][0]   = psi[0][1] = 0.0;
  t[0]        = t[1] = 0.0;
  M1          = M2 = 0;
  mapmode     = FAST_MAP_MODE;
  centrefixed = 0;
  inverse     = 0;

  /* parse args */
  while (arg < argc) {
    if (argv[arg][0] != '-') {
      printUsage();
      exit(1);
    }
    switch (argv[arg++][1]) {
    case 'p':
      if (1 != sscanf(argv[arg++], "%f", &(psi[0][0])) ||
          1 != sscanf(argv[arg++], "%f", &(psi[0][1])) ||
          1 != sscanf(argv[arg++], "%f", &(psi[1][0])) ||
          1 != sscanf(argv[arg++], "%f", &(psi[1][1]))) {
        printUsage();
        exit(1);
      }
      break;
    case 't':
      if (1 != sscanf(argv[arg++], "%f", &(t[0])) ||
          1 != sscanf(argv[arg++], "%f", &(t[1]))) {
        printUsage();
        exit(1);
      }
      break;
    case 'n':
      if (1 != sscanf(argv[arg++], "%d", &M1) ||
          1 != sscanf(argv[arg++], "%d", &M2)) {
        printUsage();
        exit(1);
      }
      break;
    case 'm':
      if (1 != sscanf(argv[arg++], "%d", &mapmode)) {
        printUsage();
        exit(1);
      }
      break;
    case 'c':
      centrefixed = 1;
      break;
    case 'i':
      inverse = 1;
      break;
    case 'f':
      targetf = fopen(argv[arg++], "r");
      if (!targetf) {
        printUsage();
        exit(1);
      }
      break;
    default:
      printUsage();
      exit(1);
      break;
    }
  }

  if (inverse) {
    a   = psi[0][0];
    b   = psi[0][1];
    c   = psi[1][0];
    d   = psi[1][1];
    det = a * d - c * b;
    if (fabs(det) < TINY) {
      printUsage();
      exit(1);
    }
    t0        = t[0];
    t1        = t[1];
    psi[0][0] = d / det;
    psi[0][1] = -b / det;
    psi[1][0] = -c / det;
    psi[1][1] = a / det;
    t[0]      = -(psi[0][0] * t0 + psi[0][1] * t1);
    t[1]      = -(psi[1][0] * t0 + psi[1][1] * t1);
  }

  /* read the source file */
  read2Dfloatimage(&fsource, &N1, &N2, &fitsin, stdin);
  if (!M1 || !M2) {
    M1 = N1;
    M2 = N2;
  }

  /* create or read the target image */
  if (targetf) {
    read2Dfloatimage(&ftarget, &M1, &M2, &fitsout, targetf);
  } else {
    fitsout       = copyfitsheader(fitsin);
    fitsout->n[0] = M1;
    fitsout->n[1] = M2;
    allocFloatArray(&ftarget, M1, M2);
  }

  /* subtract delta_ij */
  psi[0][0] -= 1.0;
  psi[1][1] -= 1.0;

  if (centrefixed) {
    t[0] = t[1] = 0;
    deflection(N2 / 2, N1 / 2, &(t[1]), &(t[0]));
    t[0] *= -1;
    t[1] *= -1;
  }

  /* apply the mapping */
  switch (mapmode) {
  case ULTRAFAST_MAP_MODE:
    ultrafastmap(ftarget, M1, M2, fsource, N1, N2, deflection);
    break;
  case FAST_MAP_MODE:
    fastmap(ftarget, M1, M2, fsource, N1, N2, deflection);
    break;
  case TRIANGLE_MAP_MODE:
    map(ftarget, M1, M2, fsource, N1, N2, deflection);
    break;
  default:
    printUsage();
    exit(1);	
    break;
  }

  /* write the target image */
  /* add history information to output header */
  appendcomment(newtextcomment("HISTORY", "", NULL), fitsout);
  snprintf(tmpstring, MAXCHAR-1, "transformimage: %s", __PIPEVERS__);
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  snprintf(tmpstring, MAXCHAR-1, "transformimage: called at %s",
           get_datetime_iso8601());
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  add_comment(argc, argv, fitsout);

  write2Dfloatimage(ftarget, fitsout);

  /* free memory */
  freeFloatArray(ftarget);
  freeFloatArray(fsource);

  /* !! The following frees produce NOT UNDERSTOOD memory
     tracing errors !! */
  /*delfitsheader(fitsin); */
  /*delfitsheader(fitsout);*/

#ifdef MEMORY_DEBUG
        xmemory_status() ;
#endif

  /* all done */
  return 0;
}

int deflection(float y, float x,
               float *dy, float *dx)
{
  *dx = psi[0][0] * x + psi[0][1] * y + t[0];
  *dy = psi[1][0] * x + psi[1][1] * y + t[1];
  return(1);
}

/*
 * This function only gives the usage message for the program
 */

void printUsage(void)
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "        transformimage - apply spatial linear transformation to a FITS image\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "	transformimage [options...]\n");
  fprintf(stdout, "		-p psi_xx psi_xy psi_yx psi_yy	# distortion matrix (1,0,0,1)\n");
  fprintf(stdout, "		-t t_x t_y			# translation vector (0,0)\n");
  fprintf(stdout, "		-n N1 N2			# size of output image\n");
  fprintf(stdout, "		-m mode				# mapping mode (1)\n");
  fprintf(stdout, "		-c				# keep centre fixed\n");
  fprintf(stdout, "		-i				# do inverse transformation\n");
  fprintf(stdout, "		-f fitsfile			# source for target image\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION\n");
  fprintf(stdout, "	\"transformimage\" applies a general linear transformation to a source\n");
  fprintf(stdout, "	image fs(x) to make a target image f(r) = fs(x(r))\n");
  fprintf(stdout, "	where the mapping is x_i(r) = psi_ij r_j + t_i.\n");
  fprintf(stdout, "	By default output image = input image size.\n");
  fprintf(stdout, "	Use -m option to specify mode, where these are (in order of expense)\n");
  fprintf(stdout, "		mode = 0:	# nearest pixel\n");
  fprintf(stdout, "		mode = 1:	# linear interpolation\n");
  fprintf(stdout, "		mode = 2:	# sum over triangles\n");
  fprintf(stdout, "	With -c option we calculate t_x, t_y so that the centre\n");
  fprintf(stdout, "	pixel is mapped to centre of output pixel.\n");
  fprintf(stdout, "	By default, target image is initialised to zero\n");
  fprintf(stdout, "	but use -f option to read in an image on which we paint\n");
  fprintf(stdout, "	the mapped pixels.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR\n");
  fprintf(stdout, "        Original Version:\n");
  fprintf(stdout, "        Nick Kaiser:  kaiser@cita.utoronto.ca\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        THELI version maintained by\n");
  fprintf(stdout, "        Thomas Erben: terben@astro.uni-bonn.de\n");
}
