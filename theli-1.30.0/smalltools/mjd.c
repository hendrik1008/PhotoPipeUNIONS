/*----------------------------------------------------------------------------

   File name    :   mjd.c
   Authors      :   Thomas Erben and Mischa Schirmer
   Created on   :   30.05.2004
   Description  :   transform civil date to Julian and Modified Julian Date

  ---------------------------------------------------------------------------*/

/*
        $Id: mjd.c,v 1.5 2015/07/02 14:20:37 thomas Exp $
        $Author: thomas $
        $Date: 2015/07/02 14:20:37 $
        $Revision: 1.5 $
 */

/*----------------------------------------------------------------------------
                                History Coments
  ---------------------------------------------------------------------------*/

/*
   12.01.2005:
   I added printout of MJD. So far, only JD was given.

   08.12.2009:
   We added the possibility to give the input date/time in ISO8601 format.
   (command line option '-f')

   02.07.2015:
   It turned out that the estimated JDs were inaccurate and we need to
   perform calculations in double precision to remedy this.
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
#define MAXSTRINGLENGTH    256
#endif

/*----------------------------------------------------------------------------
                           Function declarations
 ---------------------------------------------------------------------------*/

void printUsage(void);

/*----------------------------------------------------------------------------
                            Main code
   ---------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  char  date[MAXSTRINGLENGTH];
  char  time[MAXSTRINGLENGTH];
  char  dateobs[MAXSTRINGLENGTH];
  double month       = 0.0, day = 0.0, year = 0.0;
  double hour        = 0.0, min = 0.0;
  double sec         = 0.0, timefrac = 0.0;
  double julian_date = 0.0, modified_julian_date = 0.0;
  double A, B, C, D, E, F;
  int   i, dateobsgiven = 0;
  char *dummy, *date_do, *time_do;

  /* read command line parameters */
  if (argc < 2) {
    printUsage();
    exit(1);
  }

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (tolower((int)argv[i][1])) {
      case 't': strncpy(time, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'd': strncpy(date, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'f': strncpy(dateobs, argv[++i], MAXSTRINGLENGTH - 1);
        dateobsgiven = 1;
        break;
      default:
        printf("Wrong command line syntax !!\n");
        printUsage();
        exit(1);
        break;
      }
    } else {
      printf("Wrong command line syntax !!\n");
      printUsage();
      exit(1);
    }
  }

  /* decode date and time string; date is assumed to be
     DD/MM/YYYY and time hh:mm:ss.ss
   */

  if (dateobsgiven == 0) {
    if ((dummy = strtok(date, "/")) != NULL) {
      day = atof(dummy);
    }
    if ((dummy = strtok(NULL, "/")) != NULL) {
      month = atof(dummy);
    }
    if ((dummy = strtok(NULL, "/")) != NULL) {
      year = atof(dummy);
    }

    if ((dummy = strtok(time, ":")) != NULL) {
      hour = atof(dummy);
    }
    if ((dummy = strtok(NULL, ":")) != NULL) {
      min = atof(dummy);
    }
    if ((dummy = strtok(NULL, ":")) != NULL) {
      sec = atof(dummy);
    }
  } else {
    date_do = strtok(dateobs, "T");
    time_do = strtok(NULL, "T");
    if (date_do != NULL) {
      if ((dummy = strtok(date_do, "-")) != NULL) {
        year = atof(dummy);
      }
      if ((dummy = strtok(NULL, "-")) != NULL) {
        month = atof(dummy);
      }
      if ((dummy = strtok(NULL, "-")) != NULL) {
        day = atof(dummy);
      }
    }
    if (time_do != NULL) {
      if ((dummy = strtok(time_do, ":")) != NULL) {
        hour = atof(dummy);
      }
      if ((dummy = strtok(NULL, ":")) != NULL) {
        min = atof(dummy);
      }
      if ((dummy = strtok(NULL, ":")) != NULL) {
        sec = atof(dummy);
      }
    }
  }


  /* for an explanation of the formulas below visit
     http://aa.usno.navy.mil/faq/docs/JD_Formula.html
   */
  timefrac = hour + min / 60.0 + sec / 3600.0;
  A = year * 367.0;
  B = floor((month + 9.0) / 12.0);
  C = floor(((year + B) * 7.0) / 4.0);
  D = floor((275.0 * month) / 9.0);
  E = day + 1721013.5 + (timefrac / 24.0);
  F = (((100.0 * year) + month - 190002.5) >= 0) ? 1.0 : -1.0;
  julian_date = A - C + D + E - (0.5 * F) + 0.5;
  modified_julian_date = julian_date - 2400000.5;

  printf("Julian Date: %.5lf, Modified Julian Date: %.5lf\n",
         julian_date, modified_julian_date);

  return(0);
}

/*
 * This function only gives the usage message for the program
 */

void printUsage(void)
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "      mjd - transform civil date to Julian and Modified Julian Date\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "      mjd     -t time (hh:mm:ss.ss)\n");
  fprintf(stdout, "              -d date (DD/MM/YYYY)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "      OR\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "      mjd     -f DATE-OBS (YYYY-MM-DDThh:mm:ss, FITS convention)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "      Either '-t' and '-f' or only '-f' must be given.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION\n");
  fprintf(stdout, "      The program calculates and prints to screen the Julian\n");
  fprintf(stdout, "      Date (JD) and Modified Julian Date (MJD) of a given calendar\n");
  fprintf(stdout, "      date. The calendar date has to be provided at Universal Time\n");
  fprintf(stdout, "      (UT).\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "EXAMPLES:\n");
  fprintf(stdout, "      mjd -t 18:00:37 -d 20/08/2001\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "      mjd -f 2001-08-20T18:00:37\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHORS:\n");
  fprintf(stdout, "      Thomas Erben    (terben@astro.uni-bonn.de)\n");
  fprintf(stdout, "      Mischa Schirmer (mischa@astro.uni-bonn.de)\n");
}
