/*
 * 30.07.01:
 * converted long int -> LONG (bug with this type on INT64 machines)
 *
 * 05.05.03:
 * included the tsize.h file
 *
 * 04.01.05:
 * removed compiler warnings (gcc with -O3 optimisation)
 *
 * 29.01.2005:
 * I added history information to the output catalog
 */

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
*	Last modify:	15/01/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"
#include	"tsize.h"
#include	"tabutil_globals.h"
#include        "lists_globals.h"
#include	"utils_globals.h"

#ifndef MAXCHAR
#define MAXCHAR 256
#endif

#define		SYNTAX \
"ldacaddkey -i Catalog [-o Output_catalog]\
[options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n\
		-t <Table_name> [defaulted to OBJECTS]\n\
                -k keyname keyvalue keytype keycomment"

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacaddkey ***************************************************
 *
 *  NAME	
 *      ldacaddkey - Add a key (column) to a specific table
 *
 *  SYNOPSIS
 *      ldacaddkey -i incat [-o outcat] [-t table_name] [-q]
 *                 value type
 *
 *  FUNCTION
 *      This program adds a column filled with a single value to a specific
 *      table in the catalog file.
 *
 *  INPUTS
 *      -i incat       - The name of the input catalog
 *     [-o outcat]     - The name of the output catalog, default name is
 *                       default_renkey.cat
 *     [-t table_name] - The name of the table to whichto add the column. The
 *                       defautlt table name is OBJECTS
 *     [-q]            - Make the program verbose
 *     value           - The value of the elemenst in the column
 *     type            - The type of the column. Known types are:
 *                          T_BYTE, T_SHORT, T_LONG, T_FLOAT, T_DOUBLE
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
 *      E. Bertin - dd 26-08-1996
 *      E.R. Deul - dd 01-08-1996 Adapted to LDAC pipeline convention
 *      E. Bertin - dd 15-01-1997 Added handling of errors
 *      E.R. Deul - dd 15-04-1998 Added counter type
 *
 ******************************************************************************/
  {
   tabstruct	*tab;
   t_type	ttype = T_BYTE;
   char		outfilename[MAXCHAR], tabname[MAXCHAR];
   short int	*shptr, shval;
   LONG	        *lnptr, lnval;
   float	*flptr, flval;
   double	*dbptr, dbval;
   char		*byptr, byval;
   void		*ptr=NULL;
   int		a,ab,ae, i,k, kb=0,ke=0,ki, ni, nobj, len=0;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(tabname, "OBJECTS");
  sprintf(outfilename, "default_addkey.cat");

  ab=ae=ni=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ae = a--;
		ni = ae - ab;
	        break;
      case 'k':	for(kb = ++a; (a<argc); a++);
		ke = a--;
		ki = ke - kb;
		if (ki%4)
		  error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
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
  QFPRINTF(OUTPUT, "Changing value(s)");
  outcat = new_cat(1);
  inherit_cat(incat, outcat);
  copy_tabs(incat, outcat);

  tab = name_to_tab(outcat, tabname, 0);
  if (!tab)
    error(EXIT_FAILURE, "*Error*: found no such table: ", tabname);

  if (!tab->key)
    error(EXIT_FAILURE, "*Error*: no key found in table ", tabname);

  nobj = tab->key->nobj;
  for (k=kb; k<ke; k+=4) {
    if (strcmpcu("BYTE", argv[k+2]) == 0)   ttype = T_BYTE;
    if (strcmpcu("COUNT", argv[k+2]) == 0)  ttype = T_LONG;
    if (strcmpcu("SHORT", argv[k+2]) == 0)  ttype = T_SHORT;
    if (strcmpcu("LONG", argv[k+2]) == 0)   ttype = T_LONG;
    if (strcmpcu("FLOAT", argv[k+2]) == 0)  ttype = T_FLOAT;
    if (strcmpcu("DOUBLE", argv[k+2]) == 0) ttype = T_DOUBLE;
    if (strcmpcu("STRING", argv[k+2]) == 0) ttype = T_STRING;
    switch(ttype) {
    case T_SHORT:  shval = (short int) atoi(argv[k+1]);
                   QMALLOC(shptr, short int , nobj);
                   for (i=0;i<nobj;i++)
                      shptr[i] = shval;
                   ptr = shptr;
                   break;
    case T_LONG:   lnval = (LONG) atoi(argv[k+1]);
                   QMALLOC(lnptr, LONG, nobj);
                   if (strcmp("COUNT", argv[k+2]) == 0) {
                      for (i=0;i<nobj;i++)
                         lnptr[i] = lnval + i;
                   } else {
                      for (i=0;i<nobj;i++)
                         lnptr[i] = lnval;
                   }
                   ptr = lnptr;
                   break;
    case T_FLOAT:  flval = (float) atof(argv[k+1]);
                   QMALLOC(flptr, float, nobj);
                   for (i=0;i<nobj;i++)
                      flptr[i] = flval;
                   ptr = flptr;
                   break;
    case T_DOUBLE: dbval = (double) atof(argv[k+1]);
                   QMALLOC(dbptr, double, nobj);
                   for (i=0;i<nobj;i++)
                      dbptr[i] = dbval;
                   ptr = dbptr;
                   break;
    case T_BYTE:   byval = (char) atoi(argv[k+1]);
                   QMALLOC(byptr, char, (nobj * t_size[ttype]));
                   for (i=0;i<nobj;i++)
                      byptr[i] = byval;
                   ptr = byptr;
                   break;
    case T_STRING: len = strlen(argv[k+1]);
                   for (i=0;i<len;i++)
                     if (argv[k+1][i] == ' ') argv[k+1][i] = 0;
                   QMALLOC(byptr, char, (nobj * len * t_size[ttype]));
                   for (i=0;i<nobj;i++)
                      strcpy(&byptr[i*len], argv[k+1]);
                   ptr = byptr;
                   break;
    default: error(EXIT_FAILURE,
                   "*FATAL ERROR*: Unsupported FITS type in ",
                   "ldacaddkey()");
                   break;
    }
    if (ttype == T_STRING)
      add_key_to_tab(tab, argv[k], nobj, ptr, ttype, len, "", argv[k+3]);
    else
      add_key_to_tab(tab, argv[k], nobj, ptr, ttype, 1, "", argv[k+3]);
  }

  historyto_cat(outcat, "ldacaddkey", argc, argv);
  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, outfilename);
  QFPRINTF(OUTPUT, "Cleaning memory");

  free_cat(incat, 1);
  free_cat(outcat, 1);

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

