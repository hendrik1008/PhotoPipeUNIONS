#ifndef __LU_GLOBALS__
#define __LU_GLOBALS__

/* the type for floating point numbers used in this module */
#define LU_realtype float  /* can be float or double */
/* Numbers smaller than LU_tiny are treated as numerically zero */
#define LU_tiny       1.0e-15
#define LU_max(a, b)  ((a) > (b) ? (a) : (b))

/* perform LU factorisation of a square matrix */
int LUfactor(LU_realtype **, int, int *);

/* Solve a linear system of equations with an LU factorised matrix. */
void LUsolve(LU_realtype **, int, int *, LU_realtype *, LU_realtype *);

/* calculate the inverse of a square matrix. */
void LUinverse(LU_realtype **, LU_realtype **, int, int *);

#endif
