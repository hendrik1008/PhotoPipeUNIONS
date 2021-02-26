/*---------------------------------------------------------------------------

   File name   : preprocess.c
   Author      : T. Erben
   Created on  : August 2003
   Description : Image preprocessing

*--------------------------------------------------------------------------*/

/*
        $Id: preprocess.c,v 1.33 2017/10/04 15:25:44 thomas Exp $
        $Author: thomas $
        $Date: 2017/10/04 15:25:44 $
        $Revision: 1.33 $
 */

/*---------------------------------------------------------------------------
                                                     History comments
   ---------------------------------------------------------------------------*/

/*
   14.09.03:
   included set_verbose(1) to get comment messages printed by
   e_comment etc. The messages previously appeared on Linux SuSE but
   not on Sun and Linux Debian

   22.10.03:
   included the possibility to give a "threshhold" for the
   flatfield. Pixels that have an absolute value below this threshhold
   get a value of 0 in the preprocessed frame. Hence, pixels with a
   very high modulus in the preprocessed frame are avoided.

   16.01.05:
   I substituted the median estimation by 'image_getmedian_vig' by
   'image_getmedian_vig_lowhighreject' at several places.  As low and
   high rejection thresholds, -60000.0 and 60000.0 are hardcoded in
   the moment (has to be made configurable).  The same is true for the
   default return value which is -60000.0 right now.

   23.01.2005:
   I changed the location of the actual prefstruct definition to avoid
   double existence of it during the link process.

   27.01.2005:
   I added global version number information

   28.01.2005:
   I slightly changed the output format for the global version number

   30.01.2005:
   I largely rewrote the code: A new parameter MAXIMAGES can be given
   on the command line.  It gives the maximum number of images to be
   loaded; if more images than this limit is given, images are loaded
   in chunks of MAXIMAGES and processed. We call the processing of one
   chunk a 'run'. In this way we avoid memory problems with large
   amounts of images. An image coaddition is NO LONGER possible if
   more than one run is to be processed.

   16.02.2005:

   - I corrected a bug in the saving of output images. If several runs
     were processed, images from all runs except the first one got
     wrong image headers.
   - I removed the superfluous definition of 'give_date'
   - frames are now loaded with image_loadext instead with
     image_load. The latter memorymapped the images what led to
     problems if the fringescale image was the same than the flatfield
     that is usually being rescaled.

   08.04.2005:
   The number of input images is now dynamically allocated and no
   longer hardwired. The same for all variables depending on the
   number of input images.

   09.04.2005:
   Warning messages are given if the coadded image consists of the
   default value, i.e. if the number of input images minus the
   rejected ones is too small for a meaningful median estimation.

   16.06.2006:
   In the construction of the output name of images, the output
   directory now always ends with a '/'.

   07.03.2007:
   I added OUTPUT_BITPIX and COMBINE_BITPIX keys that allow
   configuration of the precision of preprocessed and stacked images.

   19.10.2007:
   correction of a typo in a history message.

   20.11.2009:
   Printing of the usage message is now a function - the handling in a
   'define string' was too cumbersome for the very long usage message.

   09.05.2010:
   I included the option to turn off frimge rescaling (request from
   Mischa)

   28.09.2010:
   I corrected a major bug in the fringe frame subtraction. If no
   fringe rescaling was requested fringes were not corrected at all.

   31.01.2012:
   I corrected a memory allocation error for array 'scalemedian'.

   28.03.2012:
   I introduced the possibility to provide the flatscaling factor on
   the command line instead of calculating it with flatscaling images.
   This allows us to calculate this factor outside 'preprocess' - for
   instance if the flatscaling images are not available when calling
   'preprocess'. The new command line option is 'FLAT_SCALEVALUE'.

   29.03.2012:
   Header History comments are adapted in the case of a manually provided
   flatfield scaling factor.

   20.11.2012:
   We now adapt the GAIN of an image if it is changed during the flat-
   fielding process. The name of the header keyword specifying the
   GAIN is provided in the new command line option 'GAIN_KEYWORD'.

   23.11.2012:
   We now adapt the saturation level of an image if it is changed
   during the overscan, bias or flat-fielding process. The name of the
   header keyword specifying the saturation level is provided in the
   new command line option 'SATLEV_KEYWORD'.

   15.09.2013:
   clarification of documentation for the flatfield-scaling (thanks to
   Mischa for pointing out inaccuracies in the description).

   19.06.2014:
   I introduced the possibility to treat bad CCDs. They are marked
   with a 'badccd' header keyword (configuration parameter). If used
   the behaviour of the program is as follows:
   - All output data will contain a badccd keyword in their headers.
   - If a calibration file is bad the processed data are marked as bad
     as well.
   - If a science image that should be processed is marked as bad, the
     processed image will be marked as well.

   20.06.2014:
   - The co-added image gets a badccd keyword if necessary.
   - Update of documentation with the badccd treatment.

   26.06.2014:
   A lower limit for the counts of a flat field is not considered if
   the flat is marked as bad. In this case the resulting flatfielded
   image is marked as bad in any case.

   17.03.2015:
   - A lower limit for the counts of the fringe reference image is not
     considered if the fringe image is marked as bad. In this case the
     resulting (not properly defringed) image is marked as bad in any
     case.
   - I did not yet treat all cases where 'bad flats' enter the process
     correctly - (hopefully) fixed.

   04.10.2017:
   Reading a FITS header comment could result in a NULL pointer (no comment
   present) and result in subsequent crashes - fixed.
 */

/*---------------------------------------------------------------------------
                                                                Includes
   ---------------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "global_vers.h"
#include "eclipse.h"
#include "wfip_lib.h"
#include "prefs_imred.h"

/*---------------------------------------------------------------------------
                                                                Defines
   ---------------------------------------------------------------------------*/

#define MIN(x, y)    ((x) < (y) ? (x) : (y))

static void printUsage(void);

prefstruct prefs;

/*---------------------------------------------------------------------------
                                                                        Main
   ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  int a, narg, nim, i, j, run, part = 0, subpart;
  int nruns;               /* the number of runs to be performed:
                              this is the number of input images
                              divided by maximages */
  int nimarun;             /* the number of images processed
                              in the actual run */
  char **argkey, **argval, *str;
  size_t maxargstringlength = 256;

  cube_t *      i_cube    = NULL;
  image_t *     corrected = NULL;
  image_t *     stacked   = NULL;
  image_t *     bias      = NULL;
  image_t *     flat      = NULL;
  image_t *     flatscale = NULL;
  image_t *     fringe    = NULL;
  image_t *     fringeref = NULL;
  char          name_o[MAXCHAR];
  char          tmpstr[MAXCHAR];
  qfits_header *fh;
  qfits_header *fh_stack;
  history *     hs;
  pixelvalue    maxflatscale = 0.0;
  /* correction for the gain because of image scaling during
     flatfielding */
  pixelvalue    flatgaincorr = 1.0;
  /* gain of science image */
  pixelvalue    gainvalue;
  /* comment of GAIN header keyword */
  char gaincomm[MAXCHAR];
  /* GAIN value as STRING to print it into an image header */
  char gainvaluestring[MAXCHAR];
  /* additive correction of saturation level because
     of overscan */
  pixelvalue   *satlevovscancorr;
  /* additive correction of saturation level because
     of bias */
  pixelvalue   satlevbiascorr = 0.0;
  /* saturation limit of science image */
  pixelvalue    satlev;
  /* comment of SATLEV header keyword */
  char satlevcomm[MAXCHAR];
  /* SATLEV value as STRING to print it into an image header */
  char satlevvaluestring[MAXCHAR];
  /* variables concerning badccd treatment */
  int treatbadccd = 0; /* do we treat bad ccds (1=YES) */
  int biasbad = 0;     /* is the bias image bad? (1=YES) */
  int flatbad = 0;     /* is the flat image bad? (1=YES) */
  int fringebad = 0;   /* is the fringe image bad? (1=YES) */
  int sciencebad = 0;  /* is the science image bad */
  int nsciencebad = 0; /* how many science images are bad; this is
                          necessary to know if we combine images
                          at the end */
  int *validimages;    /* an array of the good images in the current
                          processing cube; we need this if we combine
                          images in the cube at the end */
  int stackbad = 0;    /* is the stacked output image bad? */
  pixelvalue   *combinemedian;
  pixelvalue   *scalemedian;
  pixelvalue   *fringescaling;
  pixelvalue    meanmedian, combinescaling;
  pixelvalue    fringerefmedian = 0, mediantmp;

  /* pixels below lo_rej and above hi_rej are not taken into account
     when estimating medians in certain circumstances*/
  pixelvalue lo_rej = -60000.0;
  pixelvalue hi_rej = 60000.0;

  narg          = nim = 0;
  prefs.ninname = 0;

  /* the number of config parameters and input image
     names cannot be larger than the number of command
     line arguments */
  QMALLOC(argkey, char *, argc);
  QMALLOC(argval, char *, argc);
  QMALLOC(prefs.inname, char *, argc);
  strcpy(prefs.prefs_name, "default.imred");

  /* read command line arguments */
  if (argc < 2) {
    printUsage();
    exit(1);
  }

  for (a = 1; a < argc; a++) {
    if (argv[a][0] == '-') {
      if (strlen(argv[a]) < 3) {
        switch ((int)tolower((int)argv[a][1])) {
          /*-------- Config filename */
        case 'h':
          printUsage();
          exit(0);
          break;
        case 'd':
          dumpprefs();
          exit(0);
          break;
        case 'c':
            if (a < (argc - 1)) {
              strncpy(prefs.prefs_name, argv[++a], MAXCHAR - 1);
            } else {
              printUsage();
              exit(1);
            }
          break;
        default:
          printUsage();
          exit(1);
          break;
        }
      } else {
        /*------ Config parameters */
        argkey[narg] = &argv[a][1];
        argval[narg] = argv[++a];

        /* determine the maximum stringlength for command line
           arguments; the factor 2 represents a conservative upper
           limit. The command line arguments consist of key AND
           argument */
        if (2 * strlen(argval[narg]) > maxargstringlength) {
          maxargstringlength = 2 * strlen(argval[narg]);
        }
        narg++;
      }
    } else {
      /*---- The input image filename(s) */
      for (; (a < argc) && (*argv[a] != '-'); a++) {
        for (str = NULL;
             (str = strtok(str ? NULL : argv[a], notokstr)); nim++) {
          prefs.inname[nim] = str;
        }
      }
      prefs.ninname = nim;
      a--;
    }
  }

  readprefs(prefs.prefs_name, argkey, argval, narg, maxargstringlength);

  /* allocate memory for variables depending on the
     number of images */
  QCALLOC(satlevovscancorr, pixelvalue, prefs.maximages);
  QMALLOC(combinemedian, pixelvalue, prefs.maximages);
  QMALLOC(scalemedian, pixelvalue, prefs.nflatscaleimage);
  QMALLOC(fringescaling, pixelvalue, prefs.maximages);
  QMALLOC(validimages, int, prefs.maximages);

  free(argkey);
  free(argval);

  /*
   * Initialize eclipse environment.
   */
  eclipse_init();
  set_verbose(1);   /* print comment messages */

  e_comment(0, "-> Part %d: Preliminary work:", ++part);
  subpart = 1;

  /* determine number of necessary runs */
  nruns = (prefs.ninname - 1) / prefs.maximages;

  if (nruns > 0 && prefs.combineflag == 1) {
    e_warning("cannot perform stacking with several runs:");
    e_warning("no coaddition will be done!!");
    prefs.combineflag = 0;
  }

  /* sanity check whether there is something to do at all */
  if (prefs.ninname == 0 || (prefs.overscanflag == 0 && prefs.trimflag == 0 &&
                             prefs.biasflag == 0 && prefs.flatflag == 0 &&
                             prefs.fringeflag == 0 && prefs.combineflag == 0)) {
    e_warning("nothing do do!! exiting!!");
    return(0);
  }

  /* sanity check of flatfield scaling; if flatfields should be rescaled
     the rescaling hast to happen either with flatscaling image or
     with a flatscaling factor */
  if (prefs.flatscaleflag == 1) {
    if ((prefs.flatscalefactor > 1.0e-06) &&
        (prefs.nflatscaleimage) > 1) {
      e_error("Flatfield scaling needs to be done with flatscale images");
      e_error("or with a flatscale factor!");
      return(-1);
    }
  }

  /* sanity checks on BITPIX flags if necessary */
  if (prefs.outputpreprocflag == 1 &&
      (prefs.outputbitpix != -32 && prefs.outputbitpix != 16)) {
    e_error("Only BITPIX=16 or BITPIX=-32 supported for output images.");
    return(-1);
  }

  if (prefs.combineflag == 1 &&
      (prefs.combinebitpix != -32 && prefs.combinebitpix != 16)) {
    e_error("Only BITPIX=16 or BITPIX=-32 supported for stacked images.");
    return(-1);
  }

  /* do we treat bad ccds? */
  if (strcmp(prefs.badccdkey, "NONE") != 0) {
    treatbadccd = 1;
  }

  /* load necessary calibration frames: There is no need to load them
     for every run */
  if (prefs.biasflag == 1) {
    e_comment(1, "-> %d.%d: Load Bias Frame %s", part,
              subpart++, prefs.biasimage);

    /* load bias frame */
    bias = image_loadext(prefs.biasimage, 0, 0, 0);

    if (bias == NULL) {
      e_error("cannot load bias [%s]: aborting", prefs.biasimage);
      return(-1);
    }

    /* additive correction we need to apply to the saturation
       level of science images */
    satlevbiascorr = image_getmedian_vig(bias,
                                         prefs.statssec[0],
                                         prefs.statssec[2],
                                         prefs.statssec[1],
                                         prefs.statssec[3]);

    /* check whether the bias is 'bad' if we need */
    if (treatbadccd == 1) {
      fh = qfits_header_read(prefs.biasimage);

      if (fh == NULL) {
        e_error("reading FITS header from [%s]", prefs.biasimage);
        return(-1);
      }

      biasbad = (int)qfits_header_getint(fh, prefs.badccdkey, -1);

      if (biasbad == -1) {
        e_warning("image %s contains no %s key specifying goodness of CCD.",
                  prefs.biasimage, prefs.badccdkey);
        biasbad = 0;
      }

      if (biasbad != 0) {
        biasbad = 1;
      }

      qfits_header_destroy(fh);
    }
  }

  if (prefs.flatflag == 1) {
    e_comment(1, "-> %d.%d: Load Flatfield Frame %s", part,
              subpart++, prefs.flatimage);

    /* load flatfield */
    flat = image_loadext(prefs.flatimage, 0, 0, 0);

    if (flat == NULL) {
      e_error("cannot load flat [%s]: aborting", prefs.flatimage);
      return(-1);
    }

    /* check whether the flat is 'bad' if we need to */
    if (treatbadccd == 1) {
      fh = qfits_header_read(prefs.flatimage);

      if (fh == NULL) {
        e_error("reading FITS header from [%s]", prefs.flatimage);
        return(-1);
      }

      flatbad = (int)qfits_header_getint(fh, prefs.badccdkey, -1);

      if (flatbad == -1) {
        e_warning("image %s contains no %s key specifying goodness of CCD.",
                  prefs.flatimage, prefs.badccdkey);
        flatbad = 0;
      }

      if (flatbad != 0) {
        flatbad = 1;
      }

      qfits_header_destroy(fh);
    }

    /* determine flatfield normalisation */

    /* make sure that 'maxflatscale' is '1' (i.e. a valid flat number) in
       case of a 'bad' flat field */
    if (flatbad == 0) {
      maxflatscale = image_getmedian_vig_lowhighreject(flat,
                                                       prefs.statssec[0],
                                                       prefs.statssec[2],
                                                       prefs.statssec[1],
                                                       prefs.statssec[3],
                                                       0.0001,
                                                       hi_rej,
                                                       0.0);
    } else {
      maxflatscale = 1.0;
    }

    /* we do not take into account the flat threshhold if the flat
       is marked as bad already */
    if (maxflatscale < prefs.flatthreshhold && flatbad == 0) {
      e_error("Flat image [%s] has median %f which is below the threshold %f: aborting",
              prefs.flatimage, maxflatscale, prefs.flatthreshhold);
      return(-1);
    }

    e_comment(2, "Flat %s has median %f", prefs.flatimage,
              maxflatscale);

    /* Do 'flat rescaling' only if the flat is o.k. Otherwise all resulting
       (flatfielded) images are marked as bad anyway lateron */
    if (prefs.flatscaleflag == 1 && flatbad == 0) {
      /* determine scale factor: If it is not provided directly we use
         the minimum gain from all the different chips. */
      maxflatscale = 0.0;
      if (prefs.flatscalefactor > 1.0e-06) {
        maxflatscale = prefs.flatscalefactor;
        e_comment(2, "Scaling factor %f set on the commandline",
                  prefs.flatscalefactor);
      } else {
        for (i = 0; i < prefs.nflatscaleimage; i++) {
          flatscale = image_loadext(prefs.flatscaleimage[i], 0, 0, 0);

          if (flatscale == NULL) {
            e_error("cannot load image [%s]: aborting",
                    prefs.flatscaleimage[i]);
            return(-1);
          }
          scalemedian[i] = image_getmedian_vig_lowhighreject(flatscale,
                                                             prefs.statssec[0],
                                                             prefs.statssec[2],
                                                             prefs.statssec[1],
                                                             prefs.statssec[3],
                                                             0.0001,
                                                             hi_rej,
                                                             -1.0);

          if (fabs(scalemedian[i] + 1.0) < 1.0e-05) {
            e_warning("image [%s] has bad median. I do not use it!",
                    prefs.flatscaleimage[i]);
          } else {
            e_comment(2, "image %s has median %f", prefs.flatscaleimage[i],
                      scalemedian[i]);

            if (scalemedian[i] > maxflatscale) {
              maxflatscale = scalemedian[i];
            }
          }
          image_del(flatscale);
        }

        /* If the scaling factor is still 0.0 here something went really
           wrong */
        if (fabs(maxflatscale) < 1.0e-05) {
          e_error("Scaling factor is 0.0.");
          e_error("Probably all flats used for scaling are bad! Aborting!");
          return(-1);
        }
      }

      /* gain correction factor to be applied (multiplied) for
         science images */
      flatgaincorr = image_getmedian_vig_lowhighreject(flat,
                                                       prefs.statssec[0],
                                                       prefs.statssec[2],
                                                       prefs.statssec[1],
                                                       prefs.statssec[3],
                                                       0.0001,
                                                       hi_rej,
                                                       1.0);
      flatgaincorr = flatgaincorr / maxflatscale;
    }

    e_comment(2, "dividing flat %s by %f", prefs.flatimage,
              maxflatscale);

    /* do the flatfield scaling */
    for (j = 0; j < (flat->lx * flat->ly); j++) {
      if (fabs(flat->data[j]) > prefs.flatthreshhold) {
        flat->data[j] /= maxflatscale;
      } else {
        flat->data[j] = 0.0;
      }
    }
  }

  if (prefs.fringeflag == 1) {
    e_comment(1, "-> %d.%d: Load Fringe Frame %s", part,
              subpart++, prefs.fringeimage);

    /* load fringe frame */
    fringe = image_loadext(prefs.fringeimage, 0, 0, 0);

    if (fringe == NULL) {
      e_error("cannot load fringe frame [%s]: aborting", prefs.fringeimage);
      return(-1);
    }

    /* check whether the fringe image is 'bad' if we need to */
    if (treatbadccd == 1) {
      fh = qfits_header_read(prefs.fringeimage);

      if (fh == NULL) {
        e_error("reading FITS header from [%s]", prefs.fringeimage);
        return(-1);
      }

      fringebad = (int)qfits_header_getint(fh, prefs.badccdkey, -1);

      if (fringebad == -1) {
        e_warning("image %s contains no %s key specifying goodness of CCD.",
                  prefs.fringeimage, prefs.badccdkey);
        fringebad = 0;
      }

      if (fringebad != 0) {
        fringebad = 1;
      }

      qfits_header_destroy(fh);
    }

    if (prefs.fringescaleflag == 1) {
      /* load fringe reference frame  for estimating
         fringe scaling
      */
      e_comment(1, "-> using rescaling of fringe images!");

      fringeref = image_loadext(prefs.fringerefimage, 0, 0, 0);

      if (fringeref == NULL) {
        e_error("cannot load fringe reference frame [%s]: aborting",
                prefs.fringeimage);
        return(-1);
      }

      if ((fringeref->lx != fringe->lx) || (fringeref->ly != fringe->ly)) {
        e_error("fringe ref. frame and fringe frame have incompatible sizes: aborting");
        e_error("fringe ref. frame is [%d %d]", fringeref->lx, fringeref->ly);
        e_error("fringe frame is [%d %d]", fringe->lx, fringe->ly);
        image_del(fringeref);
        return(-1);
      }

      /* estimate median for reference frame */
      fringerefmedian = image_getmedian_vig(fringeref,
                                            prefs.statssec[0],
                                            prefs.statssec[2],
                                            prefs.statssec[1],
                                            prefs.statssec[3]);

      if (fabs(fringerefmedian) < 1e-4 && fringebad != 1) {
        e_error("zero value for median of fringe reference frame: aborting");
        return(-1);
      }

      image_del(fringeref);
    } else {
      e_comment(1, "-> You are not using rescaling of fringe images!");
    }
  }

  e_comment(0, "-> Part %d: performing run processing", ++part);
  subpart = 1;

  /* now do the preprocessing for all the runs */
  for (run = 0; run <= nruns; run++) {
    /* load input images from the current run into a cube */
    nimarun = MIN(prefs.maximages, prefs.ninname - prefs.maximages * run);

    if (nimarun > 0) {
      e_comment(1, "-> %d.%d: loading input images for run %d", part,
                subpart++, (run + 1));
      i_cube = cube_load_strings(&prefs.inname[run * prefs.maximages], nimarun);

      if (i_cube == NULL) {
        e_error("loading input images", "");
        return(-1);
      }

      /* perform overscan correction */
      if (prefs.overscanflag == 1) {
        e_comment(1, "-> %d.%d: performing overscan correction", part,
                  subpart++);

        for (i = 0; i < nimarun; i++) {
          /* additive correction to saturation level of science images due
           to overscan correction */
          satlevovscancorr[i] = image_getmedian_vig(i_cube->plane[i],
                                                    prefs.overscanreg[0], 1,
                                                    prefs.overscanreg[1],
                                                    i_cube->ly - 1);

          /* perform overscan correction */
          corrected = wfi_myoverscan_correction(i_cube->plane[i],
                                                prefs.overscanreg,
                                                prefs.overscanreject);

          if (corrected == NULL) {
            e_error("overscan correction failed for frame %d", i + 1);
            cube_del(i_cube);
            return(-1);
          }
        }
      }

      /* trim images */
      if (prefs.trimflag == 1) {
        e_comment(1, "-> %d.%d: trimming images", part, subpart++);

        if ((prefs.trimreg[0] >= prefs.trimreg[1]) ||
            (prefs.trimreg[2] >= prefs.trimreg[3])) {
          e_error("in crop region definition");
          cube_del(i_cube);
          return(-1);
        }

        for (i = 0; i < nimarun; i++) {
          corrected = image_getvig(i_cube->plane[i],
                                   prefs.trimreg[0],
                                   prefs.trimreg[2],
                                   prefs.trimreg[1],
                                   prefs.trimreg[3]);

          if (corrected == NULL) {
            e_error("trimming failed for frame %d", i + 1);
            cube_del(i_cube);
            return(-1);
          }

          image_del(i_cube->plane[i]);
          i_cube->plane[i] = corrected;
        }
        /* Reset global cube size */
        i_cube->lx = i_cube->plane[0]->lx;
        i_cube->ly = i_cube->plane[0]->ly;
      }

      /* perform biassubtraction */
      if (prefs.biasflag == 1) {
        e_comment(1, "-> %d.%d: Bias subtraction", part, subpart++);

        if ((bias->lx != i_cube->lx) || (bias->ly != i_cube->ly)) {
          e_error("bias and cube have incompatible sizes: aborting");
          e_error("bias is [%d %d]", bias->lx, bias->ly);
          e_error("cube is [%d %d]", i_cube->lx, i_cube->ly);
          cube_del(i_cube);
          image_del(bias);
          return(-1);
        }
        cube_sub_im(i_cube, bias);
      }

      /* perform flatfielding */
      if (prefs.flatflag == 1) {
        e_comment(1, "-> %d.%d: Flatfield correction", part, subpart++);

        if ((flat->lx != i_cube->lx) || (flat->ly != i_cube->ly)) {
          e_error("flat and cube have incompatible sizes: aborting");
          e_error("flat is [%d %d]", flat->lx, flat->ly);
          e_error("cube is [%d %d]", i_cube->lx, i_cube->ly);
          cube_del(i_cube);
          image_del(flat);
          return(-1);
        }

        cube_div_im(i_cube, flat);
      }

      /* perform fringe correction */
      if (prefs.fringeflag == 1) {
        e_comment(0, "-> %d.%d: Fringe correction", part, subpart++);

        if ((fringe->lx != i_cube->lx) || (fringe->ly != i_cube->ly)) {
          e_error("fringe frame and cube have incompatible sizes: aborting");
          e_error("fringe frame is [%d %d]", fringe->lx, fringe->ly);
          e_error("cube is [%d %d]", i_cube->lx, i_cube->ly);
          cube_del(i_cube);
          image_del(fringe);
          return(-1);
        }

        /* if necessary, estimate fringe scaling factors for all
           science frames ans ubtract (scaled) fringeframe: */
        for (i = 0; i < nimarun; i++) {
          if (prefs.fringescaleflag == 1 && fringebad != 1) {
            mediantmp = image_getmedian_vig(i_cube->plane[i],
                                            prefs.statssec[0],
                                            prefs.statssec[2],
                                            prefs.statssec[1],
                                            prefs.statssec[3]);
            fringescaling[i] = mediantmp / fringerefmedian;
          } else {
            fringescaling[i] = 1.0;
          }

          e_comment(1, "Fringe scaling for Image %s: %f", prefs.inname[i],
                      fringescaling[i]);

          for (j = 0; j < (i_cube->plane[i]->lx * i_cube->plane[i]->ly); j++) {
            i_cube->plane[i]->data[j] -=
              (fringe->data[j] * fringescaling[i]);
          }
        }
      } /* prefs.fringeflag */

      /* save result cube to disk in form of single images */
      if (prefs.outputpreprocflag == 1) {
        e_comment(1, "-> %d.%d: saving preprocessed frames", part, subpart++);

        for (i = 0; i < i_cube->np; i++) {
          /* Build output name */
          sprintf(tmpstr, prefs.outputpreprocsuffix, i);
          sprintf(name_o, "%s/%s%s", prefs.outputdirectory,
                  get_rootname(
                    get_basename(prefs.inname[i + run * prefs.maximages])),
                  tmpstr);
          e_comment(1, "saving [%s]", name_o);

          /* read header from input image */
          fh = qfits_header_read(prefs.inname[i + run * prefs.maximages]);

          if (fh == NULL) {
            e_error("reading FITS header from [%s]",
                    prefs.inname[i + run * prefs.maximages]);
            return(-1);
          }

          /* read saturation level if present */
          satlev = (pixelvalue)qfits_header_getdouble(fh,
                                                      prefs.satlevkey,
                                                      -1.0);

          if (satlev > 0.0) {
            /* get new value of saturation level due to overscan
               correction, bias subtraction and flatfielding */
            satlev = (satlev - satlevovscancorr[i] - satlevbiascorr) /
              flatgaincorr;
          }

          /* read badccd header keyword if necessary */
          if (treatbadccd == 1) {
            sciencebad = (int)qfits_header_getint(fh, prefs.badccdkey, -1);

            if (sciencebad == -1) {
              e_warning("image %s contains no %s key specifying goodness of CCD.",
                  prefs.inname[i + run * prefs.maximages], prefs.badccdkey);
            }

          }

          /* collect information about the reduction process in
             'HISTORY' keywords */
          hs = history_new();

          if (hs == NULL) {
            e_error("cannot create history object");
            return(-1);
          }

          history_add(hs, "");
          history_add(hs, "preprocess: %s", __PIPEVERS__);
          history_add(hs, "preprocess: called at %s", get_datetime_iso8601());

          if (prefs.overscanflag == 1) {
            history_add(hs, "image has been overscan corrected");
            history_add(hs, "overscan region: %d %d",
                        prefs.overscanreg[0], prefs.overscanreg[1]);

            if(satlev > 0.0) {
              history_add(hs,
                 "The saturation level was corrected for overscan by %.2f ADU",
                 satlevovscancorr[i]);
            }
          }

          if (prefs.trimflag == 1) {
            history_add(hs, "image has been trimmed");
            history_add(hs, "trim section: Xmin = %d; Xmax = %d; Ymin = %d; Ymax = %d",
                        prefs.trimreg[0], prefs.trimreg[1],
                        prefs.trimreg[2], prefs.trimreg[3]);
          }

          if (prefs.biasflag == 1) {
            history_add(hs, "image has been bias subtracted");
            history_add(hs, "Bias frame: %s", get_basename(prefs.biasimage));

            if (satlev > 0.0) {
              history_add(hs,
                 "The saturation level was corrected for bias by %.2f ADU",
                 satlevbiascorr);
            }

            if (biasbad == 1) {
              history_add(hs,
                 "Image gets set the key %s to 1 because the bias is bad!",
                 prefs.badccdkey);
            }
          }

          if (prefs.flatflag == 1) {
            history_add(hs, "image has been flat fielded");
            history_add(hs, "Flat frame: %s", get_basename(prefs.flatimage));

            if (prefs.flatscaleflag == 1) {
              history_add(hs, "The flatfield has been scaled to %f",
                          maxflatscale);

              /* If necessary add the flatscaling images to the history */
              if (fabs(prefs.flatscalefactor) < 1-0e-06) {
                for (j = 0; j < prefs.nflatscaleimage; j++) {
                  history_add(hs, "%s %f",
                              get_basename(prefs.flatscaleimage[j]),
                                           scalemedian[j]);
                }
              } else {
                history_add(hs, "The scaling factor was calculated externally");
              }

              /* check whether there is a GAIN keyword that needs to
                 be updated */
              gainvalue = (pixelvalue)qfits_header_getdouble(fh,
                                                             prefs.gainkey,
                                                             -1.0);

              /* correct the GAIN for a flatfielding scale factor */
              if(gainvalue > 0.0) {
                if(qfits_header_getcom(fh, prefs.gainkey) == NULL) {
                  strcpy(gaincomm, "");
                } else {
                  strncpy(gaincomm, qfits_header_getcom(fh, prefs.gainkey),
                          MAXCHAR - 1);
                }

                gainvalue = gainvalue * flatgaincorr;
                sprintf(gainvaluestring, "%.2f", gainvalue);

                qfits_header_mod(fh, prefs.gainkey,
                                 gainvaluestring, gaincomm);
                history_add(hs, "The gain key %s was changed from %.2f to %.2f",
                            prefs.gainkey, gainvalue / flatgaincorr, gainvalue);
              } else {
                e_warning("image %s contains no %s key specifying the GAIN.",
                          prefs.inname[i + run * prefs.maximages],
                          prefs.gainkey);
              }

              /* report on the saturation level change due to flatfield
                 scaling */
              if(satlev > 0.0) {
                history_add(hs,
                 "The sat. level was multiplied for the flat with %.2f",
                 1.0 / flatgaincorr);
              }
            } else {
              history_add(hs,
                          "The flatfield has been normalised to a median of 1");
            }

            if (flatbad == 1) {
              history_add(hs,
                 "Image gets set the key %s to 1 because the flat is bad!",
                 prefs.badccdkey);
            }

          }

          if (prefs.fringeflag == 1) {
            history_add(hs, "image has been fringe corrected");
            history_add(hs, "Fringe frame: %s; scaling %f",
                        get_basename(prefs.fringeimage),
                        fringescaling[i]);
            history_add(hs, "The scaling was determined w.r.t.: %s",
                        get_basename(prefs.fringerefimage));

            if (fringebad == 1) {
              history_add(hs,
              "Image gets set the key %s to 1 because the fringe frame is bad!",
               prefs.badccdkey);
            }
          }

          history_addfits(hs, fh);

          /* write the new saturation level if necessary */
          if(satlev > 0.0) {
            if(qfits_header_getcom(fh, prefs.satlevkey) == NULL) {
              strcpy(satlevcomm, "");
            } else {
              strncpy(satlevcomm, qfits_header_getcom(fh, prefs.satlevkey),
                      MAXCHAR - 1);
            }

            sprintf(satlevvaluestring, "%.2f", satlev);

            qfits_header_mod(fh, prefs.satlevkey,
                           satlevvaluestring, satlevcomm);
          }

          /* deal with the badccdkey if necessary */
          if (treatbadccd == 1) {
            if (biasbad == 1 || flatbad == 1 ||
                fringebad == 1 || sciencebad == 1) {
              sprintf(tmpstr, "%d", 1);
            } else {
              sprintf(tmpstr, "%d", 0);
            }

            if (sciencebad == -1) {
              qfits_header_add(fh, prefs.badccdkey,
                               tmpstr, "Is_CCD_Bad_(1=Yes)", NULL);
            } else {
              qfits_header_mod(fh, prefs.badccdkey,
                               tmpstr, "Is_CCD_Bad_(1=Yes)");
            }
          }

          /* and finally save */
          if (prefs.outputbitpix == -32) {
            image_save_fits_hdrdump(i_cube->plane[i],
                                    name_o,
                                    fh,
                                    BPP_IEEE_FLOAT);
          }
          if (prefs.outputbitpix == 16) {
            image_save_fits_hdrdump(i_cube->plane[i],
                                    name_o,
                                    fh,
                                    BPP_16_SIGNED);
          }

          qfits_header_destroy(fh);
          history_del(hs);
        }
      }

      /* free memory from the current runs cube; do not do this if
         only one run is involved as we may then still coadd the
         images */
      if (nruns > 0) {
        cube_del(i_cube);
      }
    }
  }

  /* free all calibration frames */
  if (prefs.biasflag == 1) {
    image_del(bias);
  }

  if (prefs.flatflag == 1) {
    image_del(flat);
  }

  if (prefs.fringeflag == 1) {
    image_del(fringe);
  }

  /* combine cube; this is only done if there was only ONE run */
  if (prefs.combineflag == 1) {
    e_comment(0, "-> Part %d: preparing the stacking process", ++part);

    /* deal with badccds if necesary. We do not want to consider
       BADCCD data in the combination process */
    if (treatbadccd == 1) {
      for (i = 0; i < i_cube->np; i++) {
        fh = qfits_header_read(prefs.inname[i]);

        if (fh == NULL) {
          e_error("reading FITS header from [%s]",
                  prefs.inname[i]);
          return(-1);
        }

        sciencebad = (int)qfits_header_getint(fh, prefs.badccdkey, -1);

        if (sciencebad == -1) {
          e_warning("image %s contains no %s key specifying goodness of CCD.",
                    prefs.inname[i], prefs.badccdkey);
        }

        qfits_header_destroy(fh);

        validimages[i] = 0;
        if (sciencebad > 0) {
          validimages[i] = 1;
          nsciencebad++;
        }
      }

      /* reject badccd images from the cube: */
      cube_reject_planes(&(i_cube), validimages);
    }

    if (i_cube == NULL) {
      e_error("All images to be stacked are bad! I abort!");
      return(-1);
    }

    if (nsciencebad > 0) {
      e_warning("I rejected %d bad images from the stacking process",
                nsciencebad);
    }

    if (prefs.combinerescaleflag == 0) {
      e_comment(0, "-> Part %d: stacking cube without rescaling", ++part);

      /* check whether the coadded image will be useful */
      if (i_cube->np - prefs.combinereject[0] - prefs.combinereject[1] < 3) {
        e_warning("coadded image will have the default 0 value everywhere !!!");
        e_warning("Try to reject less low/high values");

        stackbad = 1;
      }

      meanmedian = 0.0;       /* default value for combinations without
                                 rescaling */
      stacked    = cube_avg_median_minmax_rej(i_cube,
                                              prefs.combineminthresh,
                                              prefs.combinemaxthresh,
                                              prefs.combinereject[0],
                                              prefs.combinereject[1],
                                              meanmedian);
    } else {
      e_comment(0, "-> Part %d: stacking cube with rescaling",
                ++part);

      /* determine median of all images and rescale to the mean of the
         medians */
      e_comment(1, "determine median of input frames:");

      meanmedian = 0.0;
      for (i = 0; i < prefs.ninname; i++) {
        combinemedian[i] = image_getmedian_vig_lowhighreject(i_cube->plane[i],
                                                             prefs.statssec[0],
                                                             prefs.statssec[2],
                                                             prefs.statssec[1],
                                                             prefs.statssec[3],
                                                             lo_rej,
                                                             hi_rej,
                                                             lo_rej);

        if (fabs(combinemedian[i] - lo_rej) < 1.0e-05) {
          e_error("image [%s] has a bad median!",
                  prefs.inname[i]);
          return(-1);
        }

        e_comment(1, "Image %s: %f", prefs.inname[i],
                  combinemedian[i]);

        if (fabs(combinemedian[i]) < 1e-4) {
          e_warning("zero value for median of image [%s]: aborting",
                    prefs.inname[i]);
          return(-1);
        }

        meanmedian += combinemedian[i];
      }

      meanmedian /= (pixelvalue)prefs.ninname;

      e_comment(1, "mean of medians; %f", meanmedian);

      for (i = 0; i < prefs.ninname; i++) {
        combinescaling = (meanmedian / combinemedian[i]);

        for (j = 0; j < (i_cube->plane[i]->lx * i_cube->plane[i]->ly); j++) {
          i_cube->plane[i]->data[j] *= combinescaling;
        }
      }

      /* check whether the coadded image will be useful */
      if (i_cube->np - prefs.combinereject[0] - prefs.combinereject[1] < 3) {
        e_warning("coadded image will have the default value %f everywhere !!! Try to reject less low/high values",
                  meanmedian);
      }

      stacked = cube_avg_median_minmax_rej(i_cube,
                                           prefs.combineminthresh,
                                           prefs.combinemaxthresh,
                                           prefs.combinereject[0],
                                           prefs.combinereject[1],
                                           meanmedian);
    }

    if (stacked == NULL) {
      e_error("in cube combination: aborting");
      return(-1);
    }

    /* create image header for stacked frame */
    fh_stack = qfits_header_default();
    /* BITPIX */
    if (prefs.combinebitpix == -32) {
      sprintf(tmpstr, "%d", BPP_IEEE_FLOAT);
    }
    if (prefs.combinebitpix == 16) {
      sprintf(tmpstr, "%d", BPP_16_SIGNED);
    }
    qfits_header_add(fh_stack, "BITPIX", tmpstr, "Bits per pixel", NULL);
    /* NAXIS */
    qfits_header_add(fh_stack, "NAXIS", "2", "Number of axes", NULL);
    sprintf(tmpstr, "%d", stacked->lx);
    qfits_header_add(fh_stack, "NAXIS1", tmpstr, "size in X", NULL);
    sprintf(tmpstr, "%d", stacked->ly);
    qfits_header_add(fh_stack, "NAXIS2", tmpstr, "size in Y", NULL);

    if (treatbadccd == 1) {
      if (stackbad == 1) {
        qfits_header_add(fh_stack, prefs.badccdkey, "1",
                         "Is_CCD_Bad_(1=Yes)", NULL);
      } else {
        qfits_header_add(fh_stack, prefs.badccdkey, "0",
                         "Is_CCD_Bad_(1=Yes)", NULL);
      }
    }

    /* collect information about the combination process in 'HISTORY'
       keywords */
    hs = history_new();

    if (hs == NULL) {
      e_error("cannot create history object");
      return(-1);
    }

    history_add(hs, " ");
    history_add(hs, "preprocess: %s", __PIPEVERS__);
    history_add(hs, "preprocess: %s", get_datetime_iso8601());
    history_add(hs, "preprocess: median of %d images", prefs.ninname);

    history_add(hs, "preprocess: min and max threshhold used: %f %f",
                prefs.combineminthresh, prefs.combinemaxthresh);
    history_add(hs,
                "preprocess: low and high number of rejected  pixels: %d %d",
                prefs.combinereject[0], prefs.combinereject[1]);

    if (prefs.combinerescaleflag == 1) {
      history_add(hs, "preprocess: combination with rescaling to median %f",
                  meanmedian);

      for (i = 0; i < prefs.ninname; i++) {
        history_add(hs, "preprocess: %s %.2f",
                    get_basename(prefs.inname[i]),
                    combinemedian[i]);
      }
    } else {
      history_add(hs, "preprocess: combination without rescaling");

      for (i = 0; i < prefs.ninname; i++) {
        history_add(hs, "preprocess: %s",
                    get_basename(prefs.inname[i]));
      }
    }

    history_addfits(hs, fh_stack);

    if (prefs.combinebitpix == -32) {
      image_save_fits_hdrdump(stacked, prefs.combineoutput,
                              fh_stack, BPP_IEEE_FLOAT);
    }
    if (prefs.combinebitpix == 16) {
      image_save_fits_hdrdump(stacked, prefs.combineoutput,
                              fh_stack, BPP_16_SIGNED);
    }
    qfits_header_destroy(fh_stack);
    history_del(hs);
    image_del(stacked);
  }

  /* free memory and bye */
  free(prefs.inname);
  free(satlevovscancorr);
  free(combinemedian);
  free(scalemedian);
  free(fringescaling);

  if (nruns == 0) {
    cube_del(i_cube);
  }

  if (debug_active()) {
    xmemory_status();
  }
  return(0);
}

/*
 * This function only gives the usage for the program
 */

void printUsage()
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "        preprocess - Mosaic camera preprocessing software\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "        preprocess images [options]\n");
  fprintf(stdout, "                   -h    # print this help and leave\n");
  fprintf(stdout, "                   -d    # dump standard configuration file and leave\n");
  fprintf(stdout, "                   -c    # name of the configuration file\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION \n");
  fprintf(stdout, "        The program performs all steps necessary to take the\n");
  fprintf(stdout, "        instrumental signature out of optical astronomical CCD\n");
  fprintf(stdout, "        data. Given a set of input images, the program can perform the\n");
  fprintf(stdout, "        following operations, or any specified subset, on the data:\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        - overscan correction:\n");
  fprintf(stdout, "          From each line, the pixels in the overscan region\n");
  fprintf(stdout, "          (OVERSCAN_REGION) are collected, the lowest and highest\n");
  fprintf(stdout, "          values specified in OVERSCAN_REJECT rejected and the mean\n");
  fprintf(stdout, "          estimated. That mean is subtracted from all pixels in that\n");
  fprintf(stdout, "          line.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        - Trimming:\n");
  fprintf(stdout, "          all input images are trimmed to the section specified in\n");
  fprintf(stdout, "          TRIM_REGION\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        - Bias subtraction:\n");
  fprintf(stdout, "          The image specified with BIAS_IMAGE is subtracted from all\n");
  fprintf(stdout, "          input images.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        - Flat fielding:\n");
  fprintf(stdout, "          All images are divided by the image specified in\n");
  fprintf(stdout, "          FLAT_IMAGE. If scaling (FLAT_SCALE) is asked for, the\n");
  fprintf(stdout, "          flatfield is normalised by highest median of the images\n");
  fprintf(stdout, "          specified in FLAT_SCALEIMAGE. Another possibility is to\n");
  fprintf(stdout, "          directly provide a flatfield scaling factor with option\n");
  fprintf(stdout, "          FLAT_SCALEVALUE. In this case flatfields are normalised by\n");
  fprintf(stdout, "          division with FLAT_SCALEVALUE.  If a header keyword\n");
  fprintf(stdout, "          specifying the image gain is provided (GAIN_KEYWORD) that\n");
  fprintf(stdout, "          keyword is updated according to flatfieldscaling. In the\n");
  fprintf(stdout, "          case no flatfield scaling is specified, the flat is\n");
  fprintf(stdout, "          normalised to a median of unity.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        - Fringe correction:\n");
  fprintf(stdout, "          A fringe pattern given with FRINGE_IMAGE is subtracted from\n");
  fprintf(stdout, "          all images. The fringe pattern is scaled with\n");
  fprintf(stdout, "          median(FRINGE_REFIMAGE)/median(image), i.e. we assume that\n");
  fprintf(stdout, "          the intensity of the fringes scales with the sky background.\n");
  fprintf(stdout, "          This scaling can be turned off with the FRINGE_SCALE option.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        - Result output:\n");
  fprintf(stdout, "          All preprocessed images are saved to disk in a directory\n");
  fprintf(stdout, "          given in OUTPUT_DIR. The ending '.fits' of each input image\n");
  fprintf(stdout, "          is replaced by OUTPUT_SUFFIX.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        - Image stacking:\n");
  fprintf(stdout, "          THIS IS ONLY AVAILABLE IF ALL INPUT IMAGES CAN\n");
  fprintf(stdout, "          SIMULTANEOUSLY BE LOADED INTO VIRTUAL MEMORY; SEE BELOW !!!!\n");
  fprintf(stdout, "          all images are stacked. At each image position, the\n");
  fprintf(stdout, "          individual pixel values are collected. We reject all pixels\n");
  fprintf(stdout, "          being smaller than COMBINE_MINVAL and larger than\n");
  fprintf(stdout, "          COMBINE_MAXVAL. From the remaining pixels we further do not\n");
  fprintf(stdout, "          consider the COMBINE_LOREJ lowest and COMBINE_HIREJ highest\n");
  fprintf(stdout, "          values. From the rest we estimate the median. If rescaling\n");
  fprintf(stdout, "          (COMBINE_RESCALE) is demanded we rescale all images to the\n");
  fprintf(stdout, "          mean of the medians from all input data prior to coaddition.\n");
  fprintf(stdout, "          If at some stage less than 3 pixels are left, the result is\n");
  fprintf(stdout, "          the mean of the medians of input images in the rescale case\n");
  fprintf(stdout, "          and 0 otherwise. If less than 3 images are supplied in the\n");
  fprintf(stdout, "          beginning, no stacking is performed. The result is saved to\n");
  fprintf(stdout, "          COMBINE_OUTPUT.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        For all operations requiring statistics on the input frames,\n");
  fprintf(stdout, "        the image region specified in STATSSEC is used.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "        If a header keyword for the saturation level of the image is\n");
  fprintf(stdout, "        provided (option SATLEV_KEYWORD), this key is updated in the\n");
  fprintf(stdout, "        overscan, bias and flatfield operations.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        If a header keyword to signal defective CCDs is given (option\n");
  fprintf(stdout, "        BADCCD_KEYWORD; integer value. A value unequal to zero signals\n");
  fprintf(stdout, "        a defective CCD), this information is used as follows:\n");
  fprintf(stdout, "        - All output data will contain a badccd keyword in their\n");
  fprintf(stdout, "          headers.\n");
  fprintf(stdout, "        - If a calibration file is bad the processed data are marked\n");
  fprintf(stdout, "          as bad as well.\n");
  fprintf(stdout, "        - If a science image that should be processed is marked as\n");
  fprintf(stdout, "          bad, the processed image will be marked as well.\n");
  fprintf(stdout, "        - If images arestacked, defective CCDs will not be considered\n");
  fprintf(stdout, "          in this process.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        The program loads consecutively frames in chunks of MAXIMAGES\n");
  fprintf(stdout, "        into the memory. Set it to a value so that MAXIMAGES frames\n");
  fprintf(stdout, "        fit into the RAM of your machine. All processes (except\n");
  fprintf(stdout, "        stacking) can naturally be split in this way. Hence,\n");
  fprintf(stdout, "        coaddition is NOT available if more than MAXIMAGES frames have\n");
  fprintf(stdout, "        to be processed.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "EXAMPLES:\n");
  fprintf(stdout, "        - creation of a master bias frame:\n");
  fprintf(stdout, "          the creation of a master bias frame usually consists\n");
  fprintf(stdout, "          of overscan correction, trimming and combination\n");
  fprintf(stdout, "          without rescaling. This could be achieved with:\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "          preprocess *.fits -OVERSCAN Y -TRIM Y -COMBINE Y\n");
  fprintf(stdout, "                            -COMBINE_RESCALE N \n");
  fprintf(stdout, "                            -COMBINE_OUTPUT BIAS.fits\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "          The default values for overscan-, statistic region\n");
  fprintf(stdout, "          and rejection values in the coaddition process would\n");
  fprintf(stdout, "          have to be updated if necessary.\n");
  fprintf(stdout, "        \n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR\n");
  fprintf(stdout, "        Thomas Erben: terben@astro.uni-bonn.de\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        The code is largely based on the eclipse library (see\n");
  fprintf(stdout, "        http://www.eso.org/eclipse) and on sources from Emmanuel\n");
  fprintf(stdout, "        Bertin for the handling of configuration options.\n");
}
