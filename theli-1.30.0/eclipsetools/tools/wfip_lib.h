
/*---------------------------------------------------------------------------

   File name 	:	wfip_lib.h
   Author 		:	N. Devillard
   Created on	:	18 May 2000
   Description	:	WFI library utilities

 *--------------------------------------------------------------------------*/

/*
	$Id: wfip_lib.h,v 1.12 2018/03/28 12:10:31 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:10:31 $
	$Revision: 1.12 $
*/

/*
   05.09.03:
   function wfi_mysplit:
   changes to include the OBJECT keyword in the new headers

   13.10.03:
   included possibility to flip frames in x and/or y
   (function wfi_mysplit)

   27.01.2005:
   routine wfi_mysplit:
   added global version number information to the splitted
   images

   19.04.2005:
   I modified the argument list for wfi_mysplit taking into account
   changes in the image remapping and the new 'no header update'
   mode.

   11.10.2005:
   I modified the argument list for wfi_mysplit to include the
   IMAGEID argument.

   05.08.2006:
   I modified the argument list for wfi_mysplit (see changes for
   the main program 'mefsplit')

   28.09.2010:
   I modified the argument list for wfi_mysplit (see changes for
   the main program 'mefsplit')

   26.11.2010:
   I modified the argument list for wfi_mysplit (see changes for
   the main program 'mefsplit')

   19.11.2012:
   I modified the argument list for wfi_mysplit (see changes for
   the main program 'mefsplit')

   10.07.2014:
   The function wfi_mysplit has another char* argument specifying the
   header keyword containing the chip number of an extension.
*/

#ifndef _WFIP_LIB_H_
#define _WFIP_LIB_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "eclipse.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/* The following parameters are WFI-wide */

/* Prescan/Overscan region */
#define WFI_PRESCAN_X_MIN		(5)
#define WFI_PRESCAN_X_MAX		(48)

#define WFI_OVERSCAN_X_MIN		(2100)
#define WFI_OVERSCAN_X_MAX		(2142)

/* Cropping region */
/* Declared obsolete 22 Nov 2000 */
/*
#define WFI_CROP_X_MIN			(55)
#define WFI_CROP_X_MAX			(2090)
#define WFI_CROP_Y_MIN			(35)
#define WFI_CROP_Y_MAX			(4120)
*/

/* New values installed 22 Nov 2000 */
#define WFI_CROP_X_MIN			(60)
#define WFI_CROP_X_MAX			(2093)
#define WFI_CROP_Y_MIN			(30)
#define WFI_CROP_Y_MAX			(4126)


/* Number of CCD chips on WFI */
#define WFI_NCHIPS				(8)

/* Saturation level for pre-processing */
#define WFI_SATLEVEL			(45000)
/* Maximal acceptable percentage of pixels above saturation level */
#define WFI_SATMAX				(0.05)



/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

int wfi_split(char * name_i, char * name_o, int xtnum);

int wfi_mysplit(char *name_i, char *name_o, int xtnum, int,
                int, char **, char **, char **, int, char **, char **, int,
                double *, double *, double *, double *,
                double *, double *, int *, int *, int *, int *, int *, char *,
                int,  double, double, double,
                double, double, int, char *, char *, char *);

cube_t * wfi_cube_load(char * filename, int xtnum);


image_t * wfi_load_ext(char * filename, int xtnum);

int wfi_is_extension(char * filename);

image_t * wfi_overscan_correction(
    image_t    *   wfi_frame,
    int         *   prescan_x,
    int         *   overscan_x,
    int         *   rej_int,
    int         *   crop_reg
) ;

image_t * wfi_myoverscan_correction(
    image_t    *   wfi_frame,
    int         *   overscan_x,
    int         *   rej_int
) ;

int wfi_gradient_check(
    image_t    *   wfi_frame,
    double          max_grad_level
) ;

#endif
