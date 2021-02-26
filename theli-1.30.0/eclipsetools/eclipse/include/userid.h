
/*-------------------------------------------------------------------------*/
/**
  @file     userid.h
  @author   N. Devillard
  @date     Sep 1999
  @version  $Revision: 1.1 $
  @brief    Portable user identification routine.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: userid.h,v 1.1 2003/09/04 15:44:42 terben Exp $
	$Author: terben $
	$Date: 2003/09/04 15:44:42 $
	$Revision: 1.1 $
*/

#ifndef _USERID_H_
#define _USERID_H_

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Returns the user login name.
  @return   Pointer to statically allocated character string.

  Finds out what the login name of the current user is. The result is
  placed in a static character string inside this module and a pointer
  to the first character in this string is returned. Do not modify or
  free the returned string!

  If the user name cannot be determined, the returned pointer will
  point to a string which first character is a null character.
 */
/*--------------------------------------------------------------------------*/
char * get_login_name(void);

#endif
