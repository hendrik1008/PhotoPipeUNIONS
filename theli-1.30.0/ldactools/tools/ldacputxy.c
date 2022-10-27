/*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACfilter
*
*	Author:		E.Deul, DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	15/01/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
 * 04.01.05:
 * removed compiler warnings (gcc with -O3 optimisation)
 *
 * 29.01.2005:
 * I added history information to the output catalog
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>

#include	"fitscat.h"
#include	"fitscat_defs.h"
#include	"utils_globals.h"
#include	"tabutil_globals.h"
#include	"options_globals.h"

#define BIG	99999999.9
#ifndef MAXCHAR
#define MAXCHAR 256
#endif

#define		SYNTAX \
"ldacpaste -i Input_catalogs [-o Output_catalog] \
[options]\n\n"

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacfilter ***************************************************
 *
 *  NAME	
 *      ldacpaste - Paste several catalogs together
 *
 *  SYNOPSIS
 *      ldacpaste -i incats [-o outcat]
 *
 *  FUNCTION
 *      This function pastes several catalogs together into one output
 *      catalog. It merges the FIELDS and OBJECT catalogs.
 *      It puts a unique identifier in the FIELDS table in order to mark
 *      which of the aditional tables in the catalog belong to this
 *      particular field. Added tables are e.g. PREASTROM and ASTROM tables
 *
 *  INPUTS
 *      -i incats     - The name of the input catalog files
 *     [-o outcat]    - The name of the output catalog file. The default
 *                      output catalog is default_filter.cat
 *
 *  RESULTS
 *
 *  RETURNS
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 07-07-1997
 *
 ******************************************************************************/
  {
   char		**tabnames;
   char		outfilename[MAXCHAR], infiles[MAXCHAR][MAXCHAR];
   int		ab,ae,a, ni, nc, nlines=0, prev_count, prev_nr, nobj;
   int		lb,le,nl;
   int		i, j, jj, k, ntabs, *prev_fields;
   short int	*field_pos;
   int		*nobj_count, *nobj_pos;
   catstruct	*incat, *outcat;
   tabstruct	*tab, *outtab;
   keystruct	*key, *key_p, *key_c,  *lkey;
   char		line[MAXCHAR];
   int		qflag = 1, obj_first;
   int		*xwid=NULL, *ywid=NULL;
   float	*xpos, *ypos;
   float	xmin = -BIG, xmax = BIG, ymin = -BIG, ymax = BIG;


/*default parameters */
  sprintf(outfilename, "default_filter.cat");

  ab=ae=ni=nc=0;
  lb=le=nl=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++)
                   strcpy(infiles[a-ab],argv[a]);
                ae = a--;
                ni = ae - ab;
	        break;
      case 'l':	for(lb = ++a; (a<argc) && (argv[a][0]!='-'); a++)
                   strcpy(infiles[a-lb],argv[a]);
                le = a--;
                nl = le - lb;
	        break;
      case 'o':	sprintf(outfilename, "%s", argv[++a]);
	        break;
      case 'q':	qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

  QFPRINTF(OUTPUT, "Reading catalog(s)");
  if (nl > 0)
     xmin = atof(argv[lb]);
  if (nl > 1)
     xmax = atof(argv[lb+1]);
  if (nl > 2)
     ymin = atof(argv[lb+2]);
  if (nl > 3)
     ymax = atof(argv[lb+3]);
  printf("%f %f %f %f \n", xmin,xmax, ymin,ymax);
  incat = read_cats(argv+ab, ni);
  outcat = new_cat(1);
  init_cat(outcat);
  historyto_cat(outcat, "ldacputxy", argc, argv);
  prev_count = 0;
  prev_nr = 0;
  ED_CALLOC(prev_fields, "ldacpaste", int, (ni+1));
/*
 * The actual merging
 */
  for (i=0; i<ni; i++) {
     tabnames = tabs_list(&incat[i], &ntabs);
     obj_first = 0;
     for (k=1;k<ntabs;k++) {
        if (strstr(tabnames[k], "FIELDS") != NULL && obj_first != 0) {
/*
 *         Go swap the tables in the tabnames array
 */
           strcpy(tabnames[k], "OBJECTS");
           strcpy(tabnames[obj_first], "FIELDS");
        }
        if (strstr(tabnames[k], "OBJECTS") != NULL) obj_first = k;
     }
     for (k=1;k<ntabs;k++) {
        sprintf(line,"Copying table %s of catalog %d",tabnames[k],i);
        QFPRINTF(OUTPUT, line);
        if (strcmp(tabnames[k], "FIELDS") == 0) {
/*
 *         For FIELD tables, read the OBJECT_POS and OBJECT_COUNT columns,
 *         update the OBJECT_POS column, and add a SubNr column to identify
 *         to which SubSet this section belongs
 */
           tab = name_to_tab(&incat[i], tabnames[k], 0);

           ED_GETKEY(tab, key_p, "OBJECT_POS",   nobj_pos,   nlines, "ldacpaste");
           ED_GETKEY(tab, key_c, "OBJECT_COUNT", nobj_count, nlines, "ldacpaste");
           ED_GETKEY(tab, lkey, "MAPNAXS1", xwid, nlines, "ldacpaste");
           ED_GETKEY(tab, lkey, "MAPNAXS2", ywid, nlines, "ldacpaste");
           nobj_pos[0] = 1;
           nobj_count[0] = 4;
           for (j=1;j<nlines;j++) {
              nobj_pos[j] = nobj_pos[j-1] + 4;
              nobj_count[j] = 4;
           }
           copy_tab(&incat[i], tabnames[k], 0, outcat, 0);
           outtab = name_to_tab(outcat, tabnames[k], 0);
           key = name_to_key(outtab, "OBJECT_POS");
           key->ptr = nobj_pos;
           key = name_to_key(outtab, "OBJECT_COUNT");
           key->ptr = nobj_count;
           update_tab(outtab);
        } else {
/*
 *         Read the SeqNr column and update it
 */
           if (strcmp(tabnames[k], "OBJECTS") == 0) {
              tab = name_to_tab(&incat[i], tabnames[k], 0);
              outtab = new_tab("OBJECTS");
              key = tab->key;
              copy_key(tab, "FIELD_POS", outtab, 0);
              copy_key(tab, "SeqNr", outtab, 0);
              copy_key(tab, "Xpos", outtab, 0);
              copy_key(tab, "Ypos", outtab, 0);
              copy_key(tab, "PosErr", outtab, 0);
              copy_key(tab, "XM2", outtab, 0);
              copy_key(tab, "YM2", outtab, 0);
              copy_key(tab, "Corr", outtab, 0);
              copy_key(tab, "A", outtab, 0);
              copy_key(tab, "B", outtab, 0);
              copy_key(tab, "Theta", outtab, 0);
              ED_GETKEY(tab, key, "FIELD_POS", field_pos, nobj, "ldacpaste");
              field_pos = key->ptr;
              for (j=0;j<nlines*4;j=j+4) {
                 for (jj=0;jj<4;jj++) {
                    field_pos[j+jj] = j/4+1;
                 }
              }
              key = name_to_key(outtab, "FIELD_POS");
              key->nobj = nlines*4;
              key->ptr = field_pos;
              ED_GETKEY(tab, key, "Xpos", xpos, nobj, "ldacpaste");
              ED_GETKEY(tab, key, "Ypos", ypos, nobj, "ldacpaste");
              if (xmin < 1.0) xmin = 1.0;
              if (ymin < 1.0) ymin = 1.0;
              if (xmax > xwid[0]) xmax = xwid[0];
              if (ymax > ywid[0]) ymax = ywid[0];
              for (j=0;j<nlines*4;j=j+4) {
                 xpos[j+0]=xmin; ypos[j+0]=ymin;
                 xpos[j+1]=xmin; ypos[j+1]=ymax;
                 xpos[j+2]=xmax; ypos[j+2]=ymax;
                 xpos[j+3]=xmax; ypos[j+3]=ymin;
              }
              key = name_to_key(outtab, "Xpos");
              key->ptr = xpos;
              key->nobj = nlines*4;
              key = name_to_key(outtab, "Ypos");
              key->ptr = ypos;
              key->nobj = nlines*4;
              key = name_to_key(outtab, "SeqNr");
              key->nobj = nlines*4;
              key = name_to_key(outtab, "PosErr");
              key->nobj = nlines*4;
              key = name_to_key(outtab, "XM2");
              key->nobj = nlines*4;
              key = name_to_key(outtab, "YM2");
              key->nobj = nlines*4;
              key = name_to_key(outtab, "Corr");
              key->nobj = nlines*4;
              key = name_to_key(outtab, "A");
              key->nobj = nlines*4;
              key = name_to_key(outtab, "B");
              key->nobj = nlines*4;
              key = name_to_key(outtab, "Theta");
              key->nobj = nlines*4;
              update_tab(outtab);
              if (add_tab(outtab, outcat, 0) == RETURN_ERROR) {
                  syserr(FATAL, "ldacpaste", "Error in pasting OBJECTS of %s\n",
                 infiles[i]);
              }
           } else {
/*
 *               Just append a SubSet number to each table name
 */
                 tab = name_to_tab(&incat[i], tabnames[k], 0);
                 copy_tab(&incat[i], tabnames[k], 0, outcat, 0);
              }
           }
        }
     }
  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, outfilename);
  return EXIT_SUCCESS;
  }

