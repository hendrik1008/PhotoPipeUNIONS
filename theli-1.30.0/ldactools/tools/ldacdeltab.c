 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACDeltab
*
*	Author:		T.ERBEN, MPA-Garching
*
*	Contents:	parsing and main loop.
*
*	Last modify:	16.02.1999
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
  04.01.05:
  removed compiler warnings (gcc with -O3 optimisation)

  29.01.2005:
  I added history information to the output catalog
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"
#include        "utils_globals.h"

#define		SYNTAX \
"ldacdeltab -i Catalog [-o Output_catalog] -t <Table_name> \
[options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n"

#ifndef         MAXCHAR
#define		MAXCHAR 256
#endif

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacdeltab ***************************************************
 *
 *  NAME	
 *      ldacdeltab - Rename a table from a catalog
 *
 *  SYNOPSIS
 *      ldacdeltab -i incat [-o outcat] -t table_name [-q]
 *
 *
 *  FUNCTION
 *      This program allows the removal of a specific table from a
 *      catalog file.
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *      -o outcat    - The name of the output catalog file. The default name
 *                     is default_renkey.cat
 *      -t tablename
 *                   - Remove the table
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
 *      T. Erben - dd 16-02-1999
 *
 ******************************************************************************/
  {
   char		outfilename[MAXCHAR], tabname[MAXCHAR][MAXCHAR];
   int		a,ab,ae,ni,tb,te,nt=0,i;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(outfilename, "default_delkey.cat");

  ab=ae=ni=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ae = a--;
		ni = ae - ab;
	        break;
      case 't':	for(tb = ++a; (a<argc) && (argv[a][0]!='-'); a++)
                  sprintf(tabname[a-tb], "%s", argv[a]);
                te = a--;
                nt = te - tb;
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
  QFPRINTF(OUTPUT, "Removing column(s)");
  outcat = new_cat(1);
  inherit_cat(incat, outcat);
  copy_tabs(incat, outcat);

  for (i=0;i<nt;i++) {
    if(remove_tab(outcat, tabname[i], 0) != RETURN_OK)
    {
      error(EXIT_FAILURE, "Error in removing table: ", tabname[i]);
    }
  }
  historyto_cat(outcat, "ldacdeltab", argc, argv);
  QFPRINTF(OUTPUT, "Filtering catalog");
  save_cat(outcat, outfilename);
  QFPRINTF(OUTPUT, "Cleaning memory");

  free_cat(incat, 1);
  free_cat(outcat, 1);

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

