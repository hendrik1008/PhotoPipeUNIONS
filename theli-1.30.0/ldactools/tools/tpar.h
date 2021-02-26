#include <fitscat_defs.h>

/* HISTORY COMMENTS:
- 23.01.2012:
  I changed the tree_struct structure to better handle vector quantities
  in ldacfilter conditions.
*/

enum tree_types {UNKNOWN, FIELD, L_INT, L_FLOAT};

typedef union {
  int fieldnum;
  int i;
  double f;
} tree_param;

typedef struct tree_elem {
  char		oper[20];
  int		type;
  int           component; /* on which component do we apply
                              calculations in the case of vector
                              quantities */
  char		field[40];
  tree_param	par;
  struct tree_elem *l;
  struct tree_elem *r;
} tree_struct;

typedef union {
    double	*d;
    float	*f;
    short int	*i;
    LONG	*l;
    char	*b;
    char	s[10];
} rec_struct;

typedef struct {
    t_type	type;
    char	name[MAXCHAR];
    int		depth;
} desc_struct;

