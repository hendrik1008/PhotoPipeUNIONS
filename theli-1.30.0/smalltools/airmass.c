/*----------------------------------------------------------------------------

   File name    :   airmass.c
   Author       :   Thomas Erben
   Created on   :   13.03.2001
   Description  :   calculate the effective airmass of a long exposure

 ---------------------------------------------------------------------------*/

/*
	$Id: airmass.c,v 1.7 2018/03/28 12:06:29 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:29 $
	$Revision: 1.7 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
   30.05.2004:
   removed compiler warnings due to uninitialised variables

   24.02.2005:
   function calcairmass:
   I deleted the superfulous aalpha argument
*/

/*----------------------------------------------------------------------------
                                Includes and Defines
 ---------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#define __PROGRAMMVERSION__ "1.0"
#define __PROGRAMMNAME__ "airmass"

#define __USAGE__ "\
USAGE:\n\
       airmass -t Local Siderial time at start of exposure (sec)\n\
               -e exposure time (sec) \n\
               -r Right ascension of the field (deg)\n\
               -d Declination of the field (deg)\n\
               -l Latitude of observatory (deg)\n\n\
DESCRIPTION:\n\
The program airmass calculates the effective airmass of\n\
an exposure. It takes as input the Local Siderial Time (in sec)\n\
of the start of the exposure, the exposure time (in sec.),\n\
Ra and Dec of the observed field (in degrees) and the latitude\n\
of the observatory (in degree).\n\n\
List of latitues:\n\
- La Silla: -29.25694444\n\n\
EXAMPLE:\n\
  airmass -t 54962.101 -e 500 -r 253.925611 -d -23.53892 -l -29.25694444\n\n\
AUTHOR:\n\
      Thomas Erben\n"

#define DEG_RAD 3.14159/180.
#define MIN(x, y) ((x) < (y) ? (x) : (y))

double calcairmass(double ahourangle, double adec, double alat);

/*----------------------------------------------------------------------------
                            Main code
 ---------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  double lst=0.;  /* LST in seconds */
  double lst_begin, lst_middle, lst_end;
  double ra=0., dec=0.;
  double lat;
  double hourangle_begin, hourangle_middle, hourangle_end;
  double exptime=0.;
  double airmass_begin, airmass_middle, airmass_end;
  double airmass;
  int i;

  lat = -29.25694444;

  /* read command line arguments */
  if(argc < 11)
  {
    printf("%s\n", __USAGE__);
    exit(1);
  }

  for (i=1; i<argc; i++)
  {
    if (argv[i][0] == '-')
    {
       switch(tolower((int)argv[i][1]))
       {
  	 case 't': lst = atof(argv[++i]);
  		   break;
  	 case 'e': exptime = atof(argv[++i]);
  		   break;
  	 case 'r': ra = atof(argv[++i]);
  		   break;
  	 case 'd': dec = atof(argv[++i]);
  		   break;
  	 case 'l': lat = atof(argv[++i]);
  		   break;
  	 default:
  		   break;
       }
    }
    else
    {
      printf("Wrong command line syntax !!\n %s", __USAGE__);
      exit(1);
    }
  }

  lst_begin=lst;
  lst_middle=lst+0.5*exptime;
  lst_end=lst+exptime;
  ra=ra*DEG_RAD;
  dec=dec*DEG_RAD;
  lst=lst*DEG_RAD;
  lat=lat*DEG_RAD;
  hourangle_begin = (lst_begin/240.)*DEG_RAD - ra;
  hourangle_middle = (lst_middle/240.)*DEG_RAD - ra;
  hourangle_end = (lst_end/240.)*DEG_RAD - ra;

  /* the effective airmass for a long exposure is estimated
     the 'mean airmass' estimator presented in
     "Some Factors Affecting the Accuracy of Stellar Photometry with CCDs"
     by P. Stetson, DAO preprint, September 1988

     It requires the airmasses from the begin, the middle and
     the end of the target observation */

  airmass_begin=calcairmass(hourangle_begin, dec, lat);
  airmass_middle=calcairmass(hourangle_middle, dec, lat);
  airmass_end=calcairmass(hourangle_end, dec, lat);

  airmass = (airmass_begin+4.0*airmass_middle+airmass_end)/6.0;

  printf("%f\n", airmass);

  return 0;
}

double calcairmass(double ahourangle, double adec, double alat)
{
  double result;
  double sh, ch, sd, cd, sp, cp;
  double x, y, z;
  double zn, zf, zd, seczm;

  sh=sin(ahourangle);
  ch=cos(ahourangle);
  sd=sin(adec);
  cd=cos(adec);
  sp=sin(alat);
  cp=cos(alat);
  x = ch*cd*sp-sd*cp;
  y = sh*cd;
  z = ch*cd*cp+sd*sp;
  zn= sqrt(x*x+y*y);
  zf= zn/z;
  zd= atan(zf);
  seczm = 1.0/(cos(MIN(1.52, zd)))-1.0;

  /* for the conversion of zenith 'z' to airmaiss we use
     the 'polynomial approximation' (in sec(z)) presented in
     "R.H. Hardie in _Astronomical Techniques_ (W.A. Hiltner, ed.) (Chicago:
     U. Chicago Press), p. 180 (1962)." */

  result = 1.0+seczm*(0.9981833-seczm*(0.002875+seczm*0.008083));

  return result;
}


