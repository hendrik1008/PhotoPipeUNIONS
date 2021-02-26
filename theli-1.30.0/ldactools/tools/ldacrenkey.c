 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACRenKey
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
  04.01.05:
  removed compiler warnings (gcc with -O3 optimisation)

  14.01.05:
  I now check whether new names for a key exist already.

  29.01.2005:
  I added history information to the output catalog
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		SYNTAX \
"ldacrenkey -i Catalog [-o Output_catalog] -k <oldkey1> <newkey1> ... \
[options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n\
		-t <Table_name> [defaulted to OBJECTS]\n"

#define		MAXCHAR 256

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacrenkey ***************************************************
 *
 *  NAME	
 *      ldacrenkey - Rename a key (colum) in a specific table
 *
 *  SYNOPSIS
 *      ldacrenkey -i incat [-o outcat] -k oldkeyname newkeyname ...
 *                 [-t table_name] [-q]
 *
 *  FUNCTION
 *      This program allows the renaming of a specific column from a specific
 *      table in the catalog file.
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *      -o outcat    - The name of the output catalog file. The default name
 *                     is default_renkey.cat
 *      -k oldkeyname newkeyname ...
 *                   - Rename the column with name oldkeyname to newkeyname
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
 *      E. Bertin - dd 26-06-1996
 *      E.R. Deul - dd 01-08-1996 Adapted to LDAC pipeline conventions
 *      E. Bertin - dd 15-01-1997 Bugs in case of error corrected
 *
 ******************************************************************************/
  {
   keystruct	*key, *keytmp;
   tabstruct	*tab;
   char		outfilename[MAXCHAR], tabname[MAXCHAR];
   int		a,ab,ae,ob,oe,k,kb=0,ke=0,ki,ni,i,no=0;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(tabname, "OBJECTS");
  sprintf(outfilename, "default_renkey.cat");

  ab=ae=ob=oe=ni=0;
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
		if (ki%2)
		  error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
	        break;
      case 't':	sprintf(tabname, "%s", argv[++a]);
	        break;
      case 'o':	for(ob = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		oe = a--;
		no = oe - ob;
	        break;
      case 'q': qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni || ni != no)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

/*Load the input catalogs*/
  for (i=0;i<ni;i++) {
  QFPRINTF(OUTPUT, "Reading catalog");
  if (!(incat = read_cat(argv[ab+i])))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", argv[ab]);
  QFPRINTF(OUTPUT, "Changing name(s)");
  outcat = new_cat(1);
  inherit_cat(incat, outcat);
  copy_tabs(incat, outcat);

  tab = name_to_tab(outcat, tabname, 0);
  if (!tab)
    error(EXIT_FAILURE, "*Error*: found no such table: ", tabname);
  for (k=kb; k<ke; k+=2)
    {
    key = name_to_key(tab, argv[k]);
    if (!key)
    {
      error(EXIT_FAILURE, "*Error*: found no such key : ", argv[k]);
    }
    /* see if a key with the new name exists already */
    keytmp = name_to_key(tab,argv[k+1]);
    if(keytmp)
    {
      error(EXIT_FAILURE, "*Error*: key exists already: ", argv[k+1]);
    }
    strcpy(key->name, argv[k+1]);
    }

  historyto_cat(outcat, "ldacrenkey", argc, argv);
  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, argv[ob+i]);
  QFPRINTF(OUTPUT, "Cleaning memory");

  free_cat(incat, 1);
  free_cat(outcat, 1);
  }
  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

