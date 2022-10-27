/*-------------------------------------------------------------------------*/
/**
  @file     radecio.c
  @author   Nick Kaiser, Thomas Erben
  @date     2002
  @version  $Revision: 1.3 $
  @brief    functions to convert hms <=> decimal
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: radecio.c,v 1.3 2018/03/28 12:06:14 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:14 $
	$Revision: 1.3 $
*/

/*---------------------------------------------------------------------------
   								Hostory
 ---------------------------------------------------------------------------*/

/* 14.01.2004:
   I changed the output format in sexagesimal notation
   so that always two digits are written for h,d, m ans s
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "error.h"
#include "radecio.h"

/*---------------------------------------------------------------------------
  							Function codes
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

int	decimaltoxms(double angle, char *hmsstring)
{
	double 	h, m, s;
	int 	sign;

	sign = (angle < 0.0 ? -1 : 1);
	angle *= sign;
	angle = 60.0 * modf(angle, &h);
	s = 60.0 * modf(angle, &m);
	if (sign > 0) {
		sprintf(hmsstring, "%02d:%02d:%07.4f", (int) h, (int) m, s);
	} else {
		sprintf(hmsstring, "-%02d:%02d:%07.4f", (int) h, (int) m, s);
	}	
        return 1;
}

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

int	xmstodecimal(char *hmsstring, double *result)
{
	int	h, m, sign;
	double	s;

	if (hmsstring[0] == '-') {
		sign = -1;
		hmsstring = hmsstring + 1;
	} else {
		sign = 1;
	}
	if (3 != sscanf(hmsstring, "%d:%d:%lf", &h, &m, &s)) {
		return(0);
	}
	if (m < 0 || m > 59 || s < 0.0 || s > 60.0) {
		error_exit("xmstodecimal: illegal minutes or seconds\n");
	}
	*result = sign * (h + m / 60.0 + s / 3600.0);
	return 1;
}

