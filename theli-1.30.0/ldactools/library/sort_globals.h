/* 19.07.01:
   added definition for ssort1 function

   17.03.2006:
   I removed not needed quicksort routines from Numerical
   Recipes.

   27.03.2006:
   I completely rewrote the code for array sortings. The routines
   have a new naming scheme and the actual sorting algorithms were
   replaced by faster ones. The last argument of all functions
   which gave the index with which the array started (0 or 1) has
   been removed because we use in all routines the C convention
   (arrays start with index 0)
*/

#include "utils_globals.h"

/****** sort ****************************************************************
 *
 *  INTRODUCTION
 *      The sorting package allows the user to sort arrays of different
 *      type and in combination on ascending and descending order. The
 *      routines perform sorting on the first array provided in the list
 *      of arrays and swap the associated additiona arrays in the same
 *      manner as the key array.
 *
 *  USAGE
 *      To allow usage of this package one needs to include the following
 *      file in the source code:
 *         #include "sort_globals.h"
 *
 *      The naming convention for the routines is: xxx(r)sort.
 *      xxx stand for one or several variable types
 *      that have to be sorted (d=double, f=float, i=int and s=short).
 *      An r indicates sorting in descending order. The first array
 *      represents the key whose elements determine the sorting. The
 *      rest are associated arrays that will be sorted according to the
 *      key array.
 *
 *  IMPLEMENTATION
 *      The sortings are implemented by determining an int index array on
 *      the key array and to rearrange the actual arrays later. To this end
 *      indexing functions for all variable types are included.
 *
 *  AUTHOR
 *      E.R. Deul & Thomas Erben
 *
 ******************************************************************************/
extern void	isort(int, int *),  /* sort 1 int array */
        	ssort(int, short int *),  /* sort 1 int array */
		fsort(int, float *), /* sort 1 float array */
		iisort(int, int *, int *),
		                          /* sort 2 arrays on first */
		ffsort(int, float *, float *),
		                          /* sort 2 arrays on first */
		fffsort(int, float *, float *, float *),
                                          /* sort 3 arrays on first */
		ffrsort(int, float *, float *),
                                          /* reverse sort 2 arrays on first */
		fffrsort(int, float *, float *, float *),
                                          /* reverse sort 3 arrays on first */
		ddisort(int, double *, double *, int *),
		                          /* sort 2 double & 1 int on first */
		fffisort(int, float *, float *, float *, int *),
		                          /* sort 3 float  & 1 int on first */
		fffirsort(int, float *, float *, float *, int *),
		                          /* sort 3 flt, 1 int on first */
                findex(int, float *, int *),
                                          /* Get float sorting index */
                dindex(int, double *, int *),
                                          /* Get double sorting index */
                iindex(int, int *, int *),
                                          /* Get int sorting index */
                lindex(int, LONG *, int *),
                                          /* Get long sorting index */
                sindex(int, short int *, int *);
                                          /* Get short int sorting index */


