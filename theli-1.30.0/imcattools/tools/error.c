/**
 ** miscellaneous service routines
 **/

/*
  09.04.2005:
  function error_exit:
  I added information to the error message printed

  02.07.2014:
  I added a format string to an fprintf statement which
  prints a astring (security fix; fprintf commands without
  format specifier should not be used any more).
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "error.h"

void	error_exit(char *message)
{
  fprintf(stderr, "Error: ");
  fprintf(stderr, "%s", message);
  fprintf(stderr, "\nAborting program !!\n");
  exit(1);
}


