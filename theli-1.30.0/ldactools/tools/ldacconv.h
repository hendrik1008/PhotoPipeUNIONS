 /*
 				ldacconv.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACConv
*
*	Author:		E.BERTIN, DeNIS/LDAC
*
*	Contents:	Name conversion tables
*
*	Last modify:	27/07/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

typedef struct structnamemap
  {
  char	oldname[80];	/* Name of the key to be converted ("" = end) */
  char	newname[80];	/* New name to be given to the key ("" = remove it) */
  }	namemapstruct;

namemapstruct fieldmap[] =
 {
  {"SIMPLE  ",	""},
  {"BITPIX  ",	"MAPBTPIX"},
  {"NAXIS   ",	"MAPNAXIS"},
  {"NAXIS1  ",	"MAPNAXS1"},
  {"NAXIS2  ",	"MAPNAXS2"},
  {"PCOUNT  ",	""},
  {"GCOUNT  ",	""},
  {"TFIELDS ",	""},
  {"",""}
 };

namemapstruct objmap[] =
 {
  {"NUMBER",		"SeqNr"},
  {"X_IMAGE",		"Xpos"},
  {"Y_IMAGE",		"Ypos"},
  {"X2_IMAGE",		"XM2"},
  {"Y2_IMAGE",		"YM2"},
  {"XY_IMAGE",		"Corr"},
/*
  {"FLUX_ISO",		"Phot0"},
  {"FLUX_ISOCOR",	"Phot1"},
  {"FLUX_APER",		"Phot2"},
  {"FLUX_AUTO",		"Phot3"},
  {"FLUXERR_ISO",	"PhotErr"},
*/
  {"ISOAREA_IMAGE",	"NPIX"},
  {"BACKGROUND",	"BackGr"},
  {"FLAGS",		"Flag"},
  {"THRESHOLD",		"Level"},
  {"FLUX_MAX",		"MaxVal"},
  {"A_IMAGE",		"A"},
  {"B_IMAGE",		"B"},
  {"THETA_IMAGE",	"Theta"},
  {"",""}
 };
