/* History:
   13.01.2000:
   nr_free_vector -> free_vector

   19.07.01:
   wrote function ssort1 for short int arrays
   according to isort1 for ints.

   30.01.03:
   functions indexx and indexi:
   better treatment for the case n==1.
   The one index element remaining is set instead
   of simply quitting the function

   24.01.05:
   I removed superfluous nrerror definitions

   09.03.2006:
   I removed the last argument from the NR free_... functions.
   That argument is not used in the functions.

   17.03.2006:
   I removed not needed quicksort routines from Numerical
   Recipes.

   27.03.2006:
   I completely rewrote the codes for sorting. Most functions
   have a new name and the actual sorting algorithms were
   replaced by faster ones.
*/

#include <stdio.h>
#include "sort_globals.h"
#include "utils_globals.h"

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of integers in ascending order
  @param	n	number of elements in the int array
  @param	ra 	Array of ints to consider
  @return	void

  Sort on an array of ints. The sort is realised by determining an index
  array to the data an then rearranging the actual elements.
 */
/*--------------------------------------------------------------------------*/

void isort(int n, int *ra)
{
        int j,k,*idx;
        int *itmp;

        k = n-1;
        idx=ivector(0,k);
        itmp=ivector(0,k);
        iindex(n,ra,idx);

        for (j=0; j<=k; j++) itmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = itmp[idx[j]];

        free_ivector(itmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of shorts in ascending order
  @param	n	number of elements in the short array
  @param	ra 	Array of shorts to consider
  @return	void

  Sort on an array of ints. The sort is realised by determining an index
  array to the data an then rearranging the actual elements.
 */
/*--------------------------------------------------------------------------*/

void ssort(int n, short int *ra)
{
        int j,k;
        int *idx;
        short int *stmp;

        k = n-1;
        idx=ivector(0,k);
        stmp=svector(0,k);
        sindex(n,ra,idx);

        for (j=0; j<=k; j++) stmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = stmp[idx[j]];

        free_svector(stmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of floats in ascending order
  @param	n	number of elements in the float array
  @param	ra 	Array of floats to consider
  @return	void

  Sort on an array of ints. The sort is realised by determining an index
  array to the data an then rearranging the actual elements.
 */
/*--------------------------------------------------------------------------*/

void fsort(int n, float *ra)
{
        int	j,k,*idx;
        float	*ftmp;

        k = n-1;
        idx=ivector(0,k);
        ftmp=vector(0,k);
        findex(n,ra,idx);

        for (j=0;j<=k;j++) ftmp[j] = ra[j];
        for (j=0;j<=k;j++) ra[j]   = ftmp[idx[j]];

        free_vector(ftmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of integers and an associated int array in ascending order
  @param	n	number of elements in the int array
  @param	ra 	Key Array of ints to consider
  @param	rb 	Associated int Array to consider
  @return	void

  Sort on an array of ints (key array). An associated array of ints
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated array.
 */
/*--------------------------------------------------------------------------*/

void iisort(int n, int *ra, int *rb)
{
        int j,k,*idx;
        int *itmp;

        k = n-1;
        idx=ivector(0,k);
        itmp=ivector(0,k);
        iindex(n,ra,idx);

        for (j=0; j<=k; j++) itmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = itmp[idx[j]];
        for (j=0; j<=k; j++) itmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = itmp[idx[j]];

        free_ivector(itmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of floats and an associated float array in ascending order
  @param	n	number of elements in the float arrays
  @param	ra 	Key Array of floats to consider
  @param	rb 	Associated float Array to consider
  @return	void

  Sort on an array of ints (key array). An associated array of ints
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated array.
 */
/*--------------------------------------------------------------------------*/

void ffsort(int n, float *ra, float *rb)
{
        int j,k,*idx;
        float *ftmp;

        k = n-1;
        idx=ivector(0,k);
        ftmp=vector(0,k);
        findex(n,ra,idx);

        for (j=0; j<=k; j++) ftmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = ftmp[idx[j]];
        for (j=0; j<=k; j++) ftmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = ftmp[idx[j]];

        free_vector(ftmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of floats and two associated float arrays in ascending order
  @param	n	number of elements in the float arrays
  @param	ra 	Key Array of floats to consider
  @param	rb 	First Associated float Array to consider
  @param	rc 	Second Associated float Array to consider
  @return	void

  Sort on an array of floats (key array). Two associated arrays of floats
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated arraya.
 */
/*--------------------------------------------------------------------------*/

void fffsort(int n, float *ra, float *rb, float *rc)
{
        int j,k,*idx;
        float *ftmp;

        k=n-1;

        idx=ivector(0,k);
        ftmp=vector(0,k);
        findex(n,ra,idx);

        for (j=0; j<=k; j++) ftmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = ftmp[idx[j]];
        for (j=0; j<=k; j++) ftmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = ftmp[idx[j]];
        for (j=0; j<=k; j++) ftmp[j] = rc[j];
        for (j=0; j<=k; j++) rc[j]   = ftmp[idx[j]];

        free_vector(ftmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of floats and an associated float array in descending order
  @param	n	number of elements in the float arrays
  @param	ra 	Key Array of floats to consider
  @param	rb 	Associated float Array to consider
  @return	void

  Sort on an array of floats (key array). An associated array of floats
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated array.
 */
/*--------------------------------------------------------------------------*/

void ffrsort(int n, float *ra, float *rb)
{
        int j,k,*idx;
        float *ftmp;

        k=n-1;
        idx=ivector(0,k);
        ftmp=vector(0,k);
        findex(n,ra,idx);

        for (j=0; j<=k; j++) ftmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = ftmp[idx[k-j]];
        for (j=0; j<=k; j++) ftmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = ftmp[idx[k-j]];

        free_vector(ftmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of floats and two associated float arrays in descending order
  @param	n	number of elements in the float arrays
  @param	ra 	Key Array of floats to consider
  @param	rb 	First Associated float Array to consider
  @param	rc 	Second Associated float Array to consider
  @return	void

  Sort on an array of floats (key array). Two associated arrays of floats
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated arraya.
 */
/*--------------------------------------------------------------------------*/

void fffrsort(int n, float *ra, float *rb, float *rc)
{
        int j,k,*idx;
        float *ftmp;

        k=n-1;
        idx=ivector(0,k);
        ftmp=vector(0,k);
        findex(n,ra,idx);

        for (j=0; j<=k; j++) ftmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = ftmp[idx[k-j]];
        for (j=0; j<=k; j++) ftmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = ftmp[idx[k-j]];
        for (j=0; j<=k; j++) ftmp[j] = rc[j];
        for (j=0; j<=k; j++) rc[j]   = ftmp[idx[k-j]];

        free_vector(ftmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of doubles and two associated arrays (double, int) in ascending order
  @param	n	number of elements in the arrays
  @param	ra 	Key Array of doubles to consider
  @param	rb 	Associated double Array to consider
  @param	ic 	Associated int Array to consider
  @return	void

  Sort on an array of doubles (key array). Two associated arrays (double and int)
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated arrays.
 */
/*--------------------------------------------------------------------------*/

void ddisort(int n, double *ra, double *rb, int *ic)
{
        int j,k,*idx,*itmp;
        double *dtmp;

        k=n-1;
        idx=ivector(0,k);
        itmp=ivector(0,k);
        dtmp=dvector(0,k);
        dindex(n,ra,idx);

        for (j=0; j<=k; j++) dtmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = dtmp[idx[j]];
        for (j=0; j<=k; j++) dtmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = dtmp[idx[j]];
        for (j=0; j<=k; j++) itmp[j] = ic[j];
        for (j=0; j<=k; j++) ic[j]   = itmp[idx[j]];

        free_dvector(dtmp,0);
        free_ivector(itmp,0);
        free_ivector(idx,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of floats and three associated arrays (floats and int) in ascending order
  @param	n	number of elements in the float arrays
  @param	ra 	Key array of floats to consider
  @param	rb 	First associated float Array to consider
  @param	rc 	Second associated float Array to consider
  @param	ic 	Associated int Array to consider
  @return	void

  Sort on an array of floats (key array). Three associated arrays (floats and int)
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated array.
 */
/*--------------------------------------------------------------------------*/

void fffisort(int n, float *ra, float *rb, float *rc, int *ic)
{
        int j,k,*idx,*itmp;
        float *ftmp;

        k=n-1;
        idx=ivector(0,k);
        itmp=ivector(0,k);
        ftmp=vector(0,k);
        findex(n,ra,idx);

        for (j=0; j<=k; j++) ftmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = ftmp[idx[j]];
        for (j=0; j<=k; j++) ftmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = ftmp[idx[j]];
        for (j=0; j<=k; j++) ftmp[j] = rc[j];
        for (j=0; j<=k; j++) rc[j]   = ftmp[idx[j]];
        for (j=0; j<=k; j++) itmp[j] = ic[j];
        for (j=0; j<=k; j++) ic[j]   = itmp[idx[j]];

        free_vector(ftmp,0);
        free_ivector(idx,0);
        free_ivector(itmp,0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of floats and three associated arrays (floats and int) in descending order
  @param	n	number of elements in the float arrays
  @param	ra 	Key array of floats to consider
  @param	rb 	First associated float Array to consider
  @param	rc 	Second associated float Array to consider
  @param	ic 	Associated int Array to consider
  @return	void

  Sort on an array of floats (key array). Three associated arrays (floats and int)
  will be rearranged in the same way as the key array. The sort is
  realised by determining an index to the key array and then
  rearranging the actual elements of key and associated array.
 */
/*--------------------------------------------------------------------------*/

void fffirsort(int n, float *ra, float *rb, float *rc, int *ic)
{
        int j,k,*idx,*itmp;
        float *ftmp;

        k=n-1;
        idx=ivector(0,k);
        itmp=ivector(0,k);
        ftmp=vector(0,k);
        findex(n,ra,idx);

        for (j=0; j<=k; j++) ftmp[j] = ra[j];
        for (j=0; j<=k; j++) ra[j]   = ftmp[idx[k-j]];
        for (j=0; j<=k; j++) ftmp[j] = rb[j];
        for (j=0; j<=k; j++) rb[j]   = ftmp[idx[k-j]];
        for (j=0; j<=k; j++) ftmp[j] = rc[j];
        for (j=0; j<=k; j++) rc[j]   = ftmp[idx[k-j]];
        for (j=0; j<=k; j++) itmp[j] = ic[j];
        for (j=0; j<=k; j++) ic[j]   = itmp[idx[k-j]];

        free_vector(ftmp,0);
        free_ivector(idx,0);
        free_ivector(itmp,0);
}

/* the following two defines are valid for the four sorting
   routines that follow */

#define M 10 /* subarrays smaller than this number will be processed
                with insertion sort instead of quicksort */
#define STACKSIZE 100 /* The recursion of quicksort in the following routines
                         is implemented manually with an internal stack of maximum
                         size 'STACKSIZE'. In average, arrays with size
                         10**(STACKSIZE) can be processed */

/*-------------------------------------------------------------------------*/
/**
  @brief	Determine a sort index to arrange an array of floats by increasing value.
  @param	n	number of elements in the array
  @param	a 	Array of floats to consider
  @param	idx 	Array of ints to contain the sort index
  @return	void

  Fast sort on an array of floats. The array is not changed but an
  index, which represents the array order is determined.
  The alorithm is a modified, non-recursive quicksort which
  uses insertion sort for subfields with less than 'M' elements.
  The pivot element for each subdivision is chosen with a 'median
  of three' strategy.
  The memmory allocation/freeing for the index array 'idx' has to be handled
  outside this function.
 */
/*--------------------------------------------------------------------------*/

void findex(int n, float *a, int *idx)
{
	int i, j, m, r, l, s=0;
        int tmp;
	int stack[STACKSIZE]; /* internal stack that substitutes the
                                 recursions in an usual quicksort
                                 implementaion */

	for(i=0; i<n ; i++) idx[i]=i;

	/* first put the whole array on the stack. The left element
           is put first. With a pushdown stack we then have to retrieve
           the right one first */

	stack[s++] = 0;
	stack[s++] = n-1;

	/* go on until the stack is empty */
	while(s>0)
	{
	        r=stack[--s];
	        l=stack[--s];
  	        if(r-l > M){
  	          /* quicksort with a median of three strategy
                     to choose the pivot element
  	          */
  		        i=l-1; j=r;
  		        m=l+(r-l)/2;
  		        if(a[idx[l]]>a[idx[m]]){
  		                tmp=idx[l]; idx[l]=idx[m]; idx[m]=tmp;
  		        }
  		        if(a[idx[l]]>a[idx[r]]){
  		                tmp=idx[l]; idx[l]=idx[r]; idx[r]=tmp;
  		        }
  		        else if(a[idx[r]]>a[idx[m]]){
  		                tmp=idx[r]; idx[r]=idx[m]; idx[m]=tmp;
  		        }

  		        for(;;){
  			        while(a[idx[++i]]<a[idx[r]]);
  			        while(a[idx[--j]]>a[idx[r]]);
  			        if(i>=j) break;
  			        tmp=idx[i]; idx[i]=idx[j]; idx[j]=tmp;
  		        }
  		        tmp=idx[i]; idx[i]=idx[r]; idx[r]=tmp;

		        if(s>(STACKSIZE-2)) {
		                fprintf(stderr, "stacksize too small in quicksort; aborting!!");
		                exit(1);
		        }

			/* the larger subarray is processed immediately, the
                           smaller one later. This minimses the needed stack
                           elements */
		        if((i-l) > (r-i)) {
		                stack[s++]=l;
		                stack[s++]=i-1;
		                stack[s++]=i+1;
		                stack[s++]=r;
		        }
		        else {
		                stack[s++]=i+1;
		                stack[s++]=r;
		                stack[s++]=l;
		                stack[s++]=i-1;
		        }
  	        }
  	        else {
  	                /* insertion sort */
  		        for(i=l+1; i<=r; ++i){
  			        tmp=idx[i];
  			        for(j=i-1; j>=l && a[tmp]<a[idx[j]]; --j)
  				       idx[j+1]=idx[j];
  			        idx[j+1]=tmp;
  		        }
  	        }
	}
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Determine a sort index to arrange an array of doubles by increasing value.
  @param	n	number of elements in the array
  @param	a 	Array of doubles to consider
  @param	idx 	Array of ints to contain the sort index
  @return	void

  Fast sort on an array of doubles. The array is not changed but an
  index, which represents the array order is determined.
  The alorithm is a modified, non-recursive quicksort which
  uses insertion sort for subfields with less than 'M' elements.
  The pivot element for each subdivision is chosen with a 'median
  of three' strategy.
  The memmory allocation/freeing for the index array 'idx' has to be handled
  outside this function.
 */
/*--------------------------------------------------------------------------*/

void dindex(int n, double *a, int *idx)
{
	int i, j, m, r, l, s=0;
        int tmp;
	int stack[STACKSIZE]; /* internal stack that substitutes the
                                 recursions in an usual quicksort
                                 implementaion */

	for(i=0; i<n ; i++) idx[i]=i;

	/* first put the whole array on the stack. The left element
           is put first. With a pushdown stack we then have to retrieve
           the right one first */

	stack[s++] = 0;
	stack[s++] = n-1;

	/* go on until the stack is empty */
	while(s>0)
	{
	        r=stack[--s];
	        l=stack[--s];
  	        if(r-l > M){
  	          /* quicksort with a median of three strategy
                     to choose the pivot element
  	          */
  		        i=l-1; j=r;
  		        m=l+(r-l)/2;
  		        if(a[idx[l]]>a[idx[m]]){
  		                tmp=idx[l]; idx[l]=idx[m]; idx[m]=tmp;
  		        }
  		        if(a[idx[l]]>a[idx[r]]){
  		                tmp=idx[l]; idx[l]=idx[r]; idx[r]=tmp;
  		        }
  		        else if(a[idx[r]]>a[idx[m]]){
  		                tmp=idx[r]; idx[r]=idx[m]; idx[m]=tmp;
  		        }

  		        for(;;){
  			        while(a[idx[++i]]<a[idx[r]]);
  			        while(a[idx[--j]]>a[idx[r]]);
  			        if(i>=j) break;
  			        tmp=idx[i]; idx[i]=idx[j]; idx[j]=tmp;
  		        }
  		        tmp=idx[i]; idx[i]=idx[r]; idx[r]=tmp;

		        if(s>(STACKSIZE-2)) {
		                fprintf(stderr, "stacksize too small in quicksort; aborting!!");
		                exit(1);
		        }

			/* the larger subarray is processed immediately, the
                           smaller one later. This minimses the needed stack
                           elements */
		        if((i-l) > (r-i)) {
		                stack[s++]=l;
		                stack[s++]=i-1;
		                stack[s++]=i+1;
		                stack[s++]=r;
		        }
		        else {
		                stack[s++]=i+1;
		                stack[s++]=r;
		                stack[s++]=l;
		                stack[s++]=i-1;
		        }
  	        }
  	        else {
  	                /* insertion sort */
  		        for(i=l+1; i<=r; ++i){
  			        tmp=idx[i];
  			        for(j=i-1; j>=l && a[tmp]<a[idx[j]]; --j)
  				       idx[j+1]=idx[j];
  			        idx[j+1]=tmp;
  		        }
  	        }
	}
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Determine a sort index to arrange an array of ints by increasing value.
  @param	n	number of elements in the array
  @param	a 	Array of ints to consider
  @param	idx 	Array of ints to contain the sort index
  @return	void

  Fast sort on an array of ints. The array is not changed but an
  index, which represents the array order is determined.
  The alorithm is a modified, non-recursive quicksort which
  uses insertion sort for subfields with less than 'M' elements.
  The pivot element for each subdivision is chosen with a 'median
  of three' strategy.
  The memmory allocation/freeing for the index array 'idx' has to be handled
  outside this function.
 */
/*--------------------------------------------------------------------------*/

void iindex(int n, int *a, int *idx)
{
	int i, j, m, r, l, s=0;
        int tmp;
	int stack[STACKSIZE]; /* internal stack that substitutes the
                                 recursions in an usual quicksort
                                 implementaion */

	for(i=0; i<n ; i++) idx[i]=i;

	/* first put the whole array on the stack. The left element
           is put first. With a pushdown stack we then have to retrieve
           the right one first */

	stack[s++] = 0;
	stack[s++] = n-1;

	/* go on until the stack is empty */
	while(s>0)
	{
	        r=stack[--s];
	        l=stack[--s];
  	        if(r-l > M){
  	          /* quicksort with a median of three strategy
                     to choose the pivot element
  	          */
  		        i=l-1; j=r;
  		        m=l+(r-l)/2;
  		        if(a[idx[l]]>a[idx[m]]){
  		                tmp=idx[l]; idx[l]=idx[m]; idx[m]=tmp;
  		        }
  		        if(a[idx[l]]>a[idx[r]]){
  		                tmp=idx[l]; idx[l]=idx[r]; idx[r]=tmp;
  		        }
  		        else if(a[idx[r]]>a[idx[m]]){
  		                tmp=idx[r]; idx[r]=idx[m]; idx[m]=tmp;
  		        }

  		        for(;;){
  			        while(a[idx[++i]]<a[idx[r]]);
  			        while(a[idx[--j]]>a[idx[r]]);
  			        if(i>=j) break;
  			        tmp=idx[i]; idx[i]=idx[j]; idx[j]=tmp;
  		        }
  		        tmp=idx[i]; idx[i]=idx[r]; idx[r]=tmp;

		        if(s>(STACKSIZE-2)) {
		                fprintf(stderr, "stacksize too small in quicksort; aborting!!");
		                exit(1);
		        }

			/* the larger subarray is processed immediately, the
                           smaller one later. This minimses the needed stack
                           elements */
		        if((i-l) > (r-i)) {
		                stack[s++]=l;
		                stack[s++]=i-1;
		                stack[s++]=i+1;
		                stack[s++]=r;
		        }
		        else {
		                stack[s++]=i+1;
		                stack[s++]=r;
		                stack[s++]=l;
		                stack[s++]=i-1;
		        }
  	        }
  	        else {
  	                /* insertion sort */
  		        for(i=l+1; i<=r; ++i){
  			        tmp=idx[i];
  			        for(j=i-1; j>=l && a[tmp]<a[idx[j]]; --j)
  				       idx[j+1]=idx[j];
  			        idx[j+1]=tmp;
  		        }
  	        }
	}
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Determine a sort index to arrange an array of shorts by increasing value.
  @param	n	number of elements in the array
  @param	a 	Array of shorts to consider
  @param	idx 	Array of ints to contain the sort index
  @return	void

  Fast sort on an array of ints. The array is not changed but an
  index, which represents the array order is determined.
  The alorithm is a modified, non-recursive quicksort which
  uses insertion sort for subfields with less than 'M' elements.
  The pivot element for each subdivision is chosen with a 'median
  of three' strategy.
  The memmory allocation/freeing for the index array 'idx' has to be handled
  outside this function.
 */
/*--------------------------------------------------------------------------*/

void sindex(int n, short *a, int *idx)
{
	int i, j, m, r, l, s=0;
        int tmp;
	int stack[STACKSIZE]; /* internal stack that substitutes the
                                 recursions in an usual quicksort
                                 implementaion */

	for(i=0; i<n ; i++) idx[i]=i;

	/* first put the whole array on the stack. The left element
           is put first. With a pushdown stack we then have to retrieve
           the right one first */

	stack[s++] = 0;
	stack[s++] = n-1;

	/* go on until the stack is empty */
	while(s>0)
	{
	        r=stack[--s];
	        l=stack[--s];
  	        if(r-l > M){
  	          /* quicksort with a median of three strategy
                     to choose the pivot element
  	          */
  		        i=l-1; j=r;
  		        m=l+(r-l)/2;
  		        if(a[idx[l]]>a[idx[m]]){
  		                tmp=idx[l]; idx[l]=idx[m]; idx[m]=tmp;
  		        }
  		        if(a[idx[l]]>a[idx[r]]){
  		                tmp=idx[l]; idx[l]=idx[r]; idx[r]=tmp;
  		        }
  		        else if(a[idx[r]]>a[idx[m]]){
  		                tmp=idx[r]; idx[r]=idx[m]; idx[m]=tmp;
  		        }

  		        for(;;){
  			        while(a[idx[++i]]<a[idx[r]]);
  			        while(a[idx[--j]]>a[idx[r]]);
  			        if(i>=j) break;
  			        tmp=idx[i]; idx[i]=idx[j]; idx[j]=tmp;
  		        }
  		        tmp=idx[i]; idx[i]=idx[r]; idx[r]=tmp;

		        if(s>(STACKSIZE-2)) {
		                fprintf(stderr, "stacksize too small in quicksort; aborting!!");
		                exit(1);
		        }

			/* the larger subarray is processed immediately, the
                           smaller one later. This minimses the needed stack
                           elements */
		        if((i-l) > (r-i)) {
		                stack[s++]=l;
		                stack[s++]=i-1;
		                stack[s++]=i+1;
		                stack[s++]=r;
		        }
		        else {
		                stack[s++]=i+1;
		                stack[s++]=r;
		                stack[s++]=l;
		                stack[s++]=i-1;
		        }
  	        }
  	        else {
  	                /* insertion sort */
  		        for(i=l+1; i<=r; ++i){
  			        tmp=idx[i];
  			        for(j=i-1; j>=l && a[tmp]<a[idx[j]]; --j)
  				       idx[j+1]=idx[j];
  			        idx[j+1]=tmp;
  		        }
  	        }
	}
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Determine a sort index to arrange an array of longs by increasing value.
  @param	n	number of elements in the array
  @param	a 	Array of longs to consider
  @param	idx 	Array of ints to contain the sort index
  @return	void

  Fast sort on an array of longs. The array is not changed but an
  index, which represents the array order is determined.
  The alorithm is a modified, non-recursive quicksort which
  uses insertion sort for subfields with less than 'M' elements.
  The pivot element for each subdivision is chosen with a 'median
  of three' strategy.
  The memmory allocation/freeing for the index array 'idx' has to be handled
  outside this function.
 */
/*--------------------------------------------------------------------------*/

void lindex(int n, LONG *a, int *idx)
{
	int i, j, m, r, l, s=0;
        int tmp;
	int stack[STACKSIZE]; /* internal stack that substitutes the
                                 recursions in an usual quicksort
                                 implementaion */

	for(i=0; i<n ; i++) idx[i]=i;

	/* first put the whole array on the stack. The left element
           is put first. With a pushdown stack we then have to retrieve
           the right one first */

	stack[s++] = 0;
	stack[s++] = n-1;

	/* go on until the stack is empty */
	while(s>0)
	{
	        r=stack[--s];
	        l=stack[--s];
  	        if(r-l > M){
  	          /* quicksort with a median of three strategy
                     to choose the pivot element
  	          */
  		        i=l-1; j=r;
  		        m=l+(r-l)/2;
  		        if(a[idx[l]]>a[idx[m]]){
  		                tmp=idx[l]; idx[l]=idx[m]; idx[m]=tmp;
  		        }
  		        if(a[idx[l]]>a[idx[r]]){
  		                tmp=idx[l]; idx[l]=idx[r]; idx[r]=tmp;
  		        }
  		        else if(a[idx[r]]>a[idx[m]]){
  		                tmp=idx[r]; idx[r]=idx[m]; idx[m]=tmp;
  		        }

  		        for(;;){
  			        while(a[idx[++i]]<a[idx[r]]);
  			        while(a[idx[--j]]>a[idx[r]]);
  			        if(i>=j) break;
  			        tmp=idx[i]; idx[i]=idx[j]; idx[j]=tmp;
  		        }
  		        tmp=idx[i]; idx[i]=idx[r]; idx[r]=tmp;

		        if(s>(STACKSIZE-2)) {
		                fprintf(stderr, "stacksize too small in quicksort; aborting!!");
		                exit(1);
		        }

			/* the larger subarray is processed immediately, the
                           smaller one later. This minimses the needed stack
                           elements */
		        if((i-l) > (r-i)) {
		                stack[s++]=l;
		                stack[s++]=i-1;
		                stack[s++]=i+1;
		                stack[s++]=r;
		        }
		        else {
		                stack[s++]=i+1;
		                stack[s++]=r;
		                stack[s++]=l;
		                stack[s++]=i-1;
		        }
  	        }
  	        else {
  	                /* insertion sort */
  		        for(i=l+1; i<=r; ++i){
  			        tmp=idx[i];
  			        for(j=i-1; j>=l && a[tmp]<a[idx[j]]; --j)
  				       idx[j+1]=idx[j];
  			        idx[j+1]=tmp;
  		        }
  	        }
	}
}

#undef M
#undef STACKSIZE
