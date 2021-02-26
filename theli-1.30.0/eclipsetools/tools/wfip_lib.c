/*---------------------------------------------------------------------------

   File name  :	wfip_lib.c
   Author     :	N. Devillard
   Created on	:	18 May 2000
   Description	:	WFI library utilities

*--------------------------------------------------------------------------*/

/*
        $Id: wfip_lib.c,v 1.33 2017/10/04 15:25:32 thomas Exp $
        $Author: thomas $
        $Date: 2017/10/04 15:25:32 $
        $Revision: 1.33 $
 */

/*---------------------------------------------------------------------------
                         History Comments
   ---------------------------------------------------------------------------*/

/*
   05.09.03:
   function wfi_mysplit:
   changes to include the OBJECT keyword in the new headers

   13.10.03:
   included possibility to flip frames in x and/or y
   (function wfi_mysplit)
   Besides the pure flipping of image pixels, the CD matrix
   and the CRPIX values are computed out of the old ones.

   function wfi_mysplit:
   corrected a bug treating header items incorrectly if only
   a single extensions is extracted instead of all the extensions

   function wfi_mysplit:
   the wrong off diagonal terms of the CD matrix were updated when
   flipping the image.

   23.04.2004:
   function wfi_mysplit:
   - Also FITS images without extensions can be processed
     now. In this case, only the header is updated and a
     new FITS file with IMAGEID 1 is written.

   - I corrected an error in the setting of BSCALE if this
     keyword is not present in the original header (in this
     case it is set to 1 now).

   - The output format foe float number requiring high accuracy
     is chenaged from '%G% to '%E'. This format avoids
     the necessary special treatment if a float can be represented
     as integer.

   30.11.2004:
   function wfi_mysplit:
   The error message in the case that the number of extensions
   does not match data on the command line has been modified.

   07.12.2004:
   function wfi_mysplit:
   we add default values for magnitude zeropoint (ZP) and
   extinction coefficient (COEFF). A -1.0 in both components
   says that the image has not yet been calibrated.

   27.01.2005:
   routine wfi_mysplit:
   - added global version number information to the splitted
     images
   - I added timestamp information to the splitted images

   28.01.2005:
   routine wfi_mysplit:
   I slightly changed the output format for the global
   version number

   01.02.2005:
   routine wfi_mysplit:
   when checking the consistency of parameters against extensions,
   I have to use the number of extensions in the image, not
   the number of extensions I want to extract.

   18.02.2005:
   routine wfi_mysplit:
   In the HISTORY which file was splitted we now use the basename
   from the original file instead of the filename including the
   complete path.

   24.02.2005:
   routine wfi_mysplit:
   - I removed the superfluous n_ext variable.
   - I corrected a bug in the y flip. It was hardwired that
     the image was in 16 bit INT format.

   19.04.2005:
   major changes to function wfi_mysplit:
   - Images can be transformed with arbitraty shifts and 90
     degree rotations
   - We integrate a case where no header update/remapping is done
     but images are splitted with the old headers intact (mimicing
     the original wfi_split routine)

   26.04.2005:
   function wfi_mysplit:
   I reintroduced variable n_ext (see changes from 24.02.). By
   removing it the check for consistency of parameters against
   extensions failed when extracting only a single extension
   (see also change from  01.02.2005).

   05.07.2005:
   function wfi_mysplit:
   I switched back to the old way of loading and saving images
   (not using the image load/save capabilities from eclipse).
   There were problems when the input images were unsigned
   INT as this type is not supported by qfits.

   09.09.05:
   function wfi_mysplit:
   I corrected a bug in the case where images only have a main
   header (the bug was introduced with the changes from 26.04.05
   where the n_ext variable was reintroduced).

   09.09.05:
   function wfi_mysplit:
   an additional int array giving the IMAGEID numbers under that an
   extension has to be saved is introduced as argument.

   23.11.05:
   function wfi_mysplit:
   I add information to the history as which chip an extension is saved.

   29.11.05:
   function wfi_mysplit:
   If no header update was requested output filenames were not correct.
   (the 'chip' number is not determined by imageid but by extension
    number in this case).

   19.01.06:
   function wfi_mysplit:
   It is now ensured that the OBJECTS Fits key is ALWAYS a string.

   05.08.2006:
   function wfi_mysplit:
   I included in the main program 'mefsplit'
   the possibility to provide header information for
   only ONE image if a single extension has to extracted
   (command line option '-s'). Function wfi_mysplit
   was adapted accordingly.

   25.02.2010:
   - code beautifying
   - adding comments to header keywords in wfi_mysplit

   28.09.2010:
   function wfi_mysplit:
   I adapted this function to the new mefsplit functionality to set
   additional header keywords together with MEF splitting.

   26.11.2010:
   function wfi_mysplit:
   I adapted this function to the new mefsplit functionality to transfer
   header keywords from the original image to the splitted data.

   24.02.2011:
   function wfi_mysplit:
   Bug fix when checking for header keys that need to be transfered from
   the original to the splitted data.

   29.11.2012:
   function wfi_mysplit:
   I adapted this function to the new mefsplit functionality to modify
   names of transfered header keywords.

   10.07.2014:
   function wfi_mysplit:
   The fucntion got the argument imageidkey specifying a header keyword
   containing the chip number of the corresponding extension. DECAM
   has such a keyword (CCDNUM).

   21.07.2014:
   function wfi_mysplit:
   In case 'all' extensions should be extracted from the input MEF we
   now allow to not consider extensions at the very end. For DECam
   not used (probably tracking) chips are at the end of the MEF file
   and we do not want to extract them.

   15.07.2016:
   function wfi_mysplit:
   The history of split images now also contains history keywords
   from corresponding extensions of the original, unsplit data. Hence, old
   processing can be traced.

   20.07.2016:
   functions wfi_split and wfi_mysplit:
   a variable indicating the start of data sections from MEF extensions
   was modified from type int to long int. DECam MEF-files can be larger
   than 2 GB and hence the range of the int-type is not large enough.
   Note that this fix only works on 64-bit machines!

   04.10.2017:
   Reading a FITS header comment could result in a NULL pointer (no comment
   present) and result in subsequent crashes - fixed.
 */

/*---------------------------------------------------------------------------
                                                                Includes
   ---------------------------------------------------------------------------*/

#include "wfip_lib.h"
#include "qfits.h"

/*---------------------------------------------------------------------------
                                                        Function codes
   ---------------------------------------------------------------------------*/

int wfi_split(char *name_i, char *name_o, int xtnum)
{
  qfits_header *h_main;
  qfits_header *h_ext;
  int           i;
  int           n_ext;
  char          ext_name_o[FILENAMESZ];
  FILE *        extension;
  long int      data_beg;
  int           data_size;
  int           naxis1, naxis2, bitpix;
  int *         xts;
  int           nxts;
  char *        fdata;
  size_t        fsize;

  /* Sanity checks */
  if (is_fits_file(name_i) != 1) {
    e_error("cannot find file [%s]", name_i);
    return(-1);
  }

  /* Read main header */
  e_comment(0, "reading main header");
  h_main = qfits_header_read(name_i);
  if (h_main == NULL) {
    e_error("reading main header from [%s]", name_i);
    return(-1);
  }

  /* Find out how many extensions are in the file */
  e_comment(0, "finding number of extensions");
  n_ext = qfits_query_n_ext(name_i);
  if (n_ext < 1) {
    e_error("no extension found in [%s]", name_i);
    return(-1);
  }
  e_comment(0, "[%d] extensions found in file", n_ext);

  if (xtnum < 1) {
    nxts = n_ext;
    xts  = malloc(nxts * sizeof(int));
    for (i = 0; i < nxts; i++) {
      xts[i] = i + 1;
    }
  } else {
    nxts   = 1;
    xts    = malloc(nxts * sizeof(int));
    xts[0] = xtnum;
  }

  /* Go extension after extension */
  for (i = 0; i < nxts; i++) {
    /* Read extension header */
    e_comment(1, "reading extension [%d]", xts[i]);
    h_ext = qfits_header_readext(name_i, xts[i]);
    if (h_ext == NULL) {
      e_error("reading extension header #%d", xts[i]);
      continue;
    }
    /* Compute data size in bytes */
    naxis1 = qfits_header_getint(h_ext, "NAXIS1", 0);
    naxis2 = qfits_header_getint(h_ext, "NAXIS2", 0);
    bitpix = qfits_header_getint(h_ext, "BITPIX", 0);

    data_size = naxis1 * naxis2 * BYTESPERPIXEL(bitpix);
    if (data_size < 1) {
      e_error("cannot determine data size in bytes in ext[%d]", xts[i]);
      qfits_header_destroy(h_ext);
      continue;
    }

    /* Output new header to file */
    sprintf(ext_name_o, "%s_%02d.fits", name_o, xts[i]);
    if ((extension = fopen(ext_name_o, "w")) == NULL) {
      e_error("cannot output to file [%s]", ext_name_o);
      continue;
    }
    qfits_header_dump(h_main, extension);
    qfits_header_dump(h_ext, extension);
    qfits_header_destroy(h_ext);

    /* Find out pixels in main belonging to this extension */
    if (qfits_get_datinfo(name_i, xts[i], &data_beg, NULL) != 0) {
      e_error("getting offset to extension %d", xts[i]);
      continue;
    }
    /* Map input file */
    fdata = falloc(name_i, 0, &fsize);
    if (fdata == NULL) {
      e_error("mapping input file [%s]", name_i);
      continue;
    }
    /* Dump pixels from one file to the other */
    e_comment(1, "copying extension [%d]", xts[i]);
    fwrite(fdata + data_beg, 1, data_size, extension);

    /* Close file pointers */
    free(fdata);
    fclose(extension);

    /* zero-padding */
    qfits_zeropad(ext_name_o);
  }
  qfits_header_destroy(h_main);
  free(xts);
  return(0);
}

int wfi_mysplit(char *name_i, char *name_o,
                int xtnum, int headerupdate, int singleimagemode,
                char **headerkeys, char **headervalues, char **headercomments,
                int nheaderkeys,
                char **transferkeys, char **transferkeynames,
                int ntransferkeys,
                double *crpix1, double *crpix2,
                double *cd11, double *cd22, double *cd12, double *cd21,
                int *M11, int *M22, int *M12, int *M21,
                int *imageid, char *imageidkey, int ncrpix1,
                double crval1, double crval2,
                double exptime, double equinox, double airmass,
                int gabodsid, char *filter, char *object, char *version)
{
  qfits_header *h_main;
  qfits_header *h_ext;
  qfits_header *fh        = NULL;
  char         *new_image = NULL;
  FILE         *extension;
  char         *fdata = NULL;
  size_t        fsize;
  int           i, j, k, kp, l, lp, m;
  char          ext_name_o[FILENAMESZ];
  int           data_size = 0;
  long int      data_beg;
  int           naxis1, naxis2, bitpix = 0;
  int           naxis1n, naxis2n;
  /* the dimensions of the output image */
  float bscale, bzero;
  int   bytes;
  int   detM;                 /* the determinat of the image transformation
                                 matrix */
  float cd11n, cd12n, cd21n, cd22n; /* CD matrix elements of the new image */
  float crpix1n, crpix2n;     /* CRPIX elements of the new image */
  int *xts;
  int *par;                   /* which parameter set (from the function
                                 arguments to choose for some extension) */
  int n_ext;                  /* the number of extensions present in
                                 the input FITS file */
  int nxts = 0;               /* the number of extensions to extract from
                                 the file */
  char cval[80], cvaltmp[80];
  char *transferkey, *transfercomm; /* key and comment value of header
                                       entries to transfer */
  int  onlymain = 0;          /* image only has a main header and no extensions
                                 (0=NO; 1=YES) */
  history *hs;                /* history about the splitting process added
                                 to the new FITS header */

  /* Sanity checks */
  if (is_fits_file(name_i) != 1) {
    e_error("cannot find file [%s]", name_i);
    return(-1);
  }

  /* Read main header */
  e_comment(0, "reading main header");
  h_main = qfits_header_read(name_i);
  if (h_main == NULL) {
    e_error("reading main header from [%s]", name_i);
    return(-1);
  }

  /* Find out how many extensions are in the file */
  e_comment(0, "finding number of extensions");
  n_ext = qfits_query_n_ext(name_i);
  if (n_ext < 1) {
    /* the image only has a main header, i.e. we only have to update
       the header; we quit if no header update is requested as nothing
       is to do in this case. */
    if (headerupdate == 0) {
      e_error("nothing to do!! Exiting!!");
      return(-1);
    }
    e_comment(0, "image only has a main header");
    nxts     = 1;
    n_ext    = 1;
    onlymain = 1;
    xts      = malloc(nxts * sizeof(int));
    xts[0]   = 1;
    par      = malloc(nxts * sizeof(int));
    par[0]   = 0;
  } else {
    /* image has extensions; we have to do the splitting and/or the
       header update */
    e_comment(0, "[%d] extensions found in file.", n_ext);
    if (xtnum < 1) {
      /* in this case we have to extract 'all' extensions from the input
         FITS file. 'all' means that we might not consider some extensions
         at the end of the input MEF file */
      nxts = ncrpix1; /* ncrpix1 gives indirectly the number of extensions to
                         extract */
      xts  = malloc(nxts * sizeof(int));
      par  = malloc(nxts * sizeof(int));
      for (i = 0; i < nxts; i++) {
        xts[i] = i + 1;
        par[i] = i;
      }
    } else {
      nxts   = 1;
      xts    = malloc(nxts * sizeof(int));
      par    = malloc(nxts * sizeof(int));
      xts[0] = xtnum;

      if (singleimagemode == 0) {
        par[0] = xtnum - 1;
      } else {
        par[0] = 0;
      }
    }
  }

  if (headerupdate != 0) {
    /* in the case where header update is requested we have to insure
       consistency of the number of input parameters */

    /* We allow to extract less chips than extensions in the MEF file.
       Not considered extensions are at the very end of the MEF. */
    if ((singleimagemode == 0) && (ncrpix1 < n_ext)) {
      e_warning("The last %d extensions of file %s are not considered!",
                n_ext - ncrpix1, name_i);
    }

    if (((singleimagemode == 0) && (ncrpix1 > n_ext)) ||
        ((singleimagemode != 0) && (ncrpix1 != 1))) {
      e_error("nb. of values in crpix1 (%d) etc. is inconsistent with nb. of extensions expected (1 or %d) dependend on the '-s' command line option)",
              ncrpix1, n_ext);
      return(-1);
    }
  }

  /* Map input file */
  fdata = falloc(name_i, 0, &fsize);

  if (fdata == NULL) {
    e_error("error mapping input file [%s]", name_i);
    return(-1);
  }

  /* Go extension after extension */
  for (i = 0; i < nxts; i++) {
    /* Read extension header */
    if (onlymain == 1) {
      h_ext = h_main;
    } else {
      e_comment(1, "reading extension [%d]", xts[i]);
      h_ext = qfits_header_readext(name_i, xts[i]);
    }
    if (h_ext == NULL) {
      e_error("reading extension header #%d", xts[i]);
      continue;
    }

    /* name for the output file of this extension */
    if (onlymain == 1) {
      sprintf(ext_name_o, "%s_1.fits", name_o);
    } else {
      if (headerupdate != 0) {
        /* In case the imageid_key option is set we need to update
           the imageid value with the contents of a header keyword */
        if (strcmp(imageidkey, "") != 0) {
          imageid[i] = qfits_header_getint(h_ext, imageidkey, -1);

          if (imageid[i] == -1) {
            e_error("error reading header entry %s in extension %d of input file [%s]", imageidkey, i, name_i);
            return(-1);
          }
        }
        sprintf(ext_name_o, "%s_%d.fits", name_o, imageid[i]);
      } else {
        sprintf(ext_name_o, "%s_%d.fits", name_o, xts[i]);
      }
    }

    /* open output file */
    if ((extension = fopen(ext_name_o, "w")) == NULL) {
      e_error("cannot output to file [%s]", ext_name_o);
      continue;
    }

    /* load image pixel data */
    if (onlymain == 1) {
      if (qfits_get_datinfo(name_i, 0, &data_beg, NULL) != 0) {
        e_error("error getting offset to data");
        continue;
      }
    } else {
      if (qfits_get_datinfo(name_i, xts[i], &data_beg, NULL) != 0) {
        e_error("error getting offset to data");
        continue;
      }
    }

    /* Compute data size in bytes (needed for the no headerupdate
       case) */
    naxis1    = qfits_header_getint(h_ext, "NAXIS1", 0);
    naxis2    = qfits_header_getint(h_ext, "NAXIS2", 0);
    bitpix    = qfits_header_getint(h_ext, "BITPIX", 0);
    bscale    = (float)qfits_header_getdouble(h_ext, "BSCALE", 1.0);
    bzero     = (float)qfits_header_getdouble(h_ext, "BZERO", 0);
    bytes     = BYTESPERPIXEL(bitpix);
    data_size = naxis1 * naxis2 * bytes;

    if (data_size < 1) {
      e_error("cannot determine data size in bytes in ext[%d]", xts[i]);

      if (onlymain == 0) {
        qfits_header_destroy(h_ext);
      }
      continue;
    }

    /* do everything related to header update */
    if (headerupdate != 0) {
      /* Create new header to file */
      fh = qfits_header_default();

      /* create key entries for the new header */

      /* calcualte new orientation in case of image flipping */
      /* sanity check for the transformation matrix */
      detM = (M11[par[i]] * M22[par[i]] - M12[par[i]] * M21[par[i]]);

      if (detM != 1 && detM != -1) {
        e_error("error in transformation matrix for extension [%d] !! Aborting !!", i + 1);
        return(-1);
      }

      cd11n = M11[par[i]] * cd11[par[i]] +
              M12[par[i]] * cd21[par[i]];
      cd22n = M21[par[i]] * cd12[par[i]] +
              M22[par[i]] * cd22[par[i]];

      /* the detM for the offdiagonal elements accounts, in our case,
         for the order of rotation and flipping */
      cd21n = (M21[par[i]] * cd11[par[i]] +
               M22[par[i]] * cd21[par[i]]) * detM;
      cd12n = (M11[par[i]] * cd12[par[i]] +
               M12[par[i]] * cd22[par[i]]) * detM;

      /* M12 unequal to zero also implies M21 unequal to zero. In this
         case we have a rotation and the image dimensions are flipped */
      if (M12[par[i]] != 0) {
        naxis1n = naxis2;
        naxis2n = naxis1;
      } else {
        naxis1n = naxis1;
        naxis2n = naxis2;
      }

      /* calculate the new reference pixel */
      crpix1n = M11[par[i]] * crpix1[par[i]] +
                M12[par[i]] * crpix2[par[i]];
      crpix2n = M21[par[i]] * crpix1[par[i]] +
                M22[par[i]] * crpix2[par[i]];

      if (M11[par[i]] == -1 || M12[par[i]] == -1) {
        crpix1n = naxis1n + crpix1n;
      }

      if (M22[par[i]] == -1 || M21[par[i]] == -1) {
        crpix2n = naxis2n + crpix2n;
      }

      sprintf(cval, "%d", bitpix);
      qfits_header_add(fh, "BITPIX", cval, "Bits per pixel", NULL);
      qfits_header_add(fh, "NAXIS", "2", "File dimension", NULL);
      sprintf(cval, "%d", naxis1n);
      qfits_header_add(fh, "NAXIS1", cval, "Size in x", NULL);
      sprintf(cval, "%d", naxis2n);
      qfits_header_add(fh, "NAXIS2", cval, "Size in y", NULL);
      sprintf(cval, "%f", bscale);
      qfits_header_add(fh, "BSCALE", cval, "pixel scale factor", NULL);
      sprintf(cval, "%f", bzero);
      qfits_header_add(fh, "BZERO", cval, "pixel offset", NULL);
      sprintf(cval, "%s", "RA---TAN");
      qfits_header_add(fh, "CTYPE1", cval,
                       "WCS coordinate type", NULL);
      sprintf(cval, "%s", "DEC--TAN");
      qfits_header_add(fh, "CTYPE2", cval,
                       "WCS coordinate type", NULL);
      sprintf(cval, "%.2f", crpix1n);
      qfits_header_add(fh, "CRPIX1", cval,
                       "WCS Coordinate reference pixel", NULL);
      sprintf(cval, "%.2f", crpix2n);
      qfits_header_add(fh, "CRPIX2", cval,
                       "WCS Coordinate reference pixel", NULL);
      sprintf(cval, "%E", cd11n);
      qfits_header_add(fh, "CD1_1", cval,
                       "WCS Coordinate scale matrix", NULL);
      sprintf(cval, "%E", cd22n);
      qfits_header_add(fh, "CD2_2", cval,
                       "WCS Coordinate scale matrix", NULL);
      sprintf(cval, "%E", cd21n);
      qfits_header_add(fh, "CD2_1", cval,
                       "WCS Coordinate scale matrix", NULL);
      sprintf(cval, "%E", cd12n);
      qfits_header_add(fh, "CD1_2", cval,
                       "WCS Coordinate scale matrix", NULL);
      sprintf(cval, "%f", crval1);
      qfits_header_add(fh, "CRVAL1", cval,
                       "WCS Ref. value (RA in decimal degrees)", NULL);
      sprintf(cval, "%f", crval2);
      qfits_header_add(fh, "CRVAL2", cval,
                       "WCS Ref. value (DEC in decimal degrees)", NULL);
      sprintf(cval, "%s", "FK5");
      qfits_header_add(fh, "RADECSYS", cval,
                       "Coordinate system for equinox (FK4/FK5/GAPPT)", NULL);
      sprintf(cval, "%s", filter);
      qfits_header_add(fh, "FILTER", cval, "Name of filter", NULL);

      /* make sure that OBJECTS is ALWAYS a string; in case it
         initially is int, float, bool or complex we embrace it by
         double quotes */
      sprintf(cvaltmp, "%s", object);
      if (qfits_is_int(cvaltmp) ||
          qfits_is_float(cvaltmp) ||
          qfits_is_boolean(cvaltmp) ||
          qfits_is_complex(cvaltmp)) {
        sprintf(cval, "\"%s\"", cvaltmp);
      } else {
        strcpy(cval, cvaltmp);
      }
      qfits_header_add(fh, "OBJECT", cval,
                       "observed target", NULL);
      sprintf(cval, "%f", airmass);
      qfits_header_add(fh, "AIRMASS", cval,
                       "(average) Airmass during observation",
                       NULL);
      sprintf(cval, "%.2f", exptime);
      qfits_header_add(fh, "EXPTIME", cval,
                       "effective exposure time", NULL);
      sprintf(cval, "%.2f", equinox);
      qfits_header_add(fh, "EQUINOX", cval,
                       "Equinox of coordinates", NULL);
      sprintf(cval, "%d", imageid[i]);
      qfits_header_add(fh, "IMAGEID", cval, "Chip Number", NULL);
      sprintf(cval, "%d", gabodsid);
      qfits_header_add(fh, "GABODSID", cval,
                       "Obs. date in days since 31/12/1998",
                       NULL);
      sprintf(cval, "%f", -1.0);
      qfits_header_add(fh, "ZP", cval,
                       "photometric zeropoint", NULL);
      sprintf(cval, "%f", 1.0);
      qfits_header_add(fh, "COEFF", cval,
                       "photometric extiction coefficient", NULL);

      /* now additionally requested header items */
      /* firsth those headers for that a fixed value was provided */
      for (j = 0; j < nheaderkeys; j++) {
        /* first delete the requested key if it already exists.  We
           'trust' that the user does not destroy the header by not
           changing an essential key */
        e_comment(2,
                  "Adding user header key %s (value %s) to ext. [%d]",
                  headerkeys[j], headervalues[j], i + 1);
        qfits_header_del(fh, headerkeys[j]);
        qfits_header_add(fh, headerkeys[j], headervalues[j], headercomments[j],
                         NULL);
      }

      /* now those header keys that should be transfered from the
         original image extensions */
      for (j = 0; j < ntransferkeys; j++) {
        /* get keyword value and comment */
        transferkey = qfits_header_getstr(h_ext, transferkeys[j]);
        transfercomm = qfits_header_getcom(h_ext, transferkeys[j]);

        if (transferkey != NULL) {
          e_comment(2,
          "Adding original header key %s (value %s) with name %s to ext. [%d]",
          transferkeys[j], transferkey, transferkeynames[j] ,i + 1);

          qfits_header_del(fh, transferkeys[j]);

          if(transfercomm == NULL) {
            strcpy(transfercomm, "");
          }
          qfits_header_add(fh, transferkeynames[j],
                           transferkey, transfercomm, NULL);
        } else {
          e_warning("Header element %s not present in extension [%d]",
                    transferkeys[j], i + 1);
        }
      }

      /* finally, add 50 dummy keywords */
      for (j = 0; j < 50; j++) {
        sprintf(cval, "DUMMY%d", j + 1);
        qfits_header_add(fh, cval, "0",
                         "Dummy for adding new FITS cards later", NULL);
      }

      /* create new image */

      /* in case we only have the 'unity' transformation no remapping
         is required */
      if (M11[par[i]] == 1 && M22[par[i]] == 1) {
        new_image = fdata + data_beg;
      } else {
        new_image = (char *)malloc(data_size * sizeof(char));

        /* apply transformation matrix to input data and
           store them in new_image */
        e_comment(2,
              "Applying transformation M=(%d; %d; %d; %d) to extension [%d]",
                  M11[par[i]], M12[par[i]], M21[par[i]],
                  M22[par[i]], xts[i]);

        for (k = 0; k < naxis2; k++) {
          for (l = 0; l < naxis1; l++) {
            lp = M11[par[i]] * l + M12[par[i]] * k;
            kp = M22[par[i]] * k + M21[par[i]] * l;

            if (M11[par[i]] == -1 || M12[par[i]] == -1) {
              /* lp is negative in this case !! */
              lp = naxis1n - 1 + lp;
            }
            if (M22[par[i]] == -1 || M21[par[i]] == -1) {
              /* kp is negative in this case !! */
              kp = naxis2n - 1 + kp;
            }

            for (m = 0; m < bytes; m++) {
              new_image[bytes * (naxis1n * kp + lp) + m] =
                fdata[data_beg + (bytes * (naxis1 * k + l) + m)];
            }
          }
        }
      }
    }

    /* collect information about the splitting process in 'HISTORY'
       keywords */
    hs = history_new_header(h_ext);
    history_add(hs, "");
    history_add(hs, "mefsplit: %s", version);
    history_add(hs, "mefsplit: called at %s", get_datetime_iso8601());
    history_add(hs, "mefsplit: Split extension %d from %s",
                xts[i], get_basename(name_i));
    history_add(hs, "mefsplit: Save extension %d as chip %d",
                xts[i], imageid[i]);


    /* Save output image */
    if (onlymain == 1) {
      e_comment(1, "saving output image");
    }

    if (headerupdate != 0) {
      history_addfits(hs, fh);

      qfits_header_dump(fh, extension);
      e_comment(1, "saving extension [%d] as IMAGEID [%d]", xts[i], imageid[i]);
    } else {
      /* the saving has to be performed 'by hand' in case no header
         update is requested. The reason is that the image header that
         has to be conserved is 'complex' (main header and extension
         header) in this case and it cannot be handled with the
         standard routines. */
      qfits_header_dump(h_main, extension);
      history_addfits(hs, h_ext);
      qfits_header_dump(h_ext, extension);
      new_image = fdata + data_beg;
      e_comment(1, "copying extension [%d]", xts[i]);
    }

    /* Dump pixels from one file to the other */
    fwrite(new_image, 1, data_size, extension);

    /* Close file pointers */
    fclose(extension);

    /* zero-padding */
    qfits_zeropad(ext_name_o);

    /* free memory */
    if (onlymain == 0) {
      qfits_header_destroy(h_ext);
    }
    qfits_header_destroy(fh);
    history_del(hs);

    if ((headerupdate != 0) && (M11[par[i]] != 1 || M22[par[i]] != 1)) {
      free(new_image);
    }
  }

  free(fdata);
  qfits_header_destroy(h_main);
  free(xts);
  free(par);

  return(0);
}

/*
 * Loads a WFI list into a cube_t structure.
 */
cube_t *wfi_cube_load(char *filename, int xtnum)
{
  cube_t *   loaded;
  image_t *  wfi_frame;
  framelist *flist;
  int        i;
  int *      exts;
  int        err;
  int        single_frames;

  /* Check entries */
  if (filename == NULL) {
    return(NULL);
  }

  /* Load framelist */
  flist = framelist_load(filename);
  if (flist == NULL) {
    e_error("cannot load frame list [%s]", filename);
    return(NULL);
  }
  /* Get number of extensions contained in each frame */
  exts = malloc(flist->n * sizeof(int));
  for (i = 0; i < flist->n; i++) {
    exts[i] = qfits_query_n_ext(flist->name[i]);
  }

  /* Check consistency */
  err           = 0;
  single_frames = 1;
  for (i = 1; i < flist->n; i++) {
    /* Check that all files have the same number of extensions */
    if (exts[i] != exts[0]) {
      e_error("inconsistent input data set");
      err++;
    }
    /* Check that the requested extension (if any) is present */
    if (xtnum > 0) {
      if (xtnum > exts[i]) {
        e_error("inconsistent input data set");
        err++;
      }
      if (exts[i] > 0) {
        single_frames = 0;
      }
    }
  }
  /* Process errors */
  if (err) {
    e_error("%d errors occurred during reading", err);
    for (i = 0; i < flist->n; i++) {
      e_error("frame [%s] has %d extensions", flist->name[i], exts[i]);
    }
    framelist_del(flist);
    free(exts);
    return(NULL);
  }
  free(exts);


  /* A list of single frames */
  err = 0;
  if (single_frames) {
    loaded = cube_load_strings(flist->name, flist->n);
    if (loaded == NULL) {
      e_error("loading framelist [%s]", filename);
      err++;
    }
  } else {
    /* A list of whole WFI frames */
    /* Load first image */
    wfi_frame = wfi_load_ext(flist->name[i], xtnum);
    if (wfi_frame == NULL) {
      e_error("cannot load frame [%s][%d]", flist->name[i], xtnum);
      err++;
    }
    loaded           = cube_new(wfi_frame->lx, wfi_frame->ly, flist->n);
    loaded->plane[0] = wfi_frame;
    for (i = 1; i < flist->n; i++) {
      wfi_frame = wfi_load_ext(flist->name[i], xtnum);
      if (wfi_frame == NULL) {
        e_error("cannot load frame [%s][%d]", flist->name[i], xtnum);
        err++;
      }
      loaded->plane[i] = wfi_frame;
    }
  }
  /* Process errors */
  if (err) {
    e_error("an error occurred during loading: aborting");
    if (loaded != NULL) {
      cube_del(loaded);
    }
    loaded = NULL;
  }
  framelist_del(flist);
  return(loaded);
}

image_t *wfi_load_ext(char *filename, int xtnum)
{
  qfitsloader ql;
  image_t *   ext_load;

  ql.filename = filename;
  ql.xtnum    = xtnum;
  ql.pnum     = 1;
  ql.map      = 1;
#ifdef DOUBLEPIX
  ql.ptype = PTYPE_DOUBLE;
#else
  ql.ptype = PTYPE_FLOAT;
#endif

  if (qfitsloader_init(&ql) != 0) {
    return(NULL);
  }

  ext_load     = malloc(sizeof(image_t));
  ext_load->lx = ql.lx;
  ext_load->ly = ql.ly;
#ifdef DOUBLEPIX
  ext_load->data = (pixelvalue *)ql.dbuf;
#else
  ext_load->data = (pixelvalue *)ql.fbuf;
#endif

  return(ext_load);
}

int wfi_is_extension(char *filename)
{
  return(qfits_query_n_ext(filename));
}

image_t *wfi_overscan_correction(
  image_t *wfi_frame,
  int *prescan_x,
  int *overscan_x,
  int *rej_int,
  int *crop_reg
  )
{
  image_t *   cropped_frame;
  pixelvalue  filtered;
  pixelvalue *bias_lin;
  int         scan_width;
  int         i, j, k;
  pixelvalue *pix;

  /* Sanity tests with input parameters */
  scan_width = (prescan_x[1] - prescan_x[0] + 1) +
               (overscan_x[1] - overscan_x[0] + 1);

  if ((rej_int[0] + rej_int[1]) >= scan_width) {
    e_error("in rejection parameters: rejecting too many pixels");
    return(NULL);
  }

  if ((crop_reg[0] >= crop_reg[1]) || (crop_reg[2] >= crop_reg[3])) {
    e_error("in crop region definition");
    return(NULL);
  }


  /* Bring coordinates back to C convention */
  prescan_x[0]--; prescan_x[1]--; overscan_x[0]--; overscan_x[1]--;

  /* Make out bias column */
  bias_lin = malloc(scan_width * sizeof(pixelvalue));

  /* Loop on all lines */
  for (j = 0; j < wfi_frame->ly; j++) {
    pix = wfi_frame->data + j * wfi_frame->lx;
    k   = 0;
    for (i = prescan_x[0]; i <= prescan_x[1]; i++) {
      bias_lin[k++] = pix[i];
    }
    for (i = overscan_x[0]; i <= overscan_x[1]; i++) {
      bias_lin[k++] = pix[i];
    }
    filtered =
      function1d_average_reject(bias_lin,
                                scan_width,
                                rej_int[0],
                                rej_int[1]);
    for (i = 0; i < wfi_frame->lx; i++) {
      pix[i] -= filtered;
    }
  }
  free(bias_lin);

  /* Bring coordinates back to FITS convention */
  prescan_x[0]++; prescan_x[1]++; overscan_x[0]++; overscan_x[1]++;

  /* Extract cropped region */
  cropped_frame = image_getvig(wfi_frame,
                               crop_reg[0],
                               crop_reg[2],
                               crop_reg[1],
                               crop_reg[3]);


  return(cropped_frame);
}

image_t *wfi_myoverscan_correction(
  image_t *wfi_frame,
  int *overscan_x,
  int *rej_int
  )
{
  pixelvalue  filtered;
  pixelvalue *bias_lin;
  int         scan_width;
  int         i, j, k;
  pixelvalue *pix;

  /* Sanity tests with input parameters */
  scan_width = (overscan_x[1] - overscan_x[0] + 1);

  if ((scan_width <= 0) || (overscan_x[0] > wfi_frame->lx) ||
      (overscan_x[1] > wfi_frame->lx)) {
    e_error("ill defined overscan region");
    return(NULL);
  }

  if ((rej_int[0] + rej_int[1]) >= scan_width) {
    e_error("in rejection parameters: rejecting too many pixels");
    return(NULL);
  }

  /* Bring coordinates back to C convention */
  overscan_x[0]--; overscan_x[1]--;

  /* Make out bias column */
  bias_lin = malloc(scan_width * sizeof(pixelvalue));

  /* Loop on all lines */
  for (j = 0; j < wfi_frame->ly; j++) {
    pix = wfi_frame->data + j * wfi_frame->lx;
    k   = 0;
    for (i = overscan_x[0]; i <= overscan_x[1]; i++) {
      bias_lin[k++] = pix[i];
    }

    filtered =
      function1d_average_reject(bias_lin,
                                scan_width,
                                rej_int[0],
                                rej_int[1]);

    for (i = 0; i < wfi_frame->lx; i++) {
      pix[i] -= filtered;
    }
  }
  free(bias_lin);

  /* Bring coordinates back to FITS convention */
  overscan_x[0]++; overscan_x[1]++;

  return(wfi_frame);
}

/* There are 4 quadrants in a frame */
#define WFI_NQUAD    4

int wfi_gradient_check(
  image_t *wfi_frame,
  double max_grad_level
  )
{
  double avg_frame;
  double avg_quad[WFI_NQUAD];
  int    i;
  int    gradient_ok;

  /* Compute average for the whole frame */
  avg_frame = image_getmean(wfi_frame);
  if (fabs(avg_frame) < 1e-6) {
    e_warning("frame has zero average: cannot compute gradient check");
    return(1);
  }

  /* Compute average value inside each quadrant */
  /* Lower left quadrant */
  avg_quad[0] = image_getmean_vig(
    wfi_frame,
    1,
    wfi_frame->lx / 2,
    1,
    wfi_frame->ly / 2);
  /* Lower right quadrant */
  avg_quad[1] = image_getmean_vig(
    wfi_frame,
    1 + wfi_frame->lx / 2,
    wfi_frame->lx,
    1,
    wfi_frame->ly / 2);
  /* Upper left quadrant */
  avg_quad[2] = image_getmean_vig(
    wfi_frame,
    1,
    wfi_frame->lx / 2,
    1 + wfi_frame->ly / 2,
    wfi_frame->ly);
  /* Upper right quadrant */
  avg_quad[3] = image_getmean_vig(
    wfi_frame,
    1 + wfi_frame->lx / 2,
    wfi_frame->lx,
    1 + wfi_frame->ly / 2,
    wfi_frame->ly);

  e_comment(1, "average/quadrants: [%g] %4.2f %4.2f %4.2f %4.2f",
            avg_frame,
            avg_quad[0],
            avg_quad[1],
            avg_quad[2],
            avg_quad[3]);

  /* Compute ratio */
  gradient_ok = 1;
  for (i = 0; i < WFI_NQUAD; i++) {
    avg_quad[i] = (avg_frame - avg_quad[i]) / avg_frame;
    if (avg_quad[i] > max_grad_level) {
      e_warning("quadrant ratio above limit (%g)", max_grad_level);
      gradient_ok = 0;
    }
  }

  return(gradient_ok);
}
