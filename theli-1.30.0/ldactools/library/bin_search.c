#include <math.h>
#include "utils_defs.h"
#include "bin_search_globals.h"

/*
   30.01.03:
   function ibin_search_indexed:
   included separat treatment of the case n==1

   20.07.2004:
   I removed compiler warnings due to possibly
   undefined variables (compilation with optimisation
   turned on)
*/

int dbin_search_indexed(int n, double *x, int *idx, float key, float tol,
                        int *first, int *last, int root)
{
/****** dbin_search_indexed ***************************************************
 *
 *  NAME	
 *	dbin_search_indexed - Get area boundaries of sorted indexed double array
 *
 *  SYNOPSIS
 *      int dbin_search_indexed(int n, double *x, int *idx,
 *                              float key, float tol,
 *                              int *first, int *last)
 *
 *  FUNCTION
 *      The function determines the index array boundary values for an
 *      area in the double array that has as center the value of key and
 *      as 'radius' the size tolerance.
 *
 *      The index idx has first been determined through an index sorting
 *      routine such as indexd. THe calling program may then run through
 *      the array x from idx[first] through idx[last] to find values that
 *      lay between key - tol and key + tol.
 *
 *  INPUTS
 *      n          - nb of elements in the x array
 *      x          - the array of n doubles that was sorted with idx
 *      idx        - the array of n indices that sorts x ascending
 *      key        - the midpoint of the tolerance area
 *      tol        - the radius of the tolerance area
 *      root       - indicates lowest index (0 or 1)
 *
 *  RESULTS
 *      first      - the starting boundary index of the tolerance area
 *      last       - the ending boundary index of the tolerance area
 *
 *  RETURNS
 *      TRUE       - When correct boundaries have been found
 *      FALSE      - When no boundary could be determined. In that case
 *                   first = 2, last = 1.
 *
 *  MODIFIED GLOBALS
 *      There are no global variables changed by this routine
 *
 *  NOTES
 *      The sorting indexes must have been created for an ascending order.
 *
 *  BUGS
 *      The routine cannot detect the correctness of the precondition that
 *      states that the idx contains sorting indexes.
 *
 *  AUTHOR
 *      Numerical Recipies Binary search algorithm
 *      E.R. Deul 12-02-96 Modified to indexed arrays
 *
 ******************************************************************************/
    int  min = 1, max = n, middle = 0;
    int  imin, imax;
    int  found = 0, i;

    if (root == 0) {
       imax = max = n-1;
       imin = min = 0;
    } else {
       imax = max = n;
       imin = min = 1;
    }

    if (n == 2) {
       *first = root;
       *last = root+1;
       return TRUE;
    }
    while ((!found) && (max - min > 1)) {
      middle  = (min + max ) / 2;
      if (fabs(x[idx[middle]] - (double) key) <= (double) tol)
         found = 1;
      else if ( key < (x[idx[middle]] - tol))
         max = middle ;
      else if ( key > (x[idx[middle]] + tol))
         min = middle;
   }
/*
 * Not found
 */
   if (max == imax && min == (imax - 1)) {
      if (fabs(x[idx[max]] - (double) key) <= (double) tol) {
         *first = min;
         *last = max;
         return TRUE;
      }
   }
   if (!found) {
      *first = 2;
      *last  = 1;
      return FALSE;
   }

   for (i = middle; i > imin; i--) {
      if (fabs (x[idx[i]] - key) > tol)
         break;
   }
   *first = i;
   for (i = middle; i < imax; i++) {
      if (fabs (x[idx[i]] - key) > tol)
         break;
   }
   *last = i;
   return TRUE;
}

int fbin_search_indexed(int n, float *x, int *idx, float key, float tol,
                        int *first, int *last, int root)
{
/****** fbin_search_indexed ***************************************************
 *
 *  NAME	
 *	fbin_search_indexed - Get area boundaries of sorted indexed float array
 *
 *  SYNOPSIS
 *      int fbin_search_indexed(int n, float *x, int *idx,
 *                              float key, float tol,
 *                              int *first, int *last)
 *
 *  FUNCTION
 *      The function determines the index array boundary values for an
 *      area in the float array that has as center the value of key and
 *      as 'radius' the size tolerance.
 *
 *      The index idx has first been determined through an index sorting
 *      routine such as indexd. THe calling program may then run through
 *      the array x from idx[first] through idx[last] to find values that
 *      lay between key - tol and key + tol.
 *
 *  INPUTS
 *      n          - nb of elements in the x array
 *      x          - the array of n floats that was sorted with idx
 *      idx        - the array of n indices that sorts x ascending
 *      key        - the midpoint of the tolerance area
 *      tol        - the radius of the tolerance area
 *      root       - indicates lowest index (0 or 1)
 *
 *  RESULTS
 *      first      - the starting boundary index of the tolerance area
 *      last       - the ending boundary index of the tolerance area
 *
 *  RETURNS
 *      TRUE       - When correct boundaries have been found
 *      FALSE      - When no boundary could be determined. In that case
 *                   first = 2, last = 1.
 *
 *  MODIFIED GLOBALS
 *      There are no global variables changed by this routine
 *
 *  NOTES
 *      The sorting indexes must have been created for an ascending order.
 *
 *  BUGS
 *      The routine cannot detect the correctness of the precondition that
 *      states that the idx contains sorting indexes.
 *
 *  AUTHOR
 *      Numerical Recipies Binary search algorithm
 *      E.R. Deul 12-02-96 Modified to indexed arrays
 *
 ******************************************************************************/
    int  min = 1, max = n, middle = 0;
    int  imin, imax;
    int  found = 0, i;

    if (root == 0) {
       imax = max = n-1;
       imin = min = 0;
    } else {
       imax = max = n;
       imin = min = 1;
    }

    if (n == 2) {
       *first = root;
       *last = root+1;
       return TRUE;
    }
    while ((!found) && (max - min > 1)) {
      middle  = (min + max ) / 2;
      if (fabs(x[idx[middle]] - (double) key) <= (double) tol)
         found = 1;
      else if ( (double) key < ((double)x[idx[middle]] - (double)tol))
         max = middle ;
      else if ( (double)key >= ((double)x[idx[middle]] + (double)tol))
         min = middle;
   }
/*
 * Not found
 */
   if (max == imax && min == (imax - 1)) {
      if (fabs(x[idx[max]] - (double) key) <= (double) tol) {
         *first = min;
         *last = max;
         return TRUE;
      }
   }
   if (!found) {
      *first = 2;
      *last  = 1;
      return FALSE;
   }

   for (i = middle; i > imin; i--) {
      if (fabs (x[idx[i]] - key) > tol)
         break;
   }
   *first = i;
   for (i = middle; i < imax; i++) {
      if (fabs (x[idx[i]] - key) > tol)
         break;
   }
   *last = i;
   return TRUE;
}

int ibin_search_indexed(int n, int *x, int *idx, int key, int tol,
                        int *first, int *last, int root)
{
/****** ibin_search_indexed ***************************************************
 *
 *  NAME	
 *	ibin_search_indexed - Get area boundaries of sorted indexed int array
 *
 *  SYNOPSIS
 *      int ibin_search_indexed(int n, int *x, int *idx,
 *                              float key, float tol,
 *                              int *first, int *last)
 *
 *  FUNCTION
 *      The function determines the index array boundary values for an
 *      area in the int array that has as center the value of key and
 *      as 'radius' the size tolerance.
 *
 *      The index idx has first been determined through an index sorting
 *      routine such as indexd. THe calling program may then run through
 *      the array x from idx[first] through idx[last] to find values that
 *      lay between key - tol and key + tol.
 *
 *  INPUTS
 *      n          - nb of elements in the x array
 *      x          - the array of n doubles that was sorted with idx
 *      idx        - the array of n indices that sorts x ascending
 *      key        - the midpoint of the tolerance area
 *      tol        - the radius of the tolerance area
 *      root       - indicates lowest index (0 or 1)
 *
 *  RESULTS
 *      first      - the starting boundary index of the tolerance area
 *      last       - the ending boundary index of the tolerance area
 *
 *  RETURNS
 *      TRUE       - When correct boundaries have been found
 *      FALSE      - When no boundary could be determined. In that case
 *                   first = 2, last = 1.
 *
 *  MODIFIED GLOBALS
 *      There are no global variables changed by this routine
 *
 *  NOTES
 *      The sorting indexes must have been created for an ascending order.
 *
 *  BUGS
 *      The routine cannot detect the correctness of the precondition that
 *      states that the idx contains sorting indexes.
 *
 *  AUTHOR
 *      Numerical Recipies Binary search algorithm
 *      E.R. Deul 12-02-96 Modified to indexed arrays
 *
 ******************************************************************************/
    int  min = 1, max = n, middle = 0;
    int  imin, imax;
    int  found = 0, i, no_neg = FALSE, xx;

    if (root == 0) {
       imax = max = n-1;
       imin = min = 0;
    } else {
       imax = max = n;
       imin = min = 1;
    }
    if (tol < 0) {
       no_neg = TRUE;
       tol = -tol;
    }
    if (n == 1) {
       *first = root;
       *last = root;
       return TRUE;
    }
    if (n == 2) {
       *first = root;
       *last = root+1;
       return TRUE;
    }
    while ((!found) && (max - min > 1)) {
      middle  = (min + max ) / 2;
      if (no_neg) {
         xx = (int) fabs(x[idx[middle]]);
      } else {
         xx = x[idx[middle]];
      }
      if (fabs(xx - (double) key) <= (double) tol)
         found = 1;
      else if ( key < (xx - tol))
         max = middle ;
      else if ( key > (xx + tol))
         min = middle;
   }
/*
 * Not found
 */
   if (max == imax && min == (imax - 1)) {
      if (fabs(x[idx[max]] - (double) key) <= (double) tol) {
         *first = min;
         *last = max;
         return TRUE;
      }
   }
   if (max == (imin + 1) && min == imin) {
      if (fabs(x[idx[min]] - (double) key) <= (double) tol) {
         *first = min;
         *last = max;
         return TRUE;
      }
   }
   if (!found) {
      *first = 2;
      *last  = 1;
      return FALSE;
   }
   for (i = middle; i > imin; i--) {
      if (no_neg) {
         xx = (int) fabs(x[idx[i]]);
      } else {
         xx = x[idx[i]];
      }
      if (fabs (xx - key) > tol)
         break;
   }
   *first = i;
   for (i = middle; i < imax; i++) {
      if (no_neg) {
         xx = (int) fabs(x[idx[i]]);
      } else {
         xx = x[idx[i]];
      }
      if (fabs (xx - key) > tol)
         break;
   }
   *last = i;
   return TRUE;
}

int sbin_search_indexed(int n, short int *x, int *idx, int key, int tol,
                        int *first, int *last, int root)
{
/****** sbin_search_indexed ***************************************************
 *
 *  NAME	
 *	sbin_search_indexed - Get area boundaries of indexed short int array
 *
 *  SYNOPSIS
 *      int sbin_search_indexed(int n, short int *x, int *idx,
 *                              float key, float tol,
 *                              int *first, int *last)
 *
 *  FUNCTION
 *      The function determines the index array boundary values for an
 *      area in the short int array that has as center the value of key and
 *      as 'radius' the size tolerance.
 *
 *      The index idx has first been determined through an index sorting
 *      routine such as indexd. THe calling program may then run through
 *      the array x from idx[first] through idx[last] to find values that
 *      lay between key - tol and key + tol.
 *
 *  INPUTS
 *      n          - nb of elements in the x array
 *      x          - the array of n short int that was sorted with idx
 *      idx        - the array of n indices that sorts x ascending
 *      key        - the midpoint of the tolerance area
 *      tol        - the radius of the tolerance area
 *      root       - indicates lowest index (0 or 1)
 *
 *  RESULTS
 *      first      - the starting boundary index of the tolerance area
 *      last       - the ending boundary index of the tolerance area
 *
 *  RETURNS
 *      TRUE       - When correct boundaries have been found
 *      FALSE      - When no boundary could be determined. In that case
 *                   first = 2, last = 1.
 *
 *  MODIFIED GLOBALS
 *      There are no global variables changed by this routine
 *
 *  NOTES
 *      The sorting indexes must have been created for an ascending order.
 *
 *  BUGS
 *      The routine cannot detect the correctness of the precondition that
 *      states that the idx contains sorting indexes.
 *
 *  AUTHOR
 *      Numerical Recipies Binary search algorithm
 *      E.R. Deul 12-02-96 Modified to indexed arrays
 *
 ******************************************************************************/
    int  min = 1, max = n, middle = 0;
    int  imin, imax;
    int  found = 0, i, no_neg = FALSE, xx;

    if (root == 0) {
       imax = max = n-1;
       imin = min = 0;
    } else {
       imax = max = n;
       imin = min = 1;
    }
    if (tol < 0) {
       no_neg = TRUE;
       tol = -tol;
    }
    if (n == 2) {
       *first = root;
       *last = root+1;
       return TRUE;
    }
    while ((!found) && (max - min > 1)) {
      middle  = (min + max ) / 2;
      if (no_neg) {
         xx = (int) fabs(x[idx[middle]]);
      } else {
         xx = x[idx[middle]];
      }
      if (fabs(xx - (double) key) <= (double) tol)
         found = 1;
      else if ( key < (xx - tol))
         max = middle ;
      else if ( key > (xx + tol))
         min = middle;
   }
/*
 * Not found
 */
   if (max == imax && min == (imax - 1)) {
      if (fabs(x[idx[max]] - (double) key) <= (double) tol) {
         *first = min;
         *last = max;
         return TRUE;
      }
   }
   if (!found) {
      *first = 2;
      *last  = 1;
      return FALSE;
   }
   for (i = middle; i > imin; i--) {
      if (no_neg) {
         xx = (int) fabs(x[idx[i]]);
      } else {
         xx = x[idx[i]];
      }
      if (fabs (xx - key) > tol)
         break;
   }
   *first = i;
   for (i = middle; i < imax; i++) {
      if (no_neg) {
         xx = (int) fabs(x[idx[i]]);
      } else {
         xx = x[idx[i]];
      }
      if (fabs (xx - key) > tol)
         break;
   }
   *last = i;
   return TRUE;
}


int heap_search_indexed(int n, int *x, int *idx, int key, int nul)
{
/****** heap_search_indexed ***************************************************
 *
 *  NAME	
 *	heap_search_indexed - Get index of key for a sorted indexed int array
 *
 *  SYNOPSIS
 *      int heap_search_indexed(int n, int *x, int *idx,
 *                              int key, int nul)
 *
 *  FUNCTION
 *      The function determines the index value of an sorted indexed
 *      array for a given key
 *
 *  INPUTS
 *      n          - nb of elements in the x array
 *      x          - the array of n doubles that was sorted with idx
 *      idx        - the array of n indices that sorts x ascending
 *      key        - the midpoint of the tolerance area
 *      nul        - Starting value of the indices (0 or 1)
 *
 *  RESULTS
 *
 *  RETURNS
 *     The index into the array element that has the key value
 *
 *
 *  NOTES
 *      The sorting indexes must have been created for an ascending order.
 *
 *  BUGS
 *      The routine cannot detect the correctness of the precondition that
 *      states that the idx contains sorting indexes.
 *
 *  AUTHOR
 *      Numerical Recipies Binary search algorithm
 *      E.R. Deul 12-02-96 Modified to indexed arrays
 *
 ******************************************************************************/
    int  min = nul, max = n - (1 - nul), middle = 0;
    int  found = 0;

    while ((!found) && ((max - min) > 1)) {
      middle  = (min + max) / 2;
      if ((x[idx[middle]] - key) == 0)
         found = 1;
      else if ( key < x[idx[middle]])
         max = middle ;
      else if ( key > x[idx[middle]])
         min = middle;
   }
   if (!found) {
      if ((x[idx[min]] - key ) == 0)
         middle = min;
      if ((x[idx[max]] - key ) == 0)
         middle = max;
   }
   return middle;
}


