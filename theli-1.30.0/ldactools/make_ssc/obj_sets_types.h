/*
 * Processing in terms of object sets
 */

typedef struct {
   int          *seqnr,         /* List of sequence numbers */
                *chan,          /* List of channel numbers */
                *fpos,          /* List of field numbers */
                *indx,          /* List of pointers into main store */
                nelem,          /* Number of elements in structure */
                nalloc;         /* Number of elements allocated */
   short int    *ret;           /* Logical element is retained or not */
} set_struct;

