#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "fitscat.h"
#include "options_globals.h"
#include "utils_globals.h"
#include "sort_globals.h"
#include "casts_globals.h"
#include "bin_search_globals.h"
#include "ssc_globals.h"
#include "ssc_utils_globals.h"
#include "chain_globals.h"

/* 30.01.2003:
   replaced some malloc by calloc calls to ensure
   an initialisation with 0 (subroutine make_indices()

   23.01.2005:
   I moved definitions from global variables
   (col_name, col_input, col_merge, col_chan)
   to avoid double definitions in the linking
   process.

   27.03.2006:
   I renamed sorting and indexing functions to the new naming scheme
   described in the sort_globals.h file
*/

extern control_struct control;

/*
 * defined and exported variables
 */
char **col_name;
char **col_input;
char **col_merge;
char **col_chan;

/*
 *  Global parameters for pairing
 */

static int		***p,	/* List of pairing tables */
			**id,	/* List of sequence numbers */
			**idx;	/* Index for sequence numbers per channel */
static int		*nobj;	/* List of number of elements */
static int		**cn;   /* List of cluster numbers */
static int		**cidx; /* Index of cluster numbers */
static short int	**fpos; /* List of field positions */

int *pre_pair(tabstruct **tabin)
{
/****** pre_pair ***************************************************
 *
 *  NAME	
 *      pre_pair - Get object information relevant to pairing
 *
 *  SYNOPSIS
 *      itn *pre_pair(tabstruct **tabin)
 *
 *  FUNCTION
 *      This routine reads information from the object catalogs that is
 *      required to recalculate the pairing information as derived by the
 *      association program.
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
 *      The global data stores for p, id, idx, nobj, fp, and ret are
 *      filled with the appropriate columns from the OBJECTS table from
 *      the different catalog files.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul dd - 10-10-1996
 *
 ******************************************************************************/
/*
 *  Read in the object sequence numbers and the pairing pointer tables
 */
    keystruct	***keys;
    char	**keynames;
    int		i, j;
/*
 *  Default PAIR_COL = NO, so when set to YES it was purposely done by
 *  the user. Leave it at that, but make sure the CLUSTER_NR column is
 *  there.
 *  When still at default, check for the CLUSTER_NR column and use it,
 *  otherwise switch to the Pair_X columns
 */
    if (control.pair_col == NO) {
      if (!name_to_key(tabin[0], control.cluster_nr)) {
        control.pair_col = YES;
        syserr(WARNING, "pre_pair",
         "No cluster number (%s) column in table.\n \
          Switching to Pair_X column processing\n\n",
          control.cluster_nr);
        if (!name_to_key(tabin[0], "Pair_0"))
          syserr(FATAL, "pre_pair",
            "No Pair_X columns in table.\n");
      }
    } else {
      if (!name_to_key(tabin[0], "Pair_0")) {
        control.pair_col = NO;
        syserr(WARNING, "pre_pair",
         "No Pair_X columns in table.\n \
         Switching to cluster number processing\n\n");
        if (!name_to_key(tabin[0], control.cluster_nr))
          syserr(FATAL, "pre_pair",
            "No cluster number (%s) column in table\n",
            control.cluster_nr);
      }
    }

/*
 *  Allocate global memory structure for direct access info
 */
    ED_MALLOC(id,  "pre_pair", int *,     control.ilist);
    ED_MALLOC(fpos,"pre_pair", short int *,control.ilist);
    ED_MALLOC(nobj,"pre_pair", int,       control.ilist);
    if (control.pair_col == YES) {
      ED_MALLOC(p  , "pre_pair", int **,    control.ilist);
      for (i=0;i<control.ilist;i++) {
        ED_MALLOC(p[i], "pre_pair", int *, control.ilist);
      }
    } else {
       ED_CALLOC(cn , "pre_pair", int *,    control.ilist);
    }
/*
 *  Dynamic allocation of keynames for variable pair list
 */
    ED_MALLOC(keynames, "pre_pair", char *, (control.ilist+NKEYS));
    for (i=0;i<control.ilist+NKEYS;i++)
       ED_CALLOC(keynames[i], "pre_pair", char, MAXCHAR);
    strcpy(keynames[0], control.seqnr);
    strcpy(keynames[1], control.fieldpos);
    if (control.pair_col == YES) {
       for (i=0;i<control.ilist;i++)
          sprintf(keynames[NKEYS+i], "Pair_%-d", i);
    } else {
       strcpy(keynames[2], control.cluster_nr);
    }
/*
 *  And now read the actual columns from the different input catalogs
 */
    ED_MALLOC(keys, "pre_pair", keystruct **, control.ilist);
    for (i=0;i<control.ilist;i++) {
       VPRINTF(NORMAL,"Reading catalog %s\n",control.listfile[i]);
       ED_MALLOC(keys[i], "pre_pair", keystruct *,
                 (NKEYS+control.ilist));
       read_keys(tabin[i], keynames, keys[i], NKEYS+control.ilist, NULL);
/*
 *     Check the regular fields
 */
       for (j=0;j<NKEYS-1;j++) {
          if (j != 1)
             if (!keys[i][j])
                syserr(FATAL,"pre_pair", "No key %s in table %s\n",
                    keynames[j], tabin[i]);
       }
       id[i] = keys[i][0]->ptr;
       nobj[i] = keys[i][0]->nobj;

       if (keys[i][1])
          fpos[i] = keys[i][1]->ptr;
       else
          ED_CALLOC(fpos[i], "pre_pair", short int, nobj[i]);
/*
 *     Check either Pair_X or CLUSTER_NR
 */
       if (control.pair_col == YES) {
          for (j=NKEYS;j<NKEYS+control.ilist;j++) {
             if (!keys[i][j]) {
                syserr(FATAL,"pre_pair", "No key %s in table %s\n",
                    keynames[j], tabin[i]);
             } else {
               p[i][j-NKEYS]  = keys[i][j]->ptr;
             }
          }
       } else {
          if (!keys[i][NKEYS-1]) {
             syserr(FATAL,"pre_pair", "No key %s in table %s\n",
                    keynames[NKEYS-1], tabin[i]);
          } else {
             cn[i] = keys[i][NKEYS-1]->ptr;
          }
       }
    }
    for (i=0;i<control.ilist+NKEYS;i++)
       ED_FREE(keynames[i]);
    for (i=0;i<control.ilist;i++)
       ED_FREE(keys[i]);
    ED_FREE(keynames);
    ED_FREE(keys);
    return nobj;
}
/**/

void make_indices()
{
/****i* make_indices ***************************************************
 *
 *  NAME	
 *      make_indices - Create index arrays for Seqnr and field pos
 *
 *  SYNOPSIS
 *      void make_indices()
 *
 *  FUNCTION
 *      Routine to create the index arrays for sequence number and field
 *      position for all the objects in the strip. This allows for more
 *      rapid (binary search) access to the data.
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
 *      The idx list that provides sequence number sorting is created.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 10-10-1996
 *
 ******************************************************************************/
    int		i;

/*
 *  First walk along the color and mark all objects paired with another.
 *  But make an index first because the ordering of the catalog is not
 *  defined to be sorted on SeqNr.
 */
    ED_CALLOC(idx,  "identify_pairs", int *, control.ilist);
    for (i=0;i<control.ilist; i++) {
       VPRINTF(NORMAL,"Indexing channel %d\n", i);
       ED_CALLOC(idx[i],  "identify_pairs", int, nobj[i]);
       iindex(nobj[i], id[i],  idx[i]);
    }
    if (control.pair_col == NO) {
       ED_CALLOC(cidx, "identify_pairs", int *, control.ilist);
       for (i=0;i<control.ilist; i++) {
          ED_CALLOC(cidx[i], "identify_pairs", int, nobj[i]);
          iindex(nobj[i], cn[i], cidx[i]);
       }
    }
}
/**/
int get_index(int seqnr, int chan)
{
/****** get_index ***************************************************
 *
 *  NAME	
 *      get_index - Get the main object index for the seqnr/chan pair
 *
 *  SYNOPSIS
 *      int get_index(int seqnr, int chan)
 *
 *  FUNCTION
 *      This function derives the index of the objects specified by
 *      sequence number and channel number.
 *
 *  INPUTS
 *      seqnr - The sequence number of the object
 *      chan  - The channel number of the object
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The index to the object, or -1 in case it does not exist
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 06-11-1996
 *
 ******************************************************************************/
    int l, lbeg, lend;

    seqnr = (int) fabs(seqnr);
    ibin_search_indexed(nobj[chan], id[chan], idx[chan], seqnr, -3,
                        &lbeg, &lend, 0);
    for (l=lbeg;l<=lend;l++) {
       if ((int)fabs(id[chan][l]) == seqnr) {
          return l;
       }
    }
    syserr(FATAL, "get_index", "Could not find object %d chan %d\n", seqnr, chan);
    return -1;
}

/**/
void set_used(int n, int chan)
{
    id[chan][n] = -id[chan][n];
}

/**/
int if_used(int n, int chan)
{
    int retcode = FALSE;

    if (id[chan][n] < 0 )
       retcode = TRUE;

    return retcode;
}

/**/
int get_pair(int n, int chan1, int chan2)
{
/****** get_pair ***************************************************
 *
 *  NAME	
 *      get_pair - Get the pair seqence number for the index/chan1/chan2 triple
 *
 *  SYNOPSIS
 *      int get_pair(int n, int chan1, int chan2)
 *
 *  FUNCTION
 *      This function returns the sequence number of the pair objects
 *      to the index/chan1/chan2 triplet pairing information
 *
 *  INPUTS
 *      n     - The index number of the object
 *      chan1 - The channel number of the originating object
 *      chan2 - The channel number of the paired object
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The sequence number of the paired object is returned
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 07-11-1996
 *
 ******************************************************************************/
     return (p[chan1][chan2][n]);
}

/**/
int get_id(int n, int chan)
{
/****** get_id ***************************************************
 *
 *  NAME	
 *      getid - Get the seqence number of the object with index/chan tuplet
 *
 *  SYNOPSIS
 *      int get_id(int n, int chan)
 *
 *  FUNCTION
 *      This function returns the sequence number of the object pointed to
 *      by the index and channel numbers provided
 *
 *  INPUTS
 *      n    - The index number of the object
 *      chan - The channel number of the object
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The sequence number of the object is returned
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 07-11-1996
 *
 ******************************************************************************/
     return ((int) fabs(id[chan][n]));
}
int get_fpos(int n, int chan)
{
     return ((int) fpos[chan][n]);
}
/**/
set_struct *cn_pairs(int i, int chan)
/****** cn_pairs ***************************************************
 *
 *  NAME	
 *      cn_pairs - Create a set structure with all pairs of current object
 *
 *  SYNOPSIS
 *      static set_struct *cn_pairs(int i, int chan)
 *
 *  FUNCTION
 *      Find all the objects in memory that have the same cluster number
 *      as the object denoted by the parameters
 *
 *  INPUTS
 *      i    - The index number of the object in the SeqNr array
 *      chan - The channel (or catalog) number of the object
 *
 *  RESULTS
 *      Retruns a filled set structure with one or more objects associated
 *      with the provided one.
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
 *      E.R. Deul - dd 27-08-1999
 *
 *****************************************************************************/
{
    set_struct	*t;
    int		pair, k;
    int		jj, jj_beg, jj_end;
    int		cur_cn;

    t = set_init();

    pair = get_id(i, chan);
    VPRINTF(DEBUG,"Set_Pairs main: %d %d\n",pair, chan);
/*
 *  Find the current object in the main list and add it to the output set
 */
    cur_cn = cn[chan][i];

    if (cur_cn) {
/*
 *    Current object has a non-zero cluster number so find out were the
 *    range of similar cluster number in all channel (catalogs) are
 */
      for (k=0;k<control.ilist;k++) {
        ibin_search_indexed(nobj[k],cn[k],cidx[k],cur_cn,1,&jj_beg,&jj_end,0);

        for (jj=jj_beg;jj<=jj_end;jj++)
          if (cn[k][cidx[k][jj]] == cur_cn)
            set_add_elem(t, get_id(cidx[k][jj], k), k,
                            get_fpos(cidx[k][jj], k),
                            cidx[k][jj], NEW, TRUE);
      }
    } else {
      set_add_elem(t, pair, chan, get_fpos(i, chan), i, NEW, TRUE);
    }
    return t;
}
