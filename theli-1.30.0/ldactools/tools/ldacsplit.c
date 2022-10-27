/*
   21.06.01:
   The CALLOC commands for okey->naxisn are now only
   executed if they have more than 0 elements.

   19.07.2015:
   - The definition variable MAXCHAR was renamed to MAXCHARS
     to be consistent with other LDAC header files.
   - I added a new command line argument (-d) to specify the
     output directory for created files.
*/


/*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACSplit
*
*	Author:		E.R.Deul, DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	06/11/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"


#define		SYNTAX \
"ldacsplit -i Catalog [-e image_file_extension] \
[options]\n\n\
Options are:	-d output directory (defaulted to present work directory)\n\
                -q (Quiet flag: defaulted to verbose!)\n"

#ifndef         MAXCHARS
#define		MAXCHARS 256
#endif

catstruct	*incat, *outcat, *protocat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacsplit ***************************************************
 *
 *  NAME	
 *      ldacsplit - Split the OBJECTS table into individual catalogs
 *
 *  SYNOPSIS
 *      ldacsplit -i incat  [-q]
 *
 *  FUNCTION
 *      This program splits the original fields in the FIELDS/OBJECTS table
 *      structure to individual output catalogs.
 *
 *  INPUTS
 *      -i incat     - The name of the input catalog file
 *
 *  RESULTS
 *
 *  RETURNS
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      All other tables except the FIELDS and OBJECTS tables are removed
 *      from the output catalog.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 05-11-1997
 *
 ******************************************************************************/
  {
   tabstruct	*fieldstab, *objectstab;
   tabstruct	*iobjectstab, *oobjectstab, *ifieldstab, *ofieldstab;
   keystruct	*key_cnt, *key_pos, *okey_fld, *okey_pos, *key, *okey,
                *filename_key;
   char		*t, tfile[MAXCHARS];
   char		infilename[MAXCHARS], outline[MAXCHARS], *filenames;
   char		origext[MAXCHARS], outdir[MAXCHARS];
   int		a, i, j, h, k;
   int		*obj_cnt, *obj_pos;

  if (argc<2)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;
  strcpy(origext, ".fits");
  strcpy(outdir, "./");

  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	snprintf(infilename, MAXCHARS - 1, "%s", argv[++a]);
	        break;
      case 'e':	snprintf(origext, MAXCHARS - 1, "%s", argv[++a]);
	        break;
      case 'd':	snprintf(outdir, MAXCHARS - 1, "%s", argv[++a]);
	        break;
      case 'q': qflag = 1;
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

/*Load the input catalog*/
    QFPRINTF(OUTPUT, "Reading catalog");
    if (!(incat = read_cat(infilename)))
      error(EXIT_FAILURE, "*Error*: catalog not found: ", infilename);
/*
 *  Get pointers to the input FIELDS and OBJECTS table
 */
    if (!(fieldstab = name_to_tab(incat, "FIELDS", 0)))
      error(EXIT_FAILURE, "*Error*: table not found: ", "FIELDS");
    if (!(objectstab = name_to_tab(incat, "OBJECTS", 0)))
      error(EXIT_FAILURE, "*Error*: table not found: ", "OBJECTS");
    ifieldstab  = init_readobj(fieldstab);
    iobjectstab = init_readobj(objectstab);
/*
 *  Get pointers to the input OBJECT_COUNT key
 */
    if (!(key_cnt = read_key(fieldstab, "OBJECT_COUNT")))
      error(EXIT_FAILURE, "*Error*: key not found: ", "OBJECT_COUNT");
    obj_cnt = key_cnt->ptr;
    if (!(key_pos = read_key(fieldstab, "OBJECT_POS")))
      error(EXIT_FAILURE, "*Error*: key not found: ", "OBJECT_POS");
    obj_pos = key_pos->ptr;
    if (!(filename_key = read_key(fieldstab,  "FITSFILE")))
      error(EXIT_FAILURE, "*Error*: key not found: ", "FITSFILE");
    filenames = filename_key->ptr;
/*
 *  Loop over the number of FIELDS
 */
    for (i=0;i<key_cnt->nobj;i++) {
/*
 *     Start a new catalog
 */
       outcat = new_cat(1);
       init_cat(outcat);
       strncpy(tfile, &filenames[i*filename_key->nbytes],filename_key->nbytes);
/*
 *     Remove trailing spaces and .fits extension
 */
       for (j=filename_key->nbytes;j>=0;j--)
          if (tfile[j] == ' ') tfile[j] = '\0';
       if (origext[0] != '\0') {
          if (!(t=strstr(tfile, origext)))
             error(EXIT_FAILURE, "No original filename extension ", origext);
          t[0] = '\0';
       }
       snprintf(outcat->filename, MAXCHARS - 1, "%s/%s.cat", outdir, tfile);
       if (open_cat(outcat, WRITE_ONLY) != RETURN_OK)
           error(EXIT_FAILURE,"*Error*: cannot open for writing ",
                outcat->filename);
       snprintf(outline, MAXCHARS - 1, "Writing file %s with %d objects",
               outcat->filename, obj_cnt[i]);
       QFPRINTF(OUTPUT, outline);
       save_tab(outcat, outcat->tab);
/*
 *     Put a FIELDS table in the new catalog, copy structure from the input
 *     Initialize for writing
 */
       ofieldstab = new_tab("FIELDS");
       key = ifieldstab->key;
       for (j=ifieldstab->nkey; j--;) {
          okey = new_key(key->name);
          strcpy(okey->comment, key->comment);
          okey->htype = key->htype;
          okey->ttype = key->ttype;
          strcpy(okey->printf, key->printf);
          strcpy(okey->unit, key->unit);
          okey->naxis = key->naxis;
          if(okey->naxis > 0)
          {
            QCALLOC(okey->naxisn, int, okey->naxis);
          }
          for (k=0;k<okey->naxis;k++) okey->naxisn[k] = key->naxisn[k];
          okey->nobj = 1;
          okey->nbytes = key->nbytes;
          QCALLOC(okey->ptr, char, key->nbytes);
          add_key(okey, ofieldstab, 0);
          key = key->nextkey;
       }
       add_tab(ofieldstab, outcat, 0);
       init_writeobj(outcat, ofieldstab);
       ofieldstab->cat = outcat;
/*
 *     Get the FIELDS information and update the OBJECT_POS value
 */
       read_obj_at(ifieldstab, fieldstab, i);
       key = ifieldstab->key;
       for (j=ifieldstab->nkey; j--;) {
          okey = name_to_key(ofieldstab, key->name);
          memcpy(okey->ptr, key->ptr, sizeof(char)*key->nbytes);
          key = key->nextkey;
       }
       if (!(okey_pos = name_to_key(ofieldstab, "OBJECT_POS")))
         error(EXIT_FAILURE, "*Error*: key not found: ", "OBJECT_POS");
       *(int *)okey_pos->ptr = 1;
/*
 *     Write the info and save the table
 */
       write_obj(ofieldstab);
       end_writeobj(outcat, ofieldstab);
/*
 *     Put an OBJECTS table in the new catalog, copy structure from the input
 *     Initialize for writing
 */
       oobjectstab = new_tab("OBJECTS");
       key = iobjectstab->key;
       for (h=iobjectstab->nkey; h--;) {
          okey = new_key(key->name);
          strcpy(okey->comment, key->comment);
          okey->htype = key->htype;
          okey->ttype = key->ttype;
          strcpy(okey->printf, key->printf);
          strcpy(okey->unit, key->unit);
          okey->naxis = key->naxis;
          if(okey->naxis > 0)
          {
            QCALLOC(okey->naxisn, int, okey->naxis);
          }
          for (k=0;k<okey->naxis;k++) okey->naxisn[k] = key->naxisn[k];
          okey->nobj = 1;
          okey->nbytes = key->nbytes;
          QCALLOC(okey->ptr, char, key->nbytes);
          add_key(okey, oobjectstab, 0);
          key = key->nextkey;
       }
       init_writeobj(outcat, oobjectstab);
       oobjectstab->cat = outcat;
/*
 *     Get the OBJECTS information and update the FIELD_POS value
 */
       for (j=0;j<obj_cnt[i];j++) {
          read_obj_at(iobjectstab, objectstab, obj_pos[i]-1+j);
          key = iobjectstab->key;
          for (h=iobjectstab->nkey; h--;) {
             okey = name_to_key(oobjectstab, key->name);
             memcpy(okey->ptr, key->ptr, sizeof(char)*key->nbytes);
             key = key->nextkey;
          }
          if (!(okey_fld = name_to_key(oobjectstab, "FIELD_POS")))
            error(EXIT_FAILURE, "*Error*: key not found: ", "OBJECT_POS");
          *(short int *)okey_fld->ptr = 1;
/*
 *        Write the info
 */
          write_obj(oobjectstab);
       }
       end_writeobj(outcat, oobjectstab);
       close_cat(outcat);
    }

    QFPRINTF(OUTPUT, "Cleaning memory");

    free_cat(incat, 1);

    QFPRINTF(OUTPUT, "All done");
    QPRINTF(OUTPUT,"\n");

    return EXIT_SUCCESS;
  }

