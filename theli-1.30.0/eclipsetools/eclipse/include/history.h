/*-------------------------------------------------------------------------*/
/**
   @file    history.h
   @author  N. Devillard
   @date    June 2001
   @version $Revision: 1.2 $
   @brief   History handling class, useful for FITS headers
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: history.h,v 1.2 2016/07/15 19:13:40 thomas Exp $
    $Author: thomas $
    $Date: 2016/07/15 19:13:40 $
    $Revision: 1.2 $
*/

#ifndef _HISTORY_H_
#define _HISTORY_H_

/*---------------------------------------------------------------------------
      History Comments (TE)
 ---------------------------------------------------------------------------*/

/*
  15.07.2016:
  I added a function history_new_header that creates a new history
  structure and initializes it with the HISTORY of a given header - useful
  if we use an old image to create a new one but if we do not want to
  reuse the old header.
 */

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

/* FITS interoperability */
#include "qfits.h"

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	history type
 */
/*-------------------------------------------------------------------------*/
typedef struct _history_ {
	int		n ;		/* Number of entries  */
	int		size ;	/* Total storage size */
	char **	line ;	/* List of entries    */
} history ;

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Create a new history object.
  @return   1 newly allocated history object.

  The history object is meant to store historical comments. Anything can be
  stored, but the main purpose is to complete the HISTORY fields of a FITS
  header before dumping it to disk. Lines can only be added to a history
  object, not removed. A history object is a storage structure for an
  unlimited number of char strings of unlimited size, that can be dumped
  to the screen or to a FITS header.

  The returned object must be freed using history_del().
 */
/*--------------------------------------------------------------------------*/
history * history_new(void) ;

/*-------------------------------------------------------------------------*/
/**
  @brief	Create a history object using an already existing header.
  @return	1 newly allocated history object.

  The history object is meant to store historical comments. Anything can be
  stored, but the main purpose is to complete the HISTORY fields of a FITS
  header before dumping it to disk. Lines can only be added to a history
  object, not removed. A history object is a storage structure for an
  unlimited number of char strings of unlimited size, that can be dumped
  to the screen or to a FITS header.
  This version is initialised with the HISTORY comments of an already
  existing header.

  The returned object must be freed using history_del().
 */
/*--------------------------------------------------------------------------*/
history * history_new_header(qfits_header *hdr);

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete a history object.
  @param    hs  History object to deallocate.
  @return   void

  Deletes all data associated to a history object.
 */
/*--------------------------------------------------------------------------*/
void history_del(history * hs) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Append a line to a history object.
  @param    hs      History object to update.
  @param    format  printf-like format string
  @return   void

  Appends a character string as a line into a history object.
  Works just like a printf statement.
  Probably unsafe on string lengths, this has to be worked on.
 */
/*--------------------------------------------------------------------------*/
void history_add(
        history *   hs,
        char    *   format, ...) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a history object to an open FILE pointer.
  @param    hs      History object to dump.
  @param    fp      Open file pointer.
  @return   void

  This function dumps the contents of a history object onto an open
  file pointer. It is Ok to provide stdout or stderr as file pointers.
 */
/*--------------------------------------------------------------------------*/
void history_dump(
        history *   hs,
        FILE    *   fp) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a history object into a FITS header object.
  @param    hs      History object to dump.
  @param    fh      FITS header to modify.
  @return   int 0 if Ok, -1 otherwise.

  This function dumps a history object as a list of HISTORY keys into
  an allocated FITS header.
 */
/*--------------------------------------------------------------------------*/
int history_addfits(
        history         *   hs,
        qfits_header    *   fh) ;

#endif
