/*----------------------------------------------------------------------------

   File name    :   fixcr.c
   Author       :   Tim Schrabback
   Created on   :   2009-06-20
   Description  :   Undoes CR masking around stars

   ---------------------------------------------------------------------------*/

/*
        $Id: fixcr.c,v 1.3 2018/03/28 12:06:22 thomas Exp $
        $Author: thomas $
        $Date: 2018/03/28 12:06:22 $
        $Revision: 1.3 $
 */

/*---------------------------------------------------------------------------
   			 History comments (Thomas Erben)
 ---------------------------------------------------------------------------*/

/*
  16.07.2009:
  - I add history comments to the resulting image headers.
  - Output images are saved with the same BITPIX values as come
    from the input frames.
*/

/*----------------------------------------------------------------------------
                                Includes
   ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "global_vers.h"
#include "eclipse.h"

/*----------------------------------------------------------------------------
                                Main program
   ---------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  image_t      *wht_input_image, *wht_result_image;
  image_t      *flag_input_image, *flag_result_image;
  qfits_header *wht_header, *flag_header;
  history      *hs;
  FILE         *catalog; /*  Data: Xpos, Ypos*/

  int lx, ly, lxf, lyf;
  int i, j;
  int nstar;
  int cxmin, cxmax, cymin, cymax;
  int nstar_fixed, npix_fixed, npix_kept, staraffected, flagval;
  float sep, maskradius, maskradius2, xpos, ypos, px, py;
  /* bitpix values; we need them so that the output image is
     saved with the same dynamical range as the input frames */
  int bitpixweight, bitpixflag;
  /* The bit value corresponding to CR masking */
  int crflagval = 2;
  /* The value it should be set to if the mask should be undone */
  int crfixflagval = 8;

  /* Bitvalue the masked should be AND with to check if we must keep
     the mask, e.g. due to permanent bad pixel: 5-> will keep if 1 OR 4
     245 -> all first 8 bits except val=2,val=8*/
  /* int alwayskeepmaskval=5; */
  int alwayskeepmaskval = 255 - crflagval - crfixflagval;

  /* read command line arguments */
  if (argc < 7) {
    e_error("fixcr wht flag radius starcat wht_fix flag_fix");
    exit(1);
  }

  /* initialise eclipse environment (has to be done
     before any eclipse command is called !!) */
  eclipse_init();

  /* load FITS image given by the first argument into
     an image structure:
   */
  fprintf(stdout, "reading %s\n", argv[1]);
  wht_input_image = image_load(argv[1]);
  wht_header      = qfits_header_read(argv[1]);

  fprintf(stdout, "reading %s\n", argv[2]);
  flag_input_image = image_load(argv[2]);
  flag_header      = qfits_header_read(argv[2]);

  /* get BITPIX values */
  bitpixweight = qfits_header_getint(wht_header, "BITPIX", 0);
  bitpixflag   = qfits_header_getint(flag_header, "BITPIX", 0);

  /* get size from this image and create a new
     one with the same dimensions */
  lx  = wht_input_image->lx;
  ly  = wht_input_image->ly;
  lxf = flag_input_image->lx;
  lyf = flag_input_image->ly;

  if ((lx == lxf) && (ly == lyf)) {
    fprintf(stdout, "Image sizes: Nx=%d Ny=%d\n", lx, ly);
  } else {
    fprintf(stdout, "ERROR: wht and flag image have different sizes! Abort!\n");
    exit(1);
  }

  fprintf(stdout, "creating new image\n");

  /* copy the original weight and flag images; those are modified to
     give the result lateron */
  wht_result_image  = image_copy(wht_input_image);
  flag_result_image = image_copy(flag_input_image);

  /* Radius around stars in which masks will be undone -> use r^2
     for comparison*/
  fprintf(stdout, "Check masks within %s pixels from provided stars\n",
          argv[3]);
  maskradius  = atof(argv[3]);
  maskradius2 = maskradius * maskradius;

  nstar_fixed = 0;
  npix_fixed  = 0;
  npix_kept   = 0;

  /* Open the catalog with star positions */
  fprintf(stdout, "Open star catalog %s\n", argv[4]);
  catalog = fopen(argv[4], "r");
  for (nstar = 0; fscanf(catalog, "%f %f\n", &xpos, &ypos) != EOF; nstar++) {
    cxmin = (int)(xpos - maskradius);
    cxmax = (int)(xpos + maskradius);
    cymin = (int)(ypos - maskradius);
    cymax = (int)(ypos + maskradius);
    if (cxmin < 0) {
      cxmin = 0;
    }
    if (cymin < 0) {
      cymin = 0;
    }
    if (cxmax >= lx) {
      cxmax = lx - 1;
    }
    if (cymax >= ly) {
      cymax = ly - 1;
    }
    staraffected = 0;

    /* check all pixels in box around this star*/
    for (i = cymin; i < cymax; i++) {
      for (j = cxmin; j < cxmax; j++) {
        px = (float)j + 0.5;
        py = (float)i + 0.5;

        /* The 2-d data structure from a FITS image is stored
           1-dimensional. To access the pixel located at
           position (j,i), use: */
        /* compute separation of pixel from star centroid
           (compare squared to make faster */
        sep = (xpos - px) * (xpos - px) + (xpos - px) * (ypos - py);
        if (sep <= maskradius2) {
          /* Check if flag pixel has bit2==1 */
          flagval = (int)flag_input_image->data[lx * i + j];
          if (flagval & crflagval) {
            staraffected = 1;
            npix_fixed++;
            /* Change this flag.*/
            flag_result_image->data[lx * i + j] =
              flag_input_image->data[lx * i + j] - crflagval + crfixflagval;
            /* In case no other serious flags exist, unmask the weight image*/
            if (flagval & alwayskeepmaskval) {
              /* We have at least one serious bit -> don't touch weight*/
              npix_kept++;
            } else {
              /* This pixel was probably masked without need. Reset the weight*/
              wht_result_image->data[lx * i + j] = 1.0;
            }
          }
        }
      }
    }
    if (staraffected) {
      nstar_fixed++;
    }
  }
  fprintf(stdout,
          "%d/%d stars fixed, %d pixels fixed, %d weight pixels not changed due to other flags\n", nstar_fixed, nstar, npix_fixed, npix_kept);

  /* save the result to a new FITS image */
  /* first add some history comments to the image headers */
  hs = history_new();
  if(hs == NULL) {
    e_error("cannot create history object");
    return 1;
  }

  history_add(hs, "");
  history_add(hs, "fixcr: %s", __PIPEVERS__);
  history_add(hs, "fixcr: called at %s", get_datetime_iso8601());
  history_add(hs, "fixcr: maskradius of %f pixels", maskradius);
  history_add(hs, "fixcr: modified %d weight/flag pixels for %d objects",
              npix_fixed, nstar_fixed);
  history_addfits(hs, wht_header);
  history_addfits(hs, flag_header);

  fprintf(stdout, "saving %s\n", argv[5]);
  image_save_fits_hdrdump(wht_result_image, argv[5], wht_header, bitpixweight);

  fprintf(stdout, "saving %s\n", argv[6]);
  image_save_fits_hdrdump(flag_result_image, argv[6], flag_header, bitpixflag);

  /* free all memory */
  image_del(wht_input_image);
  image_del(flag_input_image);
  image_del(wht_result_image);
  image_del(flag_result_image);
  qfits_header_destroy(wht_header);
  qfits_header_destroy(flag_header);
  history_del(hs);

  return(0);
}
