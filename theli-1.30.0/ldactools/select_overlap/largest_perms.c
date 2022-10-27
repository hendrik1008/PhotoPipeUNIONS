#include "utils_globals.h"
#include "graph_globals.h"
#include "make_perms_globals.h"
#include "queue_globals.h"

int largest_perms(int graph, int n, perm_struct *perms, int from)
{
/****** largest_perms ***************************************************
 *
 *  NAME	
 *      largest_perms - Create a permutation list from a directed graph
 *
 *  SYNOPSIS
 *      int largest_perms(int graph, int n, perm_struct *perms)
 *
 *  FUNCTION
 *      This function examines a directed graph structure and creates
 *      a permutations list that allows the caller to us it for paring
 *      purposes.
 *
 *      The routine assumes the directed graph is a single set of overlap
 *      frames. It will try to start with the frame id number 0 and cascade
 *      the directed graph to obtain a permutation pair for all frames. If
 *      id odes not succeed, it tries starting with the next frame and so
 *      on until a permutation set is obtained which contains all frames.
 *
 *      If the directed graph somehow has not filled, of if the program
 *      does not succeed in getting a full permutation set, it will return
 *      0 permutations.
 *
 *  INPUTS
 *      graph   - Graph identification number.
 *      n       - The number of frames in the directed graph.
 *
 *  RESULTS
 *      perms   - The list of permutations found.
 *
 *  RETURNS
 *      > 0     - The number of permutations found (shoul always be n)
 *      = 0     - No permutations list could be derived
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 20-02-1996
 *
 ******************************************************************************/
    int  curr_perm, i;
    int  leeg, to, dum;
    int  *have_had;

    ED_CALLOC(have_had,"mk_perms", int, n);
    createQ();
    i = FALSE;
    curr_perm = 0;
    dum = 1;
    if (!first_side(graph, &from, &dum, &to, &dum))
       return curr_perm;
    have_had[from] = TRUE;
    putQ(from);
    while (!QisMT()) {
       getQ(&from);
       first_side(graph, &from, &dum, &to, &dum);
       if (!have_had[to]) {
          have_had[to] = TRUE;
          perms[curr_perm].from = from;
          perms[curr_perm].to   = to;
          curr_perm++;
          putQ(to);
       }
       leeg = NO_ID;
       while (next_side(graph, &leeg, &dum, &to, &dum)) {
          leeg = NO_ID;
          if (! have_had[to]) {
             have_had[to] = TRUE;
             perms[curr_perm].from = from;
             perms[curr_perm].to   = to;
             curr_perm++;
             putQ(to);
          }
       }
    }
    ED_FREE(have_had);
    return (curr_perm);
}

