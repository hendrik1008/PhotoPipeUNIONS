#include "datatypes.h"

typedef struct {
    char	**names;
    ttype	*types;
    int		*bpos, nelem, aelem, bsize;
} chain_desc;

typedef struct link {
    char	*bits;
    struct link	*prev, *next;
} chain_elem;

typedef struct {
    chain_desc	desc;
    chain_elem  *chain;
} chain_struct;

