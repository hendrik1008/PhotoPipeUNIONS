/* History comments:
   01.05.2006:
   I removed the inclusion of precess_defs.h and inserted
   the missing definition of DEG2RAD directly into the code
*/

#include <math.h>

#include "check_overlap_globals.h"
#include "graph_globals.h"
#include "options_globals.h"
#include "make_perms_defs.h"

/* definitions */
#ifndef DEG2RAD
#define DEG2RAD 0.017453292         /* degree/radian conversion */
#endif

int check_overlaps(int *graph,
                   double *ra, double *dec, int n,
                   float xpixsize, float ypixsize,
                   float min_over, int xwid, int ywid,
                   int condition)
{
/****** check_overlaps ***************************************************
 *
 *  NAME	
 *      check_overlaps - Create a directed graph of overlap
 *
 *  SYNOPSIS
 *      int check_overlaps(int *graph,
 *                         float *ra, float *dec, int n,
 *                         float xpixsize, float ypixsize,
 *                         float min_over, int xwid, int ywid,
 *                         int condition)
 *
 *  FUNCTION
 *      This function derives the two dimensional overlap characteristics
 *      of a set of similarly sized and scaled frames. This version of the
 *      code assumes that the (Ra,Dec) domain is rectangular!
 *
 *      To derive these overlap characteristics a list of (Ra, Dec)'s is
 *      looped. Each image has a dimension of (xwid*pixsize, ywid*pixsize).
 *      Furthermore, an overlap is detected only when it is larger than
 *      min_over pixels in any direction.
 *
 *  INPUTS
 *      graph     - Graph identification number
 *      ra        - Array of Right Ascensions
 *      dec       - Array of Declinations
 *      n         - The number of elements in the ra and dec arrays
 *      xpixsize  - X coord pixel size in same units as ra and dec
 *      ypixsize  - Y coord pixel size in same units as ra and dec
 *      min_over  - Minimum number of pixels required to detect overlap
 *      xwid      - Width (in ra direction) of a frame in pixels
 *      ywid      - Width (in dec direction) of a frame in pixels
 *      condition - Special overl requirements condition
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      The number of relations added to the directed graph.
 *
 *  MODIFIED GLOBALS
 *      The graph structure is updated with the relations
 *
 *  NOTES
 *
 *  BUGS
 *      The current implementation of overlap comparison does not take into
 *      account the true sky coordinate system.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 20-02-1996
 *
 ******************************************************************************/
    int     i, j, m = 0;
    double  x_diff, y_diff;

    *graph = init_graph(n);
    for (i=0;i<n; i++) {
       for (j=0;j<n;j++) {
          if (i != j) {
             x_diff = fabs((ra[i]-ra[j])*cos(dec[i]*DEG2RAD)/xpixsize);
             y_diff = fabs((dec[i]-dec[j])/ypixsize);
             switch (condition) {
             case DENIS_COND:
                if (((y_diff < min_over) && (x_diff < (xwid - min_over))) ||
                    ((x_diff < min_over) && (y_diff < (ywid - min_over)))) {
                   VPRINTF(DEBUG,"From %d to %d\n",i,j);
                   add_graph_side(*graph, i, 1, j, 1);
                   m++;
                }
                break;
             case GENERAL_COND:
                if ((y_diff < (ywid - min_over)) &&
                    (x_diff < (xwid - min_over))) {
                   VPRINTF(DEBUG,"From %d to %d %f %f\n",i,j,x_diff,y_diff);
                   add_graph_side(*graph, i, 1, j, 1);
                   m++;
                }
                break;
             }
          }
       }
    }
    return m;
}

