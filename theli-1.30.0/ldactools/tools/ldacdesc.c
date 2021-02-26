/*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACDesc
*
*	Author:		E.Deul, DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	15/01/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
 * 19.10.2004:
 * removed the superfluous malloc.h inclusion
 *
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>
#include        <strings.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"

#define		SYNTAX \
"ldacdesc -i Catalog1 [-t TABLE_NAME][options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n\
		-d (print dimension of table)\n\
		-e (print number of elements of table)\n\
		-f (print number of fields of table)\n"

#define		MAXCHAR 256
catstruct	*incat;
int		qflag;

/********************************** main ************************************/
int ldacdesc_main(int argc, char *argv[])

/****** ldacdesc ***************************************************
 *
 *  NAME	
 *      ldacdesc - Give a catalog description
 *
 *  SYNOPSIS
 *      ldacdesc -i incat [-t tabname][-q]
 *
 *  FUNCTION
 *      This program gives a description of the content of a catalog file.
 *      It provides information on which tables exist in the file, and
 *      for each table produces a list of charactersistics, such as
 *      the number of column and rows, the total number of bytes, etc.
 *
 *  INPUTS
 *      -i incat   - The name of the catalog to be described
 *      -t tabname - The name of the table to be described
 *     [-q]        - Make the program verbose
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
 *      E.R Deul - dd 26-06-1996
 *      E. Bertin - 15-01-97 added handling of errors
 *
 ******************************************************************************/
  {
   char	 infilename[MAXCHAR], table_name[MAXCHAR], infotype[MAXCHAR];
   int	a,ni,do_tab=0;
   tabstruct *tab;

  if (argc<2)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  ni = 0;
  strcpy(infotype,"");
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	sprintf(infilename, "%s", argv[++a]);
                ni++;
	        break;
      case 'e':	sprintf(infotype, "elem");
	        break;
      case 'd':	sprintf(infotype, "dim");
	        break;
      case 'f':	sprintf(infotype, "field");
	        break;
      case 't':	sprintf(table_name, "%s", argv[++a]);
                ni++;
                do_tab=1;
	        break;
      case 'q': qflag = 0;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

/*Load the input catalogs*/
  QFPRINTF(OUTPUT, "Reading catalog(s)");
  if (!(incat = read_cat(infilename)))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", infilename);
  if (do_tab) {
      if (infotype[0]) {
         tab = name_to_tab(incat, table_name, 0);
         if (!tab)
           error(EXIT_FAILURE, "*Error*: found no such table: ", table_name);
         if (strcmp(infotype,"dim") == 0)
            printf(" Table dimension: %d\n", tab->naxis);
         if (strcmp(infotype,"elem") == 0)
            if (tab->naxis)
              printf(" Table number of elements: %d\n", tab->naxisn[1]);
         if (strcmp(infotype,"field") == 0)
            printf(" Table number of fields: %d\n", tab->tfields);
      } else
        if (about_tab(incat, table_name, stdout)!=RETURN_OK)
          error(EXIT_FAILURE, "*Error*: found no such table: ", table_name);
  } else {
     about_cat(incat, stdout);
  }

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

void ldacdesc(char *commandline)
{
   char *t, **argv;
   int i;

   argv = (char **) calloc(sizeof(char *), 1024);
   t = strtok(commandline,"     ");
   i = 0;
   argv[i] = (char *) calloc(sizeof(char), 1024);
   sprintf(argv[i], "ldacdesc");
   while (t) {
     i++;
     argv[i] = (char *) calloc(sizeof(char), 1024);
     sprintf(argv[i], t);
     t = strtok(NULL,"  ");
   }
   i++;
   ldacdesc_main(i,argv);
}
#ifndef SWIG
int main(int argc, char *argv[])
{
   ldacdesc_main(argc,argv);

   return 0;
}
#endif

