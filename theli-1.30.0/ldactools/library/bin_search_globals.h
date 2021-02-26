/****** bin_search_indexed ****************************************************
 *
 *  INTRODUCTION
 *      The binary search routines implemented in this package are
 *      extensions of the Numerica Recipies binary search algorithms.
 *      The main drive for this extension is that with this type of
 *      routines the original information arrays need to to be reordered.
 *      In fact one could order a structure in different manners and have
 *      several indexes representing this ordering while the original
 *      array stays in tact.
 *
 *      The next enhancement is found in terms of the tolerance area. The
 *      binary search algorithm determines the boundaries in terms of index
 *      values, of a given region in the index sorted array. This allows
 *      for instance a area search in two dimensional space.
 *
 *  USAGE
 *      To allow usage of this package one needs to include the following
 *      file in the source code:
 *         #include "bin_search_globals.h"
 *
 *      There are two routines in the binary search package. One for double
 *      arrays: dbin_search_indexed, and one for integer arrays:
 *      ibin_search_indexed.
 *
 *  AUTHOR
 *      E.R. Deul
 *
 ******************************************************************************/
extern int	dbin_search_indexed(int , double *, int *, float , float ,
                        int *, int *, int),
		fbin_search_indexed(int , float *, int *, float , float ,
                        int *, int *, int),
		ibin_search_indexed(int , int *, int *, int , int,
                        int *, int *, int),
		sbin_search_indexed(int , short int *, int *, int , int,
                        int *, int *, int),
		heap_search_indexed(int, int *, int *, int, int);

