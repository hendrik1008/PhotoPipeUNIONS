 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACRenTab
*
*	Author:		E.BERTIN, DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	15/01/96
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
  14.01.2005:
  I now check whether names for a new table exist already
  within the catalog

  29.01.2005:
  I added history information to the output catalog
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>

#include        "utils_globals.h"
#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		SYNTAX \
"ldacrentab -i Catalog [-o Output_catalog] -t <oldtab1> <newtab1> ... \
[options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n"

#ifndef         MAXCHAR
#define		MAXCHAR 256
#endif

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacrentab ***************************************************
 *
 *  NAME	
 *      ldacrentab  - Rename a table in a catalog
 *
 *  SYNOPSIS
 *      ldacrentab  -i incat [-o outcat] -t oldtabname newtabname ...
 *                 [-q]
 *
 *  FUNCTION
 *      This program allows the renaming of a specific tables in the
 *      catalog file.
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *      -o outcat    - The name of the output catalog file. The default name
 *                     is default_renkey.cat
 *      -t oldtabname newtabname ...
 *                   - Rename the table with name oldtabname to newtabname
 *                     A list of pairs can be provided to the programs, so
 *                     multiple renaming is possible.
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
 *      E.R. Deul - dd 17-03-1998
 *
 ******************************************************************************/
  {
   tabstruct	*tab, *tabtmp;
   char		outfilename[MAXCHAR];
   int		a,ab,ae,t,tb=0,te=0,ti,ni;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(outfilename, "default_rentab.cat");

  ab=ae=ni=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ae = a--;
		ni = ae - ab;
	        break;
      case 't':	for(tb = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		te = a--;
		ti = te - tb;
		if (ti%2)
		  error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
	        break;
      case 'o':	sprintf(outfilename, "%s", argv[++a]);
	        break;
      case 'q': qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

/*Load the input catalogs*/
  QFPRINTF(OUTPUT, "Reading catalog");
  if (!(incat = read_cat(argv[ab])))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", argv[ab]);
  QFPRINTF(OUTPUT, "Changing name(s)");
  outcat = new_cat(1);
  inherit_cat(incat, outcat);
  copy_tabs(incat, outcat);

  for (t=tb; t<te; t+=2)
    {
    tab = name_to_tab(outcat, argv[t], 0);
    if (!tab)
    {
      error(EXIT_FAILURE, "*Error*: found no such table: ", argv[t]);
    }

    tabtmp = name_to_tab(outcat, argv[t+1], 0);
    if (tabtmp)
    {
      error(EXIT_FAILURE, "*Error*: table exists already: ", argv[t+1]);
    }
    strcpy(tab->extname, argv[t+1]);
    }

/* add HISTORY information to the output catalog */
  historyto_cat(outcat, "ldacrentab", argc, argv);

  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, outfilename);
  QFPRINTF(OUTPUT, "Cleaning memory");

  free_cat(incat, 1);
  free_cat(outcat, 1);

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

