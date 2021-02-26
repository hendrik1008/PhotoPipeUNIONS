/*-------------------------------------------------------------------------*/
/**
  @file     radecio.h
  @author   Nick Kaiser, Thomas Erben
  @date     2002
  @version  $Revision: 1.1 $
  @brief    functions to convert hms <=> decimal
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: radecio.h,v 1.1 2004/01/14 16:36:31 terben Exp $
	$Author: terben $
	$Date: 2004/01/14 16:36:31 $
	$Revision: 1.1 $
*/

#ifndef _RADECIO_H_
#define _RADECIO_H_


/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    converts an angle from decimal to sexagesimal notation
  @param    angle  the angle in degrees
  @param    hmsstring  the string for the result
  @return   dummy int value (1).

  Space for hmsstring has to be allocated before the function
  is called.
 */
/*--------------------------------------------------------------------------*/

int	decimaltoxms(double angle, char *hmsstring);

/*-------------------------------------------------------------------------*/
/**
  @brief    converts an angle from sexagesimal to decimal notation
  @param    hmsstring  the input in sexagesimal notation
  @param    result  double pointer for the result in degrees
  @return   dummy int value (1).

  Space for result has to be allocated before the function
  is called.
 */
/*--------------------------------------------------------------------------*/

int	xmstodecimal(char *hmsstring, double *result);

#endif
