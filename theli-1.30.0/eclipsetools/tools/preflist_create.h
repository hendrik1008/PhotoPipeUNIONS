#include "key.h"

#include "prefs_create.h"

pkeystruct key[] =
 {
  {"OUTPUT_IMAGE", P_STRING, &prefs.output, 0, 0, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"HEADER_TRANSFER",  P_STRINGLIST, prefs.header_keys, 0,0, 0.0,0.0,
   {""}, 0, MAXKEYWORD, &prefs.nheader_keys, 0},
  {"HEADER_DUMMY", P_INT, &prefs.header_dummy, 0, 100, 0.0, 0.0,
   {""}, 1, 1, NULL, 0},
  {"", P_BOOL, NULL, 0, 0, 0.0, 0.0, {""}, 1, 1, NULL, 0}
 };

char                    keylist[sizeof(key)/sizeof(pkeystruct)][16];

char *default_prefs[] =
{
"# Default configuration file for mefcreate V1.0",
"#",
"OUTPUT_IMAGE       create.fits    # name of output image",
"HEADER_TRANSFER    OBJECT         # list of keyowrds to transfer",
"                                  # to the MEF main header (these are",
"                                  # taken from the first image)",
"HEADER_DUMMY       0              # number of DUMMY keywords to add",
"                                  # to the main header",
""
};
