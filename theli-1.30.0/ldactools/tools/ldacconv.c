/*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACConv
*
*	Author:		E.BERTIN, DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	15/01/97
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
  05.05.03:
  included the tsize.h file

  29.01.2005:
  I added history information to the output catalog

*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<math.h>
#include        <string.h>

#include	"fitscat_defs.h"
#include	"fitscat.h"
#include	"tsize.h"
#include	"ldacconv.h"

#define		SYNTAX \
"ldacconv\n\
		-b camera_number\n\
		-c camera_name \n\
		-f filter_name \n\
		-i Catalog1 Catalog2 ... [-o Output_catalog] [options]\n\n\
Options are:	-q (Quiet flag: defaulted to verbose!)\n"

#ifndef         MAXCHAR
#define 	MAXCHAR 256
#endif
#define		SEQUENCE_NR	"SeqNr"

catstruct	*incat, *outcat;
int		qflag;

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldacconv ***************************************************
 *
 *  NAME	
 *      ldacconv - Convert list of object files to a catalog
 *
 *  SYNOPSIS
 *      ldacconv -i obj_files .. -o outcat -b band_number -c channel_name
 *
 *  FUNCTION
 *      This program converts the individual object files, as output by blend,
 *      into an LDAC pipeline catalog. The original FITS image headers that
 *      were preserved as an ASCII table are copied to a binary table 'FIELDS'.
 *      This table is simulteneously a hash table for field access as the
 *      objects from different fields are stored concatenated into an 'OBJECTS'
 *      binary table.
 *
 *  INPUTS
 *      A set of object files
 *
 *  RESULTS
 *      One object catalog
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
 *      E. Bertin - dd 20-06-1996
 *      E.R. Deul - dd 01-08-1996 Modified to adhere to LDAC pipeline concept
 *                                and added band_number/channel_name
 *      E. Bertin - dd 23-04-1997 Add column-name translation
 *      E.R. Deul - dd 26-04-1997 Make SeqNr unique in a set ALWAYS
 *
 ******************************************************************************/
  {
   keystruct	*key, *seqnr_key, *nkey, *okey;
   tabstruct	*tab, *tabout, *oldtab, *newtab;
   namemapstruct	*map;
   char		outfilename[MAXCHAR], channel_name[MAXCHAR],
		filter_name[MAXCHAR];
   short	*shptr;
   int		a,ab,ae, i,j,h, ni, nobj, pos, band, *lptr, k;

  if (argc<2)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  qflag = 1;

  sprintf(outfilename, "default_test.cat");

  ab=ae=ni=0;
  band = -1;
  strcpy(channel_name, "");
  strcpy(filter_name, "");
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	for(ab = ++a; (a<argc) && (argv[a][0]!='-'); a++);
		ae = a--;
		ni = ae - ab;
	        break;
      case 'o':	sprintf(outfilename, "%s", argv[++a]);
	        break;
      case 'q': qflag = 1;
	        break;
      case 'f': strcpy(filter_name,argv[++a]);
	        break;
      case 'b': band = atoi(argv[++a]);
	        break;
      case 'c': strcpy(channel_name,argv[++a]);
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni || band == -1 || channel_name[0] == '\0')
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

/*Load the input catalogs*/
  QFPRINTF(OUTPUT, "Reading catalog(s)");
  incat = read_cats(argv+ab, ni);
  QFPRINTF(OUTPUT, "Sticking catalog(s)");
  outcat = new_cat(1);
  inherit_cat(incat, outcat);
/*Now stick both image-headers and object data */
  h = 1;	/* the current head number */
  pos = 1;	/* the current ``first object'' */
  for (i=0; i<ni; i++)
    if ((tabout = asc2bin_tab(&incat[i], "LDAC_IMHEAD", outcat, "FIELDS"))
	!= NULL)
      {
/*Let's change the name of the object tables before sticking them */
      tab = name_to_tab(&incat[i], "LDAC_OBJECTS", 0);
      if (!tab)
        error(EXIT_FAILURE, "*Error*: cannot find the object table in ",
		incat[i].filename);
      strcpy(tab->extname, "OBJECTS");
      copy_tab(&incat[i], "OBJECTS", 0, outcat, 0);

/*----------------------------*/
/*--- First the OBJECTS table */
/*----------------------------*/
      tab = name_to_tab(outcat, "OBJECTS", h);
      if (!tab)
        error(EXIT_FAILURE, "*Error*: cannot retrieve the OBJECTS table in ",
		incat[i].filename);
/*---- Convert OBJECTS names if necessary */
      for (map=objmap; map->oldname[0]; map++)
        if ((key=name_to_key(tab, map->oldname)))
          {
          if (map->newname[0])
            strcpy(key->name, map->newname);
          else
            remove_key(tab, map->oldname);
          }

/*----Add a column to OBJECTS to link the object to a frame number*/
      key = new_key("FIELD_POS");
      strcpy(key->comment, "Reference number to field parameters");
      key->htype = H_INT;
      key->ttype = T_SHORT;
      key->nbytes = t_size[T_SHORT];
      nobj = key->nobj = tab->naxisn[1];
      QMALLOC(key->ptr, char, key->nbytes*key->nobj);
      shptr = key->ptr;
      for (j=0; j<key->nobj; j++)
        *(shptr++) = h;
      if (add_key(key, tab, 1)==RETURN_ERROR)
        warning("FIELD_POS field already present in ", incat[i].filename);

/*----Make sequence numbering unique in the set of frames---*/
/*----Use separate key to have it not accidentely overwritten somewhere else--*/
      if ((seqnr_key=name_to_key(tab, SEQUENCE_NR))) {
        nobj = seqnr_key->nobj = tab->naxisn[1];
        QMALLOC(seqnr_key->ptr, char, seqnr_key->nbytes*seqnr_key->nobj);
        lptr = seqnr_key->ptr;
        for (j=0; j<seqnr_key->nobj; j++)
          *(lptr++) = pos+j;
      } else {
        error(EXIT_FAILURE, "*Error*: No sequence number ",
                             "in the input catalog");
      }

/*-------------------------*/
/*--- Now the FIELDS table */
/*-------------------------*/
      tab = tabout;
      if (!tab)
        error(EXIT_FAILURE, "*Error*: cannot retrieve the FIELDS table in ",
		"the output catalog");
/*---- Convert key names if necessary */
      for (map=fieldmap; map->oldname[0]; map++)
        if ((key=name_to_key(tab, map->oldname)))
          {
          if (map->newname[0])
            strcpy(key->name, map->newname);
          else
            remove_key(tab, map->oldname);
          }
/*----Add 4 columns to FIELDS to link the frame to a set of objects*/
/*----I.: Object position */
      key = new_key("OBJECT_POS");
      strcpy(key->comment, "Position of the first object from that field");
      key->htype = H_INT;
      key->ttype = T_LONG;
      key->nbytes = t_size[T_LONG];
      key->nobj = tab->naxisn[1];
      QMALLOC(key->ptr, char, key->nbytes*key->nobj);
      lptr = key->ptr;
      for (j=0; j<key->nobj; j++)
        *(lptr++) = pos;
      if (add_key(key, tab, 1)==RETURN_ERROR)
        warning("OBJECT_POS field already present in ", incat[i].filename);

/*----II.: Number of objects */
      key = new_key("OBJECT_COUNT");
      strcpy(key->comment, "Number of objects in that field");
      key->htype = H_INT;
      key->ttype = T_LONG;
      key->nbytes = t_size[T_LONG];
      key->nobj = tab->naxisn[1];
      QMALLOC(key->ptr, char, key->nbytes*key->nobj);
      lptr = key->ptr;
      for (j=0; j<key->nobj; j++)
        *(lptr++) = nobj;
      if (add_key(key, tab, 2)==RETURN_ERROR)
        warning("OBJECT_COUNT field already present in ", incat[i].filename);

/*----III.: Channel Number */
      key = new_key("CHANNEL_NR");
      strcpy(key->comment, "Channel sequence number");
      key->htype = H_INT;
      key->ttype = T_LONG;
      key->nbytes = t_size[T_LONG];
      key->nobj = tab->naxisn[1];
      QMALLOC(key->ptr, char, key->nbytes*key->nobj);
      lptr = key->ptr;
      for (j=0; j<key->nobj; j++)
        *(lptr++) = band;
      if (add_key(key, tab, 3)==RETURN_ERROR)
        warning("CHANNEL_NR field already present in ", incat[i].filename);

/*----IV.: Channel Name*/
      key = new_key("CHANNEL_NAME");
      strcpy(key->comment, "Channel name");
      key->htype = H_STRING;
      key->ttype = T_STRING;
      key->nbytes = t_size[T_STRING];
      key->nobj = tab->naxisn[1];
/*----!!Temporary (?)  solution for STRINGS*/
      key->naxis = 1;
      QMALLOC(key->naxisn, int, 1);
      key->naxisn[0] = 16;
      key->nbytes *= key->naxisn[0];
      QMEMCPY(channel_name, key->ptr, char, key->naxisn[0]);
      if (add_key(key, tab, 3)==RETURN_ERROR)
        warning("CHANNEL_NAME field already present in ", incat[i].filename);

/*----V.: Filter Name*/
      key = new_key("FILTER_NAME");
      strcpy(key->comment, "Filter name");
      key->htype = H_STRING;
      key->ttype = T_STRING;
      key->nbytes = t_size[T_STRING];
      key->nobj = tab->naxisn[1];
/*----!!Temporary (?)  solution for STRINGS*/
      key->naxis = 1;
      QMALLOC(key->naxisn, int, 1);
      key->naxisn[0] = 16;
      key->nbytes *= key->naxisn[0];
      QMEMCPY(filter_name, key->ptr, char, key->naxisn[0]);
      if (add_key(key, tab, 3)==RETURN_ERROR)
        warning("FILTER_NAME field already present in ", incat[i].filename);
/*----Add this table to the outcat, but ---*/
/*----check the current table for consistancy with the existing tables---*/
      if (h == 1) {
         newtab = tabout;
      } else {
         newtab = new_tab("FIELDS");
         if ((oldtab=name_to_tab(outcat,"FIELDS",0)) != NULL) {
            key=oldtab->key;
            for (k=0;k<oldtab->nkey;k++) {
               nkey = new_key(key->name);
               nkey->htype = key->htype;
               nkey->ttype = key->ttype;
               nkey->nbytes = key->nbytes;
               nkey->nobj  = key->nobj;
               nkey->naxis = key->naxis;
               QCALLOC(nkey->ptr, char, nkey->nbytes*nkey->nobj);
               if (!(okey=name_to_key(tab, key->name))) {
                  warning("Adding missing FIELDS keyword ", nkey->name);
                  warning("From file: ", incat[i].filename);
               } else {
                  QMEMCPY(okey->ptr, nkey->ptr, char, okey->nbytes*okey->nobj);
               }
               if (add_key(nkey, newtab, 0)==RETURN_ERROR)
                  warning("Error adding FIELDS table keyword ", nkey->name);
               key = key->nextkey;
            }
/*
            key=tabout->key;
            nkeys = tab->nkey;
            for (k=0;k<nkeys;k++) {
               if (name_to_key(oldtab, key->name) == NULL) {
                  nkey=key->nextkey;
                  warning("Removing superfluous FIELDS keyword ", key->name);
                  warning("From file: ", incat[i].filename);
                  remove_key(tabout, key->name);
                  key = nkey;
               } else {
                  key = key->nextkey;
               }
            }
 */
         }
      }
      update_tab(newtab);
      if (add_tab(newtab, outcat, 0)==RETURN_ERROR)
        warning("Error adding FIELDS table ", incat[i].filename);
      h++;
      pos += nobj;
      }

  if (h==1)
  {
    warning("No suitable field header found: ", "no catalog created");
  }
  else
  {
      /* add HISTORY information to the ouput
         catalog */
      historyto_cat(outcat, "ldacconv", argc, argv);

      QFPRINTF(OUTPUT, "Saving catalog");
      save_cat(outcat, outfilename);
      QFPRINTF(OUTPUT, "Cleaning memory");
  }

  free_cat(incat, ni);
  free_cat(outcat, 1);

  QFPRINTF(OUTPUT, "All done");
  QPRINTF(OUTPUT,"\n");

  return EXIT_SUCCESS;
  }
