/* History:
   13.01.2000:
   nr_free_vector -> free_vector

   15.06.2001:
   removed the #define constructs around syserr
   keeping only the first definition works on all
   platforms so far.

   19.07.01:
   added definitions for svector and free_svector
   functions

   09.09.01:
   resolved long int - LONG issues (functions lvector,
   free_lvector, fac, divide_fac)

   29.01.2005:
   I add a definition for the get_basename function.

   09.03.2006:
   I removed the last argument from the NR free_... functions.
   That argument is not used in the functions.

   27.03.2006:
   I protect this include file from multiple inclusions by
   a define construct.
*/

/*
 *  Global function and variable definitions
 */


#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __UTILS_GLOBALS_H__
#define __UTILS_GLOBALS_H__

#include <stdio.h>

#include <stdarg.h>
#include "utils_defs.h"
#include "utils_types.h"
#include "utils_macros.h"

/* for machines where a long has 8 bytes */
#ifndef ULONGDEF
#define ULONGDEF
#ifdef INT64
typedef int             LONG;
typedef unsigned int    ULONG;
#else
typedef long            LONG;
typedef unsigned long   ULONG;
#endif
#endif

extern char *build_command_line(int, char **);

extern char *get_basename(const char *);

extern void	
                syserr(errtype, const char *, ...),
		free_vector(float *, int),   /* Free a float vector */
		free_dvector(double *, int), /* Free a double vector */
		free_ivector(int *, int),    /* Free an int vector */
		free_svector(short int *, int),    /* Free a short int vector */
		free_lvector(LONG *, int),  /* Free long int vector */
		free_matrix(float **,int ,int ,int),
		free_dmatrix(double **,int ,int ,int),
		free_imatrix(int **,int ,int ,int),
		nrerror(char *);

extern float	*vector(int, int),	/* Create a one dim float vector */
                **matrix(int ,int ,int ,int);

extern double	*dvector(int, int),	/* Create a one dim double vector */
		**dmatrix(int ,int ,int ,int);

extern int	*ivector(int, int),	/* Create a one dim int vector */
		**imatrix(int ,int ,int ,int);

extern short int *svector(int, int);	/* Create a one dim short int vector */

extern LONG	*lvector(int, int);	/* Create a one dim int vector */

extern LONG	fac(int);               /* Calcluate the faculty */
extern LONG	divide_fac(int, int);   /* Calcluate the faculty division */

extern int 	reject(float **, int **, int, float, float);
extern char	*leading_zeros(int, int),
		*to_lower(char *);

#endif

#ifdef __cplusplus
}
#endif

