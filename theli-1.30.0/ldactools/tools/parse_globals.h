#define MAXCHAR 255

#include "tpar.h"
extern tree_struct *parse_line(char *, char **, int, char *);
extern int question(tree_struct *, desc_struct *, rec_struct *);
extern char **find_idents(tree_struct *, int *);
extern double eval(tree_struct *, desc_struct *, rec_struct *);
extern void set_vec_col(tree_struct *, int);
