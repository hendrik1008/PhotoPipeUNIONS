
/*----------------------------------------------------------------------------

   File name 	:	cube_defs.h
   Author 		:	Nicolas Devillard
   Created on	:	Aug 02, 1995
   Description	:	cube size definitions (limitations)

 ---------------------------------------------------------------------------*/

/*
	$Id: cube_defs.h,v 1.2 2018/03/28 12:10:28 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:10:28 $
	$Revision: 1.2 $
 */

#ifndef _CUBES_H_
#define _CUBES_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <limits.h>

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/*
 * Defines relative to an upper image size limit
 * The maximum sizes are voluntarily limited, to avoid too cpu-intensive
 * processes.
 */

#define MAX_COLUMN_NUMBER		(40000)
#define MAX_LINE_NUMBER			(40000)
#define MAX_IMAGE_NUMBER		(10240)

/* Pixel value limits	*/
#define MAX_PIX_VALUE           ((pixelvalue)LONG_MAX)
#define MIN_PIX_VALUE           ((pixelvalue)LONG_MIN)

/* Maximum allowed comment length */
#define MAXHISTORYSZ		70

#endif
