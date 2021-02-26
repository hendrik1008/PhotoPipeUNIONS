/*
   05.09.03:
   changes to include the OBJECT keyword in the new headers

   13.10.03:
   included possibility to flip frames in x and/or y

   25.11.04:
   I relaxed limits on CRVAL1, CRVAL2 (according to Mischa
   they can be set arbitrarily in BIAS, DARKS)

   15.01.05:
   removed compiler warnings (gcc with -Wall -W -pedantic)

   19.04.2005:
   I replaced in the key structure and the default
   prefs the flip? elements by Mij matrix coefficients.

   11.10.2005:
   I introduced the IMAGEID command line argument.
   It determines which IMAGE ID the nth chip of a MEF file
   will have.

   24.10.2005:
   The IMAGEID can be empty, i.e. the minum allowed number
   of entries for this keyword is zero, not 1.

   23.11.2005:
   I introduced the OUTPUT_DIR command
   line argument. It allows to explicitely specify
   the output directory for splitted data.

   31.08.2010:
   I set the allowed upper limit of the airmass argument to 100.
   For calibration frames such as biases this value might be
   'unreasonably high' (request from Mischa)

   10.07.2014:
   I introduced the new option IMAGEID_KEY which specifies a header
   keyword containing the chip number (DECAM contains the key CCDNUM
   specifying this quantity).
*/

#include "key.h"

#include "prefs_split.h"

pkeystruct key[] =
 {
  {"CRPIX1", P_FLOATLIST, prefs.crpix1, 0, 0, -1.0e30, 1.0e30,
   {""}, 1, MAXCHIPS, &prefs.ncrpix1,0},
  {"CRPIX2", P_FLOATLIST, prefs.crpix2, 0, 0, -1.0e30, 1.0e30,
   {""}, 1, MAXCHIPS, &prefs.ncrpix2,0},
  {"CD11", P_FLOATLIST, prefs.cd11, 0, 0, -1.0, 1.0,
   {""}, 1, MAXCHIPS, &prefs.ncd11,0},
  {"CD22", P_FLOATLIST, prefs.cd22, 0, 0, -1.0, 1.0,
   {""}, 1, MAXCHIPS, &prefs.ncd22,0},
  {"CD12", P_FLOATLIST, prefs.cd12, 0, 0, -1.0, 1.0,
   {""}, 1, MAXCHIPS, &prefs.ncd12,0},
  {"CD21", P_FLOATLIST, prefs.cd21, 0, 0, -1.0, 1.0,
   {""}, 1, MAXCHIPS, &prefs.ncd21,0},
  {"M11", P_INTLIST, prefs.m11, -1, 1, 0.0, 0.0,
   {""}, 1, MAXCHIPS, &prefs.nm11,0},
  {"M22", P_INTLIST, prefs.m22, -1, 1, 0.0, 0.0,
   {""}, 1, MAXCHIPS, &prefs.nm22,0},
  {"M12", P_INTLIST, prefs.m12, -1, 1, 0.0, 0.0,
   {""}, 1, MAXCHIPS, &prefs.nm12,0},
  {"M21", P_INTLIST, prefs.m21, -1, 1, 0.0, 0.0,
   {""}, 1, MAXCHIPS, &prefs.nm21,0},
  {"IMAGEID", P_INTLIST, prefs.imageid, 1, MAXCHIPS, 0.0, 0.0,
   {""}, 0, MAXCHIPS, &prefs.nimageid,0},
  {"IMAGEID_KEY", P_STRINGLIST, &prefs.imageidkey, 0, 0, 0.0, 0.0,
   {""}, 1, 1, &prefs.nimageidkey, 0},
  {"CRVAL1", P_FLOAT, &prefs.crval1, 0, 0, -1.0e30, 1.0e30,
   {""}, 1, 1, NULL, 0},
  {"CRVAL2", P_FLOAT, &prefs.crval2, 0, 0, -1.0e30, 1.0e30,
   {""}, 1, 1, NULL, 0},
  {"EXPTIME", P_FLOAT, &prefs.exptime, 0, 0, 0.0, 4000.0,
   {""}, 1, 1, NULL, 0},
  {"EQUINOX", P_FLOAT, &prefs.equinox, 0, 0, 1900.0, 3000.0,
   {""}, 1, 1, NULL, 0},
  {"AIRMASS", P_FLOAT, &prefs.airmass, 0, 0, 1.0, 100.0,
   {""}, 1, 1, NULL, 0},
  {"GABODSID", P_INT, &prefs.gabodsid, 0, 10000, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"FILTER", P_STRING, &prefs.filter, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"OBJECT", P_STRING, &prefs.object, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"OUTPUT_DIR", P_STRINGLIST, &prefs.outputdir, 0, 0, 0.0, 0.0,
   {""}, 0, 1, &prefs.noutputdir, 0},
  {"", P_BOOL, NULL, 0, 0, 0.0, 0.0, {""}, 1, 1, NULL, 0}
 };

char                    keylist[sizeof(key)/sizeof(pkeystruct)][16];

char *default_prefs[] =
{
"CRPIX1             3867,1721,-416,-2551,-2552,-412,1725,3864",
"CRPIX2             -233,-231,-224,-225,3893,3888,3885,3886",
"CD11               -6.61e-5,-6.61e-5,-6.61e-5,-6.61e-5,-6.61e-5,-6.61e-5,-6.61e-5,-6.61e-5",
"CD22               6.61e-5,6.61e-5,6.61e-5,6.61e-5,6.61e-5,6.61e-5,6.61e-5,6.61e-5",
"CD12               0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0",
"CD21               0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0",
"M11                1,1,1,1,1,1,1,1",
"M22                1,1,1,1,1,1,1,1",
"M12                0,0,0,0,0,0,0,0",
"M21                0,0,0,0,0,0,0,0",
"CRVAL1             0.0",
"CRVAL2             0.0",
"EXPTIME            0.0",
"EQUINOX            2000.0",
"AIRMASS            1.0",
"GABODSID           0",
"FILTER             R",
"OBJECT             TEST",
"IMAGEID            #",
"IMAGEID_KEY        #",
"OUTPUT_DIR         #",
""
};
