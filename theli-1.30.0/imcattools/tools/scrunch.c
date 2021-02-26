
/*----------------------------------------------------------------------------

   File name    :   scrunch.c
   Author       :   Nick Kaiser & Thomas Erben
   Created on   :   29.08.2007
   Description  :   scrunches FITS images

 ---------------------------------------------------------------------------*/

/*
	$Id: scrunch.c,v 1.4 2018/03/28 12:06:14 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:14 $
	$Revision: 1.4 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
  29.08.2007:
  Nick Kaisers imcat tool 'scruch' integrated to THELI

  10.09.2007:
  A binning factor can now be given as command line option
  (-b). The program no longer is limited to a binning of 2!
  History information is added to the output image header.

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
#include "args.h"
#include "scrunch_stuff.h"
#include "global_vers.h"
#include "t_iso8601.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

void printUsage(void);

#ifndef MAXCHAR
#define MAXCHAR 256
#endif

/* uncomment the following line to obtain a status on allocated/freed
   memory at the end of the program. Only available if also
   __XMEM_DEBUG__ is defined at compile time.
*/

/* #define MEMORY_DEBUG */

/*----------------------------------------------------------------------------
                                Main Function
 ---------------------------------------------------------------------------*/

int     main(int argc, char *argv[])
{
    int         N1in, N2in, mode;
    int         binning;
    fitsheader  *fitsin, *fitsout;
    char        *flag;
    char        tmpstring[MAXCHAR];

    /* defaults */
    mode = MEAN;
    binning = 2;

    /* parse args */
    argsinit(argc, argv, "error in command line arguments!");
    while ((flag = getflag())) {
        switch (flag[0]) {
            case 'b':
                binning = getargi();
                break;
            case 'm':
                mode = MEDIAN;
                break;
            case 'c':
                mode = CMEAN;
                break;
            default:
	      printUsage();
              exit(1);	
        }
    }

    fitsin = readfitsheader(stdin);
    N1in = fitsin->n[0];
    N2in = fitsin->n[1];

    if((binning < 2) || (binning > N1in) || (binning > N2in))
    {
        error_exit("binning factor out of range !");
    }
    fitsout = copyfitsheader(fitsin);
    /* because the following calculation are in integer
       we never end up with 'fractional' image sizes !
    */
    fitsout->n[0] = N1in / binning;
    fitsout->n[1] = N2in / binning;

    /* add history information to output header */
    appendcomment(newtextcomment("HISTORY", "", NULL), fitsout);
    snprintf(tmpstring, MAXCHAR-1, "scrunch: %s", __PIPEVERS__);
    appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
    snprintf(tmpstring, MAXCHAR-1, "scrunch: called at %s",
             get_datetime_iso8601());
    appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
    add_comment(argc, argv, fitsout);

    /* now do the real work: */
    writefitsheader(fitsout);
    scrunch_stream(fitsin, fitsout, binning, mode);
    writefitstail(fitsout);

    /* clean memory and bye */
    delfitsheader(fitsin);
    delfitsheader(fitsout);

#ifdef MEMORY_DEBUG
   xmemory_status() ;
#endif


     return 0;
}

/*
 * This function only gives the usage message for the program
 */

void printUsage(void)
{
    fprintf(stdout, "PROGRAMNAME\n");
    fprintf(stdout, "        scrunch - demagnify a FITS image \n");
    fprintf(stdout, "\n");
    fprintf(stdout, "SYNOPSIS\n");
    fprintf(stdout, "        scrunch [-b | -m | -c]\n");
    fprintf(stdout, "                -b      # binning factor (2 is default)\n");
    fprintf(stdout, "                -m      # take median (mean is default)\n");
    fprintf(stdout, "                -c      # conservative mean\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "DESCRIPTION\n");
    fprintf(stdout, "        Scrunches a fits image down by a factor provided with -b\n");
    fprintf(stdout, "        With -c option output is MAGIC if any of source pixels are MAGIC.\n");
    fprintf(stdout, "        Astrometric information in image headers is NOT updated \n");
    fprintf(stdout, "        to reflect the new pixel scale.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "        The program works in stream mode and is hence prefered to\n");
    fprintf(stdout, "        'album' if very large images need to be scrunched.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "AUTHOR\n");
    fprintf(stdout, "        Original Version:\n");
    fprintf(stdout, "        Nick Kaiser:  kaiser@cita.utoronto.ca\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "        extended by\n");
    fprintf(stdout, "        Thomas Erben: terben@astro.uni-bonn.de\n");
}
