#include <stdio.h>
#include "utils_globals.h"
#include "graph_globals.h"

/*
 * Local function declarations
 */


static void	check_g_id(int),
		clear_side(graph_elem *),
		remove_sides(graph_elem *),
		add_side(side_ptr *, int),
		write_id(graph_elem, int),
		write_sides(graph_elem);

static int	append_graph_entry(int, int, int, int),
		add_graph_entry(int, int, int, int),
                add_graph_point(int, int, int),
		check_graph_entry(int, int, int),
		get_side(int, int *, int *, int *, int *);

static side_ptr	*create_side(int ptr);

/*
 * Local variable declarations
 */


static graph_struct *g[MAX_GRAPHS];
static int cur_elem[MAX_GRAPHS];
static side_ptr *cur_side[MAX_GRAPHS];
static int cur_graph = -1;


/* 
 * First the public routines
 */

int init_graph(int n)
{
/****** init_graph ***************************************************
 *
 *  NAME	
 *      init_graph - Initialize a directed graph with n points
 *
 *  SYNOPSIS
 *      int init_graph(int n)
 *
 *  FUNCTION
 *      This initialization routine allocates memory and correctly defines
 *      a directed graph in memory. The number of points expected for the
 *      graph is provided through the argument. It is, however, possible
 *      that the actual number of points in the graph may grow bejond the
 *      number specified; automatic reallocation of more points is done
 *      when necessary.
 *
 *      There is a maximum of MAX_GRAPHS separate directed graphs
 *      implemented in memory here.
 *
 *  INPUTS
 *      n  - The initial number of points in the directed graph.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      The identification number of the directed graph allocated. This
 *      identification number should be provided to all the other routines
 *      as the handle on which to work.
 *
 *  MODIFIED GLOBALS
 *      There are no global variables visible for the outside world. There
 *      are, however, local variables. See internal routines.
 *
 *  NOTES
 *      One should provide a good estimate of the number of points in the
 *      directed graph. The routines in this packge allow underestimating
 *      this number on the init_graph call, but reallocation of an
 *      additional point is done on a point-by-point basis, and thus is
 *      very compute expensive.
 *
 *  BUGS
 *      Please report, if found.
 *
 *  AUTHOR
 *      E.R. Deul - dd 16-02-1996
 *
 ******************************************************************************/

    int i;

    if (cur_graph < MAX_GRAPHS) {
       cur_graph++;
       ED_CALLOC(g[cur_graph],        "init_graph", graph_struct, 1);
       ED_CALLOC(g[cur_graph]->array, "init_graph", graph_elem,   n);
       for (i=0;i<n;i++) g[cur_graph]->array[i].side = create_side(NO_ID);
       g[cur_graph]->last = 0;
       g[cur_graph]->n = n;
    } else {
       syserr(FATAL,"init_graph",
              "Too many graphs in use (increase MAX_GRAPHS)\n");
    }
    return cur_graph;
}

void remove_graph(int g_id)
{
/****** remove_graph ***************************************************
 *
 *  NAME	
 *      remove_graph - Removes the identified graph from the package internals
 *
 *  SYNOPSIS
 *      void remove_graph(int g_id)
 *
 *  FUNCTION
 *      Delete all the memory allocated elements of the graph identified
 *      by the argument. It does also free the occupation of a graph id
 *      allowing another graph to be initialized.
 *
 *  INPUTS
 *      g_id  - The graph identification number
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      The internal memory allocated for the specified directed graph is
 *      freed.
 *
 *  NOTES
 *
 *  BUGS
 *      Removing a none existant graph leads to undefined problems.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int i;
    check_g_id(g_id);

    for (i=0;i<g[g_id]->n;i++) {
       remove_sides(&(g[g_id]->array[i]));
    }
    ED_FREE(g[g_id]->array);
    ED_FREE(g[g_id]);
    cur_graph--;
}

void clear_graph(int g_id)
{
/****** clear_graph ***************************************************
 *
 *  NAME	
 *      clear_graph - Clean all point and relation elements of a graph
 *
 *  SYNOPSIS
 *      void clear_graph(int g_id)
 *
 *  FUNCTION
 *      Remove all information from a graph. The graph identified by the
 *      argument is cleared of all information elements. The internal
 *      structure is kept, however. This means that there is no need for
 *      allocating and initializing the identified graph after this call.
 *
 *      The caller should keep the identification number as this gives him
 *      a handle to the cleared graph structure. He can reuse the structure
 *      by inserting new information. This routine saves de freeing and
 *      allocation of graphs in iterative loops.
 *
 *  INPUTS
 *      g_id  - The graph identification number.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      The internal graph structure is kept, but the information content
 *      is cleared.
 *
 *  NOTES
 *      Providing a non-initialized graph identification number leads to
 *      undefined behaviour.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int i;
    check_g_id(g_id);

    for (i=0;i<g[g_id]->last;i++) {
       clear_side(&(g[g_id]->array[i]));
    }
    g[g_id]->last = 0;
}

int first_side(int g_id, int *id1, int *chan1, int *id2, int *chan2)
{
/****** first_side ***************************************************
 *
 *  NAME	
 *      first_side - Obtain the two points of the first relation in the graph
 *
 *  SYNOPSIS
 *      int first_side(int g_id, int *id1, int *chan1, int *id2, int *chan2)
 *
 *  FUNCTION
 *      For the specified graph this routine initializes the stepwise
 *      descent along the graph structure. On return it provides point
 *      information for the very first relation encountered in the graph
 *      structure.
 *
 *      There is no predefined organization of the relations in the graph
 *      structure. Therefore, the caller should not rely on the order of
 *      the relations he has provided to the graph package.
 *
 *      If the caller did not explicitely set the *id1 to NO_ID, he asks
 *      for a particular objects side.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      id1   - Identification number of point at arrow tail
 *      chan1 - Channel number of point at arrow tail
 *
 *  RESULTS
 *      id1   - Identification number of point at arrow tail
 *      chan1 - Channel number of point at arrow tail
 *      id2   - Identification number of point at arrow head
 *      chan2 - Channel number of point ar arrow tail
 *
 *  RETURNS
 *      FALSE - When no relations are present in the graph structure
 *      TRUE  - When there is a first relation in the graph structure
 *
 *  MODIFIED GLOBALS
 *      The first_side and next_side routines keep local global variables
 *      to keep track of the current position in the graph structure.
 *
 *  NOTES
 *      When first_side returns a FALSE one should no successively call
 *      the associated next_side routine.
 *
 *  BUGS
 *      Specifying a non-existant graph identification number leads to
 *      undefined behaviour.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int retcode = FALSE;

    check_g_id(g_id);

    if ( *id1 == NO_ID) {
       cur_elem[g_id] = 0;
    } else {
       if ((cur_elem[g_id] = check_graph_entry(g_id, *id1, *chan1)) == NO_ID) {
          return retcode;
       }
    }
    cur_side[g_id] = g[g_id]->array[cur_elem[g_id]].side;
    return (get_side(g_id, id1, chan1, id2, chan2));
}

int next_side(int g_id, int *id1, int *chan1, int *id2, int *chan2)
{
/****** next_side ***************************************************
 *
 *  NAME	
 *      next_side - Obtain point information of the next relation in the graph
 *
 *  SYNOPSIS
 *      int next_side(int g_id, int *id1, int *chan1, int *id2, int *chan2)
 *
 *  FUNCTION
 *      The function returns the point information of the next relation
 *      in the specified graph. To correctly work, this function should
 *      be called after a single call to the first_side function.
 *
 *      If the caller did not explicitely set the *id1 to NO_ID, he asks
 *      for a particular objects side.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      id1   - Identification number of point at arrow tail
 *      chan1 - Channel number of point at arrow tail
 *
 *  RESULTS
 *      id1   - Identification number of point at arrow tail
 *      chan1 - Channel number of point at arrow tail
 *      id2   - Identification number of point at arrow head
 *      chan2 - Channel number of point ar arrow tail
 *
 *  RETURNS
 *      FALSE - When there are no more relations in the graph. The point
 *              values are undefined at that time.
 *      TRUE  - When the information provided reflects point data of the
 *              next existing relation in the graph.
 *
 *  MODIFIED GLOBALS
 *      The first_side and next_side routines keep local global variables
 *      to keep track of the current position in the graph structure.
 *
 *  NOTES
 *      This routine can only be called upon successful completion of
 *      at least one first_side call.
 *
 *  BUGS
 *      Specifying a non-existant graph identification number leads to
 *      undefined behaviour.
 *
 *  AUTHOR
 *
 ******************************************************************************/
    check_g_id(g_id);

    cur_side[g_id] = cur_side[g_id]->next;
    return (get_side(g_id, id1, chan1, id2, chan2));
}

int add_graph_side(int g_id, int id1, int chan1, int id2, int chan2)
{
/****** add_graph_side ***************************************************
 *
 *  NAME	
 *      add_graph_side - Add a point-to-point relation to the graph
 *
 *  SYNOPSIS
 *      int add_graph_side(int g_id, int id1, int chan1, int id2, int chan2)
 *
 *  FUNCTION
 *      Add a relation to the specified graph. The relation is stored in
 *      the graph by adding both points to the points list. If these points
 *      were already in the list this step is omitted. Then the relation
 *      is stored in terms of pointers in the structure.
 *
 *      When there is a need to extend the graph structure (the current
 *      number of points exceeds the initialized number of points) there
 *      will be an automatic allocation of one addition point structure.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      id1   - Identification number of point at arrow tail
 *      chan1 - Channel number of point at arrow tail
 *      id2   - Identification number of point at arrow head
 *      chan2 - Channel number of point ar arrow tail
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      FALSE - When the point at the arrow tail was already in the graph
 *              structure.
 *      TRUE  - When the point at the arrow tail was not yet stored in the
 *              graph structure before the call to this routine.
 *
 *  MODIFIED GLOBALS
 *      Information on the relation between the two points are stored in
 *      the identified graph structure that is local to these routines.
 *
 *  NOTES
 *      This routine is the main provider of information to the graph
 *      package. The return values are used to allow recursion on merge
 *      searching.
 *
 *  BUGS
 *      When there is not enough memory available the routine exist. When
 *      a non-existant graph identification number is provided this leads
 *      to undefined behaviour.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int ptr;
/*
 *  Check if this point is in the graph, otherwise add it
 */
    if ((ptr = check_graph_entry(g_id, id1, chan1)) == NO_ID) {
       ptr = add_graph_point(g_id, id1, chan1);
    }
/*
 *  If the current point was not in the graph recurse to see its assoc points
 */
    return (add_graph_entry(g_id, id2, chan2, ptr));
}

int *get_all_ids(int g_id, int chan, int *n)
{
/****** get_all_ids ***************************************************
 *
 *  NAME	
 *      get_all_ids - Return all point from the graph for a given channel
 *
 *  SYNOPSIS
 *      int *get_all_ids(int g_id, int chan, int *n)
 *
 *  FUNCTION
 *      Go through the specified graph structure and return all
 *      identification numbers of points that belong the the specified
 *      channel.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      chan  - Channel number
 *
 *  RESULTS
 *      n     - The number of point returned on the call.
 *
 *  RETURNS
 *      An array of point identification numbers (of type int).
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This routine is used to provide information on single channel
 *      points for a given graph.
 *
 *  BUGS
 *      When a non-existant graph identification number is provided this
 *      leads to undefined behaviour.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 ******************************************************************************/
     int i, k, *ids;

     check_g_id(g_id);

     ED_MALLOC(ids, "get_all_ids", int, g[g_id]->last);
     k = 0;
     for (i=0;i<g[g_id]->last;i++) {
        if (g[g_id]->array[i].chan == chan) {
           ids[k++] = g[g_id]->array[i].id;
        }
     }
     *n = k;
     return ids;
}

int **get_all_pairs(int g_id, int chan1, int chan2, int *n)
{
/****** get_all_pairs ***************************************************
 *
 *  NAME	
 *      get_all_pairs - Return all points relations from chan1 to chan2
 *
 *  SYNOPSIS
 *      int **get_all_pairs(int g_id, int chan1, int chan2, int *n)
 *
 *  FUNCTION
 *      Go through the specified graph structure and return all
 *      identification pairs of points that point from chan1 to chan2
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      chan1 - Channel from number
 *      chan2 - Channel to number
 *
 *  RESULTS
 *      n     - The number of pairs returned on the call.
 *
 *  RETURNS
 *      An 2D array of pair identification numbers (of type int [2]).
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This routine is used to provide information on relations from chan1
 *      to chan2.
 *
 *  BUGS
 *      When a non-existant graph identification number is provided this
 *      leads to undefined behaviour.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 28-05-1996
 ******************************************************************************/
     int k, from, to, dum1, dum2, **ids;

     check_g_id(g_id);

     ED_MALLOC(ids, "get_all_ids", int *, 2);
     ED_MALLOC(ids[0], "get_all_ids", int, g[g_id]->last);
     ED_MALLOC(ids[1], "get_all_ids", int, g[g_id]->last);
     k = 0;
     from = NO_ID;
     if (first_side(g_id, &from, &dum1, &to, &dum2)) {
        if (chan1 == dum1 && chan2 == dum2) {
           ids[0][k] = from;
           ids[1][k++] = to;
        }
        while (next_side(g_id, &from, &dum1, &to, &dum2)) {
           if (chan1 == dum1 && chan2 == dum2) {
              ids[0][k] = from;
              ids[1][k++] = to;
           }
        }
     }
     *n = k;
     return ids;
}

void display_graph(int g_id)
{
/****** display_graph ***************************************************
 *
 *  NAME	
 *      display_graph - Give an overview of the graph structure on stdout
 *
 *  SYNOPSIS
 *      void display_graph(int g_id)
 *
 *  FUNCTION
 *      Print on the standard output a list of the points, their
 *      information content and the relation among the points.
 *      First a list of points is given; each with its own internal
 *      reference number. Then a list of relations in terms of these
 *      reference numbers is given.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *      The graph structure to be displayed must have information in it.
 *      When a non-existant graph identification number is provided this
 *      leads to undefined behaviour.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
     int i;

     for (i=0;i<g[g_id]->last;i++) {
        write_id(g[g_id]->array[i], i);
        write_sides(g[g_id]->array[i]);
     }
}

char **build_matrix(int g_id, int *n)
{
/****** build_matrix ***************************************************
 *
 *  NAME	
 *      build_matrix - Build an association matrix for the given graph
 *
 *  SYNOPSIS
 *      char **build_matrix(int g_id)
 *
 *  FUNCTION
 *      This routine builds a matrix that describes the directed graph's
 *      relations. If a relation between two elements exists the matrix
 *      position is set to 1, otherwise it is 0.
 *
 *  INPUTS
 *      g_id    - The graph identification number
 *      n       - The size of the square matrix
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      The matrix describingthe graph relations
 *
 *  MODIFIED GLOBALS
 *      Needs the global g structure
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 13-03-1997
 *
 ******************************************************************************/

    int		i, from, to;
    char	**m;
    side_ptr	*t;

    ED_CALLOC(m, "build_matrix", char *, g[g_id]->last);
    for (i=0;i<g[g_id]->last;i++) {
       ED_CALLOC(m[i], "build_matrix", char, g[g_id]->last);
    }
    for (i=0;i<g[g_id]->last;i++) {
       from = g[g_id]->array[i].id;
       t = g[g_id]->array[i].side;
       while (t != NULL && t->ptr != NO_ID) {
          to = g[g_id]->array[t->ptr].id;
          m[from][to] = 1;
          t = t->next;
       }
   }
   *n = g[g_id]->last;
   return m;
}

/* 
 * Now the local routines
 */

static void check_g_id(int g_id)
{
/****i* check_g_id ***************************************************
 *
 *  NAME	
 *      check_g_id - Check if the specified id lays within the bounds
 *
 *  SYNOPSIS
 *      static void check_g_id(int g_id)
 *
 *  FUNCTION
 *      Check if the specified graph identification number is within the
 *      range of allowed values. These values are: 0 < g_id < MAX_GRAPHS.
 *      The routine will exit with a FATAL error and an error message.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *      The routine should be extended with a verification of
 *      initialization status of a given graph.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    if (g_id < 0 || g_id > cur_graph) {
       syserr(FATAL,"check_g_id",
              "Graph id %d out of bounds (0,%d)\n",g_id,cur_graph);
    }
}

static side_ptr *create_side(int ptr)
{
/****i* create_side ***************************************************
 *
 *  NAME	
 *      create_side - Create a side element and correctly fill the structure
 *
 *  SYNOPSIS
 *      static side_ptr *create_side(int ptr)
 *
 *  FUNCTION
 *      Create a side structure in memory (allocate it) and fill the
 *      structure with the pointer information.
 *
 *  INPUTS
 *      ptr   - Pointer to an existing point in the graph structure
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      A pointer to the allocated and filled side_ptr structure. Upon an
 *      creation error this pointer is NULL.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    side_ptr *s;

    ED_CALLOC(s, "create_side", side_ptr, 1);
    s->ptr = ptr;
    s->next = NULL;

    return s;
}

static void clear_side(graph_elem *a)
{
/****i* clear_side ***************************************************
 *
 *  NAME	
 *      clear_side - Clear all pointer entries in an existing sides chain
 *
 *  SYNOPSIS
 *      static void clear_side(graph_elem *a)
 *
 *  FUNCTION
 *      Set the pointers of all the sides in a side chain to the NO_ID.
 *
 *  INPUTS
 *      a   - A pointer to the point information element of a graph
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    side_ptr *t;

    t = a->side;
    while (t != NULL && t->ptr != NO_ID) {
      t->ptr = NO_ID;
      t = t->next;
    }
}

static void remove_sides(graph_elem *a)
{
/****i* remove_sides ***************************************************
 *
 *  NAME	
 *      remove_sides - Remove the complete sides chain
 *
 *  SYNOPSIS
 *      static void remove_sides(graph_elem *a)
 *
 *  FUNCTION
 *      Free memory for all sides in the sides chain that is part of the
 *      specified graph element. The pointer to the sides chain is set to
 *      NULL upon return.
 *
 *  INPUTS
 *      a   - A pointer to the point information element of a graph
 *
 *  RESULTS
 *      The pointer to the sides chan in the graph element a structure is
 *      set to NULL.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    side_ptr *t, *p;

    t = a->side;
    while (t != NULL) {
      p = t;
      t = t->next;
      ED_FREE(p);
    }
}

static void add_side(side_ptr *s, int ptr)
{
/****i* add_side ***************************************************
 *
 *  NAME	
 *      add_side - Add a new side to the end of the sides chain
 *
 *  SYNOPSIS
 *      static void add_side(side_ptr *s, int ptr)
 *
 *  FUNCTION
 *       Routine to add a side element to a graph array element side chain.
 *       It should set the next pointer of the previous element only in case
 *       there is a previous element. This is not the case for the first side
 *       of the graph array element.
 *
 *  INPUTS
 *      s   - A pointer to the side chain end (this may be graph_elem->side)
 *      ptr - The pointer to an existing point in the current graph
 *
 *  RESULTS
 *      The sides chain is extended by one moer element, whose information
 *      content is set with the ptr value.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    side_ptr *t, *p;

    if (ptr != NO_ID) {
       p = t = s;
       while (t != NULL && t->ptr != NO_ID) {
          p = t;
          t = t->next;
       }
       if (t == NULL) {
          t = create_side(ptr);
          p->next = t;
       } else {
          t->ptr = ptr;
       }
    }
}


static void write_id(graph_elem a, int n)
{
/****i* write_id ***************************************************
 *
 *  NAME	
 *      write_id - Print to stdout a list of the point in the graph
 *
 *  SYNOPSIS
 *      static void write_id(graph_elem a, int n)
 *
 *  FUNCTION
 *      Print the internal reference number, and the point information
 *      for the point in the provided graph structure. Currently the
 *      provided information consist of point identification number and
 *      channel number.
 *
 *  INPUTS
 *      a    - The graph element from a graph structure
 *      n    - The reference number of the current point in the graph structure
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    printf("%3d Id: %d, Chan: %d ",n,a.id,a.chan);
}

static void write_sides(graph_elem a)
{
/****i* write_sides ***************************************************
 *
 *  NAME	
 *      write_sides - Print to stdout all relations from a given graph
 *
 *  SYNOPSIS
 *      static void write_sides(graph_elem a)
 *
 *  FUNCTION
 *      Print the relations as stored in all the sides of the graph element
 *      structure. The relations are shown in the form of a list of
 *      parenthesized reference numbers.
 *
 *  INPUTS
 *      a    - The graph element for which the relations should be printed.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    side_ptr *t;

    t = a.side;
    while (t != NULL && t->ptr != NO_ID) {
       printf(" (%d)", t->ptr);
       t = t->next;
    }
    printf("\n");
}

static int append_graph_entry(int g_id, int id, int chan, int ptr)
{
/****i* append_graph_entry ***************************************************
 *
 *  NAME	
 *      append_graph_entry - Append a graph element to the graph structure
 *
 *  SYNOPSIS
 *      static int append_graph_entry(int g_id, int id, int chan, int ptr)
 *
 *  FUNCTION
 *      Add the point information and the relation to the other point for
 *      a given new point in the graph structure.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      id    - Point identification number of arrow tail.
 *      chan  - Point channel number of arrow tail.
 *      ptr   - Pointer to the arrow head object already stored in the
 *              graph structure
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      The reference number of the newly added point.
 *
 *  MODIFIED GLOBALS
 *      The graph structure of the identified number is updated with the
 *      information provided.
 *
 *  NOTES
 *      If a memory allocation problem occurs the routine exits.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    check_g_id(g_id);

    g[g_id]->array[g[g_id]->last].id   = id;
    g[g_id]->array[g[g_id]->last].chan = (short int) chan;
    add_side(g[g_id]->array[g[g_id]->last].side, ptr);
    g[g_id]->last++;

    return (g[g_id]->last - 1);
}

static int get_side(int g_id, int *id1, int *chan1, int *id2, int *chan2)
{
/****i* get_side ***************************************************
 *
 *  NAME	
 *      get_side - Obtain very first side from a graph structures chain
 *
 *  SYNOPSIS
 *      static int get_side(int g_id, int *id1, int *chan1,
 *                                    int *id2, int *chan2)
 *
 *  FUNCTION
 *      Get the very first relation of the specified graph structure. The
 *      information of this relation is returned to the caller.
 *
 *      If the caller did not explicitely set the *id1 to NO_ID, he asks
 *      for a particular objects side.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      id1   - Identification number of point at arrow tail.
 *      chan1 - Channel number of point at arrow tail.
 *
 *  RESULTS
 *      id1   - Identification number of point at arrow tail.
 *      chan1 - Channel number of point at arrow tail.
 *      id2   - Identification number of point at arrow head.
 *      chan2 - Channel number of point at arrow head.
 *
 *  RETURNS
 *      FALSE - There is no side information in the specified graph structure.
 *      TRUE  - When the information return is that of the first relation.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int retcode = FALSE, always = TRUE;

    if (cur_side[g_id] == NULL ||
        cur_side[g_id]->ptr == NO_ID) {
       if ( *id1 != NO_ID) {
/*
 *        Go to the next element in the array
 */
          while ( always == TRUE ) {
             if (g[g_id]->last < cur_elem[g_id]++) break;
/*
 *           Get the side of this existing element
 */
             cur_side[g_id] =  g[g_id]->array[cur_elem[g_id]].side;
             if (cur_side[g_id]->ptr != NO_ID) break;
          }
       } else {
/*
 *        We were stepping the sides list of a particular object and hit
 *        the end
 */
          return retcode;
       }
    }
    if (cur_side[g_id]->ptr != NO_ID) {
       *id1   = g[g_id]->array[cur_elem[g_id]].id;
       *chan1 = g[g_id]->array[cur_elem[g_id]].chan;
       *id2   = g[g_id]->array[cur_side[g_id]->ptr].id;
       *chan2 = g[g_id]->array[cur_side[g_id]->ptr].chan;
       retcode = TRUE;
    }
    return retcode;
}

static int check_graph_entry(int g_id, int id, int chan)
{
/****i* check_graph_entry ***************************************************
 *
 *  NAME	
 *      check_graph_entry - Check existance of an element the graph structure
 *
 *  SYNOPSIS
 *      static int check_graph_entry(int g_id, int id, int chan)
 *
 *  FUNCTION
 *      Find out if the point specified by the input information is already
 *      present in the specified graph structure.
 *
 *  INPUTS
 *      g_id  - Graph identification number
 *      id    - Point identification number
 *      chan  - Point channel number
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      NO_ID - When the specified point is not present in the specified
 *              graph structure.
 *      Otherwise the reference number of the specified point is returned.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int i;

    check_g_id(g_id);

    for (i=0;i<g[g_id]->last;i++) {
       if (g[g_id]->array[i].id == id &&
           g[g_id]->array[i].chan == (short int) chan) {
          break;
       }
    }

    if (i == g[g_id]->last) i = NO_ID;
    return i;
}

static void check_array(int g_id)
{
/****** check_array ***************************************************
 *
 *  NAME	
 *      check_array - Check if graph structure needs reallocation of memory
 *
 *  SYNOPSIS
 *      static void check_array(int g_id)
 *
 *  FUNCTION
 *      This function verifies if there is still room for storing one
 *      additional point's information. If there is no more room, it will
 *      allocate one point's worth of additional memory.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      The graph structure of the specified graph is memory extended if
 *      there is need to do so.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    if (g[g_id]->last == g[g_id]->n){
/*
 *     Graph array is full, add one new array element
 */
       g[g_id]->n = g[g_id]->n + 1;
       ED_REALLOC(g[g_id]->array, "check_array", graph_elem, g[g_id]->n);
       g[g_id]->array[g[g_id]->last].id = NO_ID;
       g[g_id]->array[g[g_id]->last].chan = 0;
       g[g_id]->array[g[g_id]->last].side = create_side(NO_ID);
    }
}

static int add_graph_point(int g_id, int id, int chan)
{
/****i* add_graph_point ***************************************************
 *
 *  NAME	
 *      add_graph_point - Add information into the graph structure
 *
 *  SYNOPSIS
 *      static int add_graph_point(int g_id, int id, int chan)
 *
 *  FUNCTION
 *      This function adds the point information into the specified graph
 *      structure . It checks for the presence of the point in the graph,
 *      and either adds a new graph element (optionally enlarging the graph
 *      structure) or supplements the sides list of the existing point.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      id    - Point identification number.
 *      chan  - Point channel number.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      The reference number of the newly added point.
 *
 *  MODIFIED GLOBALS
 *      The graph structure of the specified graph is memory extended if
 *      there is need to do so.
 *
 *  NOTES
 *      If a memory allocation problem occurs the routine exits.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    check_array(g_id);
    return (append_graph_entry(g_id, id, chan, NO_ID));
}

static int add_graph_entry(int g_id, int id, int chan, int ptr)
{
/****** add_graph_entry ***************************************************
 *
 *  NAME	
 *      add_graph_entry - Add a relation into the specified graph structure
 *
 *  SYNOPSIS
 *      static int add_graph_entry(int g_id, int id, int chan, int ptr)
 *
 *  FUNCTION
 *      Routine to add relation information into the specified graph structure.
 *      It checks for the presence of the point in the graph, and either adds
 *      a new graph element entry (optionally enlarging the graph structure)
 *      and supplements the sides list of the existing point.
 *
 *  INPUTS
 *      g_id  - Graph identification number.
 *      id    - Point identification number.
 *      chan  - Point channel number.
 *      ptr   - Pointer to reference number of array head.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      Reference number of the newly added point or of the existing point.
 *
 *  MODIFIED GLOBALS
 *      The graph of the specfied graph structure is updated with the
 *      information provided.
 *
 *  NOTES
 *      If a memory allocation problem occurs the routine exits.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int entry, retcode = FALSE;

    entry = check_graph_entry(g_id, id, chan);
    if (entry == NO_ID) {
       retcode = TRUE;
       check_array(g_id);
       entry = append_graph_entry(g_id, id, chan, NO_ID);
    }
    add_side(g[g_id]->array[ptr].side, entry);

    return retcode;
}

