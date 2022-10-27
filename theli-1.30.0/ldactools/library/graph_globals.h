/****** graph *****************************************************************
 *
 *  INTRODUCTION
 *      The graph package implements the directed graph data structure in C.
 *      The point information currently stored in the graph structure are the
 *      point identification number (type int) and the channel number (internal
 *      type short int).
 *      It is very simple in use, there are only a few important routines to
 *      be used by a calling program.
 *
 *  USAGE
 *      To allow usage of this package one needs to include the following
 *      file in the source code:
 *         #include "graph_globals.h"
 *
 *      The initialization is done through the init_graph routine.
 *
 *      Information is put into the graph structure thought the add_graph_side
 *      routine.
 *
 *      Information is retrieved in two manners:
 *      First on a point by point basis, the first_side, and next_side routines
 *      return successive pairs of points and their relations, or get_all_ids
 *      returns all point identification numbers for a given channel.
 *
 *      For debugging purposes the display_graph routine shows the complete
 *      informatin content of a graph.
 *
 *      Finally there are two routines the finish wit the processing of graphs.
 *      One is remove_graph that completely removes a graph structure from the
 *      package memory, and clear_graph that clears the information form a
 *      graph structure, but does not remove the structuire from the package
 *      memory.
 *
 *  AUTHOR
 *      E.R. Deul
 *
 ******************************************************************************/
#include "graph_types.h"
#include "graph_defs.h"

extern void	remove_graph(int),
                display_graph(int),
		clear_graph(int);

extern int	init_graph(int),
                add_graph_side(int, int, int, int, int),
                first_side(int, int *, int *, int *, int *),
                next_side(int, int *, int *, int *, int *),
		*get_all_ids(int, int, int *),
		**get_all_pairs(int, int, int, int *);

extern char	**build_matrix(int, int *);


