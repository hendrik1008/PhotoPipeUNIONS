#include "obj_sets_types.h"

/*
 * 15.07.01:
 * removed definition of function set_check_elem to
 * avoid compiler warnings. The function is declared
 * static and hence does not need to be in the header
 * file.
 */

extern int      set_add_elem(set_struct *, int, int, int, int, short int, int),
		empty_set(set_struct *),
		set_get_elem(set_struct *, int *, int *, int *, int *,
                             short int *),
		set_equal(set_struct *, set_struct *),
		set_partoff(set_struct *, set_struct *);


extern set_struct	*set_init(),
			*set_copy(set_struct *),
			*set_diff(set_struct *, set_struct *),
			*set_union(set_struct *, set_struct *),
			*set_eq_fpos(set_struct *, int),
			*set_eq_ret(set_struct *, int),
			*set_ne_ret(set_struct *, int);


extern void	set_free(set_struct **);
