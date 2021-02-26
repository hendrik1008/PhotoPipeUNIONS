 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACTestExits
*
*	Author:		E.Deul
*
*	Contents:	parsing and main loop.
*
*	Last modify:	07/09/2000
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
 * 15.07.01:
 * included "#include <string.h>" to avoid compiler warnings
 *
 * 19.10.2004:
 * removed the superfluous malloc.h inclusion
 *
 * 04.01.05:
 * removed compiler warnings (gcc with -O3 optimisation)
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>
#include        <strings.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		SYNTAX \
"ldactestexist -i Catalog [-o Output_catalog] -t <table> -k <key> ... \
[options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n"

#define		MAXCHAR 256

catstruct	*incat;
int		qflag;

/********************************** main ************************************/
int ldactestexist_main(int argc, char *argv[])

/****** ldactestkey ***************************************************
 *
 *  NAME	
 *      ldactestkey - Test the existance of a key (colum) in a specific table
 *
 *  SYNOPSIS
 *      ldactestkey -i incat -k keyname ...
 *                 [-t table_name] [-q]
 *
 *  FUNCTION
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *      -k keyname ...
 *                   - Test existance of the column with name keyname
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
 *      E. deul - dd 07-09-2000
 *
 ******************************************************************************/
  {
   keystruct	*key;
   tabstruct	*tab;
   char		tabname[MAXCHAR];
   int		a,ab,ae,ob,oe,k,kb,ke,ki=0,ni,i;

  if (argc<5)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(tabname, "OBJECTS");

  ab=ae=ob=oe=ni=kb=ke=0;
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
      case 'q': qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

/*Load the input catalogs*/
  for (i=0;i<ni;i++) {
  QFPRINTF(OUTPUT, "Reading catalog");
  if (!(incat = read_cat(argv[ab+i])))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", argv[ab]);

  tab = name_to_tab(incat, tabname, 0);
  if (!tab) {
     error(EXIT_FAILURE, "*Error*: found no such table: ", tabname);
  } else {
     if (!ki)
       return EXIT_SUCCESS;
  }
  for (k=kb; k<ke; k++)
    {
    key = name_to_key(tab, argv[k]);
    if (!key)
      error(EXIT_FAILURE, "*Error*: found no such key: ", argv[k]);
    }

  QFPRINTF(OUTPUT, "Cleaning memory");

  free_cat(incat, 1);
  }
  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

void ldactestexist(char *commandline)
{
   char *t, **argv;
   int i;

   argv = (char **) calloc(sizeof(char *), 1024);
   t = strtok(commandline,"     ");
   i = 0;
   argv[i] = (char *) calloc(sizeof(char), 1024);
   sprintf(argv[i], "ldactestexist");
   while (t) {
     i++;
     argv[i] = (char *) calloc(sizeof(char), 1024);
     sprintf(argv[i], t);
     t = strtok(NULL,"  ");
   }
   i++;
   ldactestexist_main(i,argv);
}
#ifndef SWIG
int main(int argc, char *argv[])
{
   ldactestexist_main(argc,argv);

   return 0;
}
#endif

