#ifndef __SSC_TYPES_H__
#define __SSC_TYPES_H__

/* 23.01.2005:
   I moved definitions from global variables
   (col_name, col_input, col_merge, col_chan)
   to avoid double definitions in the linking
   process.
*/

typedef struct {
   char conffile[MAXCHAR];
   char listfile[MAXCHAR][MAXCHAR];
   char outfile[MAXCHAR];
   char table_name[MAXCHAR];
   char bestcol[MAXCHAR];
   char besttype[MAXCHAR];
   int  best_type;
   int  best;
   int  ilist;
   bool	make_pairs, color_pairs, pair_col;
   char name[TEXTVECLEN];
   char input[TEXTVECLEN];
   char merge[TEXTVECLEN];
   char chan[TEXTVECLEN];
   char pastetab[MAXCHAR][MAXCHAR];
   char ra[MAXCHAR], dec[MAXCHAR];
   char crval1[MAXCHAR], crval2[MAXCHAR];
   char crpix1[MAXCHAR], crpix2[MAXCHAR];
   char cdelt1[MAXCHAR], cdelt2[MAXCHAR];
   char seqnr[MAXCHAR], fieldpos[MAXCHAR];
   char xwid[MAXCHAR], ywid[MAXCHAR];
   char cluster_nr[MAXCHAR];
   int  npaste,pastecat[MAXCHAR];
} control_struct;

extern control_struct control;

#endif
