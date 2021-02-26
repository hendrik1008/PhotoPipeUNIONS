/*----------------------------------------------------------------------------*/
/**
   @file	byteswap.c
   @author	N. Devillard
   @date	Sep 1999
   @version	$Revision: 1.2 $
   @brief	Low-level byte-swapping routines

   This module offers access to byte-swapping routines.
   Generic routines are offered that should work everywhere.
   Assembler is also included for x86 architectures, and dedicated
   assembler calls for processors > 386.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: byteswap.c,v 1.2 2011/05/24 12:10:55 thomas Exp $
	$Author: thomas $
	$Date: 2011/05/24 12:10:55 $
	$Revision: 1.2 $
*/

/*-----------------------------------------------------------------------------
   								History
 -----------------------------------------------------------------------------*/

/*
  24.05.2011:
  I removed assembler code for X86 processors (functions swap_bytes_16 and
  swap_bytes_32.
  The code did not compile unter MacOS 10.7
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "config.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

/** Potential tracing feature for gcc > 2.95 */
#if (__GNUC__>2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 95))
#define __NOTRACE__ __attribute__((__no_instrument_function__))
#else
#define __NOTRACE__
#endif

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Swap a 16-bit number
  @param    w A 16-bit (short) number to byte-swap.
  @return   The swapped version of w, w is untouched.

  This function swaps a 16-bit number, returned the swapped value without
  modifying the passed argument.
 */
/*----------------------------------------------------------------------------*/
unsigned short __NOTRACE__ swap_bytes_16(unsigned short w)
{
    return (((w) & 0x00ff) << 8 | ((w) & 0xff00) >> 8);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Swap a 32-bit number
  @param    dw A 32-bit (long) number to byte-swap.
  @return   The swapped version of dw, dw is untouched.

  This function swaps a 32-bit number, returned the swapped value without
  modifying the passed argument.
 */
/*----------------------------------------------------------------------------*/
unsigned int __NOTRACE__ swap_bytes_32(unsigned int dw)
{
    return ((((dw) & 0xff000000) >> 24) | (((dw) & 0x00ff0000) >>  8) |
            (((dw) & 0x0000ff00) <<  8) | (((dw) & 0x000000ff) << 24));
}

/*----------------------------------------------------------------------------*/
/**
  @fn		swap_bytes
  @brief	Swaps bytes in a variable of given size
  @param	p pointer to void (generic pointer)
  @param	s size of the element to swap, pointed to by p
  @return	void

  This byte-swapper is portable and works for any even variable size.
  It is not truly the most efficient ever, but does its job fine
  everywhere this file compiles.
 */
/*----------------------------------------------------------------------------*/
void __NOTRACE__ swap_bytes(void * p, int s)
{
    unsigned char tmp, *a, *b ;

    a = (unsigned char*)p ;
    b = a + s ;

    while (a<b) {
        tmp = *a ;
        *a++ = *--b ;
        *b = tmp ;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Find out if the local machine is big or little endian
  @return	int 1 if local machine needs byteswapping (INTEL), 0 else.

  This function determines at run-time the endian-ness of the local
  machine. An INTEL-like processor needs byte-swapping, a
  MOTOROLA-like one does not.
 */
/*----------------------------------------------------------------------------*/
int need_byteswapping(void)
{
    short     ps = 0xFF ;
    return ((*((char*)(&ps))) ? 1 : 0 ) ;
}

/* vim: set ts=4 et sw=4 tw=75 */
