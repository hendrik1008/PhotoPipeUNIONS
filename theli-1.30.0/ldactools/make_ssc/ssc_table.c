#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fitscat.h"
#include "tsize.h"
#include "fitscat_defs.h"
#include "utils_globals.h"
#include "utils_macros.h"
#include "tabutil_globals.h"
#include "options_globals.h"
#include "ssc_globals.h"
#include "ssc_table_globals.h"
#include "field_desc.h"

/* 21.02.01:
   substituted all long int by the LONG macro

   05.05.03:
   included the tsize.h file

   10.01.05:
   I removed compiler warnings under gcc with optimisation
   enabled (-O3).

   29.01.2005:
   - I add now HISTORY information to the output catalogs
     (probably has to be revisited as we generate a new
      catalogs out of preexisting ones. Hence, we should
      take info from the old catalogs as well)

   - I adapted the arguments of open_ssc_cat
     for the inclusion of HISTORY information to the output catalogs

   10.03.2005:
   function ssc_put_row:
   I introduced MIN and MAX possibilities in the channel merging.

   01.05.2006:
   I removed the inclusion of precess_defs.h and inserted
   the missing definition of DEG2RAD directly into the code

   25.09.2009:
   I comment out some code for a merging mode 'FIELD_POS' which
   I (TE) do not understand and which I never used. It avoids
   errors if FIELDS tables do not contain astrometric quantities
   (not needed if we merge by pixel coordinates).

   02.02.2010:
   For the change of 25.09.2009, I forgot to comment some code part -
   fixed.
*/

/* definitions */
#ifndef DEG2RAD
#define DEG2RAD 0.017453292         /* degree/radian conversion */
#endif

/*
 * Local defined and used variables
 */
static keystruct	*ra_key, *dec_key;
static keystruct	**col_keys;
static int		BACK_JUMP;
static catstruct	*ssc_cat;
static int		*chan_nr;
static int		ncols;

/* COMMENTED CODE FOR MODE FIELD_POS
static double		**ra_min, **ra_max, **dec_min, **dec_max;
static field_params	***field_p;
static int		*nfields;
*/

/*
 * Local defined and exported variables
 */
keystruct		**best_key;
LONG		*last_best_fpos;

/*
 * External define and used variables
 */
extern control_struct	control;

static tabstruct *create_ssc_table(tabstruct **intab, tabstruct **fintab)
{
/****i* create_ssc_table ***************************************************
 *
 *  NAME	
 *      create_ssc_table - Create the table structure for the SSC catalog
 *
 *  SYNOPSIS
 *      static tabstruct *create_ssc_table(tabstruct **intab,
 *                                         tabstruct **fintab)
 *
 *  FUNCTION
 *      This function creates the table structure required for storing
 *      the internal information (from the global parameters) into a
 *      catalog on disk.
 *
 *  INPUTS
 *      intab    - Pointer to an input OBJECTS table
 *      fintab   - Pointer to an input FIELDS table
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      The table structure for the ssc table is created.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - 23-12-1996
 *
 ******************************************************************************/
    int 	i, j, *ch_nr, equal_chan;
    keystruct	*inkey = NULL, *key;
    tabstruct	*tabout;
    int		found;
    char	*s;

    if (control.make_pairs == YES)
       tabout = new_tab("PAIRS");
    else
       tabout = new_tab("PSSC");

    ED_CALLOC(chan_nr, "create_ssc_table", int,         control.ilist);
    for (i=0;i<ncols;i++) {
/*
 *     Make the keys for the output table based on the info from the input key
 */
       key = new_key(col_name[i]);
       for (j=0,found=FALSE;j<control.ilist;j++) {
          if ((inkey = name_to_key(intab[j], col_input[i]))) {
             found = TRUE;
             break;
          }
       }
       if (!found)
          syserr(FATAL,"create_ssc_table",
                       "No key: %s for %s in the input catalog(s)\n",
                        col_input[i], col_name[i]);
       *key = *inkey;
       strcpy(key->name, col_name[i]);
       strcpy(key->comment,inkey->comment);
       strcpy(key->printf,inkey->printf);
       strcpy(key->unit,inkey->unit);
       key->nextkey = key->prevkey = NULL;
       if (control.make_pairs == YES) {
/*
 *        For pairs output we will create a vector of the two elements.
 */
          key->naxis  += 1;
          ED_MALLOC(key->naxisn, "create_ssc_table", int, key->naxis);
          for (j=1;j<key->naxis;j++) key->naxisn[j] = inkey->naxisn[j-1];
          key->naxisn[0] = 2;
       } else {
/*
 *        In the other case just copy the remaining key information
 */
          if (key->naxis != 0) {
             ED_MALLOC(key->naxisn, "create_ssc_table", int, key->naxis);
             for (j=0;j<key->naxis;j++) key->naxisn[j] = inkey->naxisn[j];
          }
       }
       key->nobj   = 1;
       key->nbytes = t_size[key->ttype];
       for (j=0;j<key->naxis;j++)
          key->nbytes *= key->naxisn[j];
       ED_MALLOC(key->ptr, "create_ssc_table", char, key->nbytes);
       if (add_key(key, tabout, 0)==RETURN_ERROR)
          syserr(FATAL, "create_ssc_table",
                        "Error adding key %s to ssc_table\n",
              key->name);
    }
    for (i=0;i<control.ilist;i++) {
       if (fintab[i] == NULL) {
          chan_nr[i] = i;
       } else {
          if (name_to_key(fintab[i], "CHANNEL_NR")) {
             ED_GETKEY(fintab[i], key,"CHANNEL_NR",ch_nr,j,"create_ssc_tab");
             chan_nr[i] = ch_nr[0];
          } else {
             chan_nr[i] = i;
          }
       }
    }
    equal_chan = FALSE;
    for (i=0;i<control.ilist;i++)
       for (j=i+1;j<control.ilist;j++)
          if (chan_nr[i] == chan_nr[j]) equal_chan = TRUE;
    if (equal_chan)
       for (i=0;i<control.ilist;i++)
          chan_nr[i] = i;
    if (control.make_pairs == YES) {
       if (control.ilist > 1) {
/*
 *       For the pairs output make sure there is a channel column if there are
 *       more than one channels as input. The user must make sure he
 *       specified a FIELD_POS for output.
 */
         if (name_to_key(tabout, "CHANNEL_NR"))
            syserr(FATAL, "create_ssc_table",
              "The %s is a propriatory key. Do not specify it as output key\n",
                 "CHANNEL_NR");
         key = new_key("CHANNEL_NR");
         if (fintab[0] == NULL) {
            key->ttype = T_LONG;
            ED_MALLOC(key->naxisn, "create_ssc_table", int, key->naxis);
         } else {
            inkey = name_to_key(fintab[0], "CHANNEL_NR");
            key->htype = inkey->htype;
            key->ttype = inkey->ttype;
            key->naxis  = inkey->naxis;
            key->naxis  += 1;
            ED_MALLOC(key->naxisn, "create_ssc_table", int, key->naxis);
            for (j=1;j<key->naxis;j++) key->naxisn[j] = inkey->naxisn[j-1];
            key->naxisn[0] = 2;
         }
         key->nbytes = t_size[key->ttype];
         key->nobj   = 1;
         for (i=0;i<key->naxis;i++)
            key->nbytes *= key->naxisn[i];
         ED_MALLOC(key->ptr, "create_ssc_table", char, key->nbytes);
         if (add_key(key, tabout, 0)==RETURN_ERROR)
           syserr(FATAL, "create_ssc_table", "Error adding key to ssc_table\n");
         ED_REALLOC(col_name,       "create_ssc_table", char *, (ncols+1));
         ED_CALLOC(col_name[ncols], "create_ssc_table", char  , MAXCHAR);
         ED_REALLOC(col_input,      "create_ssc_table", char *, (ncols+1));
         ED_CALLOC(col_input[ncols],"create_ssc_table", char  , MAXCHAR);
         ED_REALLOC(col_merge,      "create_ssc_table", char *, (ncols+1));
         ED_CALLOC(col_merge[ncols],"create_ssc_table", char  , MAXCHAR);
         ED_REALLOC(col_chan,       "create_ssc_table", char *, (ncols+1));
         ED_CALLOC(col_chan[ncols], "create_ssc_table", char  , MAXCHAR);
         sprintf(col_name[ncols], "CHANNEL_NR");
         sprintf(col_input[ncols], "CHANNEL_NR");
         sprintf(col_merge[ncols], "AVERAGE");
         sprintf(col_chan[ncols], "ALL");
       }
    }
    if (control.make_pairs == NO) {
/*
 *     Add counting abs sequence number columns to the output
 */
       BACK_JUMP = 1;
       for (i=0;i<control.ilist;i++) {
          ED_REALLOC(col_name,       "create_ssc_table", char *, (ncols+1));
          ED_CALLOC(col_name[ncols], "create_ssc_table", char  , MAXCHAR);
          ED_REALLOC(col_input,      "create_ssc_table", char *, (ncols+1));
          ED_CALLOC(col_input[ncols],"create_ssc_table", char  , MAXCHAR);
          ED_REALLOC(col_merge,      "create_ssc_table", char *, (ncols+1));
          ED_CALLOC(col_merge[ncols],"create_ssc_table", char  , MAXCHAR);
          ED_REALLOC(col_chan,       "create_ssc_table", char *, (ncols+1));
          ED_CALLOC(col_chan[ncols], "create_ssc_table", char  , MAXCHAR);
          s = leading_zeros(chan_nr[i],2);
          sprintf(col_name[ncols], "N_%s",s);
          ED_FREE(s);
          if (name_to_key(tabout, col_name[ncols]))
             syserr(FATAL, "create_ssc_table",
              "The %s is a propriatory key. Do not specify it as output key\n",
                 col_name[ncols]);
          strcpy(col_chan[ncols], "COUNT");
          strcpy(col_merge[ncols], "COUNT");
          key = new_key(col_name[ncols]);
          key->htype  = H_INT;
          key->ttype  = T_LONG;
          key->naxis  = 0;
          key->nobj   = 1;
          key->nbytes = t_size[key->ttype];
          sprintf(key->comment,
              "Number of objects merged for channel %d",chan_nr[i]);
          ED_MALLOC(key->ptr, "create_ssc_table", char, key->nbytes);
          ncols++;
          if (add_key(key, tabout, 0)==RETURN_ERROR)
             syserr(FATAL, "create_ssc_table",
                 "Error adding key %s to ssc_table\n", key->name);
       }
       ED_CALLOC(last_best_fpos, "create_ssc_table", LONG, control.ilist);
       /* COMMENTED CODE FOR MODE FIELD_POS
       if (name_to_key(tabout, control.ra) != NULL &&
           name_to_key(tabout, control.dec) != NULL) {
          BACK_JUMP++;
          for (i=0;i<control.ilist;i++) {
             ED_REALLOC(col_name,       "create_ssc_table", char *, (ncols+1));
             ED_CALLOC(col_name[ncols], "create_ssc_table", char  , MAXCHAR);
             ED_REALLOC(col_input,      "create_ssc_table", char *, (ncols+1));
             ED_CALLOC(col_input[ncols],"create_ssc_table", char  , MAXCHAR);
             ED_REALLOC(col_merge,      "create_ssc_table", char *, (ncols+1));
             ED_CALLOC(col_merge[ncols],"create_ssc_table", char  , MAXCHAR);
             ED_REALLOC(col_chan,       "create_ssc_table", char *, (ncols+1));
             ED_CALLOC(col_chan[ncols], "create_ssc_table", char  , MAXCHAR);
             s = leading_zeros(i,2);
             sprintf(col_name[ncols], "FIELD_POS_%s", s);
             ED_FREE(s);
             if (name_to_key(tabout, col_name[ncols]))
                syserr(FATAL, "create_ssc_table",
              "The %s is a propriatory key. Do not specify it as output key\n",
                 col_name[ncols]);
             sprintf(col_chan[ncols], "%d", i);
             strcpy(col_merge[ncols], "FIELD_POS");
             key = new_key(col_name[ncols]);
             key->htype  = H_INT;
             key->ttype  = T_LONG;
             key->naxis  = 0;
             key->nobj   = 1;
             key->nbytes = t_size[key->ttype];
             sprintf(key->comment,
                 "Field position reference for channel %d",chan_nr[i]);
             ED_MALLOC(key->ptr, "create_ssc_table", char, key->nbytes);
             ncols++;
             if (add_key(key, tabout, 0)==RETURN_ERROR)
                syserr(FATAL, "create_ssc_table",
                    "Error adding key %s to ssc_table\n", key->name);
          }
       } */
       ED_REALLOC(col_name,       "create_ssc_table", char *, (ncols+1));
       ED_CALLOC(col_name[ncols], "create_ssc_table", char  , MAXCHAR);
       ED_REALLOC(col_input,      "create_ssc_table", char *, (ncols+1));
       ED_CALLOC(col_input[ncols],"create_ssc_table", char  , MAXCHAR);
       ED_REALLOC(col_merge,      "create_ssc_table", char *, (ncols+1));
       ED_CALLOC(col_merge[ncols], "create_ssc_table", char  , MAXCHAR);
       ED_REALLOC(col_chan,       "create_ssc_table", char *, (ncols+1));
       ED_CALLOC(col_chan[ncols], "create_ssc_table", char  , MAXCHAR);
       sprintf(col_name[ncols], "SeqNr");
       if (name_to_key(tabout, col_name[ncols]))
          syserr(FATAL, "create_ssc_table",
            "The %s is a propriatory key. Do not specify it as output key\n",
              col_name[ncols]);
       strcpy(col_chan[ncols], "COUNT");
       strcpy(col_merge[ncols], "COUNT");
       key = new_key(col_name[ncols]);
       key->htype  = H_INT;
       key->ttype  = T_LONG;
       key->naxis  = 0;
       key->nobj   = 1;
       key->nbytes = t_size[key->ttype];
       sprintf(key->comment, "Object sequence number (from make_ssc)");
       ED_MALLOC(key->ptr, "create_ssc_table", char, key->nbytes);
       ncols++;
       if (add_key(key, tabout, 0)==RETURN_ERROR)
          syserr(FATAL, "create_ssc_table", "Error adding key to ssc_table\n");
    }
    ED_CALLOC(col_keys, "create_ssc_table", keystruct *, ncols);
    update_tab(tabout);
    for (i=0;i<ncols;i++)
       col_keys[i] = name_to_key(tabout, col_name[i]);
    return tabout;
}
/**/

/* COMMENTED CODE FOR MODE FIELD_POS
static void init_which_field(tabstruct **ftab, field_params ***p, int *nf)
{
    int		i, j;
    double	dra, temp;

    ED_CALLOC(ra_min,  "init_which_field", double *, control.ilist);
    ED_CALLOC(ra_max,  "init_which_field", double *, control.ilist);
    ED_CALLOC(dec_min, "init_which_field", double *, control.ilist);
    ED_CALLOC(dec_max, "init_which_field", double *, control.ilist);
    for (j=0;j<control.ilist;j++) {
       if (ftab[j]) {
          ED_CALLOC(ra_min[j],  "init_which_field", double, nf[j]);
          ED_CALLOC(ra_max[j],  "init_which_field", double, nf[j]);
          ED_CALLOC(dec_min[j], "init_which_field", double, nf[j]);
          ED_CALLOC(dec_max[j], "init_which_field", double, nf[j]);
          for (i=0;i<nf[j];i++) {
             dra = p[j][i]->cdelt1 / cos(p[j][i]->crpix2 * DEG2RAD);
             ra_min[j][i]=p[j][i]->crval1 - p[j][i]->crpix1 * dra;
             ra_max[j][i]=p[j][i]->crval1 +
                         (p[j][i]->xwid - p[j][i]->crpix1) * dra;
             dec_min[j][i]=p[j][i]->crval2 - p[j][i]->crpix2 *
                           p[j][i]->cdelt2;
             dec_max[j][i]=p[j][i]->crval2 + (p[j][i]->ywid -
                           p[j][i]->crpix2) * p[j][i]->cdelt2;
             if (ra_min[j][i] > ra_max[j][i]) {
                temp=ra_min[j][i];
                ra_min[j][i]=ra_max[j][i];
                ra_max[j][i]=temp;
             }
             if (dec_min[j][i] > dec_max[j][i]) {
                temp=dec_min[j][i];
                dec_min[j][i]=dec_max[j][i];
                dec_max[j][i]=temp;
             }
          }
       }
    }
}
*/

/**/
tabstruct *open_ssc_cat(char *filename, tabstruct **tab, tabstruct **ftabs,
                        int ncolumns, int argc, char **argv)
{
/****** open_ssc_cat ***************************************************
 *
 *  NAME	
 *      open_ssc_cat - Initialize global structure for SSC
 *
 *  SYNOPSIS
 *      tabstruct *open_ssc_cat(char *filename)
 *
 *  FUNCTION
 *      This routine initializes the structure required to store
 *      the object information derived by the calling routines.
 *
 *  INPUTS
 *      filename - name of the output file
 *      tab      - pointer to first input catalogs OBJECTS table
 *      ftab     - pointer to first input catalogs FIELDS table
 *      ncolumns - THe number of output columns
 *      head     - The program identification string
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - 23-12-1996
 *
 ******************************************************************************/

    tabstruct	*ssc_tab;
    int		i, j;
    void	*ptr;

/*
 *  Save in local static variable
 */
    ncols = ncolumns;

    ssc_cat = new_cat(1);
    init_cat(ssc_cat);
    historyto_cat(ssc_cat, "make_ssc", argc, argv);
    strcpy(ssc_cat->filename, filename);
    if (open_cat(ssc_cat, WRITE_ONLY) != RETURN_OK)
      syserr(FATAL, "open_ssc_cat","Cannot open output catalog %s\n",
             filename);
/*
 *  First write the Primary FITS header unit
 */
    save_tab(ssc_cat, ssc_cat->tab);
/*
 *  Then initialize the output table
 */
    ssc_tab = create_ssc_table(tab, ftabs);
    ssc_tab->cat = ssc_cat;
    init_writeobj(ssc_cat, ssc_tab);

    if (control.make_pairs == NO) {
       if ((ra_key = name_to_key(ssc_tab, control.ra)) == NULL)
          syserr(WARNING, "ssc_put_row", "No key %s in table %s\n",
                       control.ra, ssc_tab->extname);
       if ((dec_key = name_to_key(ssc_tab, control.dec)) == NULL)
          syserr(WARNING, "ssc_put_row", "No key %s in table %s\n",
                       control.dec, ssc_tab->extname);
    }
    if (control.best) {
       ED_CALLOC(best_key, "open_ssc_cat", keystruct *, control.ilist);
       for (i=0;i<control.ilist;i++) {
          if (ftabs[i] && name_to_key(ftabs[i], control.bestcol)) {
             ED_GETKEY(ftabs[i], best_key[i], control.bestcol, ptr, j,
                    "open_ssc_cat");
          } else {
             syserr(WARNING, "open_ssc_cat",
             "No FIELDS table or no %s column in FIELDS table of catalog %s\n",
                  control.bestcol, tab[i]->cat->filename);
             ED_CALLOC(best_key[i], "open_ssc_cat", keystruct, 1);
             best_key[i]->nobj= 0;
             ftabs[i] = NULL;
          }
       }
    }
/*
 *  Read the frame position information if the FIELDS table exists
 */

/*
    if (control.make_pairs == NO) {
       ED_CALLOC(field_p, "open_ssc_cat", field_params **, control.ilist);
       ED_CALLOC(nfields, "open_ssc_cat", int, control.ilist);
       for (i=0;i<control.ilist;i++) {
         if (ftabs[i])
           field_p[i] = load_field_description(ftabs[i], &nfields[i],
                    control.crval1, control.crval2,
                    control.cdelt1, control.cdelt2,
                    control.crpix1, control.crpix2,
                    control.xwid, control.ywid);
       }
       init_which_field(ftabs, field_p, nfields);
    }
*/
    return ssc_tab;
}
/**/
void close_ssc_cat(tabstruct *ssc_tab)
/****** close_ssc_cat ***************************************************
 *
 *  NAME	
 *      close_ssc_cat - Close the output table of make_ssc
 *
 *  SYNOPSIS
 *      void close_ssc_cat(catstruct *ssc_cat, tabstruct *ssc_tab)
 *
 *  FUNCTION
 *      This function finished the object by object writing and
 *      closes the output table.
 *
 *  INPUTS
 *      ssc_tab - pointer to table structure of PSSC table
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 01-07-1997
 *
 ******************************************************************************/
{
    catstruct *ssc_cat;

    ssc_cat = ssc_tab->cat;
    end_writeobj(ssc_cat, ssc_tab);
}
/**/

static void copy_pairs(tabstruct *tab, keystruct *key1, int chan1,
                                       keystruct *key2, int chan2)
/****i* copy_pairs ***************************************************
 *
 *  NAME	
 *      copy_pairs - Copy object information of two objects into the output
 *
 *  SYNOPSIS
 *      void copy_pairs(tabstruct tab, keystruct *key1, int chan1,
 *                                     keystruct *key2, int chan2)
 *
 *  FUNCTION
 *      This routine copies the object information of two input objects
 *      into the output table according to the rules defined by the user.
 *      These rules are stored in the control structure.
 *
 *  INPUTS
 *      keys1  - Key chain with information for the first object
 *      chan1  - Channel number of first object
 *      keys2  - Key chain with information for the second object
 *      chan2  - Channel number of second object
 *
 *  RESULTS
 *      tab    - Table structure containing the output information
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 01-07-1997
 *
 ******************************************************************************/
{
    int 	i, j;
    short int	schan1, schan2;
    keystruct	*key, *lkey;
    char	*iptr, *optr;

    for (i=0;i<ncols;i++) {
       key = name_to_key(tab, col_name[i]);
       optr = key->ptr;
/*
 *     Search the associated input keys
 */
       lkey = key1;
       while (lkey != NULL) {
          if (strcmp(col_input[i], lkey->name) == 0) {
             iptr = lkey->ptr;
             for (j=MIN(lkey->nbytes,key->nbytes/2);j--;) *(optr++)=*(iptr++);
             break;
          }
          lkey = lkey->nextkey;
       }
       lkey = key2;
       while (lkey != NULL) {
          if (strcmp(col_input[i], lkey->name) == 0) {
             iptr = lkey->ptr;
             for (j=MIN(lkey->nbytes,key->nbytes/2);j--;) *(optr++)=*(iptr++);
             break;
          }
          lkey = lkey->nextkey;
       }
    }
/*
 *  When a CHANNEL_NR column exists for the output, fill it
 */
    if ((key = name_to_key(tab, "CHANNEL_NR")) != NULL) {
       if (key->ttype == T_SHORT) {
          schan1 = chan_nr[chan1];
          schan2 = chan_nr[chan2];
          optr = key->ptr;
          memcpy(optr, &schan1, t_size[key->ttype]);
          optr += t_size[key->ttype];
          memcpy(optr, &schan2, t_size[key->ttype]);
       } else {
          optr = key->ptr;
          memcpy(optr, &chan_nr[chan1], t_size[key->ttype]);
          optr += t_size[key->ttype];
          memcpy(optr, &chan_nr[chan2], t_size[key->ttype]);
       }
    }
}

/**/

void ssc_put_pairs(keystruct **keys, set_struct *s, tabstruct *ssc_tab)
/****** ssc_put_pairs ***************************************************
 *
 *  NAME	
 *      ssc_put_pairs - Write the set of pairs to the output table
 *
 *  SYNOPSIS
 *      void ssc_put_pairs(keystruct **keys, set_struct *s)
 *
 *  FUNCTION
 *      This routine write all the pair combinations in the set objects
 *      list to the output catalog.
 *
 *  INPUTS
 *      keys  - The array of key chains containing the object data
 *      s     - The set oj objects
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 01-07-1997
 *
 ******************************************************************************/
{
    int		i, j;

    for (i=0;i<s->nelem;i++) {
       for (j=i+1;j<s->nelem;j++) {
/*
 *        Make sure the lowest channel number is first in the copy statement
 *        This allows different length vector to be merged, provided the
 *        longest one is given as the smallest channel
 */
          if ((control.color_pairs == YES && s->chan[i] != s->chan[j]) ||
              control.color_pairs == NO) {
             if (s->chan[i] <= s->chan[j])
                copy_pairs(ssc_tab, keys[i], s->chan[i], keys[j], s->chan[j]);
             else
                copy_pairs(ssc_tab, keys[j], s->chan[j], keys[i], s->chan[i]);
             add_count_status(0,1);
             write_obj(ssc_tab);
          }
       }
    }
}
/**/
/* COMMENTED CODE FOR MODE FIELD_POS
static int which_field(double ra, double dec, int chan)
{
    int		i, retcode;
    int		found;
    double	dpos = 0.0028;
    double      dmaxbest, dminbest, *best;
    int		nmaxbest, nminbest, *best_num;
	

    retcode = 0;
    if (field_p[chan]) {
       ED_CALLOC(best_num, "which_field", int, nfields[chan]);
       found = 0;
       for (i=0;i<nfields[chan];i++) {
          if (((ra_min[chan][i] <= ra && ra <= ra_max[chan][i]) ||
               (ra_min[chan][i] <= (ra -360.0) &&
               (ra -360.0) <= ra_max[chan][i])) &&
              dec_min[chan][i] <= dec && dec <= dec_max[chan][i]) {
             best_num[found++] = i;
          }
       }
       if (!found) {
          for (i=0;i<nfields[chan];i++) {
             if (((ra_min[chan][i]-dpos <= ra && ra <= ra_max[chan][i]+dpos) ||
                  (ra_min[chan][i]-dpos <= (ra -360.0) &&
                  (ra -360.0) <= ra_max[chan][i]+dpos)) &&
                 dec_min[chan][i]-dpos <= dec && dec <= dec_max[chan][i]+dpos) {
                best_num[found] = i;
             }
          }
       }
       if (found) {
          ED_CALLOC(best, "which_field", double, found);
          for (i=0;i<found;i++) {
             if (control.best) {
                switch (best_key[chan]->ttype) {
                case T_BYTE:   best[i] = (double) ((BYTE *)best_key[chan]->ptr)[best_num[i]];
                               break;
                case T_SHORT:  best[i] = (double) ((short int *)best_key[chan]->ptr)[best_num[i]];
                               break;
                case T_LONG:   best[i] = (double) ((LONG *)best_key[chan]->ptr)[best_num[i]];
                               break;
                case T_FLOAT:  best[i] = (double) ((float *)best_key[chan]->ptr)[best_num[i]];
                               break;
                case T_DOUBLE: best[i] = ((double *)best_key[chan]->ptr)[best_num[i]];
                               break;
                case T_STRING:
                               break;
                }
             } else {
                retcode = best_num[0];
                break;
             }
          }
          if (control.best) {
             dmaxbest = dminbest = best[0];
             nmaxbest = nminbest = best_num[0];
             for (i=1;i<found;i++) {
                if (best[i] < dminbest) {
                   dminbest = best[i];
                   nminbest = best_num[i];
                }
                if (best[i] > dmaxbest) {
                   dmaxbest = best[i];
                   nmaxbest = best_num[i];
                }
             }
             if (control.best_type == BEST_MIN)
                retcode = nminbest+1;
             if (control.best_type == BEST_MAX)
                retcode = nmaxbest+1;
          }
          ED_FREE(best);
       } else {
       //
       //   if (last_best_fpos[chan] > -1)
       //      retcode  = last_best_fpos[chan];
          retcode = 0;
       }
       ED_FREE(best_num);
    }
    return retcode;
}
*/

/**/
void ssc_put_row(keystruct **keys, set_struct *s, tabstruct *ssc_tab)
/****** ssc_put_row ***************************************************
 *
 *  NAME
 *      ssc_put_row - Write one object to the output table
 *
 *  SYNOPSIS
 *      void ssc_put_row(keystruct **keys, set_struct *s, tabstruct *ssc_tab)
 *
 *  FUNCTION
 *      This function write one objects parameters to the output table.
 *      This includes mergeing the information from the different
 *      input tables
 *
 *  INPUTS
 *      keys - Array of key chains of input object information
 *      s    - The set of objects for which keys stores the information
 *      ssc_tab - pointer to output table
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *
 *  AUTHOR
 *      E.R. Deul - dd 01-07-1997
 *      E.R. Deul - dd 04-02-1998  Moved count++ forward so that SeqNr will
 *                                 start with 1 instead of 0
 *
 ******************************************************************************/
{
    int         i, *ncol_obj, nchan, ok;
    keystruct   *key;
    /* COMMENTED CODE FOR MODE FIELD_POS
    int         j, f_pos;
    double      ra, dec;
    */

    for (i=0;i<control.ilist;i++)
       last_best_fpos[i] = -1;
    for (i=0;i<ncols;i++) {
       ok = FALSE;
       if (memcmp(col_chan[i],"ALL",3) != 0)
          if (memcmp(col_chan[i],"COUNT",5) != 0)
             if ((nchan = atoi(col_chan[i])) < 0 ||
                 nchan >= control.ilist)
                syserr(FATAL, "ssc_put_row",
                   "Channel number value %d of %s out of bounds (0, %d)\n",
                   nchan, col_name[i], control.ilist-1);
       key = col_keys[i];
       if (memcmp(col_merge[i], "AVERAGE",7) == 0 || \
           memcmp(col_merge[i], "AVE_REG",7) == 0) {
          average_regular(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "WEIGHTED",8) == 0) {
          weighted(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "AVE_PHOT_ERR",11) == 0) {
          average_phot_error(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "AVE_ERR",7) == 0) {
          average_error(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "AVE_FLAG",8) == 0) {
          average_flag(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "AVE_PHOT",8) == 0) {
          average_photometry(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "MIN",3) == 0) {
          min(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "MAX",3) == 0) {
          max(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "DETECT",6) == 0) {
          detected(key, keys, i, s);
          ok = TRUE;
       }
       if (memcmp(col_merge[i], "BEST",4) == 0) {
          best(key, keys, i, s);
          ok = TRUE;
       }
/* COMMENTED CODE FOR MODE FIELD_POS
       if (memcmp(col_merge[i], "FIELD_POS", 9) == 0) {
//
//        Because the FIELD_POS is added at the end of the columns list,
//        the Ra and Dec columns should have been calculated
//
          ra = *(double *)ra_key->ptr;
          dec = *(double *)dec_key->ptr;
          j = atoi(col_chan[i]);
          f_pos = which_field(ra, dec, j);
          *((LONG *)key->ptr) = f_pos+1;
          ok = TRUE;
       }
*/
       if (!ok && memcmp(col_merge[i], "COUNT", 5) != 0)
          syserr(FATAL, "ssc_put_row", "Unknown merge type %s for key %s\n",
              col_merge[i], col_name[i]);
    }
    add_count_status(0,1);
    if (control.make_pairs == NO) {
       ED_CALLOC(ncol_obj, "ssc_put_row", int, control.ilist+1);
       for (i=0;i<s->nelem;i++)
          ncol_obj[s->chan[i]]++;
       for (i=0;i<control.ilist;i++) {
          key = col_keys[i+ncols-1-(BACK_JUMP*control.ilist)];
          *((LONG *)key->ptr) = ncol_obj[i];
       }
       key = col_keys[ncols-1];
       *((LONG *)key->ptr) = get_count_status(0,1);
       ED_FREE(ncol_obj);
    }
    write_obj(ssc_tab);
}
