/*
 * Processing in terms of object sets
 */
#include <stdlib.h>
#include <math.h>
#include "obj_sets_globals.h"
#include "options_globals.h"
#include "utils_globals.h"
#include "utils_macros.h"

static int set_check_elem(set_struct *s, int seqnr, int chan)
{
/****i* set_check_elem ***************************************************
 *
 *  NAME	
 *      set_check_elem - Check for a specific element in the set
 *
 *  SYNOPSIS
 *      int *set_check_elem(set_struct *s, int seqnr, int chan)
 *
 *  FUNCTION
 *      The function checks if a specified object is present in the
 *      given set.
 *
 *  INPUTS
 *      seqnr - The sequence number of the object to be found
 *      chan  - The channel number of the object to be found
 *      s     - The set in which the object is to be found
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      TRUE  - If the object is member of the set
 *      FALSE - If the object is not present in the set
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 18-09-1996
 *
 ******************************************************************************/
    int		i, retcode = -1;

    for (i=0;i<s->nelem;i++) {
       if (s->seqnr[i] == seqnr && s->chan[i] == chan) {
          retcode = i;
          break;
       }
    }
    return retcode;
}
/**/
int set_add_elem(set_struct *s, int seqnr, int chan, int fpos, int indx,
                 short int ret, int do_check)
{
/****i* set_add_elem ***************************************************
 *
 *  NAME	
 *      set_add_elem - Add an element to the specified set
 *
 *  SYNOPSIS
 *      int *set_add_elem(set_struct *s, int seqnr, int chan, int fpos,
 *                        int indx, short int ret)
 *
 *  FUNCTION
 *      The function adds a given object to the specified set. If necessary
 *      it adds memory allocation to the set when required.
 *
 *  INPUTS
 *      s        - The set to which the object is to be added
 *      seqnr    - The sequence number of the object to be added
 *      chan     - The channel number of the object to be added
 *      fpos     - The field number of the object to be added
 *      indx     - The index to the main seqnr/chan store
 *      ret      - The retained status of the object to be added
 *      do_check - Perform set checking TRUE or FALSE
 *
 *  RESULTS
 *      s        - The modified set of objects
 *
 *  RETURNS
 *      The number of elements in the set after adding the object
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 18-09-1996
 *
 ******************************************************************************/
/*
 *  Do not add the element if it is already in the set
 */
    if (do_check) {
       if (set_check_elem(s, seqnr, chan) >= 0) {
          return s->nelem;
       }
    }
    s->nelem++;
    if (s->nalloc == s->nelem) {
       s->nalloc *= 1.5;
       ED_REALLOC(s->seqnr, "set_add_elem", int      , s->nalloc);
       ED_REALLOC(s->chan,  "set_add_elem", int      , s->nalloc);
       ED_REALLOC(s->fpos,  "set_add_elem", int      , s->nalloc);
       ED_REALLOC(s->indx,  "set_add_elem", int      , s->nalloc);
       ED_REALLOC(s->ret,   "set_add_elem", short int, s->nalloc);
    }

    s->seqnr[s->nelem-1] = (int) fabs(seqnr);
    s->chan[s->nelem-1]  = chan;
    s->fpos[s->nelem-1]  = fpos;
    s->indx[s->nelem-1]  = indx;
    s->ret[s->nelem-1]   = ret;

    return s->nelem;
}
/**/
set_struct *set_init()
/***** set_init ***************************************************
 *
 *  NAME	
 *      set_init - Initialize the set structure
 *
 *  SYNOPSIS
 *      set_struct *set_init()
 *
 *  FUNCTION
 *      The routine creates the internal datastructure for the set
 *      implementation.
 *
 *  INPUTS
 *      None
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The pointer to the set structure is returned
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      The datastructure is actually one element larger than the number
 *      of elements element says. This is to avoid NULL problems when
 *      filling the structure with data.
 *
 *  BUGS
 *
 *  AUTHOR
 *     E.R. Deul  - dd 12-09-199
 *
 ******************************************************************************/
{
    set_struct *s;

    ED_MALLOC(s,        "set_init", set_struct, 1);
    s->nalloc = 20;
    ED_MALLOC(s->seqnr, "set_init", int,        s->nalloc);
    ED_MALLOC(s->chan,  "set_init", int,        s->nalloc);
    ED_MALLOC(s->fpos,  "set_init", int,        s->nalloc);
    ED_MALLOC(s->indx,  "set_init", int,        s->nalloc);
    ED_MALLOC(s->ret,   "set_init", short int,  s->nalloc);

    s->nelem = 0;

    return s;
}
/**/
void set_free(set_struct **s)
{
/****** set_free ***************************************************
 *
 *  NAME	
 *      set_free - Free the data structure associated with a given set
 *
 *  SYNOPSIS
 *      void set_free(set_struct **s)
 *
 *  FUNCTION
 *      The function removes the complete datastructure from the memeory
 *      and returns a NULL pointer on success.
 *
 *  INPUTS
 *      s - The set structure to be removed frm memory
 *
 *  RESULTS
 *      s - Set to NULL
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      The data structure representation is one element larger than what
 *      the nelem structure element says.
 *
 *  BUGS
 *
 *  AUTHOR
 *     E.R. Deul - dd 18-09-1996
 *
 ******************************************************************************/
    set_struct *t;

    if ((t = *s) != NULL) {
       ED_FREE(t->seqnr);
       ED_FREE(t->chan);
       ED_FREE(t->fpos);
       ED_FREE(t->indx);
       ED_FREE(t->ret);
       ED_FREE(t);
    }
}
/**/
set_struct *set_copy(set_struct *s)
{
/****** set_copy ************************************************************
 *
 *  NAME	
 *      set_copy - Copies the provided set and outputs the newly created set
 *
 *  SYNOPSIS
 *      set_struct *set_copy(set_struct *s)
 *
 *  FUNCTION
 *      The routine copies the content of the provided set by making a new
 *      set structure and copying every element from the provided set into
 *      the new one. It returns a pointer to this new structure.
 *
 *  INPUTS
 *      s - The set to be copied
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      A pointer to the copy of the provided set
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 18-09-1996
 *
 ******************************************************************************/
    set_struct	*t;
    int		i;

    t = set_init();
    for (i=0;i<s->nelem;i++) {
       set_add_elem(t, s->seqnr[i], s->chan[i], s->fpos[i], s->indx[i],
                    s->ret[i], FALSE);
    }

    return t;
}
/**/
int empty_set(set_struct *s)
{
/****** empty_set ***************************************************
 *
 *  NAME	
 *      empty_set - Check if the specified set has no elements
 *
 *  SYNOPSIS
 *      int empty_set(set_struct *s)
 *
 *  FUNCTION
 *      Check if the set is an empty set, thus without any elements
 *
 *  INPUTS
 *      s - The set to be checked
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      TRUE  - If the set has no elements
 *      FALSE - IF the set has one or more elements
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *     The set needs to be initialized before a call to this routine
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 18-09-1996
 *
 ******************************************************************************/
    if (s->nelem == 0)
       return TRUE;
    else
       return FALSE;
}
/**/
int set_get_elem(set_struct *a, int *l_seqnr, int *l_chan, int *l_fpos,
                                int *l_indx, short int *l_ret)
{
/****** set_get_elem ***************************************************
 *
 *  NAME	
 *      set_get_elem - Get the specified element from the set
 *
 *  SYNOPSIS
 *      set_get_elem(set_struct *a, int *l_seqnr, int *l_chan, int *l_fpos,
 *                                  int *l_indx, short int *l_ret)
 *
 *  FUNCTION
 *      This routine obtains associated information for a given element
 *      as specified by it sequence number and channel number from the
 *      provided set. If the element was not present the routine returns
 *      with FALSE.
 *
 *  INPUTS
 *      a       - The set to be searched
 *      l_seqnr - The sequence number of the element to be found
 *      l_chan  - The channel number of the element to be found
 *
 *  RESULTS
 *      l_fpos  - The field number of the element found
 *      l_indx  - The data store index for the element found
 *      l_ret   - The retained logical value
 *
 *  RETURNS
 *      TRUE  If the object is found in the provided set
 *      FALSE If the object is not found in the provided set
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *     E.R. Deul - dd 13-09-1996
 *
 ******************************************************************************/
    int		i;

    for (i=0;i<a->nelem;i++) {
       if ((int) fabs(a->seqnr[i]) == *l_seqnr &&
           a->chan[i]  == *l_chan) {
          *l_fpos  = a->fpos[i];
          *l_indx  = a->indx[i];
          *l_ret   = a->ret[i];
          return TRUE;
       }
    }
    syserr(WARNING,"set_get_elem", "No element %d chan %d in list\n", *l_seqnr, *l_chan);
    return FALSE;
}
/**/
set_struct *set_diff(set_struct *b, set_struct *a)
{
/****** set_diff ***************************************************
 *
 *  NAME	
 *      set_diff - Obtain the difference set B - A
 *
 *  SYNOPSIS
 *      set_struct *set_diff(set_struct *b, set_struct *a)
 *
 *  FUNCTION
 *      The routine creates an output set that is the difference between
 *      the two input sets. It output all elements from b that are not
 *      present in set a. Thus deriving B - A.
 *
 *  INPUTS
 *      b - The main input set
 *      a - The input set to be subtracted from b
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The output set that is the result from the B - A operation
 *
 *  MODIFIED GLOBALS
 *      None
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 12-09-1996
 *
 ******************************************************************************/
    set_struct	*t;
    int		i;

    t = set_init();

    for (i=0; i<b->nelem; i++) {
       if (set_check_elem(a, b->seqnr[i], b->chan[i]) < 0) {
          set_add_elem(t, b->seqnr[i], b->chan[i],
                          b->fpos[i], b->indx[i], b->ret[i],
                          FALSE);
       }
    }

    return t;
}
/**/
set_struct *set_union(set_struct *a, set_struct *b)
{
/****** set_union ***************************************************
 *
 *  NAME	
 *      set_union - Derive the union of two sets A and B
 *
 *  SYNOPSIS
 *      set_struct *set_union(set_struct *a, set_struct *b)
 *
 *  FUNCTION
 *      This function determines the union of two sets a dand b.
 *      It returns all elements from set a and set b, with the
 *      elements that are in both sets returned only once.
 *
 *  INPUTS
 *      a - The first set
 *      b - The second set
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The set with the union of set a and b.
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
 ******************************************************************************/
    set_struct	*t;
    int		i;

    t = set_init();
/*
 *  First put all the set B element into the output set
 */
    for (i=0;i<b->nelem;i++) {
       set_add_elem(t, b->seqnr[i], b->chan[i], b->fpos[i], b->indx[i],
                       b->ret[i], FALSE);
    }
/*
 *  Then put all the set A element into the output set. set_add_elem does
 *  not add if the element already exists in the output set.
 */
    for (i=0;i<a->nelem;i++) {
       set_add_elem(t, a->seqnr[i], a->chan[i], a->fpos[i], a->indx[i],
                       a->ret[i], TRUE);
    }

    return t;
}
/**/
set_struct *set_eq_fpos(set_struct *s, int field)
{
/****** set_eq_fpos ***************************************************
 *
 *  NAME	
 *      set_eq_fpos - Extract the set from s that has fieldnumber field
 *
 *  SYNOPSIS
 *      set_struct *set_eq_fpos(set_struct *s, int field)
 *
 *  FUNCTION
 *      This routine extracts all elements from the set s for which the
 *      fpos element is set to the value of the input parameter field.
 *
 *  INPUTS
 *      s      - The input set
 *      field  - The value of the field parameter
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The set extract of s with fpos equals to field
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
 ******************************************************************************/
    set_struct	*t;
    int		i;

    t = set_init();
    for (i=0;i<s->nelem;i++) {
       if (s->fpos[i] == field)
          set_add_elem(t, s->seqnr[i], s->chan[i], s->fpos[i], s->indx[i],
                          s->ret[i], FALSE);
    }

    return t;
}
/**/
set_struct *set_eq_ret(set_struct *s, int code)
{
/****** set_eq_ret ***************************************************
 *
 *  NAME	
 *      set_eq_ret - Extract the set from s with retained equal to code
 *
 *  SYNOPSIS
 *      set_struct *set_eq_ret(set_struct *s, int code)
 *
 *  FUNCTION
 *      This routine extracts all elements from the set s for which the
 *      ret parameter is equal to the value of code.
 *
 *  INPUTS
 *      s    - Input set
 *      code - The value of the retained parameter
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The set of elements with ret equals to code
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *     E.R. Deul - dd 12=09-1996
 *
 ******************************************************************************/
    set_struct	*t;
    int		i;

    t = set_init();
    for (i=0;i<s->nelem;i++) {
        if (s->ret[i] == code)
           set_add_elem(t, s->seqnr[i], s->chan[i], s->fpos[i], s->indx[i],
                           s->ret[i], FALSE);
    }

    return t;
}
/**/
set_struct *set_ne_ret(set_struct *s, int code)
{
/****** set_eq_ret ***************************************************
 *
 *  NAME	
 *      set_eq_ret - Extract the set from s with retained not equal to code
 *
 *  SYNOPSIS
 *      set_struct *set_ne_ret(set_struct *s, int code)
 *
 *  FUNCTION
 *      This routine extracts all elements from the set s for which the
 *      ret parameter is NOT equal to the value of code.
 *
 *  INPUTS
 *      s    - Input set
 *      code - The value of the retained parameter
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The set of elements with ret not equals to code
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *     E.R. Deul - dd 10-12-1996
 *
 ******************************************************************************/
    set_struct	*t;
    int		i;

    t = set_init();
    for (i=0;i<s->nelem;i++) {
        if (s->ret[i] != code)
           set_add_elem(t, s->seqnr[i], s->chan[i], s->fpos[i], s->indx[i],
                           s->ret[i], FALSE);
    }

    return t;
}
/**/
int set_equal(set_struct *a , set_struct *b)
{
/****** set_equal ***************************************************
 *
 *  NAME	
 *      set_equal - Check if two sets are identical
 *
 *  SYNOPSIS
 *      int set_equal(set_struct *a , set_struct *b)
 *
 *  FUNCTION
 *      This routine verifies that two sets are identical by verifying
 *      that all elements from set a are in set b and visa-versa.
 *
 *  INPUTS
 *      a - First input set
 *      b - Second input set
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      TRUE  - If both sets are equal
 *      FALSE - If the sets differ
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
 ******************************************************************************/
    int		i;
/*
 *  Two sets can never be equal if they have a differnt number of elements
 */
    if (a->nelem != b->nelem) return FALSE;

/*
 *  See if each element from set a is in set b. If this is so and the above
 *  holds (a->nelem = b->nelem) then both sets are equal
 */
    for (i=0;i<a->nelem;i++) {
       if (set_check_elem(b, a->seqnr[i], a->chan[i])<0) return FALSE;
    }
    return TRUE;
}
/**/
int set_partoff(set_struct *a, set_struct *b)
{
/****** set_partoff ***************************************************
 *
 *  NAME	
 *      set_partoff - Check if set b is fully contained in set a
 *
 *  SYNOPSIS
 *      int set_partoff(set_struct *a, set_struct *b)
 *
 *  FUNCTION
 *      This function veryfies if all elements of set b are present in
 *      set a.
 *
 *  INPUTS
 *      a - Input set that is the larger of the two
 *      b - Input set that should be part of a
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      TRUE  - In case set b if fully contained in set a
 *      FALSE - If one of more elements from b are not present in set a
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
 ******************************************************************************/
    int		i;

    for (i=0;i<b->nelem;i++) {
       if (set_check_elem(a, b->seqnr[i], b->chan[i])<0) return FALSE;
    }
    return TRUE;
}
