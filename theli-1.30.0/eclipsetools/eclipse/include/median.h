/*-------------------------------------------------------------------------*/
/**
   @file    median.h
   @author  N. Devillard
   @date    1998
   @version $Revision: 1.4 $
   @brief   Fast median finding routines.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: median.h,v 1.4 2018/03/28 12:10:29 thomas Exp $
    $Author: thomas $
    $Date: 2018/03/28 12:10:29 $
    $Revision: 1.4 $

16.01.2005:
I added function 'median_pixelvalue_lowhighreject' that
is identical to 'median_pixelvalue' but rejects pixels
below/above given threshholds before the median is estimated.

I added the funtion median_qsort.
The routine finds the median by sorting the whole array and
returning the middle element (or the mean of the middle elements).
As we return the mean of the middle elements in case of an
even number of elements we have to sort the whole array. In case
of an odd number of elements we fall back to the more efficient
kth_smallest routine.

*/

#ifndef _MEDIAN_H_
#define _MEDIAN_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

/* Get the definition of a pixelvalue */
#include "local_types.h"

/* get the definition of pixel_qsort */
#include "pixel_handling.h"

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Find the kth smallest element in an array.
  @param    a   Array to consider for median search.
  @param    n   Number of elements in the array.
  @param    k   Rank of the element to find (between 0 and n-1).
  @return   One element from the array.

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
pixelvalue
kth_smallest(pixelvalue a[], int n, int k) ;

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
median_qsort(pixelvalue a[], int n);

/*-------------------------------------------------------------------------*/
/**
  @brief    Optimized search of the median of 3 values.
  @param    p   Array of 3 pixelvalues
  @return   Median of the input values.

  Found on sci.image.processing. Cannot go faster unless some
  assumptions are made about the nature of the input signal, or the
  underlying hardware.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
opt_med3(pixelvalue * p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Optimized search of the median of 5 values.
  @param    p   Array of 5 pixelvalues
  @return   Median of the input values.

  Found on sci.image.processing. Cannot go faster unless some
  assumptions are made about the nature of the input signal, or the
  underlying hardware.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
opt_med5(pixelvalue * p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Optimized search of the median of 7 values.
  @param    p   Array of 7 pixelvalues
  @return   Median of the input values.

  Found on sci.image.processing. Cannot go faster unless some
  assumptions are made about the nature of the input signal, or the
  underlying hardware.

  The input array is modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
opt_med7(pixelvalue * p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Optimized search of the median of 9 values.
  @param    p   Array of 9 pixelvalues
  @return   Median of the input values.

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
opt_med9(pixelvalue * p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Optimized search of the median of 25 values.
  @param    p   Array of 25 pixelvalues
  @return   Median of the input values.

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
opt_med25(pixelvalue * p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the median pixel value of an array.
  @param    a   Array to consider.
  @param    n   Number of pixels in the array.
  @return   The median of the array.

  This is the generic method that should be called to get the median
  out of an array of pixelvalues. It calls in turn the most efficient
  method depending on the number of values in the array.

  The input array is always modified.
 */
/*--------------------------------------------------------------------------*/
pixelvalue
median_pixelvalue(pixelvalue * a, int n) ;

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
median_pixelvalue_lowhighreject(pixelvalue * a,
                                pixelvalue lo_rej,
                                pixelvalue hi_rej,
                                pixelvalue def_median,
                                int n);
#endif
