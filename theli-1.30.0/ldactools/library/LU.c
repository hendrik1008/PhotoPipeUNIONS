/* This file contains routines to solve linear equaltion with LU
   decomposition.

   Thomas Erben (terben@astro.uni-bonn.de)
*/

/* HISTORY INFORMATION
   ===================

   24.08.2010:
   I resolved an int <-> float variable mismatch in LUfactor
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include "LU_globals.h"

/*-------------------------------------------------------------------------*/

/**
   @brief	Perform LU factorisation of a square matrix
   @param	n	dimension of A (square nxn matrix)
   @param	pivot	line permutations of matrix A (pivot strategy)
   @return      int     (1 if singular matrix detected; 0 otherwise)

   The function performs an LU factorisation of a square matrix.
   The original input matrix A is replaced by the LU decomposed
   form. The diagonal of the output matrix contains the diagonal
   of the U matrix (the L matrix has unity diagonal elements).
   The upper ands lower parts contain the corresponding parts of
   the U and L matrices. During the function the matrix lines
   may be permuted to enable robust factorisation by a pivot
   strategy. The performed line permutaions are stored in the
   'pivot' vector.
   Memory (de)allocation for the vector 'pivot' has to be
   handled outside this function. It has to contain space for
   'n' elements when entering this function.
   The routine does not check whether the input matrix A contains
   data.
   The algorithm closely follows an implementation in the numerical
   meschach matrix library.

 */
/*--------------------------------------------------------------------------*/

int LUfactor(LU_realtype **A, int n, int *pivot)
{
  int      i, j, k;
  int      i_max;
  int      singular = 0;
  int      inttemp;
  LU_realtype max1, floattemp;
  LU_realtype *scale = NULL;

  scale = (LU_realtype *)malloc(n * sizeof(LU_realtype));

  if (scale == NULL) {
    fprintf(stderr, "error in memory allocation in LUfactor !!");
    exit(1);
  }

  /* initialise pivot with identity permutation */
  for (i = 0; i < n; i++) {
    pivot[i] = i;
  }

  /* set scale parameters */
  for (i = 0; i < n; i++) {
    max1 = 0.0;
    for (j = 0; j < n; j++) {
      floattemp = fabs(A[i][j]);
      max1 = LU_max(max1, floattemp);
    }
    scale[i] = max1;
  }

  /* main loop */
  for (k = 0; k < n - 1; k++) {
    /* find best pivot row */
    max1 = 0.0; i_max = -1;
    for (i = k; i < n; i++) {
      if (fabs(scale[i]) >= LU_tiny * fabs(A[i][k])) {
        floattemp = fabs(A[i][k]) / scale[i];
        if (floattemp > max1) {
          max1 = floattemp;
          i_max = i;
        }
      }
    }

    /* if no pivot then ignore column k... */
    if (i_max == -1) {
      /* set pivot entry A[k][k] exactly to zero,
         rather than just "small" */
      /* the matrix is singular in this case */
      A[k][k]  = 0.0;
      singular = 1;
      continue;
    }

    /* do we pivot ? */
    if (i_max != k) {           /* yes we do... */
      inttemp      = pivot[i_max];
      pivot[i_max] = pivot[k];
      pivot[k]     = inttemp;

      for (j = 0; j < n; j++) {
        floattemp   = A[i_max][j];
        A[i_max][j] = A[k][j];
        A[k][j]     = floattemp;
      }
    }

    /* row operations */
    for (i = k + 1; i < n; i++) {       /* for each row do... */
      /* Note: divide by zero should never happen */
      floattemp = A[i][k] = A[i][k] / A[k][k];

      for (j = k + 1; j < n; j++) {
        A[i][j] -= floattemp * A[k][j];
      }
    }
  }
  free(scale);
  return(singular);
}

/*-------------------------------------------------------------------------*/

/**
   @brief	Solve a linear system of equations with an LU factorised matrix.
   @param	A       Matrix in LU factorisation
   @param	n	dimension of A (nxn matrix)
   @param	pivot	line permutations of matrix A (pivot strategy)
   @param       b       right hand side of the linear equation A*x=b
   @param       x       solution vector (NULL is b should be replaced!)
   @return      void

   The function solves for 'x' in the the linear equation system A*x=b where
   A is a nxn matrix in LU decomposed form (calculated together with the 'pivot'
   vector by a call to the function LUfactor).
   Memory (de)allocation for the solution vector 'b' have to be
   handled outside this function. It has to contain space for
   'n' elements when entering this function. The function exits and
   terminates the calling program if the system of equations is ill
   conditioned. The routine does not check whether the argument pointers
   contain data or whether the matrix A is in the correct form.
   The algorithm closely follows an implementation in the numerical
   meschach matrix library.

 */
/*--------------------------------------------------------------------------*/

void LUsolve(LU_realtype **A, int n, int *pivot, LU_realtype *b, LU_realtype *x)
{
  int      i, j;
  LU_realtype sum;
  LU_realtype *solution = NULL;


  /* if 'x' is the NULL pointer we want to replace the rhs vector 'b'
     with the result
  */
  if (x == NULL) {
    solution = (LU_realtype *)malloc(n * sizeof(LU_realtype));
  } else {
    solution = x;
  }

  /* apply the Matrix permutations, applied during
     LU decomposition to matrix A, also to the
     right-hand-side vector b
   */
  for (i = 0; i < n; i++) {
    solution[i] = b[pivot[i]];
  }

  /* the diagonal of the lower left matrix L is 1 */
  for (i = 0; i < n; i++) {
    sum = solution[i];
    for (j = 0; j < i; j++) {
      sum -= A[i][j] * solution[j];
    }
    solution[i] = sum;
  }

  for (i = (n - 1); i >= 0; i--) {
    sum = solution[i];
    for (j = i + 1; j < n; j++) {
      sum -= A[i][j] * solution[j];
    }

    if (fabs(A[i][i]) <= LU_tiny * fabs(sum)) {
      fprintf(stderr, "LUsolve applied to an ill conditioned system !!\n");
      exit(1);
    }

    solution[i] = sum / A[i][i];
  }

  if (x == NULL) {
    for (i = 0; i < n; i++) {
      b[i] = solution[i];
    }
    free(solution);
  }
}

/*-------------------------------------------------------------------------*/

/**
   @brief	calculate the inverse of a square matrix.
   @param	A       Matrix in LU factorised form
   @param	B       inverse matrix of A at exit of this function
   @param	n	dimension of A (nxn matrix)
   @param	pivot	line permutations of matrix A (pivot strategy)
   @return      void

   The function calculates the inverse of a square matrix A which has to
   be provided in LU decomposed form (calculated together with the 'pivot'
   vector by the function LUfactor).
   Memory (de)allocation for the result matrix 'B' have to be
   handled outside this function. It has to contain space for
   'nxn' elements when entering this function. The function exits and
   terminates the calling program if the input matrix is evaluated as close
   to singular. The routine does not check whether the argument pointers
   contain data or whether the matrix A is in the correct form.
   The algorithm closely follows an implementation in the numerical
   meschach matrix library.

 */
/*--------------------------------------------------------------------------*/

void LUinverse(LU_realtype **A, LU_realtype **B, int n, int *pivot)
{
  LU_realtype *lhs, *rhs;       /* the left hand side will consecutively
                                contain unity vectors and the right hand
                                side corresponding columns of the
                                inverse matrix */

  int i, j;

  lhs = (LU_realtype *)malloc(n * sizeof(LU_realtype));

  if (lhs == NULL) {
    fprintf(stderr, "error in memory allocation in LUinverse !!");
    exit(1);
  }

  rhs = (LU_realtype *)malloc(n * sizeof(LU_realtype));

  if (rhs == NULL) {
    fprintf(stderr, "error in memory allocation in LUinverse !!");
    exit(1);
  }

  for (i = 0; i < n; i++) {
    /* set left hand side vector */
    for (j = 0; j < n; j++) {
      lhs[j] = 0.;
    }
    lhs[i] = 1.0;

    /* calculate column of the inverse matrix
       and insert it into B */
    LUsolve(A, n, pivot, lhs, rhs);

    for (j = 0; j < n; j++) {
      B[j][i] = rhs[j];
    }
  }

  free(rhs);
  free(lhs);
}

