/*-------------------------------------------------------------------------*/
/**
   @file    matchpoint.c
   @author  Yves Jung
   @date    July 2001
   @version $Revision: 1.2 $
   @brief   Points matching
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: matchpoint.h,v 1.2 2018/03/28 12:10:29 thomas Exp $
    $Author: thomas $
    $Date: 2018/03/28 12:10:29 $
    $Revision: 1.2 $
*/

#ifndef _MATCHPOINT_H_
#define _MATCHPOINT_H_

/*---------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "comm.h"
#include "doubles.h"
#include "xmemory.h"
#include "image_handling.h"

/*---------------------------------------------------------------------------
                            Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Estimate offsets between two images
  @param    im1 First image
  @param    im2 Second image
  @param    offsetx returned x offset
  @param    offsety returned y offset
  @param    kappa   for detection
  @return   0 if ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int offsets_estimates(
        image_t *   im1,
        image_t *   im2,
        double  *   offsetx,
        double  *   offsety,
        double      kappa) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Associate points from two lists of points
  @param    det1    first list of detected points
  @param    det2    second list of detected points
  @return   look up table
 */
/*--------------------------------------------------------------------------*/
int * match_pointslist(
        double3    *   det1,
        double3    *   det2) ;

#endif
