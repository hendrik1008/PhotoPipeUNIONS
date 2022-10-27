#include "options_globals.h"

typedef struct {
   char verbose[OPTLEN];
   char conffile[MAXCHAR];
   char listfile[MAXCHAR];
   char outfile[MAXCHAR];
   char tablename[MAXCHAR];
   double min_over, xpixsize, ypixsize;
   double xwid, ywid;
   char crval1[MAXCHAR], crval2[MAXCHAR];
   double cdelt1, cdelt2;
   double crpix1, crpix2;
   char check_cond_string[MAXCHAR];
   int	check_cond;
} control_struct;

#define NOPTS 13

extern control_struct control;
