 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACDelKey
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
 * 04.01.05:
 * removed compiler warnings (gcc with -O3 optimisation)
 *
 * 29.01.2005:
 * I added history information to the output catalog
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		SYNTAX \
"ldacdelkey -i Catalog [-o Output_catalog] -k <key1> <key2> ... \
[options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n\
		-t <Table_name> [defaulted to OBJECTS]\n"

#define		MAXCHAR 256

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacdelkey ***************************************************
 *
 *  NAME	
 *      ldacdelkey - Rename a key (colum) in a specific table
 *
 *  SYNOPSIS
 *      ldacdelkey -i incat [-o outcat] -k keyname1 keyname2 ...
 *                 [-t table_name] [-q]
 *
 *  FUNCTION
 *      This program allows the removal of a specific column from a specific
 *      table in the catalog file.
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *      -o outcat    - The name of the output catalog file. The default name
 *                     is default_renkey.cat
 *      -k keyname1 keyname2 ...
 *                   - Remove the columns with names keyname1, keyname2
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
 *      E. Bertin - dd 15-01-1996
 *
 ******************************************************************************/
  {
   tabstruct	*tab;
   char		outfilename[MAXCHAR], tabname[MAXCHAR];
   int		a,ab,ae,k,kb=0,ke=0,ki, ni;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(tabname, "OBJECTS");
  sprintf(outfilename, "default_delkey.cat");

  ab=ae=ni=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ae = a--;
		ni = ae - ab;
	        break;
      case 'k':	for(kb = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ke = a--;
		ki = ke - kb;
	        break;
      case 't':	sprintf(tabname, "%s", argv[++a]);
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

  tab = name_to_tab(outcat, tabname, 0);
  if (!tab)
    error(EXIT_FAILURE, "*Error*: found no such table: ", tabname);
  for (k=kb; k<ke; k++)
    {
    if (!name_to_key(tab, argv[k]))
      error(EXIT_FAILURE, "*Error*: found no such key: ", argv[k]);
    remove_key(tab, argv[k]);
    }


  historyto_cat(outcat, "ldacdelkey", argc, argv);
  QFPRINTF(OUTPUT, "Filtering catalog");
  save_cat(outcat, outfilename);
  QFPRINTF(OUTPUT, "Cleaning memory");

  free_cat(incat, 1);
  free_cat(outcat, 1);

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

