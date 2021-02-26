 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACAddTab
*
*	Author:		E.R.Deul , DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	15/01/96
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
  29.01.2005:
  I added history information to the output catalog

  24.07.2007:
  I included explicit error checking when copying the
  tables that have to be appended
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"
#include        "utils_globals.h"


#define		SYNTAX \
"ldacaddtab -i Catalog [-o Output_catalog] -p Proto_Catalog \n\
            -t Table_name [Table_name ...]\n\
            [options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n"

#ifndef         MAXCHAR
#define		MAXCHAR 256
#endif

catstruct	*incat, *outcat, *protocat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacaddtab ***************************************************
 *
 *  NAME	
 *      ldacaddtab - Add a table from the proto cat to the input cat
 *
 *  SYNOPSIS
 *      ldacaddtab -i incat [-o outcat] -p protocat [-t table] [-q]
 *                 -t table_name1 table_name2 ...  [-q]
 *
 *  FUNCTION
 *      This program allows one to copy a specific table from the proto
 *      catalog to the input catalog.
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *      -o outcat    - The name of the output catalog file. The default name
 *                     is default_renkey.cat
 *      -p protocat  - The name of the proto catalog file
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
 *      E.R. Deul - dd 05-11-1997
 *
 ******************************************************************************/
  {
   char		outfilename[MAXCHAR];
   char		infilename[MAXCHAR],protfilename[MAXCHAR];
   int		a,ab,ae,ni;
   int          result;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(outfilename, "default_addtab.cat");

  ab=ae=ni=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	sprintf(infilename, "%s", argv[++a]);
	        break;
      case 't':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
                ae = a--;
                ni = ae - ab;
	        break;
      case 'o':	sprintf(outfilename, "%s", argv[++a]);
	        break;
      case 'p':	sprintf(protfilename, "%s", argv[++a]);
	        break;
      case 'q': qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }
  if (ni == 0)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*Load the input catalog*/
  QFPRINTF(OUTPUT, "Reading catalog");
  if (!(incat = read_cat(infilename)))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", infilename);
  if (!(protocat = read_cat(protfilename)))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", protfilename);
  outcat = new_cat(1);
  inherit_cat(incat, outcat);
  historyto_cat(outcat, "ldacaddtab", argc, argv);
  copy_tabs(incat, outcat);
  for (a=ab; a<ae; a++)
  {
     result = copy_tab(protocat, argv[a], 0, outcat, 0);
     if(result != RETURN_OK)
     {
       error(EXIT_FAILURE,"error in copying table: ", argv[a]);
     }
  }
  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, outfilename);
  QFPRINTF(OUTPUT, "Cleaning memory");

  free_cat(incat, 1);
  free_cat(outcat, 1);
  free_cat(protocat, 1);

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

