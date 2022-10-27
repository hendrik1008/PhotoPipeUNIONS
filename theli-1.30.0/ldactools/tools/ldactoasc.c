 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACTest
*
*	Author:		E.BERTIN, DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	21/04/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		SYNTAX \
"ldactoasc -i Catalog1 Catalog2 [-t <Table_name>] [-k keys] [options]\n\n\
Options are:	-b (suppress printing of the banner)\n\
		-r (Print the row number)\n\
		-s (Print strings and arrays)\n\
                -q (Quiet flag: defaulted to verbose!)\n"

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldactoasc ***************************************************
 *
 *  NAME	
 *      ldactoasc - Convert binary table infor to ASCII formatted text
 *
 *  SYNOPSIS
 *      ldactoasc -i incat1 incat2 .. [-t tablename] [-s] [-q]
 *
 *  FUNCTION
 *      This program converts the binary information stored in a specified
 *      table into ASCII formatted text that can more easily be read. The
 *      Table column names and positions are given in a # commented header
 *      to allow easy interpretation and plotting through supermongo.
 *
 *  INPUTS
 *      -i incat1 incat2 .. - The list of input catalogs
 *     [-t tablename]       - The name of the table to be converted
 *     [-b]                 - Suppress printing of a banner describing the
 *                            columns content.
 *     [-r]                 - Add to the output a dummy column, containing
 *                            the running row number.
 *     [-s]                 - Include text strings and arrays in the output.
 *                            Default operation is to plot scalars only.
 *     [-q]                 - Make the program less verbose
 *
 *  RESULTS
 *     The output is send to standard output.
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
 *      E. Bertin - dd 13-06-1996
 *      E.R. Deul - dd Adapted to LDAC pipeline conventions
 *                     Added string support
 *	E. Bertin - dd 15-01-1997 Added handling of errors.
 *	E. Bertin - dd 21-01-1997 Added Banner and Row_pos flags.
 *
 ******************************************************************************/
  {
   tabstruct	*tab;
   static char	outfilename[MAXCHARS], tabname[MAXCHARS];
   int	a,ab,ae, ni,do_strings,do_rowpos,do_banner;
   int  k,kb,ke, ki;
   char **keynames;
   keystruct **keys;

  if (argc<2)
    {
    fprintf(OUTPUT, "				ldactoasc V1.3\n\n"
			"Purpose: convert a FITS table to ASCII\n");
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
    }

/*default parameters */
  do_strings = 0;
  do_rowpos = 0;
  do_banner = 1;
  qflag = 1;

  sprintf(tabname, "FIELDS");

  sprintf(outfilename, "default_test.cat");

  ab=ae=ni=0;
  kb=ke=ki=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'b': do_banner = 0;
	        break;
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ae = a--;
		ni = ae - ab;
	        break;
      case 'r': do_rowpos = 1;
	        break;
      case 't':	sprintf(tabname, "%s", argv[++a]);
	        break;
      case 'q': qflag = 0;
	        break;
      case 's': do_strings = 1;
	        break;
      case 'k': for(kb = ++a; (a<argc) && (argv[a][0]!='-'); a++);
                ke = a--;
                ki = ke - kb;
                break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

/*Load the input catalogs*/
  QFPRINTF(OUTPUT, "Reading catalog(s)");
  if (ki) {
    QMALLOC(keynames,char *, ki)
    QMALLOC(keys,keystruct *, ki)
    for (k=kb;k<ke;k++)
       keynames[k-kb] = argv[k];
       QMALLOC(keys[k-kb],keystruct, 1)
  } else  {
    keynames = NULL;
    keys = NULL;
  }
  for (a=ab; a<ae; a++)
    {
    if (!(incat = read_cat(argv[a])))
      error(EXIT_FAILURE, "*Error*: catalog not found: ", argv[a]);
    if (!(tab = name_to_tab(incat, tabname, 0)))
      error(EXIT_FAILURE,"*Error*: found no such table: ", tabname);
    show_keys(tab, keynames, keys, ki, NULL, 0, NULL, stdout,
	do_strings, do_banner, do_rowpos,ASCII);

    }

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }
