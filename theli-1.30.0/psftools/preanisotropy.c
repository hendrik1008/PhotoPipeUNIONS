#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sort_globals.h"
#include "utils_globals.h"
#include "options_globals.h"

/*
   03.10.2000: first version based on an algorithm
               from Yannick (findstar program)
   19.01.2001: fixed a bug in the loop going determining
               the best rh. It was done one time too much.
   17.03.2001: substituted the "long int" by the LONG macro
               (I have to investigate the problem that the
                program may not work properly if the number
                of objects exceeds the range of an int variable).
   21.03.2002: if the program does not find a good stellar locus
               it tries three times with a different stepsize
               (currently it adds three times 0.02 to the stepsize
                originally given)
   21.04.2003: added two more DEBUG messages in the VERBOSE mode "DEBUG"
   19.10.2004: removed the superfluous inclusion of malloc.h
   09.12.2004: - If the number of initial bins from the preselected objects
                 is smaller than 5, it is set to 5 and the stepsize is
                 adapted accordingly. In this case we still use the original
                 stepsize for the final size of the stellar locus.
               - I removed compiler warnings
   18.03.2005: The number of minimum bins is increased to 7. 5 turned out to be
               too small for cases where only objects from the stellar
               track are present in the initial catalog.
   27.03.2006: I renamed sorting and indexing functions to the new naming scheme
               described in the sort_globals.h file
   03.09.2006: I included the zero in comparisons of the gradient distribution
               when selecting candidate bins for the stellar locus.
               Previously we only compared for gradients larger or smaller
               than zero but this led to wrong results if object numbers
               in neighbouring bins happened to be equal.
   03.08.2010: I corrected a bug in the calculation of the rh-region for
               the stellar locus
   04.08.2010: I corrected a bug in the magnitude offset addition to the low
               magnitude estimate for the stellar locus. If all stars are
               approximately at the same magnitude - for instance for
               simulations - we need to skip this addition.
 */

#define __PROGRAMMVERSION__    "1.1.3"
#define __PROGRAMMNAME__       "preanisotropy"

#define __USAGE__              "\n\
USAGE:\n\
   preanisotropy -i input catalog\n\
                 -k size-key; mag-key\n\
               [ -c conditions ]\n\
               [ -o magnitude offset (0.3) ]\n\
               [ -s step size for the size-key coordinate (0.1) ]\n\
               [ -v verbosity level (NORMAL) ]\n\
               [ -t table with the objects (OBJECTS) ]\n\n\
DESCRIPTION:\n\
The program 'preanisotropy' determines automatically \n\
the stellar locus in a size (size-key) vs. mag \n\
(mag-key) diagram. It divides the objects in table (-t)\n\
into bins of size (-s) in the size coordinate. It calculates\n\
the gradient and contrasts (change of number of objects)\n\
for all the bins. The 'best size' of stars is taken where the gradient\n\
changes after two consecutive positive values and where the\n\
number of objects is highest. The range in size is taken as\n\
two times the 'step size' around the 'best size'. The magnitude range\n\
is calculated from all objects falling within size range of stars.\n\
An offset (-o) is added to the lower magnitude limit in order to\n\
avoid saturated stars.\n\n\
EXAMPLE:\n\
  preanisotropy -i tmp.cat -t OBJECTS -k rh mag -s 0.15\n\
                -c snratio 10.0 1000.0\n\
would take the keys 'rh' and 'mag' for size and manitude respectively.\n\
Only objects with a S/N ratio of 10 and larger would be considered.\n\
The result is written to stdout.\n\n\
AUTHOR: Yannick Mellier/Thomas Erben \n"

#ifndef MAXSTRINGLENGTH
#define MAXSTRINGLENGTH        256
#endif

int main(int argc, char **argv)
{
  /* variables related to the cataog and the objects */
  char   incatname[MAXSTRINGLENGTH];
  char   tablename[MAXSTRINGLENGTH] = "OBJECTS";
  char   verbosity[MAXSTRINGLENGTH] = "NORMAL";
  int    nobjects = 0;
  char **keynames;
  float *condmin = NULL, *condmax = NULL;
  float *goodsize;
  float *goodmag;
  int    startconditions = 0;
  int    nkeys           = 0;
  int    nconditions     = 0;
  int    ngoodobjects    = 0;
  int    objectgood;

  catstruct * incat;
  tabstruct * tab;
  keystruct **keys;

  /* variables for the star finding */
  int    nbins    = 50;
  float  stepsize = 0.1;
  float  origstepsize;
  float  deltastep    = 0.02;
  int    maxsteps     = 3;
  int    step         = 0;
  int    foundlocus   = 0;
  float  sizemin      = 0.0, sizemax = 0.0;
  float  sizeminlimit = 0.5;
  float  sizemean     = 0.0, sizesigma = 0.0;
  float  size;
  float  starmagmin   = 99., starmagmax = 0.;
  float  magminoffset = 0.3;
  float  starsizemin, starsizemax;
  float *grad, *contr;
  float  contrmean, contrsigma;
  int *  objbin;
  int *  nobj;
  int    candidate, candidateobjects;
  int    minbin, maxbin;

  /* other stuff (count, temp variables etc.) */
  int  i, j;
  char tmpstring1[MAXSTRINGLENGTH];
  char tmpstring2[MAXSTRINGLENGTH];
  int  tmpint;

  /* print start message: */
  printf("\n %s  V%s (%s;%s)\n\n", __PROGRAMMNAME__, __PROGRAMMVERSION__,
         __DATE__, __TIME__);
  if (argc < 5) {
    printf("%s\n", __USAGE__);
    exit(1);
  }

  /* get the command line arguments: */
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (tolower(argv[i][1])) {
      case 'i': strncpy(incatname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 't': strncpy(tablename, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'v': strncpy(verbosity, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 's': stepsize = atof(argv[++i]);
        break;
      case 'o': magminoffset = atof(argv[++i]);
        break;
      case 'k': strncpy(tmpstring1, argv[++i], MAXSTRINGLENGTH - 1);
        strncpy(tmpstring2, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'c': startconditions = i + 1;
        while (i + 1 < argc && !(argv[i + 1][0] == '-' && strlen(argv[i + 1]) == 2)) {
          i++;
          nconditions++;
        }
      default:
        break;
      }
    } else {
      syserr(FATAL, "preanisotropy", "Wrong Command line syntax");
    }
  }

  /* set verbosity level */
  altverbose(verbosity);

  if ((nconditions % 3) != 0) {
    syserr(FATAL, "preanisotropy", "Error in conditions");
  }

  /* the stepsize is copied in case we have to change it
     when not enough bins can be created with the original one
   */
  origstepsize = stepsize;

  nconditions /= 3;

  /* the first two keys are the size- and the mag-key */
  nkeys = 2 + nconditions;

  ED_MALLOC(keynames, "preanisotropy", char *, nkeys);

  for (i = 0; i < nkeys; i++) {
    ED_MALLOC(keynames[i], "preanisotropy", char, MAXSTRINGLENGTH);
  }

  if (nconditions > 0) {
    ED_MALLOC(condmin, "preanisotropy", float, nconditions);
    ED_MALLOC(condmax, "preanisotropy", float, nconditions);
  }

  ED_MALLOC(keys, "preansiotropy", keystruct *, nkeys);

  strncpy(keynames[0], tmpstring1, MAXSTRINGLENGTH - 1);
  strncpy(keynames[1], tmpstring2, MAXSTRINGLENGTH - 1);

  for (i = 0; i < nconditions; i++) {
    strncpy(keynames[2 + i], argv[startconditions + i * 3],
            MAXSTRINGLENGTH - 1);
    condmin[i] = atof(argv[startconditions + i * 3 + 1]);
    condmax[i] = atof(argv[startconditions + i * 3 + 2]);
  }

  if (!(incat = read_cat(incatname))) {
    syserr(FATAL, "preanisotropy", "Error opening input catalog\n");
  }

  if (!(tab = name_to_tab(incat, tablename, 0))) {
    syserr(FATAL, "preanisotropy", "No %s table in catalog\n", tablename);
  }

  /* read in the keys */
  for (i = 0; i < nkeys; i++) {
    keys[i] = read_key(tab, keynames[i]);
    if (keys[i] == NULL) {
      syserr(FATAL, "preanisotropy", "No %s key in catalog\n", keynames[i]);
    }
  }

  nobjects = keys[0]->nobj;

  ED_MALLOC(goodsize, "preanisotropy", float, nobjects);
  ED_MALLOC(goodmag, "preanisotropy", float, nobjects);

  /* read in the object. For every objects we first have to check
     whather it satisfies the conditions before putting it into the arrays
   */

  for (i = 0; i < nobjects; i++) {
    objectgood = 0;
    for (j = 0; j < nconditions; j++) {
      switch (keys[2 + j]->ttype) {
      case T_SHORT:
        if ((float)((short int *)keys[2 + j]->ptr)[i] < condmin[j] ||
            (float)((short int *)keys[2 + j]->ptr)[i] > condmax[j]) {
          objectgood = 1;
        }
        break;
      case T_LONG:
        if ((float)((LONG *)keys[2 + j]->ptr)[i] < condmin[j] ||
            (float)((LONG *)keys[2 + j]->ptr)[i] > condmax[j]) {
          objectgood = 1;
        }
        break;
      case T_DOUBLE:
        if ((float)((double *)keys[2 + j]->ptr)[i] < condmin[j] ||
            (float)((double *)keys[2 + j]->ptr)[i] > condmax[j]) {
          objectgood = 1;
        }
        break;
      case T_FLOAT:
        if ((float)((float *)keys[2 + j]->ptr)[i] < condmin[j] ||
            (float)((float *)keys[2 + j]->ptr)[i] > condmax[j]) {
          objectgood = 1;
        }
        break;
      default:
        syserr(FATAL, "preanisotropy", "unsupported key type");
        break;
      }
    }
    if (objectgood == 0) {
      switch (keys[0]->ttype) {
      case T_SHORT:
        goodsize[ngoodobjects] = (float)(((short int *)keys[0]->ptr)[i]);
        break;
      case T_LONG:
        goodsize[ngoodobjects] = (float)(((LONG *)keys[0]->ptr)[i]);
        break;
      case T_DOUBLE:
        goodsize[ngoodobjects] = (float)(((double *)keys[0]->ptr)[i]);
        break;
      case T_FLOAT:
        goodsize[ngoodobjects] = (float)(((float *)keys[0]->ptr)[i]);
        break;
      default:
        syserr(FATAL, "preanisotropy", "Unsupported key type");
        break;
      }
      sizemean  += goodsize[ngoodobjects];
      sizesigma += goodsize[ngoodobjects] * goodsize[ngoodobjects];
      sizemax    = (sizemax > goodsize[ngoodobjects]) ?
        sizemax : goodsize[ngoodobjects];
      sizemin    = (sizemin < goodsize[ngoodobjects]) ?
        sizemin : goodsize[ngoodobjects];

      switch (keys[1]->ttype) {
      case T_SHORT:
        goodmag[ngoodobjects] = (float)(((short int *)keys[1]->ptr)[i]);
        break;
      case T_LONG:
        goodmag[ngoodobjects] = (float)(((LONG *)keys[1]->ptr)[i]);
        break;
      case T_DOUBLE:
        goodmag[ngoodobjects] = (float)(((double *)keys[1]->ptr)[i]);
        break;
      case T_FLOAT:
        goodmag[ngoodobjects] = (float)(((float *)keys[1]->ptr)[i]);
        break;
      default:
        syserr(FATAL, "preanisotropy", "Unsupported key type");
        break;
      }

      ngoodobjects++;
    }
  }

  if (ngoodobjects > 1) {
    VPRINTF(NORMAL, "\nPreselected %d objects in catalog.\n\n", ngoodobjects);

    sizemean /= ngoodobjects;
    sizesigma = sqrt((sizesigma / ngoodobjects) - sizemean * sizemean);

    /*
       We investigate all objects within 3 sigma of the sizemean
       for the stellar locus
     */

    sizeminlimit = (sizemin > sizeminlimit) ? sizemin : sizeminlimit;
    sizemin      = (sizemean - 3. * sizesigma > sizeminlimit) ?
                   sizemean - 3. * sizesigma : sizeminlimit;
    sizemax = (sizemax < sizemean + 3. * sizesigma) ?
      sizemax : sizemean + 3. * sizesigma;

    VPRINTF(VERBOSE, "meansize: %f; sigmasize: %f\n\n", sizemean, sizesigma);
    VPRINTF(VERBOSE, "investigating %f < %s < %f for the stellar locus\n\n",
            sizemin, keynames[0], sizemax);

    /* sort the size and mag array to increasing radii */
    ffsort(ngoodobjects, goodsize, goodmag);

    while ((foundlocus == 0) && (step <= maxsteps)) {
      nbins = (int)((sizemax - sizemin) / (stepsize));

      if (nbins < 7) {
        nbins    = 7;
        stepsize = (sizemax - sizemin) / nbins;
        VPRINTF(DEBUG, "new stepsize: %f\n\n", stepsize);
      }

      ED_MALLOC(objbin, "preanisotropy", int, nbins);
      ED_MALLOC(nobj, "preanisotropy", int, nbins);

      j    = 0;
      size = sizemin;

      /* get the index of each bin within the array */
      for (i = 0; i < nbins; i++) {
        while ((goodsize[j] <= size + stepsize) && (j < ngoodobjects)) {
          j++;
        }
        objbin[i] = j;

        VPRINTF(DEBUG, "index of bin %d: %d\n\n", i, objbin[i]);
        size += stepsize;
      }

      /* get the number of objects in each bin */
      nobj[0] = objbin[0];
      for (i = 1; i < nbins; i++) {
        nobj[i] = objbin[i] - objbin[i - 1];
        VPRINTF(DEBUG, "number of objects in bin %d: %d\n\n", i, nobj[i]);
      }

      ED_MALLOC(grad, "preanisotropy", float, nbins);
      ED_MALLOC(contr, "preanisotropy", float, nbins);

      contrmean = contrsigma = 0.0;

      for (i = 1; i < nbins - 1; i++) {
        grad[i]     = (float)((nobj[i + 1] - nobj[i - 1]) / 2.);
        contr[i]    = (float)(nobj[i] - (nobj[i + 1] + nobj[i - 1]) / 2.);
        contrmean  += contr[i];
        contrsigma += contr[i] * contr[i];
        VPRINTF(DEBUG, "grad. and contr. in bin %d: %f; %f\n\n", i,
                grad[i], contr[i]);
      }
      contrmean /= (nbins - 2);
      contrsigma = sqrt((contrsigma / (nbins - 2)) - contrmean * contrmean);

      VPRINTF(DEBUG, "contrmean, contrsigma: %f; %f\n\n",
              contrmean, contrsigma);

      /*
         We search the best size by taking that bin having a negative gradient
         after two consecutive positive ones and the highest number of
         objects
       */
      candidateobjects = 0;
      candidate        = 0;

      for (i = 2; i < nbins - 1; i++) {
        if ((grad[i - 2] >= 0) && (grad[i - 1] >= 0) && (grad[i] <= 0) &&
            ((contr[i - 2] > contrsigma) || (contr[i - 1] > contrsigma) ||
             (contr[i] > contrsigma))) {
          if (nobj[i - 1] > nobj[i]) {
            tmpint = nobj[i - 1];
            j      = i - 1;
          } else {
            tmpint = nobj[i];
            j      = i;
          }

          if (tmpint > candidateobjects) {
            candidate        = j;
            candidateobjects = nobj[j];
          }
        }
      }

      if (candidate > 0) {
        VPRINTF(NORMAL, "best rh: %f\n\n",
                sizemin + (candidate + 0.5) * stepsize);

        /*
           now get the range for rh:
           - for the moment this is 2*stepsize around the best size
             value of sizemin+(candidate+0.5)*stepsize
             (can presumably be improoved by conditions to the contrast
              etc. to catch very broad stellar loci)
         */

        minbin = maxbin = candidate;

        if (stepsize < origstepsize) {
          starsizemin = (sizemin + (candidate + 0.5) * stepsize) - origstepsize;
          starsizemax = (sizemin + (candidate + 0.5) * stepsize) + origstepsize;
        } else {
          starsizemin = (sizemin + (candidate + 0.5) * stepsize) - stepsize;
          starsizemax = (sizemin + (candidate + 0.5) * stepsize) + stepsize;
        }

        /* now get the minimum and maximum magnitude */

        for (i = minbin - 1; i <= maxbin + 1; i++) {
          for (j = 0; j < nobj[i]; j++) {
            if ((goodsize[objbin[i - 1] + j] > sizemin) &&
                (goodsize[objbin[i - 1] + j] < sizemax)) {
              if (goodmag[objbin[i - 1] + j] < starmagmin) {
                starmagmin = goodmag[objbin[i - 1] + j];
              }
              if (goodmag[objbin[i - 1] + j] > starmagmax) {
                starmagmax = goodmag[objbin[i - 1] + j];
              }
            }
          }
        }

        /* security check; for simulated data nearly all objects may lie
           at the same magnitude. In this case skip the magnitude offset
           addition to the lower limit
        */
        if ((starmagmax - (starmagmin + magminoffset)) > 1.0) {
          starmagmin += magminoffset;
        }

        VPRINTF(NORMAL,
    "I propose the following range for stars: %f < %s < %f and %f < %s < %f\n",
                starsizemin, keynames[0], starsizemax,
                starmagmin, keynames[1], starmagmax);

        foundlocus = 1;
      } else {
        VPRINTF(VERBOSE, "Found no locus with stepsize %f\n\n", stepsize);
        stepsize += deltastep;
        step     += 1;
      }


      ED_FREE(objbin);
      ED_FREE(nobj);
      ED_FREE(grad);
      ED_FREE(contr);
    }
  } else {
    printf("No objects left after preselection.\n");
  }

  /* release memory and bye. The memory for the keys is already freed
     by the free_cat command
   */

  /*printf("freeing cats\n");*/

  free_cat(incat, 1);

  if (nconditions > 0) {
    ED_FREE(condmin);
    ED_FREE(condmax);
  }

  for (i = 0; i < nkeys; i++) {
    /* ED_FREE(keys[i]); */
    ED_FREE(keynames[i]);
  }

  ED_FREE(keynames);
  /* ED_FREE(keys); */

  ED_FREE(goodsize);
  ED_FREE(goodmag);

  return(0);
}
