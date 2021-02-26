/*
  30.01.2005:
  - I added the MAXIMAGES keyword to the input pkeystruct
  - I removed compiler warnings under gcc with -Wall -W -pedantic

  07.03.2007:
  I added OUTPUT_BITPIX and COMBINE_BITPIX keys that allow
  configuration of the precision of preprocessed and stacked images.
  We support BITPIX=16 and BITPIX=-32 for the moment.

  26.03.2010:
  I increased the maximum possibile value for COMBINE_MAXVAL from
  1.0e05 to 1.0e8; request from Mischa for infrared data

  09.05.2010:
  New option to turn off fringe rescaling (request from Mischa)

  28.03.2012:
  New option to provide the flatscaling factor on the command line
  (instead of calculating it from flatscaling images)

  20.11.2012:
  New option to provide the GAIN header keyword. The gain is
  possibly modified when flatfielding imaging data.

  23.11.2012:
  New option to provide a header keyword for the saturation level.
  The saturation level is
  possibly modified by overscan, bias subtraction and flatfielding.

  20.06.2014:
  New option to provide a BADCCD header keyword. If calibration images
  are marked as bad the processed image also will be marked as bad etc.
*/

#include "key.h"

#include "prefs_imred.h"

pkeystruct key[] =
 {
  {"OVERSCAN", P_BOOL, &prefs.overscanflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"OVERSCAN_REGION", P_INTLIST, prefs.overscanreg, 0, 40000, 0.0, 0.0,
   {""}, 2, 2, &prefs.noverscanreg, 0},
  {"OVERSCAN_REJECT", P_INTLIST, prefs.overscanreject, 0, 100, 0.0, 0.0,
   {""}, 2, 2, &prefs.noverscanreject, 0},
  {"STATSSEC", P_INTLIST, prefs.statssec, 0, 40000, 0.0, 0.0,
   {""}, 4, 4, &prefs.nstatssec, 0},
  {"MAXIMAGES", P_INT, &prefs.maximages, 0, 1000, 0.0, 0.0,
   {""},1, 1, NULL, 0},
  {"GAIN_KEYWORD", P_STRING, &prefs.gainkey, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"SATLEV_KEYWORD", P_STRING, &prefs.satlevkey, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"BADCCD_KEYWORD", P_STRING, &prefs.badccdkey, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"BIAS", P_BOOL, &prefs.biasflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"BIAS_IMAGE", P_STRING, &prefs.biasimage, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"TRIM", P_BOOL, &prefs.trimflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"TRIM_REGION", P_INTLIST, prefs.trimreg, 0, 40000, 0.0, 0.0,
   {""}, 4, 4, &prefs.ntrimreg, 0},
  {"FLAT", P_BOOL, &prefs.flatflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"FLAT_IMAGE", P_STRING, &prefs.flatimage, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"FLAT_SCALE", P_BOOL, &prefs.flatscaleflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"FLAT_SCALEIMAGE", P_STRINGLIST, prefs.flatscaleimage, 0, 0, 0.0, 0.0,
   {""}, 0, 100, &prefs.nflatscaleimage, 0},
  {"FLAT_SCALEVALUE", P_FLOAT, &prefs.flatscalefactor, 0, 0, 0.0, 100000.0,
   {""}, 1, 1, NULL, 0},
  {"FLAT_THRESHHOLD", P_FLOAT, &prefs.flatthreshhold, 0, 0, 0.0, 100000.0,
   {""}, 1, 1, NULL, 0},
  {"FRINGE", P_BOOL, &prefs.fringeflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"FRINGE_IMAGE", P_STRING, &prefs.fringeimage, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"FRINGE_REFIMAGE", P_STRING, &prefs.fringerefimage, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"FRINGE_SCALE", P_BOOL, &prefs.fringescaleflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"OUTPUT", P_BOOL, &prefs.outputpreprocflag,  0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"OUTPUT_BITPIX", P_INT, &prefs.outputbitpix, -32, 16, 0.0, 0.0,
   {""},1, 1, NULL, 0},
  {"OUTPUT_DIR", P_STRING, &prefs.outputdirectory, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"OUTPUT_SUFFIX", P_STRING, &prefs.outputpreprocsuffix, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"COMBINE", P_BOOL, &prefs.combineflag,  0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"COMBINE_BITPIX", P_INT, &prefs.combinebitpix, -32, 16, 0.0, 0.0,
   {""},1, 1, NULL, 0},
  {"COMBINE_RESCALE", P_BOOL, &prefs.combinerescaleflag, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"COMBINE_OUTPUT", P_STRING, &prefs.combineoutput, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"COMBINE_MINVAL", P_FLOAT, &prefs.combineminthresh, 0, 0, -100000.0, 100000.0,
   {""}, 1, 1, NULL, 0},
  {"COMBINE_MAXVAL", P_FLOAT, &prefs.combinemaxthresh, 0, 0, -100000.0, 1.0e8,
   {""}, 1, 1, NULL, 0},
  {"COMBINE_REJECT", P_INTLIST, prefs.combinereject, 0, 10, 0.0, 0.0,
   {""}, 2, 2, &prefs.ncombinereject, 0},
  {"",  P_BOOL, NULL, 0, 0, 0.0, 0.0, {""}, 1, 1, NULL, 0}
 };

char                    keylist[sizeof(key)/sizeof(pkeystruct)][16];

char *default_prefs[] =
{
"# Default configuration file for preprocess V1.6",
"#",
"# ----------- Common settings ---------------------",
"#",
"MAXIMAGES                20                   # the maximum number of images to",
"                                              # load at a time; determined by memory",
"                                              # constraints of your machine",
"STATSSEC                 500,1500,1500,2500   # image region for performing statistics",
"                                              # XMIN,XMAX,YMIN,YMAX",
"BADCCD_KEYWORD           NONE                 # Header keyword indicating a faulty image",
"                                              # (an int unequal != 0 indicates a faulty CCD)",
"GAIN_KEYWORD             GAIN                 # Header keyword giving the image GAIN",
"                                              # (e-/ADU)",
"SATLEV_KEYWORD           SATLEVEL             # Header keyword giving the image saturation",
"                                              # level (ADU)",
"#",
"# ----------- Overscan correction -----------------",
"#",
"OVERSCAN                 N                    # perform overscan correction (Y/N)",
"OVERSCAN_REGION          10,40                # the overscan region (x-range)",
"OVERSCAN_REJECT          2,5                  # the number of low/high pixel values",
"                                              # to reject in the estimation of overscan",
"                                              # values",
"#",
"# ----------- Bias subtraction --------------------",
"#",
"BIAS                     N                    # perform Bias subtraction (Y/N)",
"BIAS_IMAGE               bias.fits            # the master bias frame",
"#",
"# ----------- Image trimming ----------------------",
"#",
"TRIM                     N                    # perform trimming",
"TRIM_REGION              51,2082,31,4122      # trim section (XMIN,XMAX,YMIN,YMAX)",
"#",
"# ----------- Flat fielding ----------------------",
"#",
"FLAT                     N                    # perform flatfielding",
"FLAT_IMAGE               flat.fits            # the flat frame",
"FLAT_SCALE               N                    # rescale flats to the maximum gain",
"                                              # several chips (flatscle images)",
"FLAT_SCALEVALUE          0.0                  # scaling factor for flatfields",
"FLAT_SCALEIMAGE          flatscale.fits       # the flatscale images",
"FLAT_THRESHHOLD          50.0                 # pixels below this threshhold are not",
"                                              # considered during flatfielding",
"#",
"# ----------- Fringe correction ------------------",
"#",
"FRINGE                   N                    # perform fringe correction",
"FRINGE_IMAGE             fringe.fits          # the fringe pattern",
"FRINGE_SCALE             Y                    # use fringe scaling",
"FRINGE_REFIMAGE          fringeref.fits       # the sky reference for fringe scaling",
"#",
"# ----------- Output -----------------------------",
"#",
"OUTPUT                   N                    # save preprocessed images",
"OUTPUT_BITPIX            -32                  # BITPIX of preprocessed images",
"                                              # (16 or -32)",
"OUTPUT_SUFFIX            OC_1.fits            # the original .fits ending is replaced",
"                                              # by this suffix",
"OUTPUT_DIR               ./                   # directory to save the output images",
"#",
"# ----------- Image stacking ---------------------",
"#",
"COMBINE                  N                    # stack preprocessed frames (Y/N)",
"COMBINE_BITPIX           -32                  # BITPIX of stacked image",
"                                              # (16 or -32)",
"COMBINE_RESCALE          Y                    # rescale median of input images to",
"                                              # the mean of the medians",
"COMBINE_OUTPUT           combined.fits        # name of the stacked image",
"COMBINE_MINVAL           -10000.0             # minimum value to consider for stacking",
"COMBINE_MAXVAL           60000.0              # maximum value to consider for stacking",
"COMBINE_REJECT           3,3                  # number of low/high pixels to reject in stack",
""
};
