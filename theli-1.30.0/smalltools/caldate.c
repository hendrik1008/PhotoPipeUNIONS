/*----------------------------------------------------------------------------

   File name    :   caldate.c
   Author       :   Thomas Erben
   Created on   :   12.01.2005
   Description  :   calculate civil date of a given GaBoDSID.

 ---------------------------------------------------------------------------*/

/*
	$Id: caldate.c,v 1.7 2018/03/28 12:06:29 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:29 $
	$Revision: 1.7 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/
/*
    13.01.2005:
    - I changed the out format of the civil date to ISO standard
    - I included ctype.h to avoid compiler warnings
    - I removed compiler warnings due to possibly undef. variables

    24.02.2005:
    I included sanity checks on the date given

    10.05.2006:
    I increased the minum number of required input parameters
    to 4.
 */

#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXSTRINGLENGTH
#define MAXSTRINGLENGTH 256
#endif

#define __USAGE__ "\
USAGE:\n\
       caldate     -d reference date (DD/MM/YYYY) \n\
                   -i night ID\n\
DESCRIPTION:\n\
The program 'caldate' calculates the civil date for given\n\
reference date and GaBoDSID. The output represents the\n\
date of the BEGINNING of the night. It performs the inverse\n\
calculation of the 'nightid' program.\n\n\
EXAMPLE:\n\
  caldate -d 31/12/1998 -i 1648\n\n\
AUTHOR:\n\
      Thomas Erben\n"

int main(int argc, char **argv)
{
  char date[MAXSTRINGLENGTH];
  long nightid = 0;

  long JD;
  long L, N, I, J, DA, M ,Y;

  long month=0, day=0, year=0;

  int i;
  char *dummy;

  /* read command line parameters */

  if(argc < 5)
  {
    printf("%s", __USAGE__);
    exit(1);
  }

  for (i=1; i<argc; i++)
  {
    if (argv[i][0] == '-')
    {
       switch(tolower((int)argv[i][1]))
       {
         case 'd': strncpy(date, argv[++i], MAXSTRINGLENGTH-1);
                   break;
         case 'i': nightid=atol(argv[++i]);
                   break;
         default:
                   printf("Wrong command line syntax !!\n %s", __USAGE__);
                   exit(1);
                   break;
       }
    }
    else
    {
      printf("Wrong command line syntax !!\n %s", __USAGE__);
      exit(1);
    }
  }

  /*
   * decode date string; it is assumed to be DD/MM/YYYY
   */

  if((dummy = strtok(date, "/")) != NULL)
  {
    day   = atol(dummy);
  }
  if(day < 1 || day > 31)
  {
    fprintf(stderr, "Impossible day: %ld; exiting\n", day);
    exit(1);
  }
  if((dummy = strtok(NULL, "/")) != NULL)
  {
    month = atol(dummy);
  }
  if(month < 1 || month > 12)
  {
    fprintf(stderr, "Impossible month: %ld; exiting\n", month);
    exit(1);
  }
  if((dummy = strtok(NULL, "/")) != NULL)
  {
    year  = atol(dummy);
  }
  if(year < 1900)
  {
    fprintf(stderr, "Impossible year: %ld; exiting\n", year);
    exit(1);
  }

  /* the following formulas are from
      'Explanatory Supplement to the Astronomical Almanac'
      (ed. K. Seidelmann; University Science Books, 1992) */

  /* first calculate Julian date from reference date */
  JD=(1461*(year+4800+(month-14)/12))/4+(367*(month-2-12*((month-14)/12)))/12-
    (3*((year+4900+(month-14)/12)/100))/4+day-32075;

  /* modify Julian Date by GaBoDS ID. */
  JD=JD+nightid;

  /* and finally calculate civil date */
  L=JD+68569;
  N=(4*L)/146097;
  L=L-(146097*N+3)/4;
  I=(4000*(L+1))/1461001;
  L=L-(1461*I)/4+31;
  J=(80*L)/2447;
  DA=L-(2447*J)/80;
  L=J/11;
  M=J+2-12*L;
  Y=100*(N-49)+I+L;

  printf("civil date: %ld-%02ld-%02ld\n", Y, M, DA);
  return 0;
}
