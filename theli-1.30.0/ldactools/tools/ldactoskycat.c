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

/*
20.04.03:
I made the part concerning the columns skycat absolutely needs
(ID, Ra, Dec, Mag) more robust.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		MAXCHAR	256
#define		SYNTAX \
"ldactoskycat -i Catalog1 Catalog2 [-t <Table_name>] [-k <Key_list> | -a] \n\
		[-l id_col <name_of_column, e.g. SeqNr>\n\
                    ra_col <name_of_column, e.g. ALPHA_J2000>\n\
                    dec_col <name_of_column, e.g. DELTA_J2000> \n\
                    mag_col <name_of_column, e.g. MAG_APER(2)>]\n\
		[options]\n\n\
Options are:	-s (Print strings and arrays)\n\
                -q (Quiet flag: defaulted to verbose!)\n"

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldactoasc ***************************************************
 *
 *  NAME	
 *      ldactoasc - Convert binary table infor to SKYCAT formatted text
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
 *     [-k keylist|-a       - A list of keys to be printer
 *                            The -a will give all known keys
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
 *      E.R. Deul = dd 24-09-1997 Copied from ldactoasc and modified
 *
 ******************************************************************************/
  {
   tabstruct	*tab;
   static char	outfilename[MAXCHARS], tabname[MAXCHARS];
   int	a,ab,ae,ni,do_strings,do_rowpos,do_banner, do_all;
   int  k,kb,ke,ki;
   int  l,lb,le,li,i,done;
   char **keynames, **SpecNames;
   keystruct **keys;

  if (argc<2)
    {
    fprintf(OUTPUT, "				ldactoasc V1.2\n\n"
			"Purpose: convert a FITS table to SKYCAT\n");
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
    }

/*default parameters */
  do_strings = 0;
  do_all = 0;
  do_rowpos = 0;
  do_banner = 1;
  qflag = 1;

  sprintf(tabname, "FIELDS");

  sprintf(outfilename, "default_test.cat");

  ab=ae=ni=0;
  kb=ke=ki=0;
  lb=le=li=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ae = a--;
		ni = ae - ab;
	        break;
      case 't':	sprintf(tabname, "%s", argv[++a]);
	        break;
      case 'q': qflag = 0;
	        break;
      case 's': do_strings = 1;
	        break;
      case 'a': do_all = 1;
	        break;
      case 'l': for(lb = ++a; (a<argc) && (argv[a][0]!='-'); a++);
                le = a--;
                li = le - lb;
                break;
      case 'k': for(kb = ++a; (a<argc) && (argv[a][0]!='-'); a++);
                ke = a--;
                ki = ke - kb;
                break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

  /* go through the names id, ra, dec and mag necessary
     for a valid skycat catalog
  */

  QMALLOC(SpecNames, char *, 8)
  for (l=0;l<8;l++)
  {
       QMALLOC(SpecNames[l], char, MAXCHAR)
  }
  strcpy(SpecNames[0], "id_col");
  strcpy(SpecNames[1], "SeqNr");
  strcpy(SpecNames[2], "ra_col");
  strcpy(SpecNames[3], "Ra");
  strcpy(SpecNames[4], "dec_col");
  strcpy(SpecNames[5], "Dec");
  strcpy(SpecNames[6], "mag_col");
  strcpy(SpecNames[7], "MAG_ISO");

  if (li)
  {
    /* is li an odd number or larger than the maximum
       allowed value 8?
    */
    if(((li %2) != 0) || (li >8))
    {
      error(EXIT_FAILURE, "*Error* in skycat colums (-l option)", "");
    }

    li/=2;
    for(l=0; l<li; l++)
    {
      done=0;
      for(i=0; i<4; i++)
      {
	if(strcmp(SpecNames[2*i], argv[lb+2*i]) == 0)
	{
	  strncpy(SpecNames[2*i+1], argv[lb+2*i+1], MAXCHAR-1);
	  done=1;
	}
      }
      if(done == 0)
      {
	error(EXIT_FAILURE, "*Error* skycat columns must be id_col, ra_col, dec_col or mag_col", "");
      }	
    }
  }
  li=4;

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
    show_keys(tab, keynames, keys, ki, SpecNames, li, NULL, stdout,
        do_strings, do_banner, do_rowpos, SKYCAT);

    }
  for (l=0;l<li*2;l++)
     QFREE(SpecNames[l]);
  QFREE(SpecNames);
  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
}
