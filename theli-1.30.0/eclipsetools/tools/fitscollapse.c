#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "eclipse.h"
#include "fitstools.h"

/*
 compiled with
 gcc fitscollapse.c -L. -leclipse -lqfits -lm -O3 -o fitscollapse
*/

double coll_x(image_t*, double*, int*, bool, int);
double coll_y(image_t*, double*, int*, bool, int);
double coll_cross(image_t*, double*, int*, bool, int, char*);
double mask_mean(double*, long);
double mask_rms(double*, long);
double mask_meanadd(double*, long, int);
double meanadd(double*, long, int);
double mean_sigma(double*, long, int, double*, double*);
void write_image(image_t*, double*, int*, char*, char*, double, int, int);
void report_resetanomaly(double*, image_t*, int*);
void create_mask(image_t*, int*, long, long, long, long, long, long, int, int, char*, char*);

float threshold, kappa, maskval = -1.e9;
int niter, masked;
bool report;
char maskimage_out[4096];
char targetimage_in[4096], targetimage_out[4096];

void usage(int i, char *argv[])
{
  if (i == 0) {
    fprintf(stderr,"\n");
    fprintf(stderr,"          Version 2.5  (2011-12-10)\n\n");
    fprintf(stderr,"  Author: Mischa Schirmer\n\n");
    fprintf(stderr,"  USAGE:  %s \n", argv[0]);
    fprintf(stderr,"           -i input_image \n");
    fprintf(stderr,"           -o output_image \n");
    fprintf(stderr,"           -c (x, y, xy, xyyx or yxxy collapse direction(s))\n");
    fprintf(stderr,"           [-m maskimage_in]\n");
    fprintf(stderr,"           [-q maskimage_out]\n");
    fprintf(stderr,"           [-d ds9 region file]\n");
    fprintf(stderr,"           [-p targetimage_in targetimage_out]\n");
    fprintf(stderr,"           [-s (subtract the collapsed image)]\n");
    fprintf(stderr,"           [-t lower threshold (-60000.)]\n");
    fprintf(stderr,"           [-k kappa-sigma-threshold (2.5)]\n");
    fprintf(stderr,"           [-x xmin xmax ymin ymax (region that is ignored)]\n");
    fprintf(stderr,"           [-j (invert the ignored region)]\n");
    fprintf(stderr,"           [-n number of iterations (5)]\n");
    fprintf(stderr,"           [-r report reset anomaly amplitude]\n");
    fprintf(stderr,"           [-v calculate rms instead of mean column]\n");
    fprintf(stderr,"           [-a add columns/rows instead of calculating mean]\n\n");
    fprintf(stderr,"  PURPOSE: Collapses images along x/y-directions.\n\n");
    fprintf(stderr,"  NOMENCLATURE: \n");
    fprintf(stderr,"           input_image:   image that is to be corrected.\n");
    fprintf(stderr,"           output_image:  name of the final corrected image.\n");
    fprintf(stderr,"           maskimage_in:  image that is zero for good, and non-zero for bad pixels.\n");
    fprintf(stderr,"           maskimage_out: input_image, with maskimage_in superimposed.\n");
    fprintf(stderr,"           targetimage_in: another image, that should have the mask imprinted on it.\n");
    fprintf(stderr,"           targetimage_out: name of the output target image.\n\n");
    fprintf(stderr,"     If the -s option is given, then the output is the input image\n");
    fprintf(stderr,"     minus the collapsed image. Otherwise the collapsed image is returned.\n");
    fprintf(stderr,"     Pixel values smaller than -t are ignored.\n\n");
    fprintf(stderr,"     An iterative loop is run for outlier rejection. Options -n and -k\n");
    fprintf(stderr,"     specify the number of iterations and the sigma threshold.\n");
    fprintf(stderr,"     Option -x specifies a box that is excluded from the measurement.\n");
    fprintf(stderr,"     With -r the amplitude of the reset anomaly is returned.\n\n");
    fprintf(stderr,"  COLLAPSE directions:\n\n");
    fprintf(stderr,"     x:    Image collapsed horizontally (to remove vertical gradients)\n");
    fprintf(stderr,"     y:    Image collapsed vertically (to remove horizontal gradients)\n");
    fprintf(stderr,"     xy:   Image collapsed horizontally and vertically\n");
    fprintf(stderr,"     xyyx: Lower left, lower right, upper left, upper right quadrant\n");
    fprintf(stderr,"           collapsed along x, y, y and x, respectively\n");
    fprintf(stderr,"     yxxy: Lower left, lower right, upper left, upper right quadrant\n");
    fprintf(stderr,"           collapsed along y, x, x and y, respectively\n\n");
    exit(1);
  }
}

//**********************************************************************

int main(int argc, char *argv[])
{
  long n, m, i, k, xmin, xmax, ymin, ymax;
  int add, subtract, invert, *mask;
  double *collap, *collap_all, sky;
  char input_image[FILEMAX], output_image[FILEMAX], ds9region[FILEMAX], maskimage_in[FILEMAX], direction[10];
  bool rms;
  image_t *image_in;

  subtract = 0;
  niter = 5;
  kappa = 2.5;
  threshold = -60000.;
  masked = 0;
  invert = 0;

  xmin = 0;
  xmax = 0;
  ymin = 0;
  ymax = 0;
  report = false;
  rms = false;
  add = 0;
  sky = 0.0;

  strcpy(targetimage_in,"");
  strcpy(targetimage_out,"");
  strcpy(direction,"");
  strcpy(ds9region,"");

  // print usage if no arguments were given
  if (argc==1) usage(0, argv);

  for (i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      switch(tolower((int)argv[i][1])) {
      case 'i': strcpy(input_image,argv[++i]);
	break;
      case 'o': strcpy(output_image,argv[++i]);
	break;
      case 'd': strcpy(ds9region,argv[++i]);
	break;
      case 'p': strcpy(targetimage_in,argv[++i]);
	strcpy(targetimage_out,argv[++i]);
	break;
      case 'm': strcpy(maskimage_in,argv[++i]);
	masked = 1;
	break;
      case 'q': strcpy(maskimage_out,argv[++i]);
	break;
      case 'c': strcpy(direction,argv[++i]);
	break;
      case 's': subtract=1;
	break;
      case 't': threshold = atof(argv[++i]);
	break;
      case 'k': kappa = atof(argv[++i]);
	break;
      case 'n': niter = atol(argv[++i]);
	break;
      case 'j': invert = 1;
	break;
      case 'x':
	xmin = atol(argv[++i]) - 1;
	xmax = atol(argv[++i]) - 1;
	ymin = atol(argv[++i]) - 1;
	ymax = atol(argv[++i]) - 1;
	break;
      case 'r': report = true;
	break;
      case 'v': rms = true;
	break;
      case 'a': add = 1;
	break;
      }
    }
  }

  if (strcmp(direction,"") == 0) {
    fprintf(stderr, "ERROR: No collapse direction pattern (option -c) was specified!\n");
    exit (1);
  }

  // read the FITS data block
  checkfile(input_image);

  image_in = image_load(input_image);

  n = image_in->lx;
  m = image_in->ly;
  collap = (double*) calloc(n*m,sizeof(double));
  mask   = (int*) calloc(n*m, sizeof(int));

  // create the mask image
  create_mask(image_in, mask, n, m, xmin, xmax, ymin, ymax, masked, invert, maskimage_in, ds9region);

  // collapse direction: x, y, yxxy, xyyx
  if (strcmp(direction, "x") == 0) sky = coll_x(image_in, collap, mask, rms, add);
  if (strcmp(direction, "y") == 0) sky = coll_y(image_in, collap, mask, rms, add);
  if (strcmp(direction, "xyyx") == 0 ||
      strcmp(direction, "yxxy") == 0 )
    sky = coll_cross(image_in, collap, mask, rms, add, direction);
  // write the result
  if (strcmp(direction, "xy") != 0 && !report)
    write_image(image_in, collap, mask, input_image, output_image, sky, subtract, rms);

  // collapse direction: xy
  if (strcmp(direction, "xy") == 0) {
    collap_all = (double*) calloc(n*m,sizeof(double));
    // collapse along x
    sky = coll_x(image_in, collap, mask, rms, add);
    image_t *image_tmp;
    image_tmp = image_new(n,m);
    for (k=0; k<n*m; k++) {
      collap_all[k] = collap[k];
      image_tmp->data[k] = image_in->data[k] - collap[k];
      collap[k] = 0.;
    }
    // collapse along y
    coll_y(image_tmp, collap, mask, rms, add);
    for (k=0; k<n*m; k++) collap_all[k] += collap[k];
    if (!report)
      write_image(image_in, collap_all, mask, input_image, output_image, sky, subtract, rms);

    image_del(image_tmp);
    free(collap_all);
  }

  // Free the memory
  free(collap);
  free(mask);
  image_del(image_in);

  exit (0);
}


//*****************************************************************
// make a mask image

void create_mask(image_t *image, int *mask, long n, long m, long xmin, long xmax,
		 long ymin, long ymax, int masked, int invert,
		 char *maskimage_in, char *ds9region)
{
  long i, j;

  // The mask image is zero for all good pixels, and 1 for all bad pixels.
  // (i.e. objects detected by sextractor, and a rectangular regioned
  // pointed out by the user, and a ds9 region file)

  // we start out with mask[i] = 0 everywhere

  // RECTANGLE (user provided)
  if (xmin >= 0 && xmax >= 0 && ymin >= 0 && ymax >= 0) {
    // if the mask is uninverted
    if (invert == 0) {
      for (j=0; j<m; j++) {
	for (i=0; i<n; i++) {
	  if (i>=xmin && i<=xmax && j>=ymin && j<=ymax) mask[i+n*j] = 1;
	}
      }
    }
    // if the mask is inverted
    if (invert == 1) {
      for (j=0; j<m; j++) {
	for (i=0; i<n; i++) {
	  if (!(i>=xmin && i<=xmax && j>=ymin && j<=ymax)) mask[i+n*j] = 1;
	}
      }
    }
  }

  // SEXTRACTOR OBJECT image (zero for good pixels)
  if (masked == 1) {
    image_t *image_mask;
    image_mask = image_load(maskimage_in);
    for (i=0; i<n*m; i++)
      if (image_mask->data[i] != 0.) mask[i] = 1;
    image_del(image_mask);
  }

  // DS9 REGION FILE
  if (strcmp(ds9region,"") != 0) {
    int *ds9mask;
    ds9mask = (int*) calloc(n*m, sizeof(int));

    // the empty strings means we use keywords in the ds9 region file
    // that is, good pixels will be one and bad are zero, which we then have to invert
    make_ds9_maskimage(ds9mask, ds9region, n, m, "", "");

    // complement the mask image with the ds9 region;
    // we assume the mask is zero for good and 1 for bad pixels
    for (i=0; i<n*m; i++)
      if (ds9mask[i] == 0) mask[i] = 1;

    free(ds9mask);
  }


  // THRESHOLDING
  for (i=0; i<n*m; i++)
    if (image->data[i] < threshold) mask[i] = 1;

}


//*****************************************************************
// the kappa-sigma average for an array

double mean_sigma(double *data, long dim, int add, double *tmp_data1, double *tmp_data2)
{
  int i, j, k, dim2;
  double avg, sdev, avgfinal;

  avgfinal = 0.;

  for (i=0; i<dim; i++) tmp_data1[i] = data[i];

  dim2 = dim;
  avg  = mask_mean(tmp_data1, dim2);
  sdev = mask_rms(tmp_data1, dim2);

  for (k=0; k<niter; k++) {
    j = 0;
    for (i=0; i<dim2; i++)  {
      if ( tmp_data1[i] < avg + kappa * sdev &&
	   tmp_data1[i] > avg - kappa * sdev) {
	tmp_data2[j] = tmp_data1[i];
	j++;
      }
    }
    avg  = mask_mean(tmp_data2, j);
    sdev = mask_rms(tmp_data2, j);

    avgfinal = mask_meanadd(tmp_data2, j, add);

    for (i=0; i<j; i++) tmp_data1[i] = tmp_data2[i];
    dim2 = j;
  }

  if (sdev == 0.) avgfinal = mask_meanadd(data, dim, add);

  return (avgfinal);
}


//*****************************************************************
// collapse along x

double coll_x(image_t *image, double *collap, int *mask, bool rms, int add)
{
  long n, m, i, j;
  double *row, *col, sky, *tmp1, *tmp2;

  n = image->lx;
  m = image->ly;

  // average columns / rows
  row = (double*) calloc(n, sizeof(double));
  col = (double*) calloc(m, sizeof(double));

  tmp1 = (double*) calloc(n, sizeof(double));
  tmp2 = (double*) calloc(n, sizeof(double));

  // extract rows, average with sigma-rejection
  for (j=0; j<m; j++) {
    for (i=0; i<n; i++) {
      if (mask[i+n*j] == 0) row[i] = image->data[i+n*j];
      else row[i] = maskval;
    }
    if (!rms) col[j] = mean_sigma(row, n, add, tmp1, tmp2);
    else col[j] = mask_rms(row, n);
  }
  // write collapsed FITS array
  for (i=0; i<n; i++)  {
    for (j=0; j<m; j++)  {
      collap[i+n*j] = col[j];
    }
  }
  // the sky background estimate
  sky = median_double(col, m);

  free(row);
  free(col);
  free(tmp1);
  free(tmp2);

  if (report) report_resetanomaly(collap, image, mask);

  return (sky);
}


//*****************************************************************
// collapse along y

double coll_y(image_t *image, double *collap, int *mask, bool rms, int add)
{
  long n, m, i, j;
  double *row, *col, sky, *tmp1, *tmp2;

  n = image->lx;
  m = image->ly;

  // average columns / rows
  row = (double*) calloc(n, sizeof(double));
  col = (double*) calloc(m, sizeof(double));

  tmp1 = (double*) calloc(m, sizeof(double));
  tmp2 = (double*) calloc(m, sizeof(double));

  // extract columns, average with sigma-rejection
  for (i=0; i<n; i++) {
    for (j=0; j<m; j++) {
      if (mask[i+n*j] == 0) col[j] = image->data[i+n*j];
      else col[j] = maskval;
    }
    if (!rms) row[i] = mean_sigma(col, m, add, tmp1, tmp2);
    else row[i] = mask_rms(col, m);
  }
  // write collapsed FITS array
  for (j=0; j<m; j++)  {
    for (i=0; i<n; i++)  {
      collap[i+n*j] = row[i];
    }
  }

  // add the sky value if it doesn't exist already
  sky = median_double(row, n);

  free(row);
  free(col);
  free(tmp1);
  free(tmp2);

  if (report) report_resetanomaly(collap, image, mask);

  return (sky);
}


//*****************************************************************
// collapse along vhhv (vertical, horizontal, horizontal, vertical readout quadrants)
// or hvvh directions
// (for certain HAWAII-2 arrays, such as FLAMINGOS2@GEMINI or MOIRCS@SUBARU)

double coll_cross(image_t *image, double *collap, int *mask, bool rms, int add, char *mode)
{
  int *maskquad;
  long n, m, i, j, nh, mh, loop, n1, m1;
  double sky;
  double *collquad;
  image_t *quadrant;

  n1 = 0;
  m1 = 0;
  sky = 0.;

  n = image->lx;
  m = image->ly;

  if (n % 2 != 0 || m % 2 != 0) {
    printf("ERROR: image must have even numbered dimensions for this operation!\n");
    printf("       Dimensions: %ld x %ld\n", n, m);
    exit (1);
  }

  nh = n/2;
  mh = m/2;

  quadrant = image_new(nh,mh);
  collquad = (double*) calloc(nh*mh, sizeof(double));
  maskquad = (int*) calloc(nh*mh, sizeof(int));

  // do the four quadrants
  loop = 0;

  while (loop <= 3) {
    // the four quadrants
    // lower left
    if (loop == 0) {
      n1 = 0;
      m1 = 0;
    }
    // lower right
    if (loop == 1) {
      n1 = nh;
      m1 = 0;
    }
    // upper left
    if (loop == 2) {
      n1 = 0;
      m1 = mh;
    }
    // upper right
    if (loop == 3) {
      n1 = nh;
      m1 = mh;
    }

    // copy the quadrant and the mask
    for (j=0; j<mh; j++) {
      for (i=0; i<nh; i++) {
	quadrant->data[i+nh*j] = image->data[i+n1+n*(j+m1)];
	maskquad[i+nh*j] = mask[i+n1+n*(j+m1)];
      }
    }

    // collapse the quadrant
    if (strcmp(mode,"yxxy") == 0) {
      if (loop == 0) sky  = coll_y(quadrant, collquad, maskquad, rms, add);
      if (loop == 1) sky += coll_x(quadrant, collquad, maskquad, rms, add);
      if (loop == 2) sky += coll_x(quadrant, collquad, maskquad, rms, add);
      if (loop == 3) sky += coll_y(quadrant, collquad, maskquad, rms, add);
    }
    if (strcmp(mode,"xyyx") == 0) {
      if (loop == 0) sky  = coll_x(quadrant, collquad, maskquad, rms, add);
      if (loop == 1) sky += coll_y(quadrant, collquad, maskquad, rms, add);
      if (loop == 2) sky += coll_y(quadrant, collquad, maskquad, rms, add);
      if (loop == 3) sky += coll_x(quadrant, collquad, maskquad, rms, add);
    }

    // reconstruct the full (n x m) sized image
    for (j=0; j<mh; j++) {
      for (i=0; i<nh; i++) {
	collap[i+n1+n*(j+m1)] = collquad[i+nh*j];
      }
    }
    loop++;
  }

  image_del(quadrant);
  free(collquad);
  free(maskquad);

  return (sky/4.);
}

//*****************************************************************
double mask_mean(double *data, long dim)
{
  long i, n;
  double avg;

  avg = 0.;
  n = 0;

  for (i=0; i<dim; i++) {
    if (data[i] > maskval) {
      avg += data[i];
      n++;
    }
  }

  if (n > 0)
    avg /= (double) n;
  else avg = 0.;

  return (avg);
}


//*****************************************************************
double mask_meanadd(double *data, long dim, int add)
{
  long i, n;
  double avg;

  avg = 0.;
  n = 0;

  for (i=0; i<dim; i++)  {
    if (data[i] > maskval) {
      avg += data[i];
      n++;
    }
  }

  if (n > 0) {
    if (add == 0) return (avg / (double) n);
    else return (avg);
  }
  else return (0.);
}


//*****************************************************************
double mask_rms(double *data, long dim)
{
  long i, n;
  double avg, sdev;

  avg = mask_mean(data, dim);
  sdev = 0.;
  n = 0;

  for (i=0; i<dim; i++)  {
    if (data[i] > maskval) {
      sdev += (avg - data[i]) * (avg - data[i]);
      n++;
    }
  }

  if (sdev != 0. && n++ > 0) {
    sdev = sqrt(sdev /( (double) n - 1.));
  }
  else sdev = 0.;

  return (sdev);
}


//********************************************************************
void write_image(image_t *image_in, double *collap, int *mask, char *input_image,
		 char *output_image, double sky, int subtract, int rms)
{
  long k, n, m;
  char skystring[FILEMAX];
  image_t *image_out;

  qfits_header *header;
  header = qfits_header_read(input_image);

  n = image_in->lx;
  m = image_in->ly;

  image_out = image_new(n,m);

  sprintf(skystring,"%.2lf", sky);
  if (qfits_query_hdr( input_image, "SKYVALUE") == NULL)
    qfits_header_add( header, "SKYVALUE", skystring, "", "");

  if (qfits_query_hdr( input_image, "COLLAPSE") == NULL)
    qfits_header_add( header, "COLLAPSE", "yes", "Collapse correction used to make this image", "");

  if (!report) {
    if (subtract == 0 || rms) {
      for (k=0; k<n*m; k++) image_out->data[k] = collap[k];
      image_save_fits_hdrdump(image_out, output_image, header,
			      BPP_IEEE_FLOAT);
    }
    if (subtract == 1 && !rms) {
      for (k=0; k<n*m; k++) image_out->data[k] = image_in->data[k] - collap[k];
      image_save_fits_hdrdump(image_out, output_image, header,
			      BPP_IEEE_FLOAT);
    }
  }

  // if a mask image exists
  if (masked == 1) {
    image_t *masked_output;
    masked_output = image_new(n,m);

    // if the input image should be masked
    if (strcmp(maskimage_out,"") != 0) {
      for (k=0; k<n*m; k++) {
	if (mask[k] != 1) masked_output->data[k] = image_in->data[k] - collap[k];
	else masked_output->data[k] = -1e09;
      }
      image_save_fits_hdrdump(masked_output, maskimage_out, header, BPP_IEEE_FLOAT);
    }

    // if an additonal image ("target_image") should be masked
    if (strcmp(targetimage_in,"")  != 0 &&
	strcmp(targetimage_out,"") != 0 &&
	qfits_header_read(targetimage_in) != NULL) {
      image_t *_targetimage_in_;
      qfits_header *targetheader;
      _targetimage_in_ = image_load(targetimage_in);
      targetheader = qfits_header_read(targetimage_in);
      if (qfits_query_hdr( targetimage_in, "COLLAPSE") == NULL)
	qfits_header_add( targetheader, "COLLAPSE", "yes", "Collapse correction used to make this image", "");
      for (k=0; k<n*m; k++) {
	if (mask[k] != 1) masked_output->data[k] = _targetimage_in_->data[k];
	else masked_output->data[k] = -1e09;
      }
      image_save_fits_hdrdump(masked_output, targetimage_out, targetheader, BPP_IEEE_FLOAT);
      image_del(_targetimage_in_);
      qfits_header_destroy(targetheader);
    }
    image_del(masked_output);
  }

  qfits_header_destroy(header);
  image_del(image_out);
}


//********************************************************************
void report_resetanomaly(double *collap, image_t *image_in, int *mask)
{
  // this function will exit main() when called!
  long nred, mred, i, j, k, n1, n2, m1, m2, n, m;
  float maxamp;

  n = image_in->lx;
  m = image_in->ly;

  nred = 0.1 * n;
  mred = 0.1 * m;
  n1 = 0.4 * n;
  n2 = 0.5 * n;
  m1 = 0.4 * m;
  m2 = 0.5 * m;
  k = 0;
  maxamp = -1.0e-9;
  for (j=m1; j<m2; j++) {
    for (i=n1; i<n2; i++) {
      if (k<nred*mred) {
	if (collap[i+n*j] > maxamp) maxamp = collap[i+n*j];
	k++;
      }
    }
  }
  printf("%f\n", maxamp);

  free(collap);
  free(mask);
  image_del(image_in);

  exit (0);
}
