/*-------------------------------------------------------------------------*/
/**
   @file	median.c
   @author	N. Devillard
   @date	1998
   @version	$Revision: 1.4 $
   @brief	Fast median finding routines.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: median.c,v 1.4 2018/03/28 12:06:20 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:20 $
	$Revision: 1.4 $

12.01.2005:
I included function 'median_pixelvalue_lowhighreject' that is
identical to 'median_pixelvalue' but rejects pixels lower/higher
than given thresholds before the median is estimated.

03.02.2005:
- I added the funtion median_qsort.
  The routine finds the median by sorting the whole array and
  returning the middle element (or the mean of the middle elements).
  As we return the mean of the middle elements in case of an
  even number of elements we have to sort the whole array. In case
  of an odd number of elements we fall back to the more efficient
  kth_smallest routine.

- The median routines 'median_pixelvalue' and
  'median_pixelvalue_lowhighreject' now use median_qsort to estimate
  the median.
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "median.h"

/*---------------------------------------------------------------------------
   								Macros
 ---------------------------------------------------------------------------*/

#define median_WIRTH(a,n) kth_smallest(a,n,(((n)&1)?((n)/2):(((n)/2)-1)))

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Find the kth smallest element in an array.
  @param	a	Array to consider for median search.
  @param	n	Number of elements in the array.
  @param	k	Rank of the element to find (between 0 and n-1).
  @return	One element from the array.

  Provide an array of n pixelvalues and the rank of the value you want
  to find. A rank of 0 means the minimum element, a rank of n-1 is the
  maximum element, and a rank of n/2 is the median. Use the
  median_WIRTH macro to find the median directly.

  NB: The input array is modified. Some elements are swapped, until
  the requested value is found. The array is left in an undefined
  sorted state.

  This algorithm was taken from the following book:
  \begin{verbatim}
			    Author: Wirth, Niklaus
			     Title: Algorithms + data structures = programs
		     Publisher: Englewood Cliffs: Prentice-Hall, 1976
  Physical description: 366 p.
			    Series: Prentice-Hall Series in Automatic Computation
  \end{verbatim}
 */
/*--------------------------------------------------------------------------*/
#define PIX_SWAP(a,b) { register pixelvalue t=(a);(a)=(b);(b)=t; }
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
kth_smallest(pixelvalue a[], int n, int k)
{
    register int i,j,l,m ;
    register pixelvalue x ;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k] ;
        i=l ;
        j=m ;
        do {
            while (a[i]<x) i++ ;
            while (x<a[j]) j-- ;
            if (i<=j) {
                PIX_SWAP(a[i],a[j]) ;
                i++ ; j-- ;
            }
        } while (i<=j) ;
        if (j<k) l=i ;
        if (k<i) m=j ;
    }
    return a[k] ;
}

#undef PIX_SWAP

/*-------------------------------------------------------------------------*/
/**
  @brief	Find the median element in an array.
  @param	a	Array to consider for median search.
  @param	n	Number of elements in the array.
  @return	One element from the array.

  This routine finds the median by sorting the whole array and
  returning the middle element (or the mean of the middle elements).
  As we return the mean of the middle elements in case of an
  even number of elements we have to sort the whole array. In case
  of an odd number of elements we fall back to the more efficient
  kth_smallest routine.
 */
/*--------------------------------------------------------------------------*/

pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
median_qsort(pixelvalue a[], int n)
{
    pixelvalue median = 0.0;
    int middle = n/2;

    if(n&1)
    {
        median = kth_smallest(a, n, middle);
    }
    else
    {
        pixel_qsort(a, n);
	median = 0.5*(a[middle]+a[middle-1]);
    }

    return median;
}

#define PIX_SORT(a,b) { if ((a)>(b)) PIX_SWAP((a),(b)); }
#define PIX_SWAP(a,b) { pixelvalue temp=(a);(a)=(b);(b)=temp; }

/*-------------------------------------------------------------------------*/
/**
  @brief	Optimized search of the median of 3 values.
  @param	p	Array of 3 pixelvalues
  @return	Median of the input values.

  Found on sci.image.processing. Cannot go faster unless some
  assumptions are made about the nature of the input signal, or the
  underlying hardware.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
opt_med3(pixelvalue * p)
{
    PIX_SORT(p[0],p[1]) ; PIX_SORT(p[1],p[2]) ; PIX_SORT(p[0],p[1]) ;
    return(p[1]) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Optimized search of the median of 5 values.
  @param	p	Array of 5 pixelvalues
  @return	Median of the input values.

  Found on sci.image.processing. Cannot go faster unless some
  assumptions are made about the nature of the input signal, or the
  underlying hardware.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
opt_med5(pixelvalue * p)
{
    PIX_SORT(p[0],p[1]) ; PIX_SORT(p[3],p[4]) ; PIX_SORT(p[0],p[3]) ;
    PIX_SORT(p[1],p[4]) ; PIX_SORT(p[1],p[2]) ; PIX_SORT(p[2],p[3]) ;
    PIX_SORT(p[1],p[2]) ; return(p[2]) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Optimized search of the median of 7 values.
  @param	p	Array of 7 pixelvalues
  @return	Median of the input values.

  Found on sci.image.processing. Cannot go faster unless some
  assumptions are made about the nature of the input signal, or the
  underlying hardware.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
opt_med7(pixelvalue * p)
{
    PIX_SORT(p[0], p[5]) ; PIX_SORT(p[0], p[3]) ; PIX_SORT(p[1], p[6]) ;
    PIX_SORT(p[2], p[4]) ; PIX_SORT(p[0], p[1]) ; PIX_SORT(p[3], p[5]) ;
    PIX_SORT(p[2], p[6]) ; PIX_SORT(p[2], p[3]) ; PIX_SORT(p[3], p[6]) ;
    PIX_SORT(p[4], p[5]) ; PIX_SORT(p[1], p[4]) ; PIX_SORT(p[1], p[3]) ;
    PIX_SORT(p[3], p[4]) ; return (p[3]) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Optimized search of the median of 9 values.
  @param	p	Array of 9 pixelvalues
  @return	Median of the input values.

  Formula from:
  \begin{verbatim}
  XILINX XCELL magazine, vol. 23 by John L. Smith
  \end{verbatim}

  The result array is guaranteed to contain the median value in middle
  position, but other elements are NOT sorted.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
opt_med9(pixelvalue * p)
{
    PIX_SORT(p[1], p[2]) ; PIX_SORT(p[4], p[5]) ; PIX_SORT(p[7], p[8]) ;
    PIX_SORT(p[0], p[1]) ; PIX_SORT(p[3], p[4]) ; PIX_SORT(p[6], p[7]) ;
    PIX_SORT(p[1], p[2]) ; PIX_SORT(p[4], p[5]) ; PIX_SORT(p[7], p[8]) ;
    PIX_SORT(p[0], p[3]) ; PIX_SORT(p[5], p[8]) ; PIX_SORT(p[4], p[7]) ;
    PIX_SORT(p[3], p[6]) ; PIX_SORT(p[1], p[4]) ; PIX_SORT(p[2], p[5]) ;
    PIX_SORT(p[4], p[7]) ; PIX_SORT(p[4], p[2]) ; PIX_SORT(p[6], p[4]) ;
    PIX_SORT(p[4], p[2]) ; return(p[4]) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Optimized search of the median of 25 values.
  @param	p	Array of 25 pixelvalues
  @return	Median of the input values.

  Formula from:
  \begin{verbatim}
  Graphic Gems source code
  \end{verbatim}

  The result array is guaranteed to contain the median value in middle
  position, but other elements are NOT sorted.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
opt_med25(pixelvalue * p)
{
    PIX_SORT(p[0], p[1]) ;   PIX_SORT(p[3], p[4]) ;   PIX_SORT(p[2], p[4]) ;
    PIX_SORT(p[2], p[3]) ;   PIX_SORT(p[6], p[7]) ;   PIX_SORT(p[5], p[7]) ;
    PIX_SORT(p[5], p[6]) ;   PIX_SORT(p[9], p[10]) ;  PIX_SORT(p[8], p[10]) ;
    PIX_SORT(p[8], p[9]) ;   PIX_SORT(p[12], p[13]) ; PIX_SORT(p[11], p[13]) ;
    PIX_SORT(p[11], p[12]) ; PIX_SORT(p[15], p[16]) ; PIX_SORT(p[14], p[16]) ;
    PIX_SORT(p[14], p[15]) ; PIX_SORT(p[18], p[19]) ; PIX_SORT(p[17], p[19]) ;
    PIX_SORT(p[17], p[18]) ; PIX_SORT(p[21], p[22]) ; PIX_SORT(p[20], p[22]) ;
    PIX_SORT(p[20], p[21]) ; PIX_SORT(p[23], p[24]) ; PIX_SORT(p[2], p[5]) ;
    PIX_SORT(p[3], p[6]) ;   PIX_SORT(p[0], p[6]) ;   PIX_SORT(p[0], p[3]) ;
    PIX_SORT(p[4], p[7]) ;   PIX_SORT(p[1], p[7]) ;   PIX_SORT(p[1], p[4]) ;
    PIX_SORT(p[11], p[14]) ; PIX_SORT(p[8], p[14]) ;  PIX_SORT(p[8], p[11]) ;
    PIX_SORT(p[12], p[15]) ; PIX_SORT(p[9], p[15]) ;  PIX_SORT(p[9], p[12]) ;
    PIX_SORT(p[13], p[16]) ; PIX_SORT(p[10], p[16]) ; PIX_SORT(p[10], p[13]) ;
    PIX_SORT(p[20], p[23]) ; PIX_SORT(p[17], p[23]) ; PIX_SORT(p[17], p[20]) ;
    PIX_SORT(p[21], p[24]) ; PIX_SORT(p[18], p[24]) ; PIX_SORT(p[18], p[21]) ;
    PIX_SORT(p[19], p[22]) ; PIX_SORT(p[8], p[17]) ;  PIX_SORT(p[9], p[18]) ;
    PIX_SORT(p[0], p[18]) ;  PIX_SORT(p[0], p[9]) ;   PIX_SORT(p[10], p[19]) ;
    PIX_SORT(p[1], p[19]) ;  PIX_SORT(p[1], p[10]) ;  PIX_SORT(p[11], p[20]) ;
    PIX_SORT(p[2], p[20]) ;  PIX_SORT(p[2], p[11]) ;  PIX_SORT(p[12], p[21]) ;
    PIX_SORT(p[3], p[21]) ;  PIX_SORT(p[3], p[12]) ;  PIX_SORT(p[13], p[22]) ;
    PIX_SORT(p[4], p[22]) ;  PIX_SORT(p[4], p[13]) ;  PIX_SORT(p[14], p[23]) ;
    PIX_SORT(p[5], p[23]) ;  PIX_SORT(p[5], p[14]) ;  PIX_SORT(p[15], p[24]) ;
    PIX_SORT(p[6], p[24]) ;  PIX_SORT(p[6], p[15]) ;  PIX_SORT(p[7], p[16]) ;
    PIX_SORT(p[7], p[19]) ;  PIX_SORT(p[13], p[21]) ; PIX_SORT(p[15], p[23]) ;
    PIX_SORT(p[7], p[13]) ;  PIX_SORT(p[7], p[15]) ;  PIX_SORT(p[1], p[9]) ;
    PIX_SORT(p[3], p[11]) ;  PIX_SORT(p[5], p[17]) ;  PIX_SORT(p[11], p[17]) ;
    PIX_SORT(p[9], p[17]) ;  PIX_SORT(p[4], p[10]) ;  PIX_SORT(p[6], p[12]) ;
    PIX_SORT(p[7], p[14]) ;  PIX_SORT(p[4], p[6]) ;   PIX_SORT(p[4], p[7]) ;
    PIX_SORT(p[12], p[14]) ; PIX_SORT(p[10], p[14]) ; PIX_SORT(p[6], p[7]) ;
    PIX_SORT(p[10], p[12]) ; PIX_SORT(p[6], p[10]) ;  PIX_SORT(p[6], p[17]) ;
    PIX_SORT(p[12], p[17]) ; PIX_SORT(p[7], p[17]) ;  PIX_SORT(p[7], p[10]) ;
    PIX_SORT(p[12], p[18]) ; PIX_SORT(p[7], p[12]) ;  PIX_SORT(p[10], p[18]) ;
    PIX_SORT(p[12], p[20]) ; PIX_SORT(p[10], p[20]) ; PIX_SORT(p[10], p[12]) ;

    return (p[12]);
}

#undef PIX_SORT
#undef PIX_SWAP


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the median pixel value of an array.
  @param	a	Array to consider.
  @param	n	Number of pixels in the array.
  @return	The median of the array.

  This is the generic method that should be called to get the median
  out of an array of pixelvalues. It calls in turn the most efficient
  method depending on the number of values in the array.

  The input array is always modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
median_pixelvalue(pixelvalue * a, int n)
{
	pixelvalue	median ;
	switch(n) {
		case 1:  median = a[0] ; break ;
		case 2:  median = (a[0]+a[1])/2 ; break ;
		case 3:  median = opt_med3(a); break ;
		case 5:  median = opt_med5(a); break ;
		case 7:  median = opt_med7(a); break ;
		case 9:  median = opt_med9(a); break ;
		case 25: median = opt_med25(a); break ;

		default:
		median = median_qsort(a,n);
		break ;
	}
	return median;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the median pixel value of an array.
  @param	a	Array to consider.
  @param	lo_rej	Low threshold for pixel values.
  @param	hi_rej	High threshold for pixel values.
  @param	def_median	default return if the median cannot be estimated.
  @param	n	Number of pixels in the array.
  @return	The median of the array with threshold rejection.

  This is the generic method that should be called to get the median
  out of an array of pixelvalues. It calls in turn the most efficient
  method depending on the number of values in the array.
  This version rejects pixel values lower/higher than given values
  before the median is estimated. If no pixels are left after rejection,
  a default value is returned.

  The input array is always modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
median_pixelvalue_lowhighreject(pixelvalue * a,
                                pixelvalue lo_rej,
                                pixelvalue hi_rej,
                                pixelvalue def_median,
                                int n)
{
	pixelvalue	median = def_median;
	pixelvalue      * a_copy;
	int             i, n_true=0 ;

	if(n>0) {
     	  a_copy=malloc(n*sizeof(pixelvalue));

     	  for(i=0; i<n; i++) {
     	    if(a[i] > lo_rej && a[i] < hi_rej) {
     	      a_copy[n_true] = a[i];
     	      n_true ++;
     	    }
     	  }
     	  switch(n_true) {
     		  case 1:  median = a_copy[0] ; break ;
     		  case 2:  median = (a_copy[0]+a_copy[1])/2 ; break ;
     		  case 3:  median = opt_med3(a_copy); break ;
     		  case 5:  median = opt_med5(a_copy); break ;
     		  case 7:  median = opt_med7(a_copy); break ;
     		  case 9:  median = opt_med9(a_copy); break ;
     		  case 25: median = opt_med25(a_copy); break ;

     		  default:
     		  median = median_qsort(a_copy,n_true);
     		  break ;
     	  }
     	  free(a_copy);
	}
	return median;
}

