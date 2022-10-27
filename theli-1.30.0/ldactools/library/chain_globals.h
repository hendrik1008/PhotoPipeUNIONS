#include "chain_types.h"
extern chain_struct	*chain_init();
extern int		chain_free(chain_struct *c),
			chain_add_desc_item(chain_struct *c, char *name,
                                            ttype type),
			chain_get_desc_item(chain_desc d, char *name),
			chain_rem_desc_item(chain_struct *c, char *name),
			chain_add_data_elem(chain_struct *c, chain_struct *t),
			chain_rem_data_elem(chain_struct *c, chain_elem *e),
			chain_add_data_item(chain_struct *c, char *name,
                                            ttype type, char *val);
extern chain_elem	*chain_get_data_elem(chain_struct *c, char **name,
                                              char **val, int n);
extern ttype		chain_get_data_item_type(chain_struct *c, char *name);
extern char		*chain_get_data_item(chain_elem *e, chain_desc d,
					     char *name);
