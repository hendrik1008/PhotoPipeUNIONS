 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACJoinKey
*
*	Author:		E.R.Deul , DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	15/01/96
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/* 26.05.2001:
  - new function copy_key_join that does a clean copy of the keys
    to the new table. The old version modified pointers in the
    ORIGINAL protocat table when copying the keys to the destination.
    This made the behaviour of the program dependent of the order in that
    the keys to copy were given (problems only were realised when trying
    to join several keys in one run)

  08.06.2003:
  - I reversed the order in that protocat and output cat
    are freed at the end of the program. The old order
    gave a core dump (NOT UNDERSTOOD; HAS TO BE REVISITED)

  - removed the freeing of catalogs completely for the moment.
    The "new" order of freeings led to a hangup in some cases

  29.01.2005:
  I added history information to the output catalog
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include        <string.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		SYNTAX \
"ldacjoinkey -i Catalog [-o Output_catalog] -p Proto_Catalog \n\
            [-t Table_name] -k Key_name [Key_name ...]\n\
            [options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n"

#define		MAXCHAR 256

catstruct	*incat, *outcat, *protocat;
int		qflag;

int	copy_key_join(tabstruct *, char *, tabstruct *, int);

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacjoin ***************************************************
 *
 *  NAME	
 *      ldacjoin - Add a column from the proto cat to the input cat/tab
 *
 *  SYNOPSIS
 *      ldacaddtab -i incat [-o outcat] -p protocat [-t table] [-q]
 *                 -k keyname1 keyname2 ...  [-q]
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
   tabstruct	*intab, *outtab, *prototab;
   char		outfilename[MAXCHAR], tablename[MAXCHAR];
   char		infilename[MAXCHAR],protfilename[MAXCHAR];
   int		a,ab,ae, ni;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  sprintf(outfilename, "default_join.cat");
  sprintf(tablename, "OBJECTS");

  ab=ae=ni=0;
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	sprintf(infilename, "%s", argv[++a]);
	        break;
      case 't':	sprintf(tablename, "%s", argv[++a]);
	        break;
      case 'k':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
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
  copy_tabs(incat, outcat);
  if (!(intab = name_to_tab(incat, tablename, 0)))
    error(EXIT_FAILURE, "*Error*: table not found in input catalog: ",
          tablename);
  if (!(outtab = name_to_tab(outcat, tablename, 0)))
    error(EXIT_FAILURE, "*Error*: table not found in output catalog: ",
          tablename);
  if (!(prototab = name_to_tab(protocat, tablename, 0)))
    error(EXIT_FAILURE, "*Error*: table not found in proto catalog: ",
          tablename);
  for (a=ab; a<ae; a++)
  {
     if((copy_key_join(prototab, argv[a], outtab, 0) == RETURN_ERROR))
     {
        error(EXIT_FAILURE, "*Error*:  error copying key: ",
          argv[a]);
     }

  }



  historyto_cat(outcat, "ldacjoinkey", argc, argv);
  QFPRINTF(OUTPUT, "Saving catalog");
  save_cat(outcat, outfilename);
  QFPRINTF(OUTPUT, "Cleaning memory");

  /*
  free_cat(incat, 1);
  free_cat(protocat, 1);
  free_cat(outcat, 1);
  */

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }


/****** copy_key_join ***********************************************************
PROTO	int copy_key_join(tabstruct *tabin, char *keyname, tabstruct *tabout, int pos)
PURPOSE	Copy a key from one table to another. The same as the copy_key
        function, only that the ptr element IS COPIED (The new table shares
        the SAME data as the old table. This implies that the new table
        has exactly the same number of objects as the old. The routine is
        primarily meant for the ldacjoinkey program.).
INPUT	Pointer to the original table,
        Name of the key,
	Pointer to the destination table,
	Position (1= first, <=0 = at the end)
OUTPUT	RETURN_OK if everything went as expected, and RETURN_ERROR otherwise.
NOTES	A preexisting key in the destination table yields a RETURN_ERROR,
AUTHOR	T. Erben (IAP)
VERSION	26.05.2001
 ***/
int	copy_key_join(tabstruct *tabin, char *keyname, tabstruct *tabout, int pos)

  {
   keystruct	*keyin, *keyout;

/*Convert the key name to a pointer*/
  if (!(keyin = read_key(tabin, keyname)))
    return RETURN_ERROR;

/*Check if a similar key doesn't already exist in the dest. cat */
  if (name_to_key(tabout, keyname))
    return RETURN_ERROR;

  tabout->nkey++;

/*First, allocate memory and copy data */
  QCALLOC(keyout, keystruct, 1);
  *keyout = *keyin;
  keyout->ptr = keyin->ptr;
  if (keyin->naxis)
    QMEMCPY(keyin->naxisn, keyout->naxisn, int, keyin->naxis);

/*Then, update the links */
  if ((keyout->nextkey = pos_to_key(tabout, pos)))
    {
    (keyout->prevkey = keyout->nextkey->prevkey)->nextkey = keyout;
    keyout->nextkey->prevkey = keyout;
/*--the first place has a special meaning*/
    if (pos==1)
      tabout->key = keyout;
    }
  else
/*There was no key before*/
    tabout->key = keyout->nextkey = keyout->prevkey = keyout;

  return RETURN_OK;
  }






