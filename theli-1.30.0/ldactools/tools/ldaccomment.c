 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACComment
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
 * 04.01.05: removed compiler warnings (gcc with -O3 optimisation)
 *
 * 29.01.2005:
 * I added history information to the output catalog
 *
 * 20.07.2015:
 * - I removed compiler warnings
 * - I now use the macro MAXCHARS instead of MAXCHAR to define maximum
 *   string length
 * - I made some sprintf statements more robust by substitution with
 *   snprintf
 * - I removed superfluous spaces at the end of lines.
 */

#include	<stdio.h>
#include	<time.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"
#include	"utils_globals.h"
#include        "tabutil_globals.h"

#define		SYNTAX \
"ldaccomment -i Catalog [-o Output_catalog] \n\
            -c comment [options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n"

#define		COMLEN  80

int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldaccomment ***************************************************
 *
 *  NAME	
 *      ldaccomment - Add a comment the input cat's COMMENT table
 *
 *  SYNOPSIS
 *      ldaccomment -i incat [-o outcat] [-q]
 *
 *  FUNCTION
 *      This program allows one to add comments to the COMMENT table
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *      -o outcat    - The name of the output catalog file. The default name
 *                     is default_renkey.cat
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
 *      E.R. Deul - dd 03-02-2000
 *
 ******************************************************************************/
  {
   static time_t        thetime;
   char		outfilename[MAXCHARS], dtstring[20];
   char		infilename[MAXCHARS],commfilename[MAXCHARS];
   char		**tabnames, *timestamp, *textstring, *c;
   keystruct	*key, *okey;
   tabstruct	*intab, *comtab, *in_objtab, *out_objtab;
   catstruct	*incat, *outcat;
   int		a,ni=0,i,j,k,ntabs,remaining;
   FILE		*fcomm;

  if (argc<2)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(outfilename, "default_commenttab.cat");
  strcpy(commfilename, "");
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	snprintf(infilename, MAXCHARS - 1, "%s", argv[++a]);
                ni = 1;
	        break;
      case 'o':	snprintf(outfilename, MAXCHARS - 1, "%s", argv[++a]);
	        break;
      case 'c':	snprintf(commfilename, MAXCHARS - 1, "%s", argv[++a]);
	        break;
      case 'q': qflag = 1;
	        break;
      default : QFPRINTF(OUTPUT, argv[a-1]);
                error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }
  if (ni == 0)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*Load the input catalog*/
  QFPRINTF(OUTPUT, "Reading catalog");
  if (!(incat = read_cat(infilename)))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", infilename);
  if (time(&thetime)==-1)
    warning("No time available for history","");
  if (!strftime(dtstring, 18, "%d/%m/%Y %H:%M", localtime(&thetime)))
    error(EXIT_FAILURE, "*Internal Error*: Time/date string too long in ",
        "ldaccomment");
  outcat = new_cat(1);
  inherit_cat(incat, outcat);
  historyto_cat(outcat,"ldaccomment", argc, argv);
  strcpy(outcat->filename, outfilename);
  if (open_cat(outcat, WRITE_ONLY) != RETURN_OK)
      syserr(FATAL, "ldaccomment","Cannot open output catalog %s\n",
             outfilename);
  if (commfilename[0] != '\0') {
     if (!(fcomm = fopen(commfilename, "r")))
       syserr(FATAL, "ldaccomment","Cannot open comment file %s\n",
             commfilename);
  } else {
     fcomm = stdin;
  }

/*
 *  First write the Primary FITS header unit
 */
    save_tab(outcat, outcat->tab);
    tabnames = tabs_list(incat, &ntabs);
/*
 *  Then copy and save all unaltered tables
 */
    for (k=1;k<ntabs;k++) {
       if (strcmp(tabnames[k], "COMMENT") != 0) {
          copy_tab(incat, tabnames[k], 0, outcat, 0);
          save_tab(outcat, name_to_tab(outcat, tabnames[k], 0));
       }
    }
    if (!(comtab = name_to_tab(incat, "COMMENT", 0))) {
       comtab =  new_tab("COMMENT");
       QCALLOC(timestamp, char, 18*100);
       QCALLOC(textstring, char, 80*100);
       i=0;
       while (fgets(&textstring[i*80], 80, fcomm)) {
         c = strchr(&textstring[i*80],'\n');
         *c = '\0';
         memcpy(&timestamp[i*18], dtstring, 18);
         if(++i>=100)
           error(EXIT_FAILURE, "*Error*: Comment too long",
                 &textstring[--i*80]);
       }
       add_key_to_tab(comtab, "TIMESTAMP", i, timestamp, T_STRING, 18, "",
                      "Time stamp of comment");
       add_key_to_tab(comtab, "TEXT", i, textstring, T_STRING, COMLEN, "",
                      "Comment string");
       if (add_tab(comtab, outcat, 0) == RETURN_ERROR)
          syserr(FATAL, "ldacpaste", "Error in adding COMMENT table\n");
       save_tab(outcat, comtab);
    } else {
      intab = name_to_tab(incat, "COMMENT", 0);
      in_objtab = init_readobj(intab);
      out_objtab =  new_tab("COMMENT");
      key = in_objtab->key;
      for (j=0;j<in_objtab->nkey;j++) {
         copy_key(in_objtab, key->name, out_objtab, 0);
         okey = name_to_key(out_objtab, key->name);
         okey->nobj   = 1;
         ED_CALLOC(okey->ptr, "ldaccomment", char, (key->nbytes));
         key = key->nextkey;
      }
      if (add_tab(out_objtab, outcat, 0) == RETURN_ERROR)
          syserr(FATAL, "ldacpaste", "Error in adding COMMENT table\n");
      out_objtab->cat = outcat;
      init_writeobj(outcat, out_objtab);
      do {
        remaining = read_obj(in_objtab, intab);
        key = in_objtab->key;
        for (j=0;j<in_objtab->nkey;j++) {
           copy_key(in_objtab, key->name, out_objtab, 0);
           okey = name_to_key(out_objtab, key->name);
           memcpy(okey->ptr, key->ptr, key->nbytes);
           key = key->nextkey;
        }
        write_obj(out_objtab);
      } while (remaining);
      i=0;
      QMALLOC(textstring, char, 80);
      while (fgets(textstring, 80, fcomm)) {
         c = strchr(textstring,'\n');
         *c = '\0';
         key = name_to_key(out_objtab, "TIMESTAMP");
         strncpy(key->ptr, dtstring, 18);
         key = name_to_key(out_objtab, "TEXT");
         strncpy(key->ptr, textstring, 80);
         write_obj(out_objtab);
      }
      end_writeobj(out_objtab->cat, out_objtab);
    }
  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, outfilename);
  QFPRINTF(OUTPUT, "Cleaning memory");

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }

