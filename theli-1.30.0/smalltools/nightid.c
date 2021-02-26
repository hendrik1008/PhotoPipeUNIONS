/*----------------------------------------------------------------------------

   File name    :   nightid.c
   Author       :   Thomas Erben
   Created on   :   29.01.2001
   Description  :   calculate the number of days passed since a given reference date.

 ---------------------------------------------------------------------------*/

/*
	$Id: nightid.c,v 1.8 2018/03/28 12:06:30 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:30 $
	$Revision: 1.8 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
   10.01.2002: removed compiler warnings from unused variables

   15.07.2003: removed error in the __USAGE__ definition that came up
               during compilation under Linux/Debian

   24.02.2005: I included sanity checks on the date/time given

   19.05.2006: I increased the number of required command line
               arguments from 2 to 7.

   14.04.2014: I introduced the possibility to provide a file with
               MJDS to be converted to nightids.
 */

/*----------------------------------------------------------------------------
                                Includes and Defines
 ---------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>



#ifndef MAXSTRINGLENGTH
#define MAXSTRINGLENGTH 256
#endif

#define __USAGE__ "\
USAGE:\n\
       nightid     -t Local Siderial time (hh:mm:ss.ss) of ref. date\n\
                   -d reference date (DD/MM/YYYY) \n\
                  (-m MJD date of observation (in days))\n\
                  (-f file with one MJD per line)\n\
DESCRIPTION:\n\
The program 'nightid' calculates for a supplied Median Julian Date\n\
the number of nights passed since a given reference date/time.\n\
The GaBoDSID is defined as the number of nights passed since\n\
31/12/1998. For this concept to be meaningful regardless of your\n\
observatory you have to supply for time the Greenwhich time when your\n\
local clock reads 12:00:00.\n\
The program either calculates nightid for one MJD given with '-m' or\n\
it calculates a nightod for each MJD given in a file provided with the\n\
'-f' option.\n\
The program 'caldate' performs the reverse calculation.\n\n\
EXAMPLES:\n\
  nightid -t 16:00:00 -d 31/12/1998 -m  51180.25\n\n\
  The integer part of the output represents the GaBoDSID \n\
  for the MJD 51180.25 at the La Silla observatory (which is\n\
  4 hours west of Greenwhich).\n\n\
  nightid -t 22:00:00 -d 31/12/1998 -m  51180.25\n\n\
  is the corresponding command for CFHT on Hawaii\n\
  (10 hours west of Greenwhich).\n\n\
  In the examples, the night from 01/01/1999 to 02/01/1999\n\
  has GaBoDS ID 1.\n\n\
  nightid -t 22:00:00 -d 31/12/1998 -m dates.txt\n\n\
  Calculates nightids for each MJD provided in the file 'dates.txt'.\n\
  The file contains one MJD per line. The output is written to\n\
  STDOUT.\n\
AUTHOR:\n\
      Thomas Erben\n"

/*----------------------------------------------------------------------------
                            Main code
 ---------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  char date[MAXSTRINGLENGTH];
  char time[MAXSTRINGLENGTH];
  FILE *inputfile = NULL;
  char *filename = NULL;
  double month = 0.0, day = 0.0, year = 0.0;
  double hour = 0.0, min = 0.0;
  double sec = 0.0, timefrac = 0.0;
  double julian_date = 0.0;
  double A, B, C, D, E, F;
  double mjdobs = 0.0;
  int i;
  char *dummy;

  /* read command line parameters */

  if(argc < 7) {
    printf("%s", __USAGE__);
    exit(1);
  }

  for (i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
       switch(tolower((int)argv[i][1])) {
         case 't': strncpy(time, argv[++i], MAXSTRINGLENGTH-1);
                   break;
         case 'd': strncpy(date, argv[++i], MAXSTRINGLENGTH-1);
                   break;
         case 'm': mjdobs = atof(argv[++i]);
                   break;
         case 'f': filename = argv[++i];
                   break;
         default:
                   printf("Wrong command line syntax !!\n %s", __USAGE__);
                   exit(1);
                   break;
       }
    }
    else {
      printf("Wrong command line syntax !!\n %s", __USAGE__);
      exit(1);
    }
  }

  /* decode date and time string; date is assumed to be
     DD/MM/YYYY and time hh:mm:ss.ss
  */

  if ((dummy = strtok(date, "/")) != NULL) {
    day   = atof(dummy);
  }
  if (day < 1. || day > 31.) {
    fprintf(stderr, "Impossible day: %f; exiting\n", day);
    exit(1);
  }
  if ((dummy = strtok(NULL, "/")) != NULL) {
    month = atof(dummy);
  }
  if (month < 1. || month > 12.) {
    fprintf(stderr, "Impossible month: %f; exiting\n", month);
    exit(1);
  }
  if ((dummy = strtok(NULL, "/")) != NULL) {
    year  = atof(dummy);
  }
  if (year < 1900.) {
    fprintf(stderr, "Impossible year: %f; exiting\n", year);
    exit(1);
  }

  if ((dummy = strtok(time, ":")) != NULL) {
    hour  = atof(dummy);
  }

  if (hour < 0.0 || hour > 23.0) {
    fprintf(stderr, "Impossible hour: %f; exiting\n", hour);
    exit(1);
  }

  if ((dummy = strtok(NULL, ":")) != NULL) {
    min   = atof(dummy);
  }

  if (min < 0.0 || min > 59.0) {
    fprintf(stderr, "Impossible min: %f; exiting\n", min);
    exit(1);
  }

  if ((dummy = strtok(NULL, ":")) != NULL) {
    sec   = atof(dummy);
  }

  if (sec < 0.0 || sec > 59.0) {
    fprintf(stderr, "Impossible sec: %f; exiting\n", sec);
    exit(1);
  }

  /* for an explanation of the following formulae visit
     http://aa.usno.navy.mil/faq/docs/JD_Formula.html
  */

  timefrac = hour + min / 60.0 + sec / 3600.0;

  A = year * 367.0;
  B = floor((month + 9.0) / 12.0);
  C = floor(((year + B) * 7.0) / 4.0);
  D = floor((275.0 * month) / 9.0);
  E = day + 1721013.5 + (timefrac / 24.0);
  F = (((100.0 * year) + month - 190002.5) >= 0) ? 1.0 : -1.0;
  julian_date = A - C + D + E - (0.5 * F) + 0.5 - 2400001.0 + 0.5;

  if (mjdobs > 0.0) {
    printf("Julian date of reference time: %f\n", julian_date);
    printf("Days passed since reference time: %f\n", mjdobs - julian_date);
  } else {
    if (filename != NULL) {
      if ((inputfile = fopen(filename, "r")) != NULL) {
        while (fscanf(inputfile, "%lf", &mjdobs) > 0) {
          printf("%lf\n", mjdobs - julian_date);
        }
        fclose(inputfile);
      } else {
        fprintf(stderr, "Cannot open file %s\n", filename);
        exit(1);
      }
    } else {
      fprintf(stderr, "No MJD given! Nothing to do!\n");
      exit(1);
    }
  }

  return 0;
}



