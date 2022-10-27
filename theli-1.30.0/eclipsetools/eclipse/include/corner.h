/*-------------------------------------------------------------------------*/
/**
   @file    corner.h
   @author  N. Devillard
   @date    Nov 2000
   @version $Revision: 1.1 $
   @brief   Corner detector
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: corner.h,v 1.1 2003/09/04 15:44:41 terben Exp $
    $Author: terben $
    $Date: 2003/09/04 15:44:41 $
    $Revision: 1.1 $
*/

#ifndef _CORNER_H_
#define _CORNER_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

#include "image_handling.h"
#include "doubles.h"

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Detect corners in an image.
  @param    in      Input image.
  @param
  @return   1 newly allocated list of pixel positions.

  This function applies a corner detector to an image and returns a list of
  pixel positions where corners have been located.
 */
/*--------------------------------------------------------------------------*/
image_t * image_detect_corners(image_t * in) ;

#endif
