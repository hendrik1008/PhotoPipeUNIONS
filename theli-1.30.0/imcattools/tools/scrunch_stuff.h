
/*----------------------------------------------------------------------------

   File name    :   scrunch_stuff.h
   Author       :   Nick Kaiser & Thomas Erben
   Created on   :   29.08.2007
   Description  :   scrunch handling routines

 ---------------------------------------------------------------------------*/

/*
	$Id: scrunch_stuff.h,v 1.3 2018/03/28 12:10:26 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:10:26 $
	$Revision: 1.3 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
  29.08.2007:
  Nick Kaisers imcat tool 'scruch' integrated to THELI

  10.09.2007:
  A binning factor can now be given as command line option
  (-b). The program no longer is limited to a binning of 2!
 */

/*----------------------------------------------------------------------------
                                Definitions
 ---------------------------------------------------------------------------*/


#define	MEAN	1
#define	MEDIAN	2
#define CMEAN	3

/*-------------------------------------------------------------------------*/
/**
  @brief    Performs image scrunching in stream mode
  @param    fitsin      pointer to input image structure
  @param    fitsout     pointer to output image structure
  @param    binning     binning factor
  @param    mode        binning mode (1=MEAN, 2=MEDIAN, 3=SECURE MEAN

  The function 'writefitsheader(fitsout)' has to be called before
  entering this function.
  The function writes the output image to disk. The calling function
  still needs to write padding to fill a complete FITS block if this
  is necessary.
 */
/*--------------------------------------------------------------------------*/
void scrunch_stream(fitsheader *fitsin,
                    fitsheader *fitsout,
                    int binning, int mode);

