#include "make_perms_defs.h"

extern int	make_perms(int, int, perm_struct *),
		read_perms(char *, int, perm_struct *);
int             largest_perms(int, int, perm_struct *, int);
