/*
 * C version of the source extraction program used in the Astroscan pipeline
 *
 * Types definition include file
 *
 * History
 * Creation: dd 25-09-1994  E.R. Deul
 * Modified: dd 21-02-1995  E.R. Deul
 * Modified: dd 16-02-1996  E.R. Deul
 *
 */

typedef enum {BOOLEAN, FLOAT, INT, STRING, IARRAY, FARRAY, TEXT, TEXTVEC} mem_type;
typedef enum {YES, NO} bool;

typedef struct {
   char   name[OPTLEN];
   mem_type type;
   void   *ptr;
   int    imin, imax;
   double dmin, dmax;
   int    ival;
   double dval;
   bool   bval;
   char   cval[OPTLEN];
   char   list[MAXOPTS][OPTLEN];
   int    nelem;
   char	  *expl;
} option;
