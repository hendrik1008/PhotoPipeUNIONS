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
*	Last modify:	27/07/99
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
  21.03.02:
  updated the program so that it can mask filtered
  objects instead of deleting them

  14.06.2004:
  The string holding the filtering condition now
  has a variable length.

  04.01.05:
  removed compiler warnings (gcc with -O3 optimisation)

  29.01.2005:
  I added argc and argv to the argument list of filter
  as they are needed for writing the HISTORY to the
  output catalog.

  26.08.2010:
  I pushed the version number to 1.4.0 because we now can treat
  vector quantities in conditions.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>

#include	"filter_globals.h"
#include	"options_globals.h"
#include        "utils_globals.h"

#define NAME	"Catalog Filter Program"
#define VERS	"1.4.0"
#define DATE	__DATE__


#define		SYNTAX \
"ldacfilter -i Input_catalog [-o Output_catalog] -c <filter_condition> \
 [ -m maskname ] [options]\n\n\
Options are:	-t <Table_name> [defaulted to OBJECTS]\n"


/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacfilter ***************************************************
 *
 *  NAME	
 *      ldacfilter - Use conditional argument to filter a table from a catalog
 *
 *  SYNOPSIS
 *      ldacfilter -i incat [-o outcat] -c condition [-t tablename]
 *
 *  FUNCTION
 *      This routine filters the input catalog using the provided condition
 *      and stores the rows of the specified table that adhere to the
 *      condition in the output catalog. All other tables are left in tact.
 *
 *      The syntax of the condition statement can be written down in the
 *      following  representation, where | is the or character, a string
 *      surrounded by (..)* may exist zero to n times, and -> means resolves
 *      into. Any string or character to be taken literal is surrounded by ''.`
 *
 *      [condition] -> '(' [condition] ')' ( ('AND'|'OR') '(' [condition] ')' )*
 *                      | ident ('='|'<='|'>='|'<'|'>') [expr]
 *                      | [func]
 *
 *      [expr]      -> [term] (('+'|'-') [term])*
 *
 *      [term]      -> [factor] (('*'|'/') [factor])*
 *
 *      [factor]    -> '(' [expr] ')' | [number] | ident
 *
 *      [number]    -> ('+'|'-'|$) (digit)* ('.'|$) (digit)*
 *
 *      [func]      -> (char) (char)* '(' [number] ',' [number]')'
 *
 *      The ident denoted above is in fact the name of an individual column
 *      from the specified table.
 *
 *  INPUTS
 *      -i incat      - The name of the input catalog file
 *     [-o outcat]    - The name of the output catalog file. The default
 *                      output catalog is default_filter.cat
 *      -c condition  - The conditional statement
 *     [-t tabename]  - The name of the table for which the condition holds.
 *                      The default table name is OBJECTS.
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
 *      E.R. Deul - dd 30-06-1996
 *
 ******************************************************************************/
  {
   char		outfilename[MAXCHAR], tabname[MAXCHAR];
   char		infilename[MAXCHAR], header[MAXCHAR];
   char         maskname[MAXCHAR];
   char         *condition=NULL;
   int		a, ni, nc;
   char         *cline;
   int          dofilter = 1; /* delete objects (dofilter=1) or mask them
                                 only (filter=0)
                              */

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  sprintf(tabname, "OBJECTS");
  sprintf(outfilename, "default_filter.cat");
  sprintf(maskname, "FILTERMASK");

  ni=nc=0;
  altverbose("NORMAL");
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	sprintf(infilename, "%s", argv[++a]);
                ni=1;
	        break;
      case 'c':	ED_CALLOC(condition, "ldacfilter", char,
                          strlen(argv[++a])+1);
	        sprintf(condition, "%s", argv[a]);
                nc=0;
	        break;
      case 'n':	ED_CALLOC(condition, "ldacfilter", char,
                          strlen(argv[++a])+1);
                sprintf(condition, "%s", argv[++a]);
                nc=1;
	        break;
      case 't':	strcpy(tabname, argv[++a]);
	        break;
      case 'o':	sprintf(outfilename, "%s", argv[++a]);
	        break;
      case 'v':	altverbose(argv[++a]);
	        break;
      case 'm':	dofilter=0;
                sprintf(maskname, "%s", argv[++a]);	
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);
    sprintf(header,"%s Version: %s (%s)",NAME,VERS,DATE);
    VPRINTF(NORMAL,"\n   %s\n\n",header);

  cline = build_command_line(argc, argv);

  filter(infilename, outfilename, tabname, condition, nc, argc, argv,
         dofilter, maskname);

  ED_FREE(condition);

  return EXIT_SUCCESS;
  }

