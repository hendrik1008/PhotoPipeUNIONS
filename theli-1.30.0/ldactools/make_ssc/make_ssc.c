#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "fitscat.h"
#include "utils_globals.h"
#include "options_globals.h"
#include "sort_globals.h"
#include "casts_globals.h"
#include "bin_search_globals.h"
#include "ssc_globals.h"
#include "ssc_utils_globals.h"
#include "ssc_table_globals.h"
#include "chain_globals.h"

/* 21.02.01:
 * substituted all long int by the LONG macro

 * 15.07.01:
 * added dummy return to function get_count_status to
 * avoid compiler warnings
 *
 * 30.01.03:
 * included a security check in save_set_ssc:
 * I shifted a test for a NULL pointer from
 * the end to the start of a loop (problems
 * with catalogs with only one element)
 *
 * 10.01.05:
 * I removed compiler warnings under gcc with optimisation
 * enabled (-O3).
 *
 * 29.01.2005:
 * I adapted the arguments of open_ssc_cat (routine main)
 * for the inclusion of HISTORY information to the output catalogs
 *
 * 10.05.2003:
 * I removed compiler warnings under gcc with flags -W -pedantic
 * -Wall
 *
 */

control_struct control;

static catstruct	**catin;
static tabstruct	**tabin, **itabin, **ftabin, *ssc_tab;

option my_opts[] = {
       {"MAKE_PAIRS", BOOLEAN, &control.make_pairs, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "",
        {"YES", "NO", "", "", ""}, 0,
        "Let's make_ssc create a pairs catalog or not"},
       {"COLOR_PAIRS", BOOLEAN, &control.color_pairs, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "",
        {"YES", "NO", "", "", ""}, 0,
        "Restrict the pairs catalog to cross-color pairs or not"},
       {"PAIR_COL", BOOLEAN, &control.pair_col, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "",
        {"YES", "NO", "", "", ""}, 0,
        "Restrict the pairs catalog to cross-color pairs or not"},
       {"BEST_COL", TEXT, control.bestcol, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "SEXSFWHM",
        {"", "", "", "", ""}, 0,
        "In case of BEST merge option, the name of the FIELDS table column on which to decided"},
       {"BEST_TYPE", TEXT, control.besttype, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "MIN",
        {"MIN", "MAX", "", "", ""}, 0,
        "How to decide what is the BEST"},
       {"COL_NAME", TEXTVEC, control.name, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "",
        {"", "", "", "", ""}, 0,
        "The name of the output table column"},
       {"COL_INPUT", TEXTVEC, control.input, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "",
        {"", "", "", "", ""}, 0,
        "The name of the input table column"},
       {"COL_MERGE", TEXTVEC, control.merge, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "",
        {"", "", "", "", ""}, 0,
        "The type of merging to be done"},
       {"COL_CHAN", TEXTVEC, control.chan, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "",
        {"", "", "", "", ""}, 0,
        "Merging to be one using which input channels (catalog #)"},
       {"RA", TEXT, control.ra, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "Ra",
         {"", "", "", "", ""}, 0,
         "Name of the output table column containing Right Ascension"},
       {"DEC", TEXT, control.dec, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "Dec",
         {"", "", "", "", ""}, 0,
         "Name of the output table column containing Declination"},
       {"CRVAL1", TEXT, control.crval1, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "CRVAL1",
         {"", "", "", "", ""}, 0,
         "Name of the input table column containing Right Ascension"},
       {"CRVAL2", TEXT, control.crval2, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "CRVAL2",
         {"", "", "", "", ""}, 0,
         "Name of the input table column containing Declination"},
       {"CRPIX1", TEXT, control.crpix1, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "CRPIX1",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing reference x coordinate pixel"},
       {"CRPIX2", TEXT, control.crpix2, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "CRPIX2",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing reference y coordinate pixel"},
       {"CDELT1", TEXT, control.cdelt1, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "CDELT1",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing x coordinate pixel size"},
       {"CDELT2", TEXT, control.cdelt2, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "CDELT2",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing y coordinate pixel size"},
       {"XWID", TEXT, control.xwid, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "MAPNAXS1",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing x coordinate pixel width"},
       {"YWID", TEXT, control.ywid, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "MAPNAXS2",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing y coordinate pixel width"},
       {"SeqNr", TEXT, control.seqnr, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "SeqNr",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing sequence number of the object"},
       {"FIELD_POS", TEXT, control.fieldpos, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "FIELD_POS",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing field number of the object"},
       {"CLUSTER_NR", TEXT, control.cluster_nr, 0, 0, 0.0, 0.0,
         0, 0.0, NO, "CLUSTER_NR",
         {"", "", "", "", ""}, 0,
         "Name of the table column containing cluster sequence number"},
        {"", BOOLEAN, NULL, 0, 0, 0.0, 0.0, 0, 0.0, NO, "", {"YES", "NO", "", "", ""}, 0,
        "dummy key to end structure my_opts"}
       };

/**/
static void usage(char *me, char *head)
{
/****i* usage ***************************************************
 *
 *  NAME	
 *      usage - Print usage information to the user
 *
 *  SYNOPSIS
 *      void usage(char *me)
 *
 *  FUNCTION
 *      Print information of command line arguments and general usage
 *      of the program to the user on standard error channel.
 *
 *  INPUTS
 *      me - String containing the calling name of the program
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
 *      E.R. Deul - dd 10-10-1996
 *
 ******************************************************************************/
    fprintf(stderr," %s \n\n", head);
    fprintf(stderr,"Usage: %s \n", me);
    fprintf(stderr,"		-i incat1 [incat2 [..]]	(input catalogs)\n");
    fprintf(stderr,"		-o outcat		(output catalog)\n");
    fprintf(stderr,"		[-t table_name]		(input catalog tablename to merge)\n");
    fprintf(stderr,"		[-p table_name catalog_nr [table_name catalog_nr ...]]\n");
    fprintf(stderr,"					(tables to copy from input to output)\n");
    fprintf(stderr,"		[-c config_file]\n");
    fprintf(stderr,"		[-OPTION1 OPTVAL1 [-OPTION OPTVAL2] ...]\n\n");
    exit(1);
}
/**/

static int init(int argc, char *argv[], char *head)
{
/****i* init ***************************************************
 *
 *  NAME	
 *      init - Parse the command line arguments
 *
 *  SYNOPSIS
 *      void init(int argc, char *argv[])
 *
 *  FUNCTION
 *      This routine interprets the command line arguments and fills the
 *      global controls structure with the specified values.
 *
 *  INPUTS
 *      argc - Number of arguments
 *      argv - Array of argument strings
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The number of output columns
 *
 *  MODIFIED GLOBALS
 *      The control structure is modified
 *
 *  NOTES
 *      Uses global variables:
 *         col_name, col_input, col_merge, col_chan
 *         control
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul dd - 10-10-1996
 *
 ******************************************************************************/
/*
 *  Default file name is calling name with .conf extension
 */
   int		i, j, k, ncols;

   sprintf(control.conffile,"%s.conf",argv[0]);
   sprintf(control.table_name,"OBJECTS");
   if (argc == 1) usage(argv[0], head);
   control.npaste = 0;
   init_options(0, my_opts);
   for (i=1; i<argc; i++) {
     if (argv[i][0] == '-') {
        switch(argv[i][1]) {
        case 'c': strcpy(control.conffile,argv[++i]);
                  break;
        case 't': strcpy(control.table_name,argv[++i]);
                  break;
        case 'i': control.ilist = 0;
                  strcpy(control.listfile[control.ilist],argv[++i]);
                  while (i+1<argc && argv[i+1][0] != '-') {
                     strcpy(control.listfile[++control.ilist],argv[++i]);
                  }
                  control.ilist++;
                  break;
        case 'o': strcpy(control.outfile,argv[++i]);
                  break;
        case 'p': control.npaste = 0;
                  strcpy(control.pastetab[control.npaste],argv[++i]);
                  control.pastecat[control.npaste] = atoi(argv[++i]) - 1;
                  while (i+2<argc && argv[i+1][0] != '-') {
                     strcpy(control.pastetab[++control.npaste],argv[++i]);
                     control.pastecat[control.npaste] = atoi(argv[++i]) - 1;
                  }
                  control.npaste++;
                  break;
        case '@': print_options(argv[0]);
                  exit(0);
        default:  parse_options(control.conffile);
                  j = i++; k = i;
                  if (argv[j] && argv[k])
                    set_command_option(argv[j], argv[k]);
                  else
                    usage(argv[0], head);
                  break;
        }
     } else {
        syserr(WARNING,"init","Command line syntax error\n");
        usage(argv[0], head);
     }
   }
   finish_options(control.conffile);
   VPRINTF(NORMAL,"\n %s \n\n", head);
   col_name = copy_textvec(control.name, &ncols);
   if (ncols == 0)
      syserr(FATAL,"init","No output columns defined for %s\n",control.outfile);
   col_input = copy_textvec(control.input, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_INPUT keys differs from COL_NAME\n");
   col_merge = copy_textvec(control.merge, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_MERGE keys differs from COL_NAME\n");
   col_chan = copy_textvec(control.chan, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_CHAN keys differs from COL_NAME\n");
/*
 * See if the user has specified a BEST merge condition
 */
   for (i=0,control.best=FALSE;i<ncols;i++) {
      if (strcmp(col_merge[i],"BEST") == 0) {
         control.best = TRUE;
         break;
      }
   }
   if (strcmp(control.besttype, "MIN") == 0)
       control.best_type = BEST_MIN;
   if (strcmp(control.besttype, "MAX") == 0)
       control.best_type = BEST_MAX;

   return ncols;
}
/**/
static LONG wcount;
static LONG rcount;

void init_count_status(int r, int w)
{
    rcount = r;
    wcount = w;
}

void add_count_status(int r, int w)
{
    rcount += r;
    wcount += w;
    if ((rcount) % 2048 == 0 || (wcount) % 2048 == 0)
       VPRINTF(NORMAL,"Objects read: %d, written: %d\n", rcount, wcount);
}

LONG get_count_status(int r, int w)
{
    if (r) return rcount;
    if (w) return wcount;

    /* dummy (compiler warnings) */
    return 0;
}
/**/

static int *open_catalogs()
{
/****i* open_catalogs ***************************************************
 *
 *  NAME	
 *      open_catalogs - Open the catalog files and intialize data structures
 *
 *  SYNOPSIS
 *      static int *open_catalogs()
 *
 *  FUNCTION
 *      This routine opens the connections to the catalog files and
 *      initializes the appropriate global structures to allow storage of
 *      object information. The reconstruction of object association is
 *      is also performed.
 *
 *  INPUTS
 *      None
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The number of objects per catalog
 *
 *  MODIFIED GLOBALS
 *      The global variables containing the object parameters to be used
 *      and parsed to the ssc are initialized.
 *
 *  NOTES
 *      Uses global variables:
 *        catin[], tabin[], ftabin[], itabin[]
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul dd - 10-10-1996
 *
 ******************************************************************************/
/*
 *  Open pointers to the appropriate catalogs and the tables therein.
 *  In addition prepair the id's columns for identifying associations.
 */
    int		i, *nobj;

/*
 *  Now actually open the catalogs and obtain the pointers
 */
    ED_MALLOC(catin,     "open_catalogs", catstruct *, control.ilist);
    ED_MALLOC(tabin,     "open_catalogs", tabstruct *, control.ilist);
    ED_MALLOC(itabin,    "open_catalogs", tabstruct *, control.ilist);
    ED_MALLOC(ftabin,    "open_catalogs", tabstruct *, control.ilist);
    for (i=0;i<control.ilist;i++) {
       if ((catin[i] = read_cat(control.listfile[i])) == NULL) {
          syserr(FATAL, "open_catalogs", "Could not read catalog: %s\n",
                 control.listfile[i]);
       }
       if ((tabin[i] = name_to_tab(catin[i], control.table_name, 0)) == NULL) {
          syserr(FATAL, "open_catalogs", "No table %s in catalog %s\n",
                 control.table_name, control.listfile[i]);
       }
    }
/*
 *  Prepair the association markings
 */
    nobj = pre_pair(tabin);
    make_indices();
/*
 *  Reinitialize the catalogs
 */
    for (i=0;i<control.ilist;i++)
       close_cat(catin[i]);
    for (i=0;i<control.ilist;i++) {
       if ((catin[i] = read_cat(control.listfile[i])) == NULL) {
          syserr(FATAL, "open_catalogs", "Could not read catalog: %s\n",
                 control.listfile[i]);
       }
       if ((ftabin[i] = name_to_tab(catin[i], "FIELDS", 0)) == NULL) {
          syserr(WARNING, "open_catalogs",
                 "No table FIELDS in catalog %s assuming channel numbers\n",
                 control.listfile[i]);
          ftabin[i] = NULL;
       }
       if ((tabin[i] = name_to_tab(catin[i], control.table_name, 0)) == NULL) {
          syserr(FATAL, "open_catalogs", "No table %s in catalog %s\n",
                 control.table_name, control.listfile[i]);
       }
    }

    return nobj;
}

static void close_catalogs(tabstruct *ssc_tab)
/****** close_catalogs ***************************************************
 *
 *  NAME	
 *      close_catalogs - Close all catalogs
 *
 *  SYNOPSIS
 *      static void close_catalogs(tabstruct *ssc_tab)
 *
 *  FUNCTION
 *      This is the routine to close all open catalogs.
 *
 *  INPUTS
 *      None
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
 *      Uses globals:
 *        control
 *        catin[]
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 01-07-1997
 *
 ******************************************************************************/
{
    int		i;
    catstruct	*ssc_cat;
    tabstruct	*tab;

    ssc_cat = ssc_tab->cat;

    for (i=0;i<control.npaste;i++) {
       copy_tab (catin[control.pastecat[i]],control.pastetab[i],0,ssc_cat,0);
       tab = name_to_tab(ssc_cat, control.pastetab[i],0);
       if (control.ilist > 1) {
          sprintf(tab->extname, "%s_%s", control.pastetab[i],
                  leading_zeros(control.pastecat[i],2));
          update_tab(tab);
       }
       save_tab(ssc_cat, tab);
    }
    for (i=0;i<control.ilist;i++) {
       close_cat(catin[i]);
    }
}

static keystruct *copy_keys(tabstruct *tab, int ncols)
/****** copy_keys ***************************************************
 *
 *  NAME	
 *      copy_keys - Copy the keystruct and the data content to the output
 *
 *  SYNOPSIS
 *      keystruct *copy_keys(tabstruct *tab)
 *
 *  FUNCTION
 *      This routine creates a chain of keys from the input table
 *      key chain. It also copies the content of the keys.
 *
 *  INPUTS
 *      tab  - Pointer to the input table structure
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The pointer to a chain of keys that is the copy from the tab
 *      key chain
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      The routine copies the content of the keychain to the output key
 *      chain. The output one is a open chain with NULL pointers for the
 *      outer bounds.
 *      Uses global variables:
 *        col_name, col_input, col_merge, col_chan
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 01-07-1997
 *
 ******************************************************************************/
{
    int		i, j;
    keystruct	*key, *newkey, *prevkey, *topkey;
    char	*newptr, *ptr;

    topkey = prevkey = NULL;
    for (i=0;i<ncols;i++) {
       if ((key = name_to_key(tab, col_input[i])) != NULL) {
          newkey = new_key(col_name[i]);
          *newkey = *key;
          newkey->ptr = newkey->prevkey = newkey->nextkey = NULL;
          newkey->tab = NULL;
          if (prevkey == NULL) {
             topkey = newkey;
          } else {
             newkey->prevkey = prevkey;
             prevkey->nextkey = newkey;
          }
          if (newkey->naxis)
          ED_CALLOC(newkey->naxisn, "copy_keys", int, newkey->naxis);
          for (j=0;j<newkey->naxis;j++)
             newkey->naxisn[j] = key->naxisn[j];
          ED_MALLOC(newkey->ptr, "copy_keys", char, newkey->nbytes);
          newptr = newkey->ptr;
          ptr    =    key->ptr;
          for (j=newkey->nbytes;j--;)
             *(newptr++) = *(ptr++);
          prevkey = newkey;
          }
    }
    return topkey;
}
/**/
static keystruct **ssc_get_objects(set_struct *s, int n)
/****** ssc_get_objects ***************************************************
 *
 *  NAME	
 *      ssc_get_objects - Get the objects info for a given set
 *
 *  SYNOPSIS
 *      keystruct **ssc_get_objects(setstruct *s)
 *
 *  FUNCTION
 *      This routine retrieves from a number of input catalogs the
 *      table information for a set of objects. The information retrieved
 *      is defined in the control structure, and the list of objects
 *      to retrieve is given as set in the argument.
 *
 *  INPUTS
 *      s   - The set of objects to retrieve
 *      n   - The number of keys in the output table
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      An array of key chains is returned with an exact mapping of
 *      object keystruct [i] <-> s[i]
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      Use global variables:
 *         rcout, wcount
 *         itabin[], tabin{}
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 01-07-1997
 *
 ******************************************************************************/
{
    int		i;
    keystruct	**keys;

    ED_MALLOC(keys, "ssc_get_objects", keystruct *, s->nelem);
    for (i=0;i<s->nelem;i++) {
       add_count_status(1,0);
       if (read_obj_at(itabin[s->chan[i]], tabin[s->chan[i]], s->indx[i]) ==
           RETURN_ERROR)
          syserr(FATAL, "ssc_get_objects", "Getting object at %d\n",
                        s->indx[i]);
       keys[i] = copy_keys(itabin[s->chan[i]], n);
    }
    return keys;
}
/**/
static set_struct *set_pairs(int i, int chan)
{
/****i* set_pairs ***************************************************
 *
 *  NAME	
 *      set_pairs - Get all pairs for the given set of objects
 *
 *  SYNOPSIS
 *      set_struct *set_pairs(set_struct *s)
 *
 *  FUNCTION
 *      This function finds all associated objects for each element in
 *      the input set. It creates a new set that contains the parameters
 *      of the associated objects with respect to the input set.
 *
 *  INPUTS
 *      i    - The index number of the object in the SeqNr array
 *      chan - The channel (or catalog) number of the object
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      Retruns a filled set structure with one or more objects associated
 *      with the provided one.
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *     The routine relies on the presence of several global variables.
 *     Uses global variables:
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 12-09-1996
 *
 *****************************************************************************/
    set_struct	*t;
    int		pair, j, k, l, *m;

    t = set_init();

    ED_CALLOC(m, "set_pairs", int, control.ilist);

    pair = get_id(i, chan);
    VPRINTF(DEBUG,"Set_Pairs main: %d %d\n",pair, chan);
/*
 *  Check its pairs list for associations and get all top-level objects
 */
    m[chan] = pair;
    for (k=0;k<control.ilist;k++) {
       if (k != chan && (m[k] = get_pair(i, chan, k)) != 0) {
/*
 *        Here we have the top level other-channel pairs
 */
          l = get_index(m[k], k);
          if ((l = get_pair(l, k, chan)) != 0) {
             m[chan] = l;
          }
       }
    }
    for (k=0;k<control.ilist;k++) {
       j = m[k];
       do {
          if (j != 0) {
             l = get_index(j, k);
/*
 *           Add the associated object to the output set.
 *           If element is already in the set it is not added.
 */
             set_add_elem(t, get_id(l, k), k, get_fpos(l, k), l, NEW, TRUE);
             j = get_pair(l, k, k);
          }
       } while (j != m[k] && j != 0);
    }
    ED_FREE(m);
    return t;
}
/**/
static void mark_set_used(set_struct *s)
{
/****** mark_set_used ***************************************************
 *
 *  NAME	
 *      mark_set_used - Mark the objects as already used in a pairing
 *
 *  SYNOPSIS
 *      void mark_set_used(set_struct *s)
 *
 *  FUNCTION
 *      This function marks the objects of the set as already used in a
 *      pairing. The net result is they should not be used again.
 *
 *  INPUTS
 *      s  - The set of individual object measures
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
 *      E.R. Deul - dd 12-09-1996
 *
 *****************************************************************************/
    int		i;

    for (i=0;i<s->nelem;i++) {
       set_used(s->indx[i],s->chan[i]);
    }
}

/**/
static void save_set_ssc(set_struct *s, tabstruct *ssc_tab)
{
/***i** save_set_ssc ***************************************************
 *
 *  NAME	
 *      save_set_ssc - Save the set information into a single source
 *
 *  SYNOPSIS
 *      void save_set_ssc(set_struct *s, int *num)
 *
 *  FUNCTION
 *      This function merges all the information that is in the provided
 *      set into a single multi-color source.
 *
 *  INPUTS
 *      s       - The set of individual object measures
 *      ssc_tab - pointer to the ssc output table
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      The global data store of sources is updated with the new source
 *      information
 *
 *  NOTES
 *      Uses global variables:
 *        control
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 12-09-1996
 *
 *****************************************************************************/
    int		i, j, doread = TRUE;
    keystruct	**objkeys = NULL, *key, *tkey;

    if (control.make_pairs == YES && control.color_pairs == YES) {
      doread = FALSE;
      for (j=1;j<s->nelem;j++)
        if (s->chan[0] != s->chan[j]) {
          doread = TRUE;
          break;
        }
    }
    if (doread) objkeys = ssc_get_objects(s, ssc_tab->nkey);
    if (control.make_pairs == YES) {
/*
 *     Pairs file processing
 */
       if (s->nelem > 1 && doread) ssc_put_pairs(objkeys, s, ssc_tab);
    } else {
/*
 *     Merge file processing
 */
       ssc_put_row(objkeys, s, ssc_tab);
    }
    if (doread) {
       for (i=0;i<s->nelem;i++) {
          tkey = objkeys[i];
          while (tkey != NULL)
          {
               key = tkey;
               tkey = tkey->nextkey;
               free_key(key);
          };
       }
       ED_FREE(objkeys);
    }
}

/**/

static void process_object(int i, int chan, tabstruct *ssc_tab)
{
/****i* process_object ***************************************************
 *
 *  NAME	
 *      process_object - Process object that may have multiple associations
 *
 *  SYNOPSIS
 *      void process_object(int i, int chan)
 *
 *  FUNCTION
 *      This routine process an object that has been associated with
 *      several other objects, both in different frames and at different
 *      passbands.
 *
 *  INPUTS
 *      i         - Sequence number of the primary objects
 *      chan      - Channel number of the current object
 *
 *  RESULTS
 *      Object and its associates have either been written to the output
 *      file or are stored in the global retained objects store
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      The global retained objects is extended with new information or
 *      previous field information is removed.
 *
 *  NOTES
 *      Uses global variables:
 *         control
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 12-09-1996
 *
 ******************************************************************************/
    set_struct	*setA;

/*
 *  Start with the object under consideration
 */
    VPRINTF(VERBOSE,"Processing multi-object","\n");
    if (control.pair_col == NO)
       setA = cn_pairs(i, chan);
    else
       setA = set_pairs(i, chan);
/*
 *  So we do have all information here to output the single astronomical
 *  source data
 */
    save_set_ssc(setA, ssc_tab);
/*
 *  Mark all objects used
 */
    mark_set_used(setA);
    set_free(&setA);
}


/**/
int main(int argc, char *argv[])

/****** make_ssc ***************************************************
 *
 *  NAME	
 *      make_ssc - Create the final SSC catalog output
 *
 *  SYNOPSIS
 *      make_ssc -i incat1 incat2 ... -o outcat
 *              [-c configfile] [CONFOPTION]
 *
 *  FUNCTION
 *      This program applies the derived associations and produces a
 *      small sources catalog output file. The associations as stored
 *      in the input catalog are the ones derived by the association
 *      program. At this stage of the processing all calibration must have
 *      been applied.
 *
 *  INPUTS
 *      -i incat1 incat2 ...    - The list of input catalogs
 *      -o outcat               - The name of the partial Small Sources
 *                                output catalog file
 *     [-c configfile]          - The name of the configuration file. The
 *                                default name is /denisoft/conf/make_ssc.conf
 *     [CONFOPTION]             - Pairs of configuration VARIABLE VALUE
 *                                sequences that allow dynamically
 *                                overwritting of configuration file values.
 *
 *  RESULTS
 *
 *  RETURNS
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      Program still under development.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 15-10-1996
 *
 ******************************************************************************/
{
    int		i, j, ncols, *nobj;
    char	header[30000];

    sprintf(header,"%s Version: %s (%s)",PROGRAM,VERS,DATE);
    sprintf(header,"%-70.70s", header);
    if (!(ncols = init(argc, argv, header)))
       syserr(FATAL, "make_ssc", "No output columns defined!\n");

    for (i=1;i<argc;i++)
      sprintf(header, "%s %s", header, argv[i]);

    nobj = open_catalogs();
    ssc_tab = open_ssc_cat(control.outfile, tabin, ftabin, ncols, argc, argv);
/*
 *  Setup direct access read system
 */
    for (j=0;j<control.ilist;j++) {
       itabin[j] = init_readobj(tabin[j]);
    }

    init_count_status(0,0);
    for (i=0;i<control.ilist;i++) {
       for (j=0;j<nobj[i];j++) {
          if (!if_used(j, i))
	    {
             process_object(j, i, ssc_tab);
	    }
       }
    }

    close_ssc_cat(ssc_tab);
    close_catalogs(ssc_tab);

    return 0;

}
