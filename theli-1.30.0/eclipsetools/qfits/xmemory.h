
/*-------------------------------------------------------------------------*/
/**
  @file     xmemory.h
  @author   Nicolas Devillard
  @date     Oct 2000
  @version  $Revision: 1.3 $
  @brief    POSIX-compatible extended memory handling.

  xmemory is a small and efficient module offering memory
  extension capabitilies to ANSI C programs running on
  POSIX-compliant systems. If offers several useful features such
  as memory leak detection, protection for free on NULL or
  unallocated pointers, and virtually unlimited memory space.
  xmemory requires the @c mmap() system call to be implemented in
  the local C library to function. This module has been tested on
  a number of current Unix flavours and is reported to work fine.

  See the documentation attached to this module for more information.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: xmemory.h,v 1.3 2016/07/20 12:25:46 thomas Exp $
	$Author: thomas $
	$Date: 2016/07/20 12:25:46 $
	$Revision: 1.3 $
*/

#ifndef _XMEMORY_H_
#define _XMEMORY_H_

/*---------------------------------------------------------------------------
   								History
 ---------------------------------------------------------------------------*/

/* 24.01.2005:
   routine xmemory_falloc:
   I substituted type size_t by int for argument offs. This makes
   the sanity check performed on this parameter useful

   20.07.2015:
   routine xmemory_falloc:
   I substituted type int by long int for argument offs. This
   was necessary to deal with DECam data whose MEF-files are
   larger than 2GB in 32-bit float format.
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/*
 The following symbol is useful to know if the current module has been
 linked against xmemory.c or not.
 */
#define _XMEMORY_	1


/*---------------------------------------------------------------------------
   								Macros
 ---------------------------------------------------------------------------*/

/*
 Protect strdup redefinition on systems which #define it rather than
 having it as a function.
 */

#ifdef strdup
#undef strdup
#endif

#define malloc(s)		xmemory_malloc(s,		__FILE__,__LINE__)
#define calloc(n,s) 	xmemory_calloc(n,s,		__FILE__,__LINE__)
#define free(p)			xmemory_free(p,			__FILE__,__LINE__)
#define strdup(s)		xmemory_strdup(s,		__FILE__,__LINE__)
#define falloc(f,o,s)	xmemory_falloc(f,o,s,	__FILE__,__LINE__)

/*
 Trick to have xmemory status display the file name and line where it has
 been called.
 */
#define xmemory_status() xmemory_status_(__FILE__,__LINE__)
#define xmemory_diagnostics() xmemory_status_(__FILE__,__LINE__)

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/
void * 	xmemory_malloc(size_t, char *, int);
void * 	xmemory_calloc(size_t, size_t, char *, int);
void   	xmemory_free(void *, char *, int);
char * 	xmemory_strdup(char *, char *, int);
char *	xmemory_falloc(char *, long int, size_t *, char *, int);


/*-------------------------------------------------------------------------*/
/**
  @brief    Activate xmemory
  @return   void

  This function activates xmemory activity. Make sure it is always
  used consistently with xmemory_off().
 */
/*--------------------------------------------------------------------------*/
void xmemory_on(void);

/*-------------------------------------------------------------------------*/
/**
  @brief    Deactivate xmemory
  @return   void

  This function deactivates xmemory activity. Make sure it is always
  used consistently with xmemory_on().
 */
/*--------------------------------------------------------------------------*/
void xmemory_off(void);

/*-------------------------------------------------------------------------*/
/**
  @brief    Display memory status information.
  @return   void

  This function is meant for debugging purposes, but it is recommended to
  call it at the end of every executable making use of the extended memory
  features. This function should be called through the xmemory_status()
  macro, which provides automatically the name of the source file and line
  number where the call happens.
 */
/*--------------------------------------------------------------------------*/
void xmemory_status_(char * filename, int lineno);

/*-------------------------------------------------------------------------*/
/**
  @brief    Set name of temporary directory for VM files.
  @param    dirname     Name of assigned directory.
  @return   void

  This function assigns a new value for the temporary directory name.
  It does not check the directory existence or writability, this is
  up to the caller to check that before calling this function.
  Provide a directory name without trailing slash, e.g. "/tmp".
 */
/*--------------------------------------------------------------------------*/
void xmemory_settmpdir(char * dirname);

/*-------------------------------------------------------------------------*/
/**
  @brief    Get name of temporary directory for VM files.
  @return   pointer to statically allocated string within this module.

  This function returns a pointer to an internal (static) string giving the
  current name for temporary directory. Do not modify or try to free the
  returned string.
 */
/*--------------------------------------------------------------------------*/
char * xmemory_gettmpdir(void);


/*-------------------------------------------------------------------------*/
/**
  @brief	Map a file's contents to memory as a char pointer.
  @param	name		Name of the file to map
  @param	offs		Offset to the first mapped byte in file.
  @param	size		Returned size of the mapped file in bytes.
  @param	srcname		Name of the source file making the call.
  @param	srclin		Line # where the call was made.
  @return	A pointer to char, to be freed using xmemory_free().

  This function takes in input the name of a file. It tries to map the
  file into memory and if it succeeds, returns the file's contents as
  a char pointer. It also modifies the input filesize variable to be
  the size of the mapped file in bytes. This function is normally
  never directly called but through the falloc() macro.

  The offset indicates the starting point for the mapping, i.e. if you
  are not interested in mapping the whole file but only from a given
  place.
 */
/*--------------------------------------------------------------------------*/

char * xmemory_falloc(char * name, long int offs, size_t * size, char * srcname, int srclin);

#endif
/* vim: set ts=4 et sw=4 tw=75 */
