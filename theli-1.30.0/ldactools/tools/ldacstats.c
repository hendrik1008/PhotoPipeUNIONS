#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sort_globals.h"
#include "utils_globals.h"
#include "options_globals.h"

/* 21.01.00: first version calculating mean and
 *           standard deviation of an LDAC key
 * 22.01.00: included weighted means, sigmas
 *
 * 24.01.00: included uncertainty of the mean;
 *           corrected calculation of no weighted
 *           sigma
 *
 * 28.01.01: modifications
 *           - ability to calculate statistics from more
 *             than one quantity
 *           - ability to reject high and low extreme values
 *             for the calculations
 *
 * 23.02.01: substituted all "long int" by the LONG macro.
 *
 * 03.04.2001: corrected an error in the calculation of sigma
 *             (an integer has to be casted to a float in order to
 *              result in a float rather than an integer number;
 *              discovered by Hanne)
 *
 * 07.05.2002: corrected a bug introduced with the changes
 *             on 28.01.01: when sorting the data points (in
 *             order to easily reject extreme values) the weights
 *             have to be sorted as well
 *
 * 19.10.2004: removed the superfluous malloc.h inclusion
 *
 * 04.01.05: removed compiler warnings (gcc with -O3 optimisation)
 *
 * 27.03.2006:
 * I renamed sorting and indexing functions to the new naming scheme
 * described in the sort_globals.h file
 *
 * 27.11.2008:
 * statistical quantities are now estimated with double precision
 */

#define __PROGRAMMVERSION__    "1.2"
#define __PROGRAMMNAME__       "ldacstats"

#define __USAGE__              "\n\
USAGE:\n\
       ldacstats2 -i input catalog\n\
                 -k statistics key\n\
               [ -c conditions ]\n\
               [ -e number ]\n\
               [ -w weight ]\n\
               [ -r number of objects; randomications ]\n\
               [ -t table with the objects (OBJECTS) ]\n\n\
DESCRIPTION:\n\
The program 'ldacstats' calculates statistics for\n\
one or more keys (-k) in table (-t). The program gives mean, sigma\n\
and uncertainty of the mean. If a weightkey (-w) is\n\
specified all objects are weighted with the corresponding\n\
values. By calculating the sigmas and means of uncertainties from\n\
a weighted quantity it is assumed that the values originate from\n\
the same probability distribution. \n\
With (-r) a bootstrap analysis can be done.\n\
\"Numer of objects\" objects are taken randomly out of\n\
the statistics key and calculations are done with this\n\
subsample. The procedure is repeated \"randomications\"\n\
times. With (-c) a preselection of objects can be done.\n\
All results are written to the screen.\n\
One can also calculate the stats without using the (-e) N highest\n\
and M lowest values. In the case of bootstrapping this means that\n\
the N highest and M lowest values of the ORIGINAL array are not\n\
considered as valid randomisation points.\n\n\
EXAMPLE:\n\
  ldacstats -i tmp.cat -t OBJECTS -k e1 -w weight -r 30 1000\n\
            -c snratio 2.0 1000.0 -e 2 4\n\
would take the e1 key of 30 random objects with a snratio\n\
between 2.0 and 1000.0, calculate a weighted mean, weighted\n\
standard deviation and weighted mean uncertainty and repeat\n\
this 1000 times. Hereby the 2 lowest and 4 highest values\n\
would not enter the calculations.\n\n\
AUTHORS: Thomas Erben & Hannelore Haemmerle\n"

void makestats(char *akeyname,
               float *statskeyname,
               int anobjects,
               float *aweights,
               int adoweight,
               int *arandarray);

int intrand(LONG *aidum, int astart, int aend);

float ran2(LONG *idum);

#define MAXSTRINGLENGTH    256
#define EPSILON            1e-06

int main(int argc, char **argv)
{
  char   incatname[MAXSTRINGLENGTH];
  char   tablename[MAXSTRINGLENGTH]     = "OBJECTS";
  char   statskeyname[MAXSTRINGLENGTH]  = "";
  char   weightkeyname[MAXSTRINGLENGTH] = "";
  int    nobjects;
  char **condnames = NULL;
  float *condmin   = NULL, *condmax = NULL;
  float *goodobjects;
  float *weights = NULL;
  int *  randobjects;
  int    startconditions = 0;
  int    startkeys       = 0;
  int    nkeys           = 0;
  int    nconditions     = 0;
  int    ngoodobjects    = 0;
  int    nrandobjects    = 0;
  int    nextremmin      = 0;
  int    nextremmax      = 0;
  int    objectgood;
  int    doweight = 0;
  int    dorand   = 0;
  int    nrands   = 0;
  LONG   idum     = -671;

  catstruct * incat;
  tabstruct * tab;
  keystruct * statskey, *weightkey = NULL;
  keystruct **conditionskey = NULL;

  int  i, j, k;
  char verbosity[MAXSTRINGLENGTH] = "NORMAL";

  /* print start message: */
  printf("\n %s  V%s (%s;%s)\n\n", __PROGRAMMNAME__,
         __PROGRAMMVERSION__, __DATE__, __TIME__);
  if (argc < 5) {
    printf("%s\n", __USAGE__);
    exit(1);
  }

  /* get the command line arguments: */
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (tolower((unsigned char)argv[i][1])) {
      case 'i': strncpy(incatname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 't': strncpy(tablename, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'k': startkeys = i + 1;
        while ((i + 1 < argc) &&
               !(argv[i + 1][0] == '-' && strlen(argv[i + 1]) == 2)) {
          i++;
          nkeys++;
        }
        break;
      case 'w': strncpy(weightkeyname, argv[++i], MAXSTRINGLENGTH - 1);
        doweight = 1;
        break;
      case 'r': nrandobjects = atoi(argv[++i]);
        nrands = atoi(argv[++i]);
        dorand = 1;
        break;
      case 'e': nextremmin = atoi(argv[++i]);
        nextremmax         = atoi(argv[++i]);
        break;
      case 'c': startconditions = i + 1;
        while ((i + 1 < argc) &&
               !(argv[i + 1][0] == '-' && strlen(argv[i + 1]) == 2)) {
          i++;
          nconditions++;
        }
        break;
      case 'v': strncpy(verbosity, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      default:
        break;
      }
    } else {
      syserr(FATAL, "ldacstats", "Wrong Command line syntax\n");
    }
  }

  altverbose(verbosity);

  if ((nconditions % 3) != 0) {
    syserr(FATAL, "ldacstats", "Error in conditions");
  }

  nconditions /= 3;

  if (nconditions > 0) {
    ED_MALLOC(condnames, "ldacstats", char *, nconditions);
  }

  for (i = 0; i < nconditions; i++) {
    ED_MALLOC(condnames[i], "ldacstats", char, MAXSTRINGLENGTH);
  }

  if (nconditions > 0) {
    ED_MALLOC(condmin, "ldacstats", float, nconditions);
    ED_MALLOC(condmax, "ldacstats", float, nconditions);
    ED_MALLOC(conditionskey, "ldacstats", keystruct *, nconditions);
  }

  for (i = 0; i < nconditions; i++) {
    strncpy(condnames[i], argv[startconditions + i * 3], MAXSTRINGLENGTH - 1);
    condmin[i] = atof(argv[startconditions + i * 3 + 1]);
    condmax[i] = atof(argv[startconditions + i * 3 + 2]);
  }

  if (!(incat = read_cat(incatname))) {
    syserr(FATAL, "ldacstats", "Error opening input catalog\n");
  }

  if (!(tab = name_to_tab(incat, tablename, 0))) {
    syserr(FATAL, "ldacstats", "No %s table in catalog\n", tablename);
  }

  if (doweight == 1) {
    weightkey = read_key(tab, weightkeyname);
    if (weightkey == NULL) {
      syserr(FATAL, "ldacstats", "No %s key in catalog\n", weightkeyname);
    }
  }

  for (i = 0; i < nconditions; i++) {
    conditionskey[i] = read_key(tab, condnames[i]);
    if (conditionskey[i] == NULL) {
      syserr(FATAL, "ldacstats", "No %s key in catalog\n", condnames[i]);
    }
  }

  for (k = 0; k < nkeys; k++) {
    ngoodobjects = 0;
    strncpy(statskeyname, argv[startkeys + k], MAXSTRINGLENGTH - 1);
    statskey = read_key(tab, statskeyname);
    if (statskey == NULL) {
      syserr(FATAL, "ldacstats", "No %s key in catalog\n", statskeyname);
    }
    VPRINTF(DEBUG, "\n input key: %d %s\n", k, statskeyname);

    nobjects = statskey->nobj;
    ED_MALLOC(goodobjects, "ldacstats", float, nobjects);
    ED_MALLOC(randobjects, "ldacstats", int, nobjects);

    if (doweight == 1) {
      ED_MALLOC(weights, "ldacstats", float, nobjects);
    }

    for (i = 0; i < nobjects; i++) {
      objectgood = 0;
      for (j = 0; j < nconditions; j++) {
        switch (conditionskey[j]->ttype) {
        case T_SHORT:
          if ((float)((short int *)conditionskey[j]->ptr)[i] < condmin[j] ||
              (float)((short int *)conditionskey[j]->ptr)[i] > condmax[j]) {
            objectgood = 1;
          }
          break;
        case T_LONG:
          if ((float)((LONG *)conditionskey[j]->ptr)[i] < condmin[j] ||
              (float)((LONG *)conditionskey[j]->ptr)[i] > condmax[j]) {
            objectgood = 1;
          }
          break;
        case T_DOUBLE:
          if ((float)((double *)conditionskey[j]->ptr)[i] < condmin[j] ||
              (float)((double *)conditionskey[j]->ptr)[i] > condmax[j]) {
            objectgood = 1;
          }
          break;
        case T_FLOAT:
          if ((float)((float *)conditionskey[j]->ptr)[i] < condmin[j] ||
              (float)((float *)conditionskey[j]->ptr)[i] > condmax[j]) {
            objectgood = 1;
          }
          break;
        default:
          syserr(FATAL, "ldacstats", "Only float's and float's supported");
          break;
        }
      }
      if (objectgood == 0) {
        switch (statskey->ttype) {
        case T_SHORT:
          goodobjects[ngoodobjects] = (float)(((short int *)statskey->ptr)[i]);
          break;
        case T_LONG:
          goodobjects[ngoodobjects] = (float)(((LONG *)statskey->ptr)[i]);
          break;
        case T_DOUBLE:
          goodobjects[ngoodobjects] = (float)(((double *)statskey->ptr)[i]);
          break;
        case T_FLOAT:
          goodobjects[ngoodobjects] = (float)(((float *)statskey->ptr)[i]);
          break;
        default:
          syserr(FATAL, "ldacstats", "Only float's and float's supported");
          break;
        }
        if (doweight == 1) {
          switch (weightkey->ttype) {
          case T_SHORT:
            weights[ngoodobjects] = (float)(((short int *)weightkey->ptr)[i]);
            break;
          case T_LONG:
            weights[ngoodobjects] = (float)(((LONG *)weightkey->ptr)[i]);
            break;
          case T_DOUBLE:
            weights[ngoodobjects] = (float)(((double *)weightkey->ptr)[i]);
            break;
          case T_FLOAT:
            weights[ngoodobjects] = (float)(((float *)weightkey->ptr)[i]);
            break;
          default:
            syserr(FATAL, "ldacstats", "Only float's and float's supported");
            break;
          }
        }

        ngoodobjects++;
      }
    }

    if ((nextremmin + nextremmax) >= ngoodobjects) {
      syserr(FATAL, "ldacstats",
    "not enough objects after condition preselection and extrema rejection\n");
    }

    if (k == 0) {
      printf("I calculate statistics from %d good points\n",
             ngoodobjects);
    }

    if (doweight == 1) {
      ffsort(ngoodobjects, goodobjects, weights);
    } else {
      fsort(ngoodobjects, goodobjects);
    }
    if (ngoodobjects > 1) {
      if (dorand == 1) {
        for (j = 0; j < nrands; j++) {
          for (i = 0; i < nrandobjects; i++) {
            randobjects[i] = intrand(&idum, 0 + nextremmin,
                                     ngoodobjects - 1 - nextremmax);
          }

          makestats(statskeyname, goodobjects, nrandobjects,
                    weights, doweight, randobjects);
        }
      } else {
        for (i = 0; i < ngoodobjects; i++) {
          randobjects[i] = i;
        }
        makestats(statskeyname, goodobjects + nextremmin,
                  ngoodobjects - (nextremmin + nextremmax),
                  weights, doweight, randobjects);
      }
    }

    printf("\n");

    ED_FREE(goodobjects);
    ED_FREE(randobjects);
    if (doweight == 1) {
      ED_FREE(weights);
    }
  }

  /* release memory and bye */
  free_cat(incat, 1);

  if (nconditions > 0) {
    ED_FREE(condmin);
    ED_FREE(condmax);
    for (i = 0; i < nconditions; i++) {
      ED_FREE(condnames[i]);
    }
    ED_FREE(condnames);
    ED_FREE(conditionskey);
  }

  return(0);
}

/*
   calculate statistics and print them:
   For the moment mean and sigma
 */

void makestats(char *akeyname, float *agoodobjects, int angoodobjects,
               float *aweights, int doweight, int *arandarray)
{
  int    i;
  double mean      = 0.0, sigma = 0.0, meanuncertainty = 0.0;
  double weightsum = 0.0, weightsqsum = 0.0;

  if (doweight == 0) {
    for (i = 0; i < angoodobjects; i++) {
      mean  += agoodobjects[arandarray[i]];
      sigma += agoodobjects[arandarray[i]] * agoodobjects[arandarray[i]];
    }

    mean /= angoodobjects;
    sigma = sqrt(sigma / (angoodobjects - 1)
                 - (mean * mean) *
                 ((double)angoodobjects / ((double)angoodobjects - 1)));

    meanuncertainty = sigma / sqrt(angoodobjects);
  } else {
    for (i = 0; i < angoodobjects; i++) {
      mean        += aweights[arandarray[i]] * agoodobjects[arandarray[i]];
      sigma       += aweights[arandarray[i]] *
                     agoodobjects[arandarray[i]] * agoodobjects[arandarray[i]];
      weightsum   += aweights[arandarray[i]];
      weightsqsum += aweights[arandarray[i]] * aweights[arandarray[i]];
    }

    mean /= weightsum;
    sigma = sqrt((weightsum / (weightsum * weightsum - weightsqsum)) *
                 (sigma - weightsum * mean * mean));
    meanuncertainty = sigma * sqrt(weightsqsum) / weightsum;
  }

  printf("key %s: mean: %lf sigma: %lf meanuncertainty: %lf\n",
         akeyname, mean, sigma, meanuncertainty);
}

int intrand(LONG *aidum, int astart, int aend)
{
  return((int)(ran2(aidum) * (aend - astart) + astart));
}

#define IM1     2147483563
#define IM2     2147483399
#define AM      (1.0 / IM1)
#define IMM1    (IM1 - 1)
#define IA1     40014
#define IA2     40692
#define IQ1     53668
#define IQ2     52774
#define IR1     12211
#define IR2     3791
#define NTAB    32
#define NDIV    (1 + IMM1 / NTAB)
#define EPS     1.2e-7
#define RNMX    (1.0 - EPS)

float ran2(LONG *idum)
{
  int         j;
  LONG        k;
  static LONG idum2 = 123456789;
  static LONG iy    = 0;
  static LONG iv[NTAB];
  float       temp;

  if (*idum <= 0) {
    if (-(*idum) < 1) {
      *idum = 1;
    } else { *idum = -(*idum); }
    idum2 = (*idum);
    for (j = NTAB + 7; j >= 0; j--) {
      k     = (*idum) / IQ1;
      *idum = IA1 * (*idum - k * IQ1) - k * IR1;
      if (*idum < 0) {
        *idum += IM1;
      }
      if (j < NTAB) {
        iv[j] = *idum;
      }
    }
    iy = iv[0];
  }
  k     = (*idum) / IQ1;
  *idum = IA1 * (*idum - k * IQ1) - k * IR1;
  if (*idum < 0) {
    *idum += IM1;
  }
  k     = idum2 / IQ2;
  idum2 = IA2 * (idum2 - k * IQ2) - k * IR2;
  if (idum2 < 0) {
    idum2 += IM2;
  }
  j     = iy / NDIV;
  iy    = iv[j] - idum2;
  iv[j] = *idum;
  if (iy < 1) {
    iy += IMM1;
  }
  if ((temp = AM * iy) > RNMX) {
    return(RNMX);
  } else { return(temp); }
}

#undef IM1
#undef IM2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX
