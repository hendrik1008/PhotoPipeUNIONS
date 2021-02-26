/*---------------------------------------------------------------------------

   File name   : swarpfilter.c
   Author      : M. Schirmer & T. Erben
   Created on  : April 2010
   Description : Outlier rejection in swarp resampled images

*--------------------------------------------------------------------------*/

/*
        $Id: swarpfilter.c,v 1.7 2018/03/28 12:06:23 thomas Exp $
        $Author: thomas $
        $Date: 2018/03/28 12:06:23 $
        $Revision: 1.7 $
 */

/*---------------------------------------------------------------------------
                              History comments
   ---------------------------------------------------------------------------*/

/* 18.05.2010:
   I simplified the qfits adminitsration of involved input files.
   Previously qfits objects (qfitsloader) for all science and weight
   files were created and destroyed multiple times. Now this happens
   only once.

   19.05.2010:
   I made the reading of the coadded sections (fuction get_coaddblock)
   more efficient.

   31.08.2010:
   The maximum clustersize for the number of pixels around a position
   which need to be bad so that pixels are flagged, goes from 5 to 9.
   We now consider 'all' pixels around a certain position and not only
   the four direct horizontal and vertical neighbours.

   27.11.2010:
   I disabled automatic setting of processing blocksize. The implemented
   procedure does not compile under MAC/Darwin!

   21.01.2014:
   The variable FILEMAX is now 'defined' and no longer declared as integer.
   The latter led to uncompilable code on MAC OS10.9. I think that Mischas
   original code was not fully C-standard compliant.
*/

/*---------------------------------------------------------------------------
                                  Includes
   ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "qfits.h"

#define FILEMAX  4096

/*---------------------------------------------------------------------------
                              Global variables
   ---------------------------------------------------------------------------*/

qfitsloader *ql;
qfitsloader *qw;
qfitsdumper qd;

long **badpixindex, nthreads;
long *maxbadpix;

/*---------------------------------------------------------------------------
                             Function declarations
   ---------------------------------------------------------------------------*/

int get_coaddblock(long blocksize,
                   long naxis1, long naxis2, float fluxscale, float sky,
                   long xoffset, long yoffset, float **, char **, float **,
                   long coadd_naxis1, long block, long currentimage);

void identify_bad_pixels(float **data, char **weight, float **sky, long dim,
                         float kappa, long currentpixel, long l,
                         long *running_bpi, int *presentimages,
                         int npresentimages, int last);

void float_qsort_ind_nonrec(float *a, int *idx, int n);

void usage(int i, char *argv[])
{
  if (i == 0) {
    fprintf(stderr, "\n");
    fprintf(stderr, "          Version 2.7  (2010-04-25)\n\n");
    fprintf(stderr, "  Author: Mischa Schirmer\n\n");
    fprintf(stderr, "  USAGE:  %s \n", argv[0]);
    fprintf(stderr, "           -i input_list \n");
    fprintf(stderr, "           [ -k kappa-sigma-threshold (4.0)\n");
    fprintf(stderr, "           [ -m minimum number of bad pixels in a cluster (max: 5)]\n");
    fprintf(stderr, "           [ -e border width]\n");
    fprintf(stderr, "           [ -n number of lines read in one chunk\n");
    fprintf(stderr, "  PURPOSE: Detects outliers in a set of resampled \n");
    fprintf(stderr, "     images. Corresponding pixels in the weight maps \n");
    fprintf(stderr, "     are set to zero.\n");
    fprintf(stderr, "     If the -m option is given, then bad pixels are only masked\n");
    fprintf(stderr, "     if AT LEAST this many bad pixels are grouped together.\n");
    fprintf(stderr, "     If the value is larger than 5, it is reset to 5.\n");
    fprintf(stderr, "     If -e 1, 3x3 pixels centred on the bad pixel will be masked.\n");
    fprintf(stderr, "     If -e 2, 5x5 pixels centred on the bad pixel will be masked, etc ...\n");
    fprintf(stderr, "     The programme determines its optimal memory usage automatically.\n");
    fprintf(stderr, "     This can be overridden by specifying the number of lines that\n");
    fprintf(stderr, "     should be processed in one go (suggest: 100 to be on the safe side).\n\n");
    exit(1);
  }
}

/*---------------------------------------------------------------------------
                                  Main
   ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  FILE           *input;
  long           i, j, k, l, n, m, s, t, o;
  long           masklinemin, masklinemax;
  long           maskcolmin, maskcolmax;
  long           num_images, currentpixel, maskwidth;
  long           blocksize, block, chunksize, coadd_naxis1, coadd_naxis2;
  /* long           pages; CURRENTLY DISABLED FEATURE */
  long           *running_bpi;
  int            clustersize, clustercount;
  /* int            pagesize; CURRENTLY DISABLED FEATURE */
  int            getcoaddcount;
  char           *weight_out, *weight_cpy;
  int            imagepresent;
  float          **block_img_comp, **block_sky_comp;
  char           **block_wgt_comp;
  float          xoffset, yoffset, kappa;
  float          coadd_crpix1, coadd_crpix2;
  char           input_file[FILEMAX];
  qfits_header   *header;
  /* float          blockfr; CURRENTLY DISABLED FEATURE */
  int            *presentimages;
  int            npresentimages;
  int            last;
  register float *float_pointer;
  register char  *char_pointer;
  FILE           *new_weight;
  char           newweight[FILEMAX];


  kappa       = 4.0;
  blocksize   = -1;
  clustersize = 1;
  maskwidth   = 0;

  /* get some system info (number of online CPUs and available memory) */
  /* DISABLED because it does not compile under Darwin
  nthreads = sysconf(_SC_NPROCESSORS_ONLN);
  pages    = sysconf(_SC_PHYS_PAGES);
  pagesize = sysconf(_SC_PAGESIZE);
  */

  /* print usage if no arguments were given */
  if (argc == 1) {
    usage(0, argv);
  }

  /* read the arguments */
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (tolower((int)argv[i][1])) {
      case 'i': strcpy(input_file, argv[++i]);
        break;
      case 'k': kappa = atof(argv[++i]);
        break;
      case 'n': blocksize = atol(argv[++i]);
        break;
      case 'm': clustersize = atoi(argv[++i]);
        break;
      case 'e': maskwidth = atol(argv[++i]);
        break;
      }
    }
  }

  /* upper limit for the cluster size */
  if (clustersize > 9) {
    printf("clustersize is %d; I reset to 9!", clustersize);
    clustersize = 9;
  }

  /* read the file list */
  input = fopen(input_file, "r");
  if (fscanf(input, "%ld %ld %ld %f %f",
             &num_images, &coadd_naxis1, &coadd_naxis2,
             &coadd_crpix1, &coadd_crpix2) == 0) {
    printf("Error: Could not read from %s, or file empty!\n", input_file);
    exit(1);
  }

  nthreads = 1;  /* no multithreading implemented yet */
  if (blocksize == -1) {
    /* maxmimum memory used: 50% of the available RAM */
    /* DISABLED because it does not compile under Darwin:

    blocksize = 0.5 * pages * pagesize / coadd_naxis1 /
                      num_images / sizeof(float) / 3 / nthreads;
    blockfr = 0.5 * pages * pagesize /
              (coadd_naxis1 * coadd_naxis2 * sizeof(float) * 3 * 12);
    blocksize = (long)(blockfr * coadd_naxis2);

    if (blocksize < 1) {
      printf("ERROR: NOT ENOUGH MEMORY AVAILABLE FOR SWARPFILTER!\n");
      exit(1);
    }
    if (blocksize < 10) {
      printf("WARNING: You seem to run low on memory! Swarpfilter will run fine,\n");
      printf("but next time consider stopping other memory consuming applications.\n");
    }
    printf("Best memory usage: BLOCKSIZE = %ld\n", blocksize);
    */
    printf("Blocksize '-n' must be provided on the command line!\n");
    exit(1);
  } else {
    printf("Manually provided blocksize: %ld\n", blocksize);
  }

  /* number of pixels to be analysed in one step */
  chunksize = coadd_naxis1 * blocksize;

  struct image {
    char  image[FILEMAX];
    char  weight[FILEMAX];
    long  naxis1;
    long  naxis2;
    float crpix1;
    float crpix2;
    float fluxscale;
    float sky;
  } data[num_images];

  ql = (qfitsloader *)malloc(num_images * sizeof(qfitsloader));
  qw = (qfitsloader *)malloc(num_images * sizeof(qfitsloader));

  /* read the file names and image geometries etc */
  for (i = 0; i < num_images; i++) {
    if (fscanf(input, "%s %s %ld %ld %f %f %f",
               &data[i].image[0],
               &data[i].weight[0],
               &data[i].naxis1, &data[i].naxis2,
               &data[i].crpix1, &data[i].crpix2,
               &data[i].fluxscale) == 0) {
      printf("Error: Could not read from %s, or file empty!\n", input_file);
      exit(1);
    } else {
      ql[i].filename = data[i].image;
      ql[i].xtnum    = 0;
      ql[i].pnum     = 0;
      ql[i].map      = 0;
      ql[i].ptype    = PTYPE_FLOAT;
      qfitsloader_init(&(ql[i]));

      qw[i].filename = data[i].weight;
      qw[i].xtnum    = 0;
      qw[i].pnum     = 0;
      qw[i].map      = 0;
      qw[i].ptype    = PTYPE_FLOAT;
      qfitsloader_init(&(qw[i]));

      if (qfits_query_hdr(data[i].image, "SKYVALUE") == NULL) {
        data[i].sky = 0.0;
      } else {
        data[i].sky = atof(qfits_query_hdr(data[i].image, "SKYVALUE"));
      }
    }
  }
  fclose(input);

  /* allocate some memory */
  presentimages = (int *)calloc(num_images, sizeof(int));
  running_bpi = (long *)calloc(num_images, sizeof(long));
  badpixindex = (long **)calloc(num_images, sizeof(long *));
  maxbadpix = (long *)malloc(num_images * sizeof(long));
  for (i = 0; i < num_images; i++) {
    maxbadpix[i] = 10000;
    badpixindex[i] = (long *)calloc(maxbadpix[i], sizeof(long));
  }

  /* loop over all lines in coadd.fits */
  block_img_comp = (float **)calloc(num_images, sizeof(float *));
  block_wgt_comp = (char **)calloc(num_images, sizeof(char *));
  block_sky_comp = (float **)calloc(num_images, sizeof(float *));
  for (i = 0; i < num_images; i++) {
    block_img_comp[i] = (float *)calloc(chunksize, sizeof(float));
    block_wgt_comp[i] = (char *)calloc(chunksize, sizeof(char));
    block_sky_comp[i] = (float *)calloc(chunksize, sizeof(float));
  }

  last = 0;
  for (block = 0; block < (long)(coadd_naxis2 / blocksize + 1.); block++) {
    printf("processing BLOCK %ld / %ld\n",
           block, (long)(coadd_naxis2 / blocksize + 1.) - 1);

    /* loop over all images */
    getcoaddcount = 0;
    npresentimages = 0;

    printf("reading images for block %ld\n", block);
    for (i = 0; i < num_images; i++) {
      xoffset = coadd_crpix1 - data[i].crpix1;
      yoffset = coadd_crpix2 - data[i].crpix2;

      /* read a chunk of data */
      imagepresent =
        get_coaddblock(blocksize, data[i].naxis1, data[i].naxis2,
                       data[i].fluxscale, data[i].sky,
                       xoffset, yoffset,
                       block_img_comp, block_wgt_comp, block_sky_comp,
                       coadd_naxis1, block, i);

      if (imagepresent == 1) {
        getcoaddcount++;
        presentimages[npresentimages] = i;
        npresentimages++;
      }
    }

    printf("There are %d images in block %ld\n", npresentimages, block);

    /* if no data was obtained, continue */
    if (getcoaddcount == 0) {
      continue;
    }

    /* find the bad pixels */
    printf("identify_bad_pixels for block %ld\n", block);
    for (l = 0; l < chunksize; l++) {
      currentpixel = l + block * chunksize;
      if (block == ((long)(coadd_naxis2 / blocksize + 1.) - 1) &&
          l == (chunksize - 1)) {
        last = 1;
      }
      identify_bad_pixels(block_img_comp, block_wgt_comp, block_sky_comp,
                          num_images, kappa, currentpixel, l, running_bpi,
                          presentimages, npresentimages, last);
    }

    for (i = 0; i < npresentimages; i++) {
      memset(block_img_comp[presentimages[i]], 0, chunksize * sizeof(float));
      memset(block_wgt_comp[presentimages[i]], 0, chunksize * sizeof(char));
      memset(block_sky_comp[presentimages[i]], 0, chunksize * sizeof(float));
    }
  }

  for (i = 0; i < num_images; i++) {
    free(block_img_comp[i]);
    free(block_wgt_comp[i]);
    free(block_sky_comp[i]);
  }
  free(block_img_comp);
  free(block_wgt_comp);
  free(block_sky_comp);

  /* free some memory */
  free(running_bpi);

  printf("Writing results ...\n");

  for (i = 0; i < num_images; i++) {
    s          = 0;
    t          = 0;
    n          = data[i].naxis1;
    m          = data[i].naxis2;
    xoffset    = coadd_crpix1 - data[i].crpix1;
    yoffset    = coadd_crpix2 - data[i].crpix2;
    weight_out = (char *)calloc(n * m, sizeof(char));
    weight_cpy = (char *)calloc(n * m, sizeof(char));

    /* mask all bad pixels found */
    for (j = yoffset; j < (yoffset + m); j++) {
      for (l = xoffset; l < (xoffset + n); l++) {
        k  = l + coadd_naxis1 * j;
        weight_out[s] = 1;
        weight_cpy[s] = 1;
        if (k == badpixindex[i][t]) {
          weight_out[s] = 0;
          if (clustersize < 2) {
            weight_cpy[s] = 0;
          }
          t++;
        }
        s++;
      }
    }

    /* mask only clusters consisting of at least 'clustersize' pixels */
    if (clustersize > 1) {
      /* we ignore the 1 pixel wide border of the image */
      for (j = 1; j < m - 1; j++) {
        for (k = 1; k < n - 1; k++) {
          if (weight_out[k + n * j] == 0) {
            clustercount = 1;
            for (l = j - 1; l <= j + 1; l++) {
              for (o = k - 1; o <= k + 1; o++) {
                clustercount += (weight_out[o + (l * n)] ^ 1);
              }
            }
            if (clustercount > clustersize) {
              for (l = j - 1; l <= j + 1; l++) {
                for (o = k - 1; o <= k + 1; o++) {
                  weight_cpy[o + (l * n)] = weight_out[o + (l * n)];
                }
              }
            }
          }
        }
      }
      memcpy(weight_out, weight_cpy, n * m * sizeof(char));
    }

    /* if a border of width 'width' pixels should be masked around a
       bad pixel, too
    */
    if (maskwidth > 0) {
      for (j = 0; j < m; j++) {
        for (k = 0; k < n; k++) {
          if (weight_cpy[k + n * j] == 0) {
            masklinemin =
              (j - maskwidth) < 0 ? 0 : (j - maskwidth);
            masklinemax =
              (j + maskwidth) > (m - 1) ? (m - 1) : (j + maskwidth);
            maskcolmin =
              (k - maskwidth) < 0 ? 0 : (k - maskwidth);
            maskcolmax =
              (k + maskwidth) > (n - 1) ? (n - 1) : (k + maskwidth);

            for (l = masklinemin; l <= masklinemax; l++) {
              for (o = maskcolmin; o <= maskcolmax; o++) {
                weight_out[o + (l * n)] = 0;
              }
            }
          }
        }
      }
    }

    free(weight_cpy);

    /* write the new weight image */
    /* read the FITS header and the data block */
    header      = qfits_header_read(data[i].weight);

    if (qfits_loadpix(&(qw[i])) == 0) {
      float_pointer = qw[i].fbuf;
      char_pointer = weight_out;
      for (j = 0; j < (m * n); j++) {
        *(float_pointer++) *= (float)(*(char_pointer++));
      }
      free(weight_out);

      qd.filename  = newweight;
      qd.npix      = n * m;
      qd.ptype     = PTYPE_FLOAT;
      qd.out_ptype = -32;
      qd.fbuf      = qw[i].fbuf;

      strcpy(newweight, data[i].weight);
      new_weight = fopen(newweight, "w");
      qfits_header_dump(header, new_weight);
      fclose(new_weight);

      if (qfits_pixdump(&qd) != 0) {
        printf("ERROR while dumping filtered weight!\n");
        exit(1);
      }
      qfits_zeropad(newweight);
      qfits_header_destroy(header);
      free(qd.fbuf);
    } else {
      printf("ERROR in main(): could not read %s !\n",
             ql[i].filename);
      exit(1);
    }
  }

  /* Free the memory */
  for (i = 0; i < num_images; i++) {
    free(badpixindex[i]);
  }
  free(badpixindex);
  free(maxbadpix);
  free(presentimages);
  free(ql);
  free(qw);

  exit(0);
}

/* ***************************************************************** */

void identify_bad_pixels(float **data, char **weight, float **sky,
                         long dim, float kappa, long currentpixel,
                         long l, long *running_bpi, int *presentimages,
                         int npresentimages, int last)
{
  long  i;
  float mean, rms, thresh;
  static float *gooddata = NULL;
  static float *goodsky = NULL;
  static int *gooddataind = NULL;
  int ngoodweight;

  if (gooddata == NULL) {
    gooddata = (float *)malloc(dim * sizeof(float));
    goodsky = (float *)malloc(dim * sizeof(float));
    gooddataind = (int *)malloc(dim * sizeof(int));
  }

  /* get the number of non-zero-weight pixels in the stack */

  ngoodweight = 0;
  for (i = 0; i < npresentimages; i++) {
    if (weight[presentimages[i]][l] > 0) {
      gooddata[presentimages[i]] = data[presentimages[i]][l];
      goodsky[presentimages[i]] = sky[presentimages[i]][l];
      gooddataind[ngoodweight] = presentimages[i];
      ngoodweight++;
    }
  }

  if (ngoodweight < 2) {
    return;
  }

  float_qsort_ind_nonrec(gooddata, gooddataind, ngoodweight);

  /* if 2 pixels are in the stack, use poisson rms */
  if (ngoodweight == 2) {
    mean = 0.5 * (gooddata[gooddataind[0]] + gooddata[gooddataind[1]]);
    rms  = (goodsky[gooddataind[0]] + goodsky[gooddataind[1]]);

    if (rms > 0.) {
      rms = sqrt(0.5 * rms);
    } else {
      return;
    }
    thresh = rms;
  }
  /* if 3 pixels are in the stack, reject max pixel */
  else if (ngoodweight == 3) {
    mean = 0.;
    rms  = 0.;
    for (i = 0; i < (ngoodweight - 1); i++) {
        mean += gooddata[gooddataind[i]];
        rms += gooddata[gooddataind[i]] * gooddata[gooddataind[i]];
    }
    if (rms > 0) {
      mean /= (float)(ngoodweight - 1);
      rms = sqrt(((ngoodweight - 1) / (ngoodweight - 2)) *
                 (rms / (ngoodweight - 1) - mean * mean));
    } else {
      return;
    }
    thresh = 6. * kappa / ngoodweight * rms;
  } else {
    /* if 4 or more pixels are in the stack, reject min and max pixel */
    mean = 0.;
    rms  = 0.;
    for (i = 1; i < (ngoodweight - 1); i++) {
      mean += gooddata[gooddataind[i]];
      rms += gooddata[gooddataind[i]] * gooddata[gooddataind[i]];
    }
    if (rms > 0) {
      mean /= (float)(ngoodweight - 2);
      rms = sqrt(((ngoodweight - 2) / (ngoodweight - 3)) *
                 (rms / (ngoodweight - 2) - mean * mean));
    } else {
      return;
    }

    if (ngoodweight < 6) {
      thresh = 6. * kappa / ngoodweight * rms;
    } else {
      thresh = kappa * rms;
    }
  }

  /* running_bpi = "running_badpixindex" */
  for (i = 0; i < ngoodweight; i++) {
    if (fabs(gooddata[gooddataind[i]] - mean) > thresh) {
      if (ngoodweight > 2) {
        badpixindex[gooddataind[i]][running_bpi[gooddataind[i]]] =
          currentpixel;
        running_bpi[gooddataind[i]]++;
      } else {
        /* reject pixel only if it is the brighter one */
        if (i > 0) {
          badpixindex[gooddataind[i]][running_bpi[gooddataind[i]]] =
            currentpixel;
          running_bpi[gooddataind[i]]++;
        }
      }

      /* dynamic memory reallocation */
      if (running_bpi[gooddataind[i]] >= maxbadpix[gooddataind[i]]) {
        maxbadpix[gooddataind[i]] *= 2;
        badpixindex[gooddataind[i]] =
          (long *)realloc(badpixindex[gooddataind[i]],
                          maxbadpix[gooddataind[i]] * sizeof(long));
      }
    }
  }

  if (last == 1) {
    free(gooddata);
    gooddata = NULL;
    free(gooddataind);
    gooddataind = NULL;
  }
}

/* ***************************************************************** */

int get_coaddblock(long blocksize,
                   long naxis1, long naxis2, float fluxscale, float sky,
                   long xoffset, long yoffset, float **block_img_coadd,
                   char **block_wgt_coadd, float **block_sky_coadd,
                   long coadd_naxis1, long block, long currentimage)
{
  long   i, j, k, l;
  long   maxline, minline;
  long   firstline2read, lastline2read; /* we start counting at 0! */
  long   bbs0, bbs1, numpix2read, numlines2read;

  numpix2read = naxis1 * blocksize;

  bbs0 = block * blocksize;
  bbs1 = bbs0 + blocksize;

  /* xoffset: number of pixels between the left border of
              coadd.fits and the image
     yoffset: number of pixels between the lower border of
              coadd.fits and the image */
  /* 0 <= xoffset <= coadd_naxis1 - naxis1 */
  /* 0 <= yoffset <= coadd_naxis2 - naxis2 */

  /* if the image is entirely below or above the
     current coadd block, then there is nothing to do:
  */
  if (yoffset + naxis2 <= bbs0 || yoffset >= bbs1) {
    return(0);
  }

  /* the rest of this function then only applies to images that overlap
     with the current coadd block
  */

  /* initialise the image and weight blocks to zero;
     these arrays can carry the entire block.
     we will only fill part of them.
  */

  /* assume that the image covers the block entirely: */
  firstline2read = bbs0 - yoffset;
  lastline2read  = firstline2read + blocksize - 1;

  /* if the image does not cover the lower part of the block
     but the upper part entirely
  */
  if (firstline2read < 0 && yoffset + naxis2 >= bbs1) {
    lastline2read  = bbs1 - yoffset - 1;
    firstline2read = 0;
  }

  /* if the image does not cover the upper part of the block
     but the lower part entirely
  */
  if (yoffset + naxis2 < bbs1 && yoffset <= bbs0) {
    lastline2read  = naxis2 - 1;
    firstline2read = bbs0 - yoffset;
  }

  /* if the image is entirely contained in the block
     without touching its borders
  */
  if (yoffset > bbs0 && yoffset + naxis2 < bbs1) {
    firstline2read = 0;
    lastline2read  = naxis2 - 1;
  }

  numlines2read = lastline2read - firstline2read + 1;
  numpix2read   = naxis1 * numlines2read;

  /*printf("reading science image %s\n", image);*/
  /* read the corresponding part of the image */
  if (qfits_loadpix_window(&(ql[currentimage]), 1, firstline2read + 1,
                           naxis1, lastline2read + 1) != 0) {
    printf("ERROR in get_coaddblock(): could not read %s !\n",
           ql[currentimage].filename);
    exit(1);
  }

  /*printf("reading weight image %s\n", weight);*/
  /* read corresponding part of the weight */
  if (qfits_loadpix_window(&(qw[currentimage]), 1, firstline2read + 1,
                           naxis1, lastline2read + 1) != 0) {
    printf("ERROR in get_coaddblock(): could not read %s !\n",
           qw[currentimage].filename);
    exit(1);
  }

  /* map the image block onto the coadded image (block_img_coadd) */
  l = 0;  /* the running index of the image */
  k = 0;  /* the running index of the coadd */

  minline = (yoffset - bbs0) < 0 ? 0 : (yoffset - bbs0);
  maxline = (yoffset - bbs0 + naxis2) > blocksize ?
    blocksize : (yoffset - bbs0 + naxis2);
  k = (minline * coadd_naxis1 + xoffset);

  for (j = minline; j < maxline; j++) {
    for (i = xoffset; i < (xoffset + naxis1); i++) {
      /* if the image overlaps with the coadd block */
      /* (if not then these arrays are initialised to zero anyway) */
      block_img_coadd[currentimage][k] =
        ql[currentimage].fbuf[l] * fluxscale;
      block_wgt_coadd[currentimage][k] =
        (qw[currentimage].fbuf[l] > 0.) ? 1 : 0;
      block_sky_coadd[currentimage][k] =
        (ql[currentimage].fbuf[l] + sky) * fluxscale;
      l++;
      k++;
    }
    k += (coadd_naxis1 - naxis1);
  }

  free(ql[currentimage].fbuf);
  free(qw[currentimage].fbuf);

  return(1);
}

#define M 10 /* subarrays smaller than this number will be processed
                with insertion sort */
#define STACKSIZE 100 /* The recursion of quicksort is implemented
                         manually with an internal stack of maximum
                         size 'STACKSIZE'. On average, arrays with size
                         10**(STACKSIZE) can be processed */

void float_qsort_ind_nonrec(float *a, int *idx, int n)
{
  int i, j, m, r, l, s = 0;
  int tmp;
  int stack[STACKSIZE]; /* internal stack that substitutes the
                           recursions in an usual quicksort
                           implementaion */

  /*
  for(i = 0; i < n ; i++) {
    idx[i] = i;
  }
  */

  /* first put the whole array on the stack. The left element
     is put first. With a pushdown stack we then have to retrieve
     the right one first */

  stack[s++] = 0;
  stack[s++] = n - 1;

  /* go on until the stack is empty */
  while(s > 0) {
    r = stack[--s];
    l = stack[--s];
    if (r - l > M) {
      /* quicksort with a median of three strategy
         to choose the pivot element
      */
      i = l - 1; j = r;
      m = l + (r - l) / 2;
      if (a[idx[l]] > a[idx[m]]) {
        tmp = idx[l];
        idx[l] = idx[m];
        idx[m] = tmp;
      }
      if (a[idx[l]] > a[idx[r]]){
        tmp = idx[l];
        idx[l] = idx[r];
        idx[r] = tmp;
      }
      else if (a[idx[r]]>a[idx[m]]){
        tmp = idx[r];
        idx[r] = idx[m];
        idx[m] = tmp;
      }

      for (;;) {
        while (a[idx[++i]] < a[idx[r]]);
        while (a[idx[--j]] > a[idx[r]]);
        if (i >= j) {
          break;
        }
        tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
      }
      tmp = idx[i];
      idx[i] = idx[r];
      idx[r] = tmp;

      if(s > (STACKSIZE - 2)) {
        fprintf(stderr,
                "stacksize too small in quicksort; aborting!!");
        exit(1);
      }

      /* the larger subarray is processed immediately, the
         smaller one later. This minimses the needed stack
         elements */
      if ((i - l) > (r - i)) {
        stack[s++] = l;
        stack[s++] = i - 1;
        stack[s++] = i + 1;
        stack[s++] = r;
      } else {
        stack[s++] = i + 1;
        stack[s++] = r;
        stack[s++] = l;
        stack[s++] = i - 1;
      }
    } else {
      /* insertion sort */
      for (i = l + 1; i <= r; ++i) {
        tmp = idx[i];
        for(j = i - 1; j >= l && a[tmp] < a[idx[j]]; --j) {
          idx[j + 1] = idx[j];
        }
        idx[j + 1] = tmp;
      }
    }
  }
}
#undef M
#undef STACKSIZE
