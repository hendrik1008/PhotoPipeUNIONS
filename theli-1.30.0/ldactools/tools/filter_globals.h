/*
  21.03.02:
  updated the program so that it can mask filtered
  objects instead of deleting them

  29.01.2005:
  I added argc and argv to the argument list of filter
  as they are needed for writing the HISTORY to the
  output catalog.

*/

extern void filter(char *, char *, char *, char *, int,
                   int, char **, int, char *);
