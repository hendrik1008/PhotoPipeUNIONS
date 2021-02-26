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
*	Last modify:	15/01/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
 * 05.05.03:
 * included the tsize.h file
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	"fitscat.h"
#include	"fitscat_defs.h"
#include	"tsize.h"
#include	"utils_globals.h"
#include	"tabutil_globals.h"
#include	"options_globals.h"
#ifndef MAXCHAR
#define MAXCHAR 256
#endif

#define		SYNTAX \
"ldaccheck -i Input_fits_files \
[options]\n\n"


void readfull_header(tabstruct *tab)
{
  keystruct	*key;
  static char	comment[82],keyword[16],ptr[82];
  h_type	htype;
  t_type	ttype;
  char		*lptr;
  int		i;

  lptr = tab->headbuf;
  for (i=tab->tabsize/80; i-- && strncmp(lptr, "END     ", 8);)
    {
/*Interprete the next FITS line */
    if (fitspick(lptr, keyword, ptr, &htype, &ttype, comment) != RETURN_OK)
{
      char line[81];
      int  qflag=1;
      strncpy(line, lptr, 80);
      line[80] = '\0';
      QFPRINTF(OUTPUT, line);
      error(EXIT_FAILURE, "*Error*: incorrect FITS field in ",
        tab->cat->filename);
    }
    if (htype != H_COMMENT)
      {
/*----Create a new key and fill it with the right parameters*/
      key = new_key(keyword);
      strcpy(key->comment, comment+strspn(comment, " "));
      key->htype = htype;
      key->ttype = ttype;
      key->nbytes = t_size[ttype];
/*----!!Temporary (?)  solution for STRINGS*/
      if (htype==H_STRING)
        {
        key->naxis = 1;
        QMALLOC(key->naxisn, int, 1);
        key->naxisn[0] = 16;
        key->nbytes *= key->naxisn[0];
        }
      key->nobj = 1;
      if (add_key(key, tab, 0)==RETURN_ERROR)
        {
        sprintf(comment, "%s keyword found twice in ",
                keyword);
        warning(comment, tab->cat->filename);
        }
      }
    lptr += 80;
    }
}
/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldaccheck ***************************************************
 *
 *  NAME	
 *      ldaccheck - Check FITS image header consistancy
 *
 *  SYNOPSIS
 *      ldaccheck -i infits
 *
 *  FUNCTION
 *      This function checks for header consistancy in a series of
 *      FITS image files. The primary header is checked only!
 *
 *  INPUTS
 *      -i infits     - The name of the input FITS files
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
 *      E.R. Deul - dd 02-09-1997
 *
 ******************************************************************************/
  {
   char		infiles[MAXCHAR][MAXCHAR];
   int		ab,ae,a, ni, nc;
   int		i, j, k, l;
   catstruct	*incat;
   tabstruct	*tab, *t_tab;
   keystruct	*key, *t_key;
   int		qflag = 1, ntab;

  if (argc<3)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

  ab=ae=ni=nc=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++)
                   strcpy(infiles[a-ab],argv[a]);
                ae = a--;
                ni = ae - ab;
	        break;
      case 'q':	qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

  QFPRINTF(OUTPUT, "Reading FITS file(s)");
  incat = read_cats(argv+ab, ni);
  for (i=0, ntab = incat[0].ntab; i<ni; i++) {
     readfull_header(incat[i].tab);
     if (incat[i].ntab != ntab) {
        syserr(WARNING,"ldaccheck",
           "Number of tables of file %s differs from %s\n",
           incat[i].filename, incat[0].filename);
     }
  }
  for (i=0; i<ni; i++) {
     for (l=1,tab=incat[1].tab->nexttab;l<ntab;l++) {
        key = tab->key;
        if ((t_tab = name_to_tab(&incat[l], tab->extname, 0)) == NULL) {
           syserr(WARNING,"ldaccheck",
             "No table %s in file %s\n", tab->extname, incat[l].filename);
        } else {
           if (tab->nkey != t_tab->nkey)
                 syserr(WARNING,"ldaccheck",
                      "Table %s of file %s has %d keys, while %s has %d\n",
                      tab->extname,incat[i].filename, tab->nkey,
                      incat[l].filename,t_tab->nkey);
           for (j=0;j<tab->nkey;j++) {
              for (k=0;k<ni;k++) {
                 if ((t_key = name_to_key(t_tab, key->name)) == NULL)
                    syserr(WARNING,"ldaccheck",
                           "Key %s in file %s not in file %s\n",
                           key->name,  incat[i].filename, incat[k].filename);
                 if (t_key->ttype != key->ttype)
                    syserr(WARNING,"ldaccheck","Keys %s have different types\n",
                          key->name);
              }
              key = key->nextkey;
           }
        }
     }
  }
  return EXIT_SUCCESS;
}

