 /*
 				misc.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	SExtractor
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	miscellaneous functions.
*
*	Last modify:	13/12/2002
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
   23.05.2013:
   I included code for the get_basename function that returns the
   basename for a file containing the complete path.
*/

#ifdef HAVE_CONFIG_H
#include        "config.h"
#endif

#include        <string.h>
#include	"define.h"
#include	"globals.h"


/******************************** hmedian ***********************************/
/*
Median using Heapsort algorithm (for float arrays) (based on Num.Rec algo.).
Warning: changes the order of data!
*/
float	hmedian(float *ra, int n)

  {
   int		l, j, ir, i;
   float	rra;


  if (n<2)
    return *ra;
  ra--;
  for (l = ((ir=n)>>1)+1;;)
    {
    if (l>1)
      rra = ra[--l];
    else
      {
      rra = ra[ir];
      ra[ir] = ra[1];
      if (--ir == 1)
        {
        ra[1] = rra;
        return n&1? ra[n/2+1] : (float)((ra[n/2]+ra[n/2+1])/2.0);
        }
      }
    for (j = (i=l)<<1; j <= ir;)
      {
      if (j < ir && ra[j] < ra[j+1])
        ++j;
      if (rra < ra[j])
        {
        ra[i] = ra[j];
        j += (i=j);
        }
      else
        j = ir + 1;
      }
    ra[i] = rra;
    }

/* (the 'return' is inside the loop!!) */
  }

/******************************* get_basename *******************************/
/*
return 'basename' for a filename including complete path
*/
char * get_basename(const char *filename)
{
  char *p ;
  p = strrchr (filename, '/');
  return p ? p + 1 : (char *) filename;
}

