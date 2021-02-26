/****** chain ***************************************************
 *
 *  NAME	
 *      chain - Implementation of single linked chain with free data format
 *
 *  SYNOPSIS
 *
 *  FUNCTION
 *      The routines from this package implement a single linked chain
 *      structure that allows for a free data format
 *
 *  INPUTS
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
 *      E.R Deul - dd 19-09-1996
 *
 ******************************************************************************/

/* HISTORY COMMENTS

   30.12.2012:
   I removed compiler warnings by removing not used variables
*/


#include <string.h>
#include "chain_defs.h"
#include "chain_globals.h"
#include "utils_globals.h"
#include "casts_globals.h"
#include "utils_macros.h"

chain_struct	*chain_init()
{
/****** chain_init ***************************************************
 *
 *  NAME	
 *      chain_init - Initialize the chain structure
 *
 *  SYNOPSIS
 *      chain_struct *chain_init()
 *
 *  FUNCTION
 *      Allocate the memory structure for a new chain structure
 *
 *  INPUTS
 *      None
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      A pointer to the newly created structure
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      No memory is allocated for the description and the data part.
 *      The pointers to those non-existing structures are set to NULL
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 19-09-1996
 *
 ******************************************************************************/
    chain_struct	*c;

    ED_CALLOC(c, "chain_init", chain_struct, 1);
    c->desc.names = NULL;
    c->desc.types = NULL;
    c->desc.bpos  = NULL;
    c->desc.nelem = 0;
    c->desc.bsize = 0;
    c->chain = NULL;

    return c;
}

int chain_free(chain_struct *c)
{
/****** chain_free ***************************************************
 *
 *  NAME	
 *      chain_free - Free the memory structure for a complete chain
 *
 *  SYNOPSIS
 *      void chain_free(chain_struct **c)
 *
 *  FUNCTION
 *      The routine deallocates the memory structure of a complete chain.
 *      It steps down the chain removing all elements in that chain. On
 *      exit a NULL pointer is return on success.
 *
 *  INPUTS
 *      c - The chain structure to be free-ed from memory
 *
 *  RESULTS
 *      c - Is set to NULL on success, left to the original value on failure
 *
 *  RETURNS
 *      TRUE  - If the deallocation was successful
 *      FALSE - If an error occured during deallocation
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 20-09-1996
 *
 ******************************************************************************/
    chain_elem	*elem, *t_elem;
    int		i;

    elem = c->chain;
/*
 *  First delete the data elements from the chain
 */
    while (elem != NULL) {
       t_elem = elem;
       elem = elem->next;
       ED_FREE(t_elem->bits);
       ED_FREE(t_elem);
    }
/*
 *  Second delete the description structure
 */
    for (i=0;i<c->desc.nelem;i++)  {
       ED_FREE(c->desc.names[i]);
    }
    ED_FREE(c->desc.names);
    ED_FREE(c->desc.types);
    ED_FREE(c->desc.bpos);
/*
 *  Last delete the chain main structure
 */
    ED_FREE(c->chain);
    ED_FREE(c);

    return TRUE;
}

int chain_add_desc_item(chain_struct *c, char *name, ttype type)
{
/****** chain_add_desc_item ***************************************************
 *
 *  NAME	
 *      chain_add_desc_item - Add an item to the chain description
 *
 *  SYNOPSIS
 *      int chain_add_desc_item(chain_desc *c, char *name, ttype type)
 *
 *  FUNCTION
 *      Adds the description of a new item to the chain description.
 *
 *  INPUTS
 *      c    - Pointer to the chain structure to which to add the item
 *      name - The name of the item to be added (must be unique)
 *      type - The internal representation type of the element to be added
 *
 *  RESULTS
 *      c    - The chain structure's desciption is modified (not the data)
 *
 *  RETURNS
 *      TRUE  - If the addition was successful
 *      FALSE - If the addition was not successful, e.g. because the
 *              element description already existed, or there is no
 *              more memory for allocation.
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 20-09-1996
 *
 ******************************************************************************/
    chain_desc	*desc;

    desc = &c->desc;
/*
 *  First allocate the required memory sections
 */
    if (desc->nelem == 0) {
       desc->nelem = 1;
       ED_CALLOC(desc->names, "chain_add_desc_item", char *, desc->nelem);
       ED_CALLOC(desc->types, "chain_add_desc_item", ttype,  desc->nelem);
       ED_CALLOC(desc->bpos , "chain_add_desc_item", int,    desc->nelem);
    } else {
       desc->nelem++;
       ED_REALLOC(desc->names, "chain_add_desc_item", char *, desc->nelem);
       ED_REALLOC(desc->types, "chain_add_desc_item", ttype,  desc->nelem);
       ED_REALLOC(desc->bpos,  "chain_add_desc_item", int,    desc->nelem);
    }
    ED_CALLOC(desc->names[desc->nelem-1], "chain_add_desc_item", char, NAMLEN);
/*
 *  Now fill the appropriate elements with the correct values
 */
    strcpy(desc->names[desc->nelem-1], name);
    desc->types[desc->nelem-1] = type;
    desc->bsize += c_size[type];
    if (desc->nelem != 1) {
       type = desc->types[desc->nelem-2];
       desc->bpos[desc->nelem-1] = desc->bpos[desc->nelem-2] + c_size[type];
    } else {
       desc->bpos[desc->nelem-1] = 0;
    }
    return TRUE;
}

int chain_get_desc_item(chain_desc d, char *name)
{
/****** chain_get_desc_item ***************************************************
 *
 *  NAME	
 *      chain_get_desc_item - Get the pointer to a description item
 *
 *  SYNOPSIS
 *      int chain_get_desc_item(chain_desc d, char *name)
 *
 *  FUNCTION
 *      This function return the index in the description structure of
 *      the element whose name was specified on the call
 *
 *  INPUTS
 *      d    - The description structure of the chain
 *      name - The unique name of the elemnt to be found
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      Integer value of the index to the element found
 *      Negative value is return upon unsuccessful search
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R Deul - dd 20-09-1996
 *
 ******************************************************************************/
    int		i;

    for (i=0;i<d.nelem; i++) {
       if (strstr(d.names[i], name) != NULL) {
          return i;
       }
    }
    return -1;
}

void display_data_elems(chain_struct *c, char **name, int n)
{
    chain_elem          *elem, *t_elem;
    int                 i, j;
    int                 *name_indx;
    union {
       char  c[2];
       short v;
    } shrt;
    union {
       char  c[4];
       long  v;
    } lng;

/*
 *  Find the index for the names elements and limit allowed matching to
 *  simple int's
 */
    ED_MALLOC(name_indx, "chain_get_data_elem", int, n);
    for (i=0;i<n;i++ ) {
       if ((name_indx[i] = chain_get_desc_item(c->desc, name[i])) < 0) {
          syserr(FATAL,"No element with name %s in chain\n", name[i]);
       }
       if (c->desc.types[name_indx[i]] != C_SHORT &&
           c->desc.types[name_indx[i]] != C_LONG) {
          syserr(FATAL, "chain_get_data_elem",
                     "Current implementation for integer type only\n");
       }
    }
/*
 *  Step along the chain and find the element that matches the condition
 */
    elem = t_elem = c->chain;
    while (elem != NULL) {
       t_elem = elem;
       elem = elem->next;

       for (i=0;i<n;i++) {
          for (j=0;j<c_size[c->desc.types[name_indx[i]]];j++) {
              switch (c->desc.types[name_indx[i]]) {
              case C_SHORT: shrt.c[j] =
                            t_elem->bits[c->desc.bpos[name_indx[i]]+j];
                            break;
              case C_LONG:  lng.c[j] =
                            t_elem->bits[c->desc.bpos[name_indx[i]]+j];
                            break;
              case C_BYTE:
              case C_FLOAT:
              case C_DOUBLE:
              case C_STRING:
                            break;
              }
           }
           switch (c->desc.types[name_indx[i]]) {
           case C_SHORT: printf("%d ",shrt.v);
                         break;
           case C_LONG:  printf("%ld ",lng.v);
                         break;
           case C_BYTE:
           case C_FLOAT:
           case C_DOUBLE:
           case C_STRING:
                         break;
           }
       }
       printf("\n");
    }
    ED_FREE(name_indx);
    return;
}

chain_elem *chain_get_data_elem(chain_struct *c, char **name, char **val,
                                int n)
{
/****** chain_get_data_elem ***************************************************
 *
 *  NAME	
 *      chain_get_data_elem - Get data element that matches the condition
 *
 *  SYNOPSIS
 *      chain_elem *chain_get_data_elem(chain_struct *c, char **name,
 *                                       char **val, int n)
 *
 *  FUNCTION
 *      Return the chain element that adheres to the matching condition
 *      One can provide a list of names and values in order to create a
 *      condition that will match only one element. The first matching
 *      element as stored in the chain is returned!
 *      Current implementation is restricted to integer type of retrieval
 *
 *  INPUTS
 *      c    - Pointer to the chain structure to be searched
 *      name - A list of names of the elements to be checked
 *      val  - A list of values of the elements to be returned
 *      n    - The number of items in the list
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The element that matches the conditions given by the arguments.
 *      If no elements match a NULL is returned
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      A pointer to the data is returned, not a new copy of the chain
 *      structure
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 20-09-1996
 *
 ******************************************************************************/
    chain_elem		*elem, *t_elem;
    int			i, j, ok;
    int			*name_indx;

/*
 *  Find the index for the names elements and limit allowed matching to
 *  simple int's
 */
    ED_MALLOC(name_indx, "chain_get_data_elem", int, n);
    for (i=0;i<n;i++ ) {
       if ((name_indx[i] = chain_get_desc_item(c->desc, name[i])) < 0) {
          syserr(FATAL,"No element with name %s in chain\n", name[i]);
       }
       if (c->desc.types[name_indx[i]] != C_SHORT &&
           c->desc.types[name_indx[i]] != C_LONG) {
          syserr(FATAL, "chain_get_data_elem",
                     "Current implementation for integer type only\n");
       }
    }
/*
 *  Step along the chain and find the element thta matches the condition
 */
    elem = t_elem = c->chain;
    while (elem != NULL) {
       t_elem = elem;
       elem = elem->next;
       ok = TRUE;
       for (i=0;i<n;i++) {
          for (j=0;j<c_size[c->desc.types[name_indx[i]]];j++) {
             if (t_elem->bits[c->desc.bpos[name_indx[i]]+j] !=
                       val[i][j]) {
                ok = FALSE;
                break;
             }
          }
       }
       if (ok) {
          ED_FREE(name_indx);
          return t_elem;
       }
    }
    ED_FREE(name_indx);
    return NULL;
}

int chain_add_data_elem(chain_struct *c, chain_struct *t)
{
/****** chain_add_data_elem ***************************************************
 *
 *  NAME	
 *      chain_add_data_elem - Add a chain element to the chain
 *
 *  SYNOPSIS
 *      int chain_add_data_elem(chain_struct *c, chain_struct *t)
 *
 *  FUNCTION
 *      Add the information given in the argument list to the current chain.
 *      The description of the provided chain is checked against the one
 *      provided with the call. In the bvals data area, the data need not be
 *      ordered in the same ordering as in the provided chain.
 *
 *  INPUTS
 *      c     - The chain to which to add the information
 *      t     - The chain element to be added to the chain
 *
 *  RESULTS
 *      c     - The update chain structure
 *
 *  RETURNS
 *      TRUE  - If the addition process was successful
 *      FALSE - If the addition process failed, due to incompatibility
 *              of the two description structures, or due to lack
 *              of memory
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 20-09-1996
 *
 ******************************************************************************/
    int 		i, j, *idx;
    chain_elem		*elem, *n_elem, *tt_elem;
/*
 *  First check if all element of the input data exist in the output chain
 *  or just create the output chain
 */
    if (c->desc.nelem == 0) {
       ED_CALLOC(idx, "chain_add_data_elem", int, t->desc.nelem);
       for (i=0;i<t->desc.nelem;i++) {
          chain_add_desc_item(c, t->desc.names[i], t->desc.types[i]);
          idx[i] = i;
       }
    } else {
       if (t->desc.nelem != c->desc.nelem) {
          syserr(WARNING, "chain_add_data_elem",
                  "Description structures differ\n");
          return FALSE;
       }
       ED_CALLOC(idx, "chain_add_data_elem", int, t->desc.nelem);
       for (i=0;i<t->desc.nelem;i++) {
          if ((idx[i] = chain_get_desc_item(c->desc, t->desc.names[i])) < 0) {
             syserr(WARNING, "chain_add_data_elem",
                     "No element %s in output chain\n", t->desc.names[i]);
             ED_FREE(idx);
             return FALSE;
          }
       }
    }
/*
 *  Now copy the information to the chain structure in the right order to
 *  the beginning of the chain
 */
    elem = c->chain;
    ED_CALLOC(n_elem,       "chain_add_data_elem", chain_elem, 1);
    ED_CALLOC(n_elem->bits, "chain_add_data_elem", char,       t->desc.bsize);
    n_elem->next = elem;
    if (elem != NULL) elem->prev = n_elem;
    c->chain = n_elem;
    tt_elem = t->chain;
    for (i=0;i<t->desc.nelem;i++) {
       for (j=0;j<c_size[t->desc.types[i]];j++) {
          n_elem->bits[c->desc.bpos[idx[i]]+j] =
               tt_elem->bits[t->desc.bpos[i]+j];
       }
    }
    n_elem->prev = NULL;
    ED_FREE(idx);
    return TRUE;
}

int chain_rem_desc_item(chain_struct *c, char *name)
{
/****** chain_rem_desc_item ***************************************************
 *
 *  NAME	
 *      chain_rem_desc_item - Remove an item from the description structure
 *
 *  SYNOPSIS
 *      int chain_rem_desc_item(chain_dec *c)
 *
 *  FUNCTION
 *      This routine removes one item from the description and the data
 *      structures
 *
 *  INPUTS
 *      c    - Pointer to chain structure from whicgh to remove element
 *      name - The name of the element to be removed
 *
 *  RESULTS
 *      c    - the modified chain structure
 *
 *  RETURNS
 *      TRUE  - If the remove operation was successful
 *      FALSE - If something went wrong during the remove operation
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 20-09-1996
 *
 ******************************************************************************/
    int		i, j, grootte;
    chain_elem	*elem, *t_elem;

    if ((i = chain_get_desc_item(c->desc, name)) < 0) {
       syserr(WARNING, "chain_rem_desc_item",
                       "Element %s not in chain description\n", name);
       return FALSE;
    }
    elem = t_elem = c->chain;
    while (elem != NULL) {
       t_elem = elem;
       elem = elem->next;
       for (j=0;j<c->desc.bsize-c->desc.bpos[i]-c_size[c->desc.types[i]];j++) {
          t_elem->bits[c->desc.bpos[i]+j] =
                  t_elem->bits[c->desc.bpos[i]+j+c_size[c->desc.types[i]]];
       }
    }
    c->desc.bsize -= c_size[c->desc.types[i]];
    grootte = c_size[c->desc.types[i]];
    for (j=0;j<c->desc.nelem-i-1;j++) {
       strcpy(c->desc.names[i+j],c->desc.names[i+j+1]);
       c->desc.types[i+j] = c->desc.types[i+j+1];
       c->desc.bpos[i+j] -= grootte;
    }
    c->desc.nelem--;

    return TRUE;
}

int chain_rem_data_elem(chain_struct *c, chain_elem *e)
{
/****** chain_rem_data_elem ***************************************************
 *
 *  NAME	
 *      chain_rem_data_elem - Remove the chain_elem from the chain
 *
 *  SYNOPSIS
 *      int chain_rem_data_elem(chain_struct *c, chain_elem *e)
 *
 *  FUNCTION
 *      This routine removes the element as provided from the chain structure.
 *
 *  INPUTS
 *      c - The chain structure from which to remove the elements
 *      e - Pointer to the element to be removed
 *
 *  RESULTS
 *      c - The modified chain structure
 *
 *  RETURNS
 *      TRUE  - If the remove operation was successful
 *      FALSE - If an error occured during the remove operation
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 20-09-1996
 *
 ******************************************************************************/
    chain_elem	*t;

    if (e == NULL) return FALSE;
    if ((t = e->prev) != NULL) {
       t->next = e->next;
    }
    if (e->next != NULL ) e->next->prev = t;
    ED_FREE(e->bits);
    if (c->chain == e) c->chain = e->next;
    ED_FREE(e);
    return TRUE;
}

int chain_add_data_item(chain_struct *c, char *name, ttype type, char *val)
{
/****** chain_add_data_item ***************************************************
 *
 *  NAME	
 *      chain_add_data_item - Add one item to the given chain
 *
 *  SYNOPSIS
 *      int chain_add_data_item(chain_struct *c, char *name, void *val)
 *
 *  FUNCTION
 *      The routine adds a data item to the data structure of a chain
 *      element. The chain structure must contain only one data element
 *      as there is no prefered ordering in the chain structure, there is
 *      no way to tell to which of a chain of elements to add a specific
 *      item value
 *
 *  INPUTS
 *     c    - The chain structure to whichto add the element
 *     name - The name of the item to be added
 *     type - The type of the item to be added
 *     val  - The binary representation of the value to be added
 *
 *  RESULTS
 *     c    - The chain structure modified
 *
 *  RETURNS
 *     TRUE  - If the addition process succeeded
 *     FALSE - If the addition process failed
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      One can build a full data element with this routine item by item
 *      and use the chain_add_data_elem to add the information to a full
 *      chain of data.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 20-09-1996
 *
 ******************************************************************************/
    chain_desc	d;
    chain_elem	*e;
    int		i, j;

    chain_add_desc_item(c, name, type);
    if ((i = chain_get_desc_item(c->desc, name)) < 0) {
       syserr(WARNING,"chain_add_data_item",
              "There is no %s in chain description,\nuse chain_add_desc_item to add the description\n", name);
       return FALSE;
    }
    d = c->desc;
    e = c->chain;

    if (d.bsize == c_size[d.types[i]]) {
       ED_CALLOC(e,       "chain_add_data_item", chain_elem, 1);
       ED_CALLOC(e->bits, "chain_add_data_item", char,       d.bsize);
    } else  {
       ED_REALLOC(e->bits, "chain_add_data_item", char, d.bsize);
    }
    c->chain = e;
    for (j=0;j<c_size[d.types[i]];j++) {
       e->bits[d.bpos[i]+j] = val[j];
    }
    return TRUE;
}

ttype chain_get_data_item_type(chain_struct *c, char *name)
{
/****** chain_get_data_item_type **********************************************
 *
 *  NAME	
 *      chain_get_data_item_type - Return the type of the given data item
 *
 *  SYNOPSIS
 *      ttype chain_get_data_item_type(chain_struct *c, char *name)
 *
 *  FUNCTION
 *      The routine returns the data type of a given data item. Type
 *      enumerated type ttype is returned, or a negative value is returned
 *      to identify an error in the processing
 *
 *  INPUTS
 *      c    - The chain structure off which the item is part
 *      name - The name of the item of which to return the type
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The type enumerator of the requested item is returnd. THis is
 *      always a number > 0. If an error, item name does not exist in
 *      chain, occurs, the returned value is negative
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 25-09-1996
 *
 ******************************************************************************/
    int		i;

    if ((i = chain_get_desc_item(c->desc, name)) < 0) {
       syserr(WARNING,"chain_add_data_item",
              "There is no %s in chain description,\nuse chain_add_desc_item to add the description\n", name);
       return FALSE;
    }
    return c->desc.types[i];
}

char *chain_get_data_item(chain_elem *e, chain_desc d, char *name)
{
/****** chain_get_data_item ***************************************************
 *
 *  NAME	
 *      chain_get_data_item - Get the data for a given item from the chain
 *
 *  SYNOPSIS
 *      char *chain_get_data_item(chain_elem *e, chain_desc d, char *name)
 *
 *  FUNCTION
 *      The routine return the value of a given data item from the chain.
 *      The chain must contain only one element as there is now way to
 *      distinguish between elements in the chain structure directly.
 *
 *  INPUTS
 *      e    - The element chain continuing the sought item
 *      name - The name of the item to be returned
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The byte string containing the value of the requested item
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *     E.R. Deul - dd 25-09-1996
 *
 ******************************************************************************/
    int		i, j;
    char	*val;

    if ((i = chain_get_desc_item(d, name)) < 0) {
       syserr(WARNING,"chain_add_data_item",
              "There is no %s in chain description,\nuse chain_add_desc_item to add the description\n", name);
       return FALSE;
    }
    ED_CALLOC(val, "chain_get_data_item", char, c_size[d.types[i]]);
    for (j=0;j<c_size[d.types[i]];j++) {
       val[j] = e->bits[d.bpos[i]+j];
    }
    return val;
}
