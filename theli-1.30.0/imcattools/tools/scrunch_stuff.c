
/*----------------------------------------------------------------------------

   File name    :   scrunch_stuff.c
   Author       :   Nick Kaiser & Thomas Erben
   Created on   :   29.08.2007
   Description  :   scrunch handling routines

 ---------------------------------------------------------------------------*/

/*
	$Id: scrunch_stuff.c,v 1.3 2018/03/28 12:06:15 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:15 $
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
                                Includes and Defines
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include    "fits.h"
#include    "scrunch_stuff.h"
#include    "error.h"
#include    "arrays.h"

/*----------------------------------------------------------------------------
                                                Private function prototypes
 ---------------------------------------------------------------------------*/

static int xfloatcmp();

/*----------------------------------------------------------------------------
                                                        Function codes
 ---------------------------------------------------------------------------*/

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
                    int binning, int mode)
{
    int     i, j, k, l, N1in, N2in, N1out, N2out;
    int     ngood;
    float       **fin, *fout, *f;
    double      fsum;

    N1in = fitsin->n[0];
    N2in = fitsin->n[1];
    N1out = N1in / binning;
    N2out = N2in / binning;

    IMCAT_CALLOC(fin, float *, binning);

    for(i = 0; i < binning; i++)
    {
        IMCAT_CALLOC(fin[i], float, N1in);
    }

    IMCAT_CALLOC(fout, float, N1out);
    IMCAT_CALLOC(f,    float, binning * binning);

    for (i = 0; i < N2in; i += binning)
    {
        for(j = 0; j < binning; j++)
	{
            readfitsline(fin[j], fitsin);
	}

        for (j = 0; j < N1out; j++)
        {
            ngood = 0;

            for(k = 0; k < binning; k++)
	    {
    	        for(l = 0; l < binning; l++)
		{
                    if (fin[k][binning * j + l] != FLOAT_MAGIC)
		    {
                        f[ngood++] = fin[k][binning * j + l];
		    }
		}
	    }

            switch (mode) {
                case    MEAN:
                    if (ngood > 0)
                    {
                        fsum = 0;
                        for (k = 0; k < ngood; k++)
			{
                            fsum += f[k];
			}
                        fout[j] = fsum / ngood;
                    } else
                        fout[j] = FLOAT_MAGIC;
                    break;
                case    CMEAN:
                    if (ngood == (binning * binning))
                    {
                        fsum = 0;
                        for (k = 0; k < ngood; k++)
			{
                            fsum += f[k];
			}
                        fout[j] = fsum / ngood;
                    } else {
                        fout[j] = FLOAT_MAGIC;
                    }
                    break;
                case    MEDIAN:
    		    if(ngood > 0)
		    {
                        qsort(f, ngood, sizeof(float), xfloatcmp);

                        if(ngood % 2 == 0)
			{
			    fout[j] = 0.5 * (f[ngood / 2] + f[(ngood / 2) - 1]);
			}
                        else
			{
			    fout[j] = f[ngood / 2];
			}
                    }
                    else
		    {
                        fout[j] = FLOAT_MAGIC;
                    }
            }
        }
        writefitsline(fout, fitsout);
    }

    for(i = 0; i < binning; i++)
    {
        IMCAT_FREE(fin[i]);
    }

    /* clean up memory allocations */
    IMCAT_FREE(fin);
    IMCAT_FREE(fout);
    IMCAT_FREE(f);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Compares to floats for sorting purposes
  @param    s1      pointer to first float
  @param    s2      pointer to second float
  @return   -1 if s1 < s2, 1 if s2 > s1 and 0 for s1 == s2

 */
/*--------------------------------------------------------------------------*/
int     xfloatcmp(float *s1, float *s2)
{
    if (*s1 < *s2)
        return (-1);
    else if (*s1 > *s2)
        return (1);
    else
        return (0);
}

