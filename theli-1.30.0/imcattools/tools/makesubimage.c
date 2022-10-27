/*
  24.01.2005:
  I changed exit(0) to return 0 in the main program
  to avoid compiler warnings

  28.01.2005:
  I added HISTORY information to the produced FITS
  images

  01.03.2007:
  - If CRPIX1 and CRPIX2 header keywords are present
    in the original headers they are updated in the
    extracted subimage.
  - I introduced the new command line option '-c'.
    If given the subimage is extracted w.r.t. CRPIX1,
    CRPIX2 header keywords.

  16.10.2007:
  I introduced the possibility to specify a default value
  (it was always zero or magic before) for unmapped pixels.
  The default value has to be provided together with the '-o'
  command line option.
*/

#define usage "\n\n\n\
NAME\n\
	makesubimage --- extracts a subimage from a fits file\n\
\n\
SYNOPSIS:\n\
	makesubimage x y dx dy [options...]\n\
		-o number # set unmapped pixels to 'number'\n\
		-c	  # extract subimage w.r.t. CRPIX\n\
                          # header keywords.\n\
\n\
DESCRIPTION:\n\
	'makesubimage' extracts a dx x dy subimage with\n\
	origin at (x,y) from the input image. If '-c' is\n\
        given the origin is the pixel given in the CRPIX\n\
        header keywords. By default,\n\
	if the subimage extends beyond the source image\n\
	then extra pixels are set to MAGIC.  Use -o\n\
	option to force these to zero.\n\
        If CRPIX coordinate values are present in the input\n\
        image they are modified to relect the configuartion in\n\
        the output image.\n\
	Reads from stdin, writes to stdout.\n\
\n\
EXAMPLES:\n\
        makesubimage 100 100 200 200 < image.fits > out.fits\n\n\
        Extracts a subimage of size 200x200 with (100,100) as\n\
        lower left corner.\n\n\
        makesubimage -100 -100 200 200 -c < image.fits > out.fits\n\n\
        Extracts a subimage of size 200x200 with the pixel given by\n\
        (CRPIX1, CRPIX2) in its centre.\n\n\
AUTHORS:\n\
	Nick Kaiser:  kaiser@cita.utoronto.ca\n\
        Thomas Erben: terben@astro.uni-bonn.de\n\
\n\n\n"


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "fits.h"
#include "error.h"
#include "global_vers.h"
#include "t_iso8601.h"

#define MAGIC FLOAT_MAGIC

int		main(int argc, char *argv[])	
{
    int		arg = 1, N1, N2, M1, M2, i0, j0, i, j, jj;
    fitsheader	*fitsin , *fitsout;
    fitscomment *crpix1comment, *crpix2comment;
    float	*fin, *fout, defaultvalue;
    float       crpix1in, crpix2in, crpix1out, crpix2out;
    char        *tmpstring[256];
    /* for centrecut = 0 the command line arguments x and y are
       interpreted as lower left origin of the result image;
       with centrecut = 1 they are interpreted relative to the
       CRPIX1, CRPIX2 coordinates given in the image header
    */
    int         centrecut = 0;


    /* defaults */
    defaultvalue = MAGIC;

    if (argc < 5)
    {
        error_exit(usage);
    }
    	
    if (1 != sscanf(argv[arg++], "%d", &j0) ||
    	1 != sscanf(argv[arg++], "%d", &i0) ||
    	1 != sscanf(argv[arg++], "%d", &M1) ||
    	1 != sscanf(argv[arg++], "%d", &M2))
    {
        error_exit(usage);
    }

    while (arg < argc)
    {
    	if (argv[arg][0] != '-')
	{
    	    error_exit(usage);
	}
    	switch (argv[arg++][1])
	{
    	    case 'o':
              if(arg <= (argc-1))
              {
    	        defaultvalue = atof(argv[arg++]);
              }
              else
              {
                error_exit(usage);
              }
    	      break;
    	    case 'c':
    	      centrecut = 1;
    	      break;
    	    default:
    	      error_exit(usage);
    	}
    }


    fitsin = readfitsheader(stdin);

    N1 = fitsin->n[0];
    N2 = fitsin->n[1];
    fin = (float *) calloc(N1, sizeof(float));
    fout = (float *) calloc(M1, sizeof(float));

    fitsout = copyfitsheader(fitsin);

    /* read CRPIX values from input image header if those
       keywords are present. In case of centrecut = 1
       they have to be there.
    */
    if((crpix1comment = getcommentbyname("CRPIX1", fitsout)) &&
       (crpix2comment = getcommentbyname("CRPIX2", fitsout)))
    {
        crpix1in = getnumericvalue(crpix1comment);
        crpix2in = getnumericvalue(crpix2comment);

	if(centrecut == 0)
	{
	    crpix1out = crpix1in - (float)j0;
	    crpix2out = crpix2in - (float)i0;
	}
	else
	{
            crpix1out = (crpix1in - (int)crpix1in) - j0;
            crpix2out = (crpix2in - (int)crpix2in) - i0;
	    j0 = j0 + (int)crpix1in;
	    i0 = i0 + (int)crpix2in;
	}
	sprintf(crpix1comment->value, "%20.8E", crpix1out);
	sprintf(crpix2comment->value, "%20.8E", crpix2out);
    }
    else
    {
        if(centrecut == 1)
        {
	    error_exit("CRPIX header keywords must be present in input image!!");
        }
    }

    /* add history comments to FITS headers */
    appendcomment(newtextcomment((char *)"HISTORY",
                  (char *)"", NULL), fitsout);
    sprintf((char *)tmpstring, "makesubimage: %s", __PIPEVERS__);
    appendcomment(newtextcomment((char *)"HISTORY",
                  (char *)tmpstring, NULL), fitsout);
    sprintf((char *)tmpstring, "makesubimage: called at %s",
                  get_datetime_iso8601());
    appendcomment(newtextcomment((char *)"HISTORY",
                  (char *)tmpstring, NULL), fitsout);

    add_comment(argc, argv, fitsout);
    fitsout->n[0] = M1;
    fitsout->n[1] = M2;
    writefitsheader(fitsout);

    if (i0 > 0)
    {
        skiplines(fitsin, i0);
    }
    for (i = i0; i < i0 + M2; i++)
    {
    	if (i >= N2 || i < 0)
	{
    	    for (j = 0; j < N1; j++)
	    {
    	        fin[j] = defaultvalue;
	    }
	}
    	else
	{
    	    readfitsline(fin, fitsin);
	}
    	for (jj = 0; jj < M1; jj++)
	{
    	    fout[jj] = (jj + j0 >= N1 ? defaultvalue : (jj + j0 < 0 ? defaultvalue : fin[jj + j0]));
	}
    	writefitsline(fout, fitsout);
    }
    writefitstail(fitsout);

return 0;
}


