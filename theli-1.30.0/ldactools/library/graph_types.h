typedef struct structside {	/* Chain structure for sides */
    int                ptr;	/* Index to point on other side */
    struct structside *next;	/* Pointer to the next side structure */
} side_ptr;


typedef struct {		/* Graph structure element */
    int       id;		/* Point id number */
    short int chan;		/* Point channel attribute */
    side_ptr  *side;		/* Pointer to chain of sides */
} graph_elem;

typedef struct {		/* Main container of graph structure */
    int         n;		/* Number of elements in graph array */
    int         last;		/* Last filled graph array element */
    graph_elem  *array;		/* Pointer to first element of graph array */
} graph_struct;

