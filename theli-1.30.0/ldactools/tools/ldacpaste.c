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

/* 11.01.01:
   fixed a bug when not the objects table has to merged.
   (an integer i has to be initialised to zero in this case)

   04.01.05:
   removed compiler warnings (gcc with -O3 optimisation)

   29.01.2005:
   I added history information to the output catalog

   02.05.2005:
   - I fixed a bug if FIELDS tables contained different
     sets of keywords. The keyword set of the pasted
     catalog was not correct in this case.
   - I added version number information in the welcome
     message of the program.

   20.06.2007:
   I fixed a bug in the parsing of command line arguments.
   The program crashed when the maximum number of allowed
   input files was reached.

   27.07.2011:
   Bug fix in the treatment of OBJECTS tables. The query whether
   we want to pasteb the OBECTS table was wrong. The testing
   expression also returned tru if the table name contained
   'OBJECTS' as a substring.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	"fitscat.h"
#include	"fitscat_defs.h"
#include	"utils_globals.h"
#include	"tabutil_globals.h"
#include	"options_globals.h"
#include        "global_vers.h"

#ifndef MAXCHAR
#define MAXCHAR 256
#endif

#ifndef MAXFILES
#define MAXFILES 1024
#endif

#define         PROGRAM "Program to paste LDAC catalogs"
#define         VERS    "1.1"
#define         DATE    __DATE__

#define		SYNTAX \
"ldacpaste -i Input_catalogs [-o Output_catalog] [-t Table]\
[options]\n\n"

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacpaste ***************************************************
 *
 *  NAME	
 *      ldacpaste - Paste several catalogs together
 *
 *  SYNOPSIS
 *      ldacpaste -i incats [-o outcat]
 *
 *  FUNCTION
 *      This function pastes several catalogs together into one output
 *      catalog. It merges the FIELDS and OBJECT catalogs.
 *      It puts a unique identifier in the FIELDS table in order to mark
 *      which of the aditional tables in the catalog belong to this
 *      particular field. Added tables are e.g. PREASTROM and ASTROM tables
 *
 *  INPUTS
 *      -i incats     - The name of the input catalog files
 *     [-o outcat]    - The name of the output catalog file. The default
 *                      output catalog is default_filter.cat
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
 *      E.R. Deul - dd 07-07-1997
 *
 ******************************************************************************/
  {
   char		**tabnames, **keynames=NULL, **tkeynames;
   char		outfilename[MAXCHAR], infiles[MAXFILES][MAXCHAR];
   int		ab,ae,a, ni, nc, nlines=0, prev_count, prev_nr, nobj;
   int		i=0, j, jj, k, l, ntabs, *prev_fields;
   short int	*field_pos;
   int		*nobj_count=NULL, *nobj_pos=NULL, *seqnr, *sub_nr;
   catstruct	*incat, *outcat;
   tabstruct	*tab, *outtab;
   keystruct	*key, *key_p, *key_c;
   char		line[MAXCHAR], pastetable[MAXCHAR];
   int		qflag = 1, obj_first, nkeys, ntkeys;

  if (argc<6)
  {
    fprintf(stdout, "\n%s; Version: %s (Compile Time: %s)\n", PROGRAM, VERS, DATE);
    fprintf(stdout, "(%s)\n", __PIPEVERS__);
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
  }

/*default parameters */
  sprintf(outfilename, "default_filter.cat");
  strcpy(pastetable, "OBJECTS");

  ab=ae=ni=nc=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++)
	        {
		  if((a-ab) == MAXFILES)
		  {
		    sprintf(line, "Maximum allowed number is: %d", MAXFILES);
		    error(EXIT_FAILURE,"Too many input files.", line);
		  }
		  else
		  {
                    strncpy(infiles[a-ab],argv[a], MAXCHAR-1);
		  }
                }
                ae = a--;
                ni = ae - ab;
	        break;
      case 't': strcpy(pastetable, argv[++a]);
                break;
      case 'o':	sprintf(outfilename, "%s", argv[++a]);
	        break;
      case 'q':	qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

  QFPRINTF(OUTPUT, "Reading catalog(s)");
  incat = read_cats(argv+ab, ni);
  outcat = new_cat(1);
  init_cat(outcat);
  inherit_cat(&incat[0], outcat);
  historyto_cat(outcat, "ldacpaste", argc, argv);
  ED_CALLOC(prev_fields, "ldacpaste", int, (ni+1));
  prev_count = 0;
  prev_nr = 0;

  if (strcmp(pastetable,"OBJECTS") == 0) {
    for (i=0; i<ni; i++) {
/*
 *     Get the list of keys from the catalogs and derive the common denominator.
 */
       if (i == 0) {
          if (!(tab = name_to_tab(&incat[i], "FIELDS", 0))) {
             syserr(FATAL, "ldacpaste", "No table FIELDS in input catalog\n");
          }
          keynames = keys_list(tab, &nkeys);
       } else {
          tab = name_to_tab(&incat[i], "FIELDS", 0);
          tkeynames = keys_list(tab, &ntkeys);
          for (j=0;j<nkeys;j++)
          {
             for (k=0;k<ntkeys;k++)
             {
	        /* the second condition is true if the keyword
                   has been removed already */
                if ((strcmp(keynames[j],tkeynames[k]) == 0) ||
                    keynames[j][0] == '\0') goto isthere;
             }
             sprintf(line,"Removing key: %s",keynames[j]);
	     QFPRINTF(OUTPUT, line);
             keynames[j][0] = '\0';
isthere:     continue;
          }
          for (k=0;k<ntkeys;k++) {
             QFREE(tkeynames[k]);
          }
          QFREE(tkeynames);
       }
    }
    ntkeys = 0;
    for (j=0;j<nkeys;j++) {
       if (keynames[ntkeys][0] != '\0') {
          ntkeys++;
       } else {
	 /* move all keys 1 up in case we have to skip one */
	 for(l=ntkeys; l < nkeys-1; l++)
	 {
            strcpy(keynames[l],keynames[l+1]);
	 }
       }
    }
    nkeys = ntkeys;

/*
 *   The actual merging
 */
    for (i=0; i<ni; i++) {
       tabnames = tabs_list(&incat[i], &ntabs);
       obj_first = 0;
       for (k=1;k<ntabs;k++) {
          if (strstr(tabnames[k], "FIELDS") != NULL && obj_first != 0) {
/*
 *           Go swap the tables in the tabnames array
 */
             strcpy(tabnames[k], "OBJECTS");
             strcpy(tabnames[obj_first], "FIELDS");
          }
          if (strstr(tabnames[k], "OBJECTS") != NULL) obj_first = k;
       }
       for (k=1;k<ntabs;k++) {
          sprintf(line,"Copying table %s of catalog %d",tabnames[k],i);
          QFPRINTF(OUTPUT, line);
          if (strcmp(tabnames[k], "FIELDS") == 0) {
/*
 *           For FIELD tables, read the OBJECT_POS and OBJECT_COUNT columns,
 *           update the OBJECT_POS column, and add a SubNr column to identify
 *           to which SubSet this section belongs
 */
             tab = name_to_tab(&incat[i], tabnames[k], 0);
             ED_GETKEY(tab, key_p, "OBJECT_POS",   nobj_pos,
                       nlines, "ldacpaste");
             ED_GETKEY(tab, key_c, "OBJECT_COUNT", nobj_count,
                       nlines, "ldacpaste");
             ED_MALLOC(sub_nr, "ldacpaste", int, nlines);
             prev_fields[i+1] = nlines + prev_fields[i];
             for (j=0;j<nlines;j++) {
                nobj_pos[j] += prev_count;
                sub_nr[j] = i+1;
             }
             outtab = new_tab("FIELDS");
/*
 *           Now here copy only those keys that are common to all the catalogs
 */
             for (jj=0;jj<nkeys;jj++) {
                copy_key(tab, keynames[jj], outtab, 0);
             }
             key = name_to_key(outtab, "OBJECT_POS");
             key->ptr = key_p->ptr;
             key = name_to_key(outtab, "OBJECT_COUNT");
             key->ptr = key_c->ptr;
             prev_count = nobj_pos[nlines-1] - 1 + nobj_count[nlines-1];
             if ((key = name_to_key(outtab, "SubNr")) == NULL) {
                key = add_key_to_tab(outtab, "SubNr", nlines, sub_nr, T_LONG, 1,
                  "","Subset Number");
                key->tab = outtab;
             } else {
                key->ptr = sub_nr;
             }
             update_tab(outtab);
             if (add_tab(outtab, outcat, 0) == RETURN_ERROR) {
                syserr(FATAL, "ldacpaste", "Error in pasting FIELDS of %s\n",
                infiles[i]);
             }
          } else {
/*
 *           Read the SeqNr column and update it
 */
             if (strcmp(tabnames[k], "OBJECTS") == 0) {
                tab = name_to_tab(&incat[i], tabnames[k], 0);
                outtab = new_tab("OBJECTS");
                key = tab->key;
                for (jj=0;jj<tab->nkey;jj++) {
                   copy_key(tab, key->name, outtab, 0);
                   key = key->nextkey;
                }
		nobj = key->nobj;
                if (nobj > 0) {
                  ED_GETKEY(tab, key, "FIELD_POS", field_pos,
                            nobj, "ldacpaste");
                  field_pos = key->ptr;
                  for (j = 0; j < nobj; j++) {
                      field_pos[j] += prev_fields[i];
                  }
                  key = name_to_key(outtab, "FIELD_POS");
                  key->ptr = field_pos;
                  key = name_to_key(outtab, "SeqNr");
                  ED_MALLOC(seqnr, "ldacpaste", int, nobj);
                  for (j=0;j<nobj;j++) {
                      seqnr[j] = j + prev_nr + 1;
                  }
                  key->ptr = seqnr;
                  prev_count = nobj_pos[nlines-1] -1 + nobj_count[nlines-1];
                  prev_nr = seqnr[nobj-1];
                }
                update_tab(outtab);
                if (add_tab(outtab, outcat, 0) == RETURN_ERROR) {
                    syserr(FATAL, "ldacpaste",
                                  "Error in pasting OBJECTS of %s\n",
                                  infiles[i]);
                }
             } else {
                if (strcmp(tabnames[k], "DISTORTIONS") == 0) {
                   tab = name_to_tab(&incat[i], tabnames[k], 0);
                   outtab = new_tab("DISTORTIONS");
                   key = tab->key;
                   for (jj=0;jj<tab->nkey;jj++) {
                      copy_key(tab, key->name, outtab, 0);
                      key = key->nextkey;
                   }
                   ED_GETKEY(tab, key, "FIELD_POS", field_pos,
                             nobj, "ldacpaste");
                   field_pos = key->ptr;
                   for (j=0;j<nobj;j++) {
                       field_pos[j] += prev_fields[i];
                   }
                   key = name_to_key(outtab, "FIELD_POS");
                   key->ptr = field_pos;
                   update_tab(outtab);
                   if (add_tab(outtab, outcat, 0) == RETURN_ERROR) {
                       syserr(FATAL, "ldacpaste",
                              "Error in pasting DISTORTIONS of %s\n",
                      infiles[i]);
                   }
                } else {
/*
 *                 Just append a SubSet number to each table name
 */
                   tab = name_to_tab(&incat[i], tabnames[k], 0);
                   sprintf(tab->extname, "%s%s", tab->extname,
                           leading_zeros(i+1,4));
                   copy_tab(&incat[i], tab->extname, 0, outcat, 0);
                }
             }
          }
       }
    }
  }
  else
  {
    if (name_to_tab(&incat[i], "FIELDS", 0) ||
        name_to_tab(&incat[i], "OBJECTS", 0))
    {
       syserr(FATAL, "ldacpaste",
              "Cannot paste catalogs with FIELD or OBJECTS tables in them\n");
    }

    for (i=0; i<ni; i++) {
       tabnames = tabs_list(&incat[i], &ntabs);
       for (k=1;k<ntabs;k++) {
         if (strcmp(tabnames[k], pastetable) == 0) {
           copy_tab(&incat[i], tabnames[k], 0, outcat, 0);
         } else {
/*
 *         Just append a SubSet number to each table name
 */
           tab = name_to_tab(&incat[i], tabnames[k], 0);
           sprintf(tab->extname,"%s%s",tab->extname,leading_zeros(i+1,4));
           copy_tab(&incat[i], tab->extname, 0, outcat, 0);
         }
      }
    }
  }
  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, outfilename);

  return EXIT_SUCCESS;
  }

