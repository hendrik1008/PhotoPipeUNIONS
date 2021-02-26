/* History:
   13.01.2000:
   nr_free_vector -> free_vector

   15.06.2001:
   removed the #define constructs around syserr
   keeping only the first definition works on all
   platforms so far.
   removed compiler warnings under DEC

   19.07.01:
   wrote functions svector and free_svector to
   allocate and free vectors with short int
   arrays.

   09.09.01:
   resolved long int - LONG issues (functions lvector,
   free_lvector, fac, divide_fac)

   29.01.2005:
   - I added the get_basename utility function from the
     eclipse library
   - build_command_line now takes the basename from the first
     argument given.

   09.03.2006:
   I removed the last argument from the NR free_... functions.
   That argument is not used in the functions.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils_globals.h"

/*
 *  Local function declarions
 */

char *build_command_line(int argc, char **argv)
{
    char *line = NULL;
    int  i, l=0;

    if(argc > 0 && argv != NULL)
    {
      for(i=0; i < argc; i++)
      {
        l += strlen(argv[i])+1;
      }

      ED_CALLOC(line, "build_command_line", char, l+1);
      strcpy(line, get_basename(argv[0]));

      for (i=1; i<argc; i++)
      {
        strcat(line, " ");
        strcat(line, argv[i]);
      }

    }
    return line;
}

void nrerror(char *);

/* 
 *  First the global function definitions
 */

/****** syserr ***************************************************
 *
 *  NAME	
 *      syserr - Print error message and optionall exit program
 *
 *  SYNOPSIS
 *      void syserr(code, va_alist)
 *
 *  FUNCTION
 *      Print a string with optional arguments to the stdout, either as
 *      an information message, warning message, or error message. For the
 *      latter the routine exits.
 *
 *      The allowed codes are defined in the utils_types.h file and are:
 *      INFO, WARNING, or FATAL. The argument list in va_alist is like
 *      the arguement list in the standard C printf funtion.
 *
 *  INPUTS
 *      code     - Severity code, one of INFO, WARNING, or FATAL.
 *      va_alist - The argument list for message printing (like printf)
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      The routine exists on the FATAL code.
 *
 *  BUGS
 *
 *  AUTHOR
 *      Basic routine as given in vprintf man page
 *      E.R. Deul  - dd 16-02-1996 (Modified)
 *
 ******************************************************************************/
void syserr(errtype code, const char *func, ...)
{
    va_list args;
    char *fmt;
    va_start(args, func);
/*
 *  print out name of function causing error
 */
    switch (code) {
    case INFO:
       (void)fprintf(stderr, "INFO(%s): ", func);
       fmt = va_arg(args, char *);
       if (strlen(fmt) > 0) (void)vfprintf(stderr, fmt, args);
       break;
    case WARNING:
       (void)fprintf(stderr, "WARNING(%s): ", func);
       fmt = va_arg(args, char *);
       if (strlen(fmt) > 0) (void)vfprintf(stderr, fmt, args);
       break;
    case FATAL:
       (void)fprintf(stderr, "ERROR(%s): ", func);
       fmt = va_arg(args, char *);
       if (strlen(fmt) > 0) (void)vfprintf(stderr, fmt, args);
       exit(1);
       break;
    }
    va_end(args);
}

float *vector(int nl, int nh)
{
/****** vector ***************************************************
 *
 *  NAME	
 *      vector - Allocate memory for a floating array
 *
 *  SYNOPSIS
 *      float *vector(int nl, int nh)
 *
 *  FUNCTION
 *      Allocate memory for an array of floats where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      A pointer to the nl index array element of the just allocated array
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	float *v;

	v=(float *)malloc((unsigned) (nh-nl+1)*sizeof(float));
	if (!v) nrerror("allocation failure in vector()");
	return v-nl;
}

double *dvector(int nl, int nh)
{
/****** dvector ***************************************************
 *
 *  NAME	
 *      dvector - Allocate memory for a dlouble array
 *
 *  SYNOPSIS
 *      double *dvector(int nl, int nh)
 *
 *  FUNCTION
 *      Allocate memory for an array of doubles where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      A pointer to the nl index array element of the just allocated array
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	double *v;

	v=(double *)malloc((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) nrerror("allocation failure in vector()");
	return v-nl;
}

int *ivector(int nl, int nh)
{
/****** ivector ***************************************************
 *
 *  NAME	
 *      ivector - Allocate memory for a integer array
 *
 *  SYNOPSIS
 *      int *ivector(int nl, int nh)
 *
 *  FUNCTION
 *      Allocate memory for an array of ints where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      A pointer to the nl index array element of the just allocated array
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	int *v;

	v=(int *)malloc((unsigned) (nh-nl+1)*sizeof(int));
	if (!v) nrerror("allocation failure in ivector()");
	return v-nl;
}
short int *svector(int nl, int nh)
{
/****** ivector ***************************************************
 *
 *  NAME	
 *      svector - Allocate memory for a short integer array
 *
 *  SYNOPSIS
 *      int *svector(int nl, int nh)
 *
 *  FUNCTION
 *      Allocate memory for an array of ints where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      A pointer to the nl index array element of the just allocated array
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	short int *v;

	v=(short int *)malloc((unsigned) (nh-nl+1)*sizeof(short int));
	if (!v) nrerror("allocation failure in svector()");
	return v-nl;
}


LONG *lvector(int nl, int nh)
{
/****** lvector ***************************************************
 *
 *  NAME	
 *      lvector - Allocate memory for a long integer array
 *
 *  SYNOPSIS
 *      long int *lvector(int nl, int nh)
 *
 *  FUNCTION
 *      Allocate memory for an array of ints where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      A pointer to the nl index array element of the just allocated array
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	LONG *v;

	v=(LONG *)malloc((unsigned) (nh-nl+1)*sizeof(LONG));
	if (!v) nrerror("allocation failure in lvector()");
	return v-nl;
}

float **matrix(int nrl,int nrh,int ncl,int nch)
{
        int i;
        float **m;

        m=(float **) malloc((unsigned) (nrh-nrl+1)*sizeof(float*));
        if (!m) nrerror("allocation failure 1 in matrix()");
        m -= nrl;

        for(i=nrl;i<=nrh;i++) {
                m[i]=(float *) malloc((unsigned) (nch-ncl+1)*sizeof(float));
                if (!m[i]) nrerror("allocation failure 2 in matrix()");
                m[i] -= ncl;
        }
        return m;
}

double **dmatrix(int nrl,int nrh,int ncl,int nch)
{
        int i;
        double **m;

        m=(double **) malloc((unsigned) (nrh-nrl+1)*sizeof(double*));
        if (!m) nrerror("allocation failure 1 in dmatrix()");
        m -= nrl;

        for(i=nrl;i<=nrh;i++) {
                m[i]=(double *) malloc((unsigned) (nch-ncl+1)*sizeof(double));
                if (!m[i]) nrerror("allocation failure 2 in dmatrix()");
                m[i] -= ncl;
        }
        return m;
}

int **imatrix(int nrl,int nrh,int ncl,int nch)
{
        int i,**m;

        m=(int **)malloc((unsigned) (nrh-nrl+1)*sizeof(int*));
        if (!m) nrerror("allocation failure 1 in imatrix()");
        m -= nrl;

        for(i=nrl;i<=nrh;i++) {
                m[i]=(int *)malloc((unsigned) (nch-ncl+1)*sizeof(int));
                if (!m[i]) nrerror("allocation failure 2 in imatrix()");
                m[i] -= ncl;
        }
        return m;
}


void free_vector(float *v, int nl)
{
/****** free_vector ***************************************************
 *
 *  NAME	
 *      free_vector - Deallocate memory of a floating array
 *
 *  SYNOPSIS
 *      void free_vector(float *v, int nl, int nh)
 *
 *  FUNCTION
 *      Free the memory of an array of floats where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	free((char*) (v+nl));
}

void free_dvector(double *v, int nl)
{
/****** free_dvector ***************************************************
 *
 *  NAME	
 *      free_dvector - Deallocate memory of a double array
 *
 *  SYNOPSIS
 *      void free_dvector(double *v, int nl, int nh)
 *
 *  FUNCTION
 *      Free the memory of an array of doubles where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	free((char*) (v+nl));
}

void free_ivector(int *v, int nl)
{
/****** free_ivector ***************************************************
 *
 *  NAME	
 *      free_ivector - Deallocate memory of a integer array
 *
 *  SYNOPSIS
 *      void free_vector(int *v, int nl, int nh)
 *
 *  FUNCTION
 *      Free the memory of an array of ints where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	free((char*) (v+nl));
}

void free_svector(short int *v, int nl)
{
/****** free_svector ***************************************************
 *
 *  NAME	
 *      free_svector - Deallocate memory of a integer array
 *
 *  SYNOPSIS
 *      void free_svector(short int *v, int nl, int nh)
 *
 *  FUNCTION
 *      Free the memory of an array of ints where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	free((char*) (v+nl));
}

void free_lvector(LONG *v, int nl)
{
/****** free_lvector ***************************************************
 *
 *  NAME	
 *      free_lvector - Deallocate memory of a long integer array
 *
 *  SYNOPSIS
 *      void free_lvector(long int *v, int nl, int nh)
 *
 *  FUNCTION
 *      Free the memory of an array of ints where the indices run from
 *      nl to nh inclusive.
 *
 *  INPUTS
 *      nl    - Staring index for array
 *      nh    - Ending index for array
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This code is part of the Numerical Recipies C implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	free((char*) (v+nl));
}

void free_matrix(float **m,int nrl,int nrh,int ncl)
{
        int i;

        for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
        free((char*) (m+nrl));
}

void free_dmatrix(double **m,int nrl,int nrh,int ncl)
{
        int i;

        for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
        free((char*) (m+nrl));
}

void free_imatrix(int **m,int nrl,int nrh,int ncl)
{
        int i;

        for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
        free((char*) (m+nrl));
}

/* 
 *  Then the local function definitions
 */

void nrerror(char *error_text)
{
/****i* nerror ***************************************************
 *
 *  NAME	
 *      nrerror - Print error message on stderr
 *
 *  SYNOPSIS
 *      static void nrerror(char *error_text)
 *
 *  FUNCTION
 *      Print the message on the standard error pipe.
 *
 *  INPUTS
 *      error_text - Character string to be printed
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      This routone is part of the Numerical Recipies C - implementation
 *
 *  BUGS
 *
 *  AUTHOR
 *      Numerical Recipies
 *
 ******************************************************************************/
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}


LONG fac(int num)
{
/****** fac ***************************************************
 *
 *  NAME	
 *      fac - Calculate the faculty
 *
 *  SYNOPSIS
 *      LONG fac(int num)
 *
 *  FUNCTION
 *      Calculate the faulcty of a given number.
 *
 *  INPUTS
 *      num  -  Integer of which the calculate the faculty
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      The faculty of the input parameter
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      A. Holl  - dd 01-01-1994
 *
 ******************************************************************************/
   LONG k;

   for(k = 1; num > 1; num--) k *= num;
   return k;
}

LONG divide_fac(int num1, int num2)
{
/****** fac ***************************************************
 *
 *  NAME	
 *      divide_fac - Calculate fac(num1) / fac(num2)
 *
 *  SYNOPSIS
 *      LONG divide_fac(int num1, int num2)
 *
 *  FUNCTION
 *      Calculate the faculty division of a two numbers.
 *
 *  INPUTS
 *      num1  -  Integer of which the calculate the counter faculty
 *      num2  -  Integer of which the calculate the denominator faculty
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      The faculty division of the input parameters
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 13-11-1996
 *
 ******************************************************************************/
   LONG k;

   for(k = 1; num1 > num2; num1--) k *= num1;
   return k;
}

/****** get_basename ***************************************************
 *
 *  NAME	
 *      get_basename - return basename of a filename, i.e.
 *      the filename without prefix path.
 *
 *  SYNOPSIS
 *      LONG divide_fac(int num1, int num2)
 *
 *  FUNCTION
 *      return basename of a filename, i.e.
 *      the filename without prefix path.
 *
 *  INPUTS
 *      filename  -  char * from the file for that the basename
 *                   has to be determined
 *
 *  RESULTS
 *      A char pointer to the basename part of the provided path
 *
 *  RETURNS
 *      A char pointer to the basename part of the provided path
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      Eclipse Library
 *
 ******************************************************************************/

char * get_basename(const char *filename)
{
  char *p ;
  p = strrchr (filename, '/');
  return p ? p + 1 : (char *) filename;
}



