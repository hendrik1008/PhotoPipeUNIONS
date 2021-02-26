/* The program 'anisotropy' fits a two dimensional polynomial
   to the ellipticities of stars. It is the successor of the
   former 'ksb_seeingframes' program. This program now works on
   the basis of a LDAC catalog.
   It first fits e1 doing a given number of iterations and throwing
   away objects above a certain threshhold. With the remaining stars
   it does the same procedure with e2. From the final set, e1 and e2
   are fittet finally. Also e1/Psm and e2/Psm are fittet. (It is
   configurable whether for Psm the single Psm11 and Psm22 components
   are taken or whether the mean of this is used.
   All information is stored in a 'STARS' table.
   The program also make the anisotropy correction and stores the
   result as e?anisocorr (?=1,2).

   The plots that were done by the ksb_seeingframes program are done now
   by the check_anisotropy program.


   24.10.: there were stars that had Psm's of 0. These are now
           sorted out in the preselection phase

   03.12.: corrected the bug that maxmag was saved to the STARS
           table instead the maxellip.

   16.12.: corrected a bug with maxellipsq; it was not set correctly and
           was zero always. (prob. introduced in the last modification);
           changed the type from cl from short to long.

   25.02.99: changed cl to short to reconcile with analyseldac.

   14.04.1999: included full tensor anisotropy correction

   19.11.00: rescaled the calculated polynomials to ar range x\in [-1,1]
             and y\in [-1,1]

   27.11.00: introduced magkeyname and rhkeyname as variables


   04.12.00: completely reworte the code:
             - now, arbitrary conditions (number) can be put on the
               star preselection
             - the rejection is now done on (e1/sigma1)**2+(e1/sigma1)**2
               instead of on each component
             - the fitting is now done with gauss jordan elimintation
               instead of powell.

   08.12.00: the algorithm for the final sample of stars is now as follows:
             - do a fit
             - calculate the sigma for all the stars in the fit
             - reject stars from the WHOLE initial sample of stars with
               the sigma and the polynomials calculated above
             - repeat the procedure until the sigmas do not change anymore
             (with this procedure we should not bias ourselves against
              a result rejecting outliers).

   09.03.01: made the variables last and lastgood int from short int.
             The program is unstable if there are more objects in
             the catalog than an int can hold (To be examined).

   22.05.2001: replaced all doubles by floats. The accuracy is sufficient,
               the program needs less memory and the type is consistent
               with SExtractor.
               The values of the polynomial anisotropy correction are now also
               saved (keys e1corrpol, e2corrpol)

   05.05.03:
   included the tsize.h file

   19.10.04:
   removed the superfluous inclusion of malloc.h

   10.01.05:
   I removed compiler warnings under gcc with optimisation
   enabled (-O3).

   16.01.2005:
   corrected a bug in the condition for a while statement
   (why did a condition like a < b > c work up to now ???)

   29.01.2005:
   I now add HISTORY information to the output catalog.

   01.02.2010:
   I rewrote the code to use an LU module, instead of the NR gaussj
   for solving linear equations.

   30.09.2010:
   code beautifying
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include "fitscat.h"
#include "tsize.h"
#include "LU_globals.h"
#include "utils_globals.h"
#include "options_globals.h"
#include "tabutil_globals.h"

#ifndef MAXSTRINGLENGTH
#define MAXSTRINGLENGTH        256
#endif

#define __PROGRAMMVERSION__ "1.3.0"
#define __PROGRAMMNAME__ "anisotropy"

void printUsage(void);

void  build_equations(float **acov, float **ab,
                      float *ax, float *ay,
                      float *ae1, float *ae2,
                      int *ainext, int aistart, int anobjects,
                      int adegree);

float evaluatepolynomial(const float *aa, int an,
                         float ax, float ay);

void printmatrix(float **, int, int);

int main(int argc, char **argv)
{
  int nstars = 0; /* the number of preselected stars */
                  /* (preselected from the conditions) */
  int nobjects;   /* the number of objects in the input cat. */

  /* variables to configure the fitting process: */
  char    **keynames;
  float    *condmin, *condmax;
  int       startconditions = 0;
  int       nkeys           = 0;
  int       nconditions     = 0;
  short int degree          = 2; /* the degree of polynomials (default) */
  short int trace           = 1; /* use trace for Psm components (default) */
  short int iterations      = 2; /* the number of iterations for the fitting */
                                 /* (objects are rejected iterations-1 times) */
                                 /* (default) */
  short int iterationsdone;
  float     threshhold = 16.0;   /* the sigma thresshold for object rejection */
                                 /* (default) */
  float maxellip = 1.0;          /* the maximum ellipticity up to that stars */
                                 /* are included in the fitting process */
                                 /* (default) */
  double    maxellipsq = 1.0;
  int       coeffs;
  int       offset           = 0;
  short int tensorcorrection = 0;
  float   **cov; /* the matrix first containing elements of the normaleq.
                  and later
                  the covariance matrix of the fit */
  float   **b; /* the right hand sides of the normaleq. */
  float   **bpsm;
  int      *pivot;
  float     sigmae1 = 0.0, sigmae2 = 0.0;
  float     sigmae1old, sigmae2old;
  int       pointer;
  float     deviation;
  short int flag = 0;
  int       last = 0, lastgood = 0;
  short int nrejected;

  /* stuff for reading the keys/tabs from the catalogs */
  catstruct  *inKSBcat, *outKSBcat;
  keystruct **keys;
  tabstruct  *tab, *starstab;
  float      *x, *y, *e1, *e2, **cond;
  float      *Psh11, *Psh22, *Psm11, *Psm22, *Psm12, *Psh12;
  float      *Psm21, *Psh21;
  float      *e1Psm, *e2Psm;
  float     **allkeys;
  short int  *cl   = NULL;
  float       xmax = -10000.0, ymax = -10000.0;
  float       xmin = 10000.0, ymin = 10000.0;

  /* other stuff: */
  int    i, j;
  char   tmpstring1[MAXSTRINGLENGTH];
  char   tmpstring2[MAXSTRINGLENGTH];
  char   incatname[MAXSTRINGLENGTH];
  char   outcatname[MAXSTRINGLENGTH];
  char   tablename[MAXSTRINGLENGTH]   = "OBJECTS";
  char   xposkeyname[MAXSTRINGLENGTH] = "Xpos";
  char   yposkeyname[MAXSTRINGLENGTH] = "Ypos";
  char   verbosity[MAXSTRINGLENGTH]   = "NORMAL";
  int   *inext, *inextcopy;          /* stuff for the linked lists */
  int    istart = 0, istartcopy = 0; /*           " */
  int    ilast  = 0;                 /*           " */
  int    goodobject;                 /* does the object fulfill the star
                                        conditions */
  float  Psm11mean = 0.0, Psm22mean = 0.0;
  float  Psh11mean = 0.0, Psh22mean = 0.0;
  float  Psm12mean = 0.0, Psh12mean = 0.0;
  float  Psm21mean = 0.0, Psh21mean = 0.0;
  float *e1acorr, *e2acorr; /* the anisotropy corrected ellipticities. */
  float *e1acorrpol, *e2acorrpol; /* the anisotropy polynomial values for
                                     all the objects. */
  float *PshstPsmst; /* the factor Psh* Psm*, needed for the isotropy
                        correction. In the moment this is calculated by
                        the traces. Maybe in the future we will have to
                        substitute this by a fit. */
  float dummy;


  if (argc < 2) {
    printUsage();
    exit(1);
  }

  VPRINTF(NONE, "\n %s  V %s (%s; %s)\n\n\n",
          __PROGRAMMNAME__, __PROGRAMMVERSION__, __DATE__, __TIME__);

  /* get the command line arguments: */
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (tolower((int)argv[i][1])) {
      case 'c': startconditions = i + 1;
        while (i + 1 < argc &&
               !(argv[i + 1][0] == '-' && strlen(argv[i + 1]) == 2)) {
          i++;
          nconditions++;
        }
        break;
      case 'e': degree = atoi(argv[++i]);
        break;
      case 'f': trace = atoi(argv[++i]);
        break;
      case 'g': iterations = atoi(argv[++i]);
        break;
      case 'h': maxellip = atof(argv[++i]);
        maxellipsq       = maxellip * maxellip;
        break;
      case 'j': threshhold = atof(argv[++i]);
        break;
      case 'k': tensorcorrection = 1;
        break;
      case 'i': strncpy(incatname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'o': strncpy(outcatname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 't': strncpy(tablename, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'x': strncpy(xposkeyname, argv[++i], MAXSTRINGLENGTH - 1);
        strncpy(yposkeyname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'v': strncpy(verbosity, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      default:
        break;
      }
    } else {
      syserr(FATAL, "anisotropy", "Wrong Command line syntax\n");
    }
  }

  altverbose(verbosity);

  coeffs = (degree + 2) * (degree + 1) / 2;

  if ((nconditions % 3) != 0) {
    syserr(FATAL, "anisotropy", "Error in conditions\n");
  }

  nconditions /= 3;

  VPRINTF(DEBUG, "number of conditions: %d\n", nconditions);

  if (nconditions < 1) {
    syserr(FATAL, "anisotropy", "No conditions given !!\n");
  }

  nkeys = 13 + nconditions;

  /* the first 11 keys are:
     - x and y
     - e1 and e2
     - Psh11, Psh22, Psh12, Psh21
     - Psm11, Psm22, Psm12, Psm21
     - cl
   */

  /* open catatlogs and initialice the output cat */

  inKSBcat = read_cat(incatname);
  if (inKSBcat == NULL) {
    syserr(FATAL, "anisotropy", "Error opening input catalog\n");
  }

  VPRINTF(DEBUG, "read input catalog\n");

  outKSBcat = new_cat(1);
  inherit_cat(inKSBcat, outKSBcat);
  copy_tabs(inKSBcat, outKSBcat);

  tab = name_to_tab(outKSBcat, tablename, 0);

  if (tab == NULL) {
    syserr(FATAL, "anisotropy", "No %s table in catalog\n", tablename);
  }

  ED_MALLOC(keynames, "anisotropy", char *, nkeys);

  for (i = 0; i < nkeys; i++) {
    ED_MALLOC(keynames[i], "preanisotropy", char, MAXSTRINGLENGTH);
  }

  ED_MALLOC(allkeys, "anisotropy", float *, nkeys);

  ED_MALLOC(condmin, "anisotropy", float, nconditions);
  ED_MALLOC(condmax, "anisotropy", float, nconditions);
  ED_MALLOC(cond, "anisotropy", float *, nconditions);


  ED_MALLOC(keys, "anisotropy", keystruct *, nkeys);

  strncpy(keynames[0], xposkeyname, MAXSTRINGLENGTH - 1);
  strncpy(keynames[1], yposkeyname, MAXSTRINGLENGTH - 1);
  strncpy(keynames[2], "e1", MAXSTRINGLENGTH - 1);
  strncpy(keynames[3], "e2", MAXSTRINGLENGTH - 1);
  strncpy(keynames[4], "Psh11", MAXSTRINGLENGTH - 1);
  strncpy(keynames[5], "Psh22", MAXSTRINGLENGTH - 1);
  strncpy(keynames[6], "Psh12", MAXSTRINGLENGTH - 1);
  strncpy(keynames[7], "Psh21", MAXSTRINGLENGTH - 1);
  strncpy(keynames[8], "Psm11", MAXSTRINGLENGTH - 1);
  strncpy(keynames[9], "Psm22", MAXSTRINGLENGTH - 1);
  strncpy(keynames[10], "Psm12", MAXSTRINGLENGTH - 1);
  strncpy(keynames[11], "Psm21", MAXSTRINGLENGTH - 1);
  strncpy(keynames[12], "cl", MAXSTRINGLENGTH - 1);

  for (i = 0; i < nconditions; i++) {
    strncpy(keynames[13 + i], argv[startconditions + i * 3],
            (MAXSTRINGLENGTH - 1));
    condmin[i] = atof(argv[startconditions + i * 3 + 1]);
    condmax[i] = atof(argv[startconditions + i * 3 + 2]);

    VPRINTF(DEBUG, "%s: %f - %f\n", keynames[13 + i], condmin[i], condmax[i]);
  }

  /* read in the keys */
  for (i = 0; i < nkeys; i++) {
    keys[i] = read_key(tab, keynames[i]);
    if (keys[i] == NULL) {
      syserr(FATAL, "anisotropy", "No %s key in catalog\n", keynames[i]);
    }
  }

  VPRINTF(DEBUG, "input keys read\n");

  nobjects = keys[0]->nobj;

  VPRINTF(DEBUG, "input objects: %d\n", nobjects);

  for (j = 0; j < nkeys; j++) {
    /*
       with all keys except the cl key we work with
       float precision; as we change the cl key we work
       with key[12] in this case and not with a symbolic
       name
     */
    if (j != 12) {
      switch (keys[j]->ttype) {
      case T_SHORT:
        ED_GETVEC(allkeys[j], "anisotropy", keys[j], keys[j]->name,
                  float, short int);
        break;
      case T_LONG:
        ED_GETVEC(allkeys[j], "anisotropy", keys[j], keys[j]->name,
                  float, LONG);
        break;
      case T_DOUBLE:
        ED_GETVEC(allkeys[j], "anisotropy", keys[j], keys[j]->name,
                  float, double);
        break;
      case T_FLOAT:
        ED_GETVEC(allkeys[j], "anisotropy", keys[j], keys[j]->name,
                  float, float);
        break;
      default:
        syserr(FATAL, "anisotropy", "Unsupported key type");
        break;
      }
    }
  }

  x     = allkeys[0];
  y     = allkeys[1];
  e1    = allkeys[2];
  e2    = allkeys[3];
  Psh11 = allkeys[4];
  Psh22 = allkeys[5];
  Psh12 = allkeys[6];
  Psh21 = allkeys[7];
  Psm11 = allkeys[8];
  Psm22 = allkeys[9];
  Psm12 = allkeys[10];
  Psm21 = allkeys[11];

  if (keys[12]->ttype != T_SHORT) {
    syserr(FATAL, "anisotropy", "The key %s has to be of type short",
           keys[12]->name);
  } else {
    cl = (short int *)keys[12]->ptr;
  }

  for (i = 0; i < nconditions; i++) {
    cond[i] = allkeys[13 + i];
  }

  /* populate the inext array for the linked list  */
  ED_MALLOC(inext, "anisotropy", int, nobjects);
  ED_MALLOC(inextcopy, "anisotropy", int, nobjects);

  /* preselect stars; give these objects a cl of 1 */
  for (i = 0; i < nobjects; i++) {
    goodobject = 1;

    for (j = 0; j < nconditions; j++) {
      if (cond[j][i] < condmin[j] || cond[j][i] > condmax[j]) {
        goodobject = 0;
      }
    }

    if (goodobject == 1) {
      cl[i] = 1;
      if (((e1[i] * e1[i] + e2[i] * e2[i]) <= maxellipsq) &&
          Psm11[i] > 1.0e-08 && Psm22[i] > 1.0e-08) {
        if (nstars == 0) {
          istart     = i;
          istartcopy = i;
        } else {
          inext[ilast]     = i;
          inextcopy[ilast] = i;
        }
        ilast = i;
        nstars++;
      }
    }
  }

  VPRINTF(DEBUG, "stars preselected\n");

  if (nstars < coeffs) {
    syserr(FATAL, "anisotropy", "not enough stars after preselection");
  }

  /* determine min and max of x and y and rescale the position quantities */
  for (i = 0; i < nobjects; i++) {
    xmax = (xmax > x[i]) ? xmax : x[i];
    xmin = (xmin < x[i]) ? xmin : x[i];
    ymax = (ymax > y[i]) ? ymax : y[i];
    ymin = (ymin < y[i]) ? ymin : y[i];
  }

  VPRINTF(DEBUG, "xmin: %f; xmax: %f\n", xmin, xmax);
  VPRINTF(DEBUG, "ymin: %f; ymax: %f\n", ymin, ymax);
  VPRINTF(NORMAL, "preselected stars: %d\n\n", nstars);

  for (i = 0; i < nobjects; i++) {
    x[i] = ((x[i] - xmin) / (xmax - xmin)) - 0.5;
    y[i] = ((y[i] - ymin) / (ymax - ymin)) - 0.5;
  }

  /* prepare things for the fits */
  ED_MALLOC(cov, "anisotropy", float *, coeffs);
  for (i = 0; i < coeffs; i++) {
    ED_MALLOC(cov[i], "anisotropy", float, coeffs);
  }

  ED_MALLOC(pivot, "anisotropy", int, coeffs);

  /* the right hand sides consist of e1 and e2 */
  ED_MALLOC(b, "anisotropy", float *, 2);
  ED_MALLOC(b[0], "anisotropy", float, coeffs);
  ED_MALLOC(b[1], "anisotropy", float, coeffs);
  ED_MALLOC(bpsm, "anisotropy", float *, 2);
  ED_MALLOC(bpsm[0], "anisotropy", float, coeffs);
  ED_MALLOC(bpsm[1], "anisotropy", float, coeffs);

  build_equations(cov, b, x, y, e1, e2, inext, istart, nstars, degree);

  if (strcmp(verbosity, "DEBUG") == 0) {
    printmatrix(cov, coeffs, coeffs);
    printmatrix(b, 2, coeffs);
  }

  LUfactor(cov, coeffs, pivot);
  LUsolve(cov, coeffs, pivot, b[0], NULL);
  LUsolve(cov, coeffs, pivot, b[1], NULL);

  if (strcmp(verbosity, "DEBUG") == 0) {
    printmatrix(b, 2, coeffs);
  }

  nrejected      = 0;
  iterationsdone = 0;
  sigmae1old     = sigmae2old = 1.0; /* dummy to enter the following loop */

  while (iterationsdone < iterations &&
         ((fabs(sigmae1 - sigmae1old) > 1.0e-04) &&
          (fabs(sigmae2 - sigmae2old) > 1.0e-04))) {
    /* calculate the sigmas of the fit just done */
    sigmae1old = sigmae1;
    sigmae2old = sigmae2;
    sigmae1 = sigmae2 = 0.0;

    pointer = istart;

    for (i = 0; i < nstars - nrejected; i++) {
      sigmae1 += pow(e1[pointer] -
                     evaluatepolynomial(b[0], degree,
                                        x[pointer],
                                        y[pointer]), 2);
      sigmae2 += pow(e2[pointer] -
                     evaluatepolynomial(b[1], degree,
                                        x[pointer],
                                        y[pointer]), 2);
      pointer = inext[pointer];
    }

    if ((nstars - nrejected - coeffs) > 0) {
      sigmae1 = sqrt(sigmae1 / (nstars - coeffs));
      sigmae2 = sqrt(sigmae2 / (nstars - coeffs));
      VPRINTF(VERBOSE, "sigmae1: %f; sigmae2: %f\n", sigmae1, sigmae2);
    } else {
      syserr(FATAL, "anisotropy",
             "unable to calculate sigma of fit; to less stars left\n");
    }

    /* now reject objects */
    nrejected = 0;

    /* rebuild the inext array */
    pointer = istartcopy;
    for (i = 0; i < nstars; i++) {
      inext[pointer] = inextcopy[pointer];
      pointer = inextcopy[pointer];
    }

    istart = istartcopy;
    pointer = istart;

    for (i = 0; i < nstars; i++) {
      deviation =
        pow(e1[pointer] -
            evaluatepolynomial(b[0], degree, x[pointer], y[pointer]), 2) /
        (sigmae1 * sigmae1) +
        pow(e2[pointer] -
            evaluatepolynomial(b[1], degree, x[pointer], y[pointer]), 2) /
        (sigmae2 * sigmae2);

      if (deviation > threshhold) {
        /*VPRINTF(DEBUG, "outlier; deviation: %f\n", deviation);*/

        if (pointer == istart) {
          istart = inext[pointer];
          lastgood = istart;
        } else {
          if (flag == 0) {
            inext[last] = inext[pointer];
            lastgood = last;
          } else {
            inext[lastgood] = inext[pointer];
          }
        }
        nrejected++;
        flag = 1;
      } else {
        flag = 0;
      }
      last = pointer;
      pointer = inext[pointer];
    }

    VPRINTF(VERBOSE, "stars rejected: %d\n", nrejected);
    /*nstars-=nrejected;*/

    /* now do the next fit */
    build_equations(cov, b, x, y, e1, e2, inext, istart,
                    nstars - nrejected, degree);
    LUfactor(cov, coeffs, pivot);
    LUsolve(cov, coeffs, pivot, b[0], NULL);
    LUsolve(cov, coeffs, pivot, b[1], NULL);

    VPRINTF(NORMAL, "%d iterations done\n", iterationsdone + 1);

    if (strcmp(verbosity, "DEBUG") == 0) {
      printf("result for e1, e2\n");
      printmatrix(b, 2, coeffs);
    }

    iterationsdone++;
  }

  VPRINTF(NORMAL, "final result for e1, e2:\n");
  printmatrix(b, 2, coeffs);

  VPRINTF(NORMAL, "\nfinal sigmae1, sigmae2: %f %f\n\n", sigmae1, sigmae2);

  VPRINTF(VERBOSE, "final covariance matrix:\n");

  if ((strcmp(verbosity, "DEBUG") == 0) ||
      (strcmp(verbosity, "VERBOSE") == 0)) {
    printmatrix(cov, coeffs, coeffs);
  }

  VPRINTF(NORMAL, "\nstars left in the final fit: %d\n\n\n",
          nstars - nrejected);

  /* now fit e1/Psm and e2/Psm */
  ED_MALLOC(e1Psm, "anisotropy", float, nobjects);
  ED_MALLOC(e2Psm, "anisotropy", float, nobjects);

  pointer = istart;

  for (i = 0; i < nstars - nrejected; i++) {
    if (trace == 1) {
      e1Psm[pointer] = 2. * e1[pointer] / (Psm11[pointer] + Psm22[pointer]);
      e2Psm[pointer] = 2. * e2[pointer] / (Psm11[pointer] + Psm22[pointer]);
    } else {
      e1Psm[pointer] = e1[pointer] / Psm11[pointer];
      e2Psm[pointer] = e2[pointer] / Psm22[pointer];
    }

    /* calculate the mean values of Psm11 and Psm22
       and give all the stars that have survived the
       fit a cl of 2
    */

    Psm11mean += Psm11[pointer];
    Psm22mean += Psm22[pointer];
    Psm12mean += Psm12[pointer];
    Psm21mean += Psm21[pointer];
    Psh11mean += Psh11[pointer];
    Psh22mean += Psh22[pointer];
    Psh12mean += Psh12[pointer];
    Psh21mean += Psh21[pointer];
    cl[pointer] = 2;

    pointer = inext[pointer];
  }

  Psm11mean /= (nstars - nrejected);
  Psm22mean /= (nstars - nrejected);
  Psm12mean /= (nstars - nrejected);
  Psm21mean /= (nstars - nrejected);
  Psh11mean /= (nstars - nrejected);
  Psh22mean /= (nstars - nrejected);
  Psh12mean /= (nstars - nrejected);
  Psh21mean /= (nstars - nrejected);

  VPRINTF(VERBOSE,
          "Psm11mean: %f; Psm22mean: %f; Psh11mean: %f; Psh22mean: %f\n",
          Psm11mean, Psm22mean, Psh11mean, Psh22mean);

  build_equations(cov, bpsm, x, y, e1Psm, e2Psm, inext, istart,
                  nstars - nrejected, degree);
  LUfactor(cov, coeffs, pivot);
  LUsolve(cov, coeffs, pivot, bpsm[0], NULL);
  LUsolve(cov, coeffs, pivot, bpsm[1], NULL);

  VPRINTF(NORMAL, "final result for e1/Psm, e2/Psm:\n");
  printmatrix(bpsm, 2, coeffs);
  printf("\n");

  /* now correct ellipticities for anisotropy and add them to
     the objects table: */

  ED_MALLOC(e1acorr, "anisotropy", float, nobjects);
  ED_MALLOC(e2acorr, "anisotropy", float, nobjects);
  ED_MALLOC(e1acorrpol, "anisotropy", float, nobjects);
  ED_MALLOC(e2acorrpol, "anisotropy", float, nobjects);
  ED_MALLOC(PshstPsmst, "anisotropy", float, nobjects);

  dummy = (Psh11mean + Psh22mean) / (Psm11mean + Psm22mean);

  for (i = 0; i < nobjects; i++) {
    e1acorrpol[i] = Psm11[i] * evaluatepolynomial(bpsm[0], degree, x[i], y[i]);
    e2acorrpol[i] = Psm22[i] * evaluatepolynomial(bpsm[1], degree, x[i], y[i]);

    if (tensorcorrection == 0) {
      e1acorr[i] = e1[i] - Psm11[i] * evaluatepolynomial(bpsm[0],
                                                         degree, x[i], y[i]);
      e2acorr[i] = e2[i] - Psm22[i] * evaluatepolynomial(bpsm[1],
                                                         degree, x[i], y[i]);
    } else {
      e1acorr[i] = e1[i] - Psm11[i] * evaluatepolynomial(bpsm[0],
                                                         degree, x[i], y[i])
        - Psm12[i] * evaluatepolynomial(bpsm[1],
                                        degree, x[i], y[i]);
      e2acorr[i] = e2[i] - Psm22[i] * evaluatepolynomial(bpsm[1],
                                                         degree, x[i], y[i])
        - Psm21[i] * evaluatepolynomial(bpsm[0],
                                        degree, x[i], y[i]);
    }
    PshstPsmst[i] = dummy;
  }

  add_key_to_tab(tab, "e1anisocorr", nobjects, e1acorr, T_FLOAT,
                 1, "", "anisotropy corrected e1");
  add_key_to_tab(tab, "e2anisocorr", nobjects, e2acorr, T_FLOAT,
                 1, "", "anisotropy corrected e2");
  add_key_to_tab(tab, "e1corrpol", nobjects, e1acorrpol, T_FLOAT,
                 1, "", "anisotropy polynomial e1");
  add_key_to_tab(tab, "e2corrpol", nobjects, e2acorrpol, T_FLOAT,
                 1, "", "anisotropy polynomial e2");
  add_key_to_tab(tab, "PshstPsmst", nobjects, PshstPsmst, T_FLOAT,
                 1, "", "Psh^*/Psm^* for isotropy correction");


  /* write the results of the fit to a new STARS table: */

  VPRINTF(NORMAL, "writing STARS table\n");

  starstab = new_tab("STARS");
  add_key_to_tab(starstab, "XMIN", 1, &(xmin), T_FLOAT,
                 1, "", "minimum x position of objects");
  add_key_to_tab(starstab, "XMAX", 1, &(xmax), T_FLOAT,
                 1, "", "maximum x position of objects");
  add_key_to_tab(starstab, "YMIN", 1, &(ymin), T_FLOAT,
                 1, "", "minimum y position of objects");
  add_key_to_tab(starstab, "YMAX", 1, &(ymax), T_FLOAT,
                 1, "", "maximum y position of objects");
  add_key_to_tab(starstab, "PSM11_MEAN", 1, &(Psm11mean), T_FLOAT,
                 1, "", "Mean of PSM11 from all stars");
  add_key_to_tab(starstab, "PSM22_MEAN", 1, &(Psm22mean), T_FLOAT,
                 1, "", "Mean of PSM11 from all stars");
  add_key_to_tab(starstab, "PSM12_MEAN", 1, &(Psm12mean), T_FLOAT,
                 1, "", "Mean of PSM12 from all stars");
  add_key_to_tab(starstab, "PSM21_MEAN", 1, &(Psm21mean), T_FLOAT,
                 1, "", "Mean of PSM21 from all stars");
  add_key_to_tab(starstab, "PSH11_MEAN", 1, &(Psh11mean), T_FLOAT,
                 1, "", "Mean of PSH11 from all stars");
  add_key_to_tab(starstab, "PSH22_MEAN", 1, &(Psh22mean), T_FLOAT,
                 1, "", "Mean of PSH22 from all stars");
  add_key_to_tab(starstab, "PSH12_MEAN", 1, &(Psh12mean), T_FLOAT,
                 1, "", "Mean of PSH12 from all stars");
  add_key_to_tab(starstab, "PSH21_MEAN", 1, &(Psh21mean), T_FLOAT,
                 1, "", "Mean of PSH21 from all stars");
  add_key_to_tab(starstab, "ELLIP_MAX", 1, &(maxellip), T_FLOAT,
                 1, "", "max ellipticity accepted in starselection");
  add_key_to_tab(starstab, "POL_DEGREE", 1, &(degree), T_SHORT,
                 1, "", "degree of fittet polynomials");
  add_key_to_tab(starstab, "POL_THRESHHOLD", 1, &(threshhold), T_FLOAT,
                 1, "", "threshhold to reject objects in fitting");
  add_key_to_tab(starstab, "POL_ITERATIONS", 1, &(iterations), T_SHORT,
                 1, "", "number of iterations for fitting");
  add_key_to_tab(starstab, "POLPSM_TRACE", 1, &(trace), T_SHORT,
                 1, "", "was traced used for e/psm fit (1=yes)");
  add_key_to_tab(starstab, "TENSOR", 1, &(tensorcorrection), T_SHORT,
                 1, "", "was full Psm tensor used for correction (1=yes)");

  for (i = 0; i < nconditions; i++) {
    sprintf(tmpstring1, "%s_MIN", keynames[13 + i]);
    sprintf(tmpstring2, "min %s for star selection", keynames[13 + i]);

    add_key_to_tab(starstab, tmpstring1, 1, &(condmin[i]), T_FLOAT,
                   1, "", tmpstring2);

    sprintf(tmpstring1, "%s_MAX", keynames[13 + i]);
    sprintf(tmpstring2, "max %s for star selection", keynames[13 + i]);

    add_key_to_tab(starstab, tmpstring1, 1, &(condmax[i]), T_FLOAT,
                   1, "", tmpstring2);
  }

  offset = 0;

  for (j = 0; j <= degree; j++) {
    for (i = 0; i <= j; i++) {
      sprintf(tmpstring1, "e1 pol.: (x-%.2f)^%d*(y-%.2f)^%d/(%.2f-%.2f)",
              xmin, i, ymin, j - i,
              (xmax - xmin), (ymax - ymin));
      sprintf(tmpstring2, "E1POL_%d", offset + i);
      add_key_to_tab(starstab, tmpstring2, 1,
                     &(b[0][(coeffs - 1) - (offset + i)]),
                     T_FLOAT, 1, "", tmpstring1);
      sprintf(tmpstring1, "e2 pol.: (x-%.2f)^%d*(y-%.2f)^%d/(%.2f-%.2f)",
              xmin, i, ymin, j - i,
              (xmax - xmin), (ymax - ymin));
      sprintf(tmpstring2, "E2POL_%d", offset + i);
      add_key_to_tab(starstab, tmpstring2, 1,
                     &(b[1][(coeffs - 1) - (offset + i)]),
                     T_FLOAT, 1, "", tmpstring1);
    }
    offset += j + 1;
  }

  offset = 0;
  for (j = 0; j <= degree; j++) {
    for (i = 0; i <= j; i++) {
      sprintf(tmpstring1, "e1Psm pol.: (x-%.2f)^%d*(y-%.2f)^%d/(%.2f-%.2f)",
              xmin, i, ymin, j - i,
              (xmax - xmin), (ymax - ymin));
      sprintf(tmpstring2, "E1PSMPOL_%d", offset + i);
      add_key_to_tab(starstab, tmpstring2, 1,
                     &(bpsm[0][(coeffs - 1) - (offset + i)]),
                     T_FLOAT, 1, "", tmpstring1);
      sprintf(tmpstring1, "e2Psm pol.: (x-%.2f)^%d*(y-%.2f)^%d/(%.2f-%.2f)",
              xmin, i, ymin, j - i,
              (xmax - xmin), (ymax - ymin));
      sprintf(tmpstring2, "E2PSMPOL_%d", offset + i);
      add_key_to_tab(starstab, tmpstring2, 1,
                     &(bpsm[1][(coeffs - 1) - (offset + i)]),
                     T_FLOAT, 1, "", tmpstring1);
    }
    offset += j + 1;
  }

  add_tab(starstab, outKSBcat, 2);

  /* add HISTORY information to the output catalog */
  historyto_cat(outKSBcat, "anisotropy", argc, argv);

  /* release memory and bye */

  VPRINTF(NORMAL, "saving catalogs\n");

  save_cat(outKSBcat, outcatname);
  free_cat(inKSBcat, 1);
  free_cat(outKSBcat, 0);

  for (i = 0; i < nkeys; i++) {
    ED_FREE(keynames[i]);
  }

  ED_FREE(keynames);

  ED_FREE(condmin);
  ED_FREE(condmax);
  ED_FREE(cond);

  for (j = 0; j < nkeys; j++) {
    if (j != 12) {
      ED_FREE(allkeys[j]);
    }
  }
  ED_FREE(allkeys);

  ED_FREE(inext);
  ED_FREE(inextcopy);

  for (i = 0; i < coeffs; i++) {
    ED_FREE(cov[i]);
  }
  ED_FREE(cov);

  ED_FREE(b[0]);
  ED_FREE(b[1]);
  ED_FREE(b);

  ED_FREE(bpsm[0]);
  ED_FREE(bpsm[1]);
  ED_FREE(bpsm);

  ED_FREE(e1Psm);
  ED_FREE(e2Psm);

  return(0);
}

/*
 * This function only gives the usage for the program
 */

void printUsage()
{
  fprintf(stdout, "PROGRAMNAME:\n");
  fprintf(stdout, "      anisotropy - fit two-dimensional polynomials to object ellipticities\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS:\n");
  fprintf(stdout, "      anisotropy -i inputcatalog\n");
  fprintf(stdout, "                 -o outputcatalog\n");
  fprintf(stdout, "                 -c conditions for star preselection\n");
  fprintf(stdout, "                 [ -e degree of polynomials (2) ]\n");
  fprintf(stdout, "                 [ -f use trace for Psm (1=Yes, 0=No) (1) ]\n");
  fprintf(stdout, "                 [ -g iterations for rejection (2) ]\n");
  fprintf(stdout, "                 [ -h maximum ellipticity for stars (1.0) ]\n");
  fprintf(stdout, "                 [ -j sigma threshhold for rejection (16.0) ]\n");
  fprintf(stdout, "                 [ -k use full tensor correction for anisotropy ]\n");
  fprintf(stdout, "                 [ -t table with the objects (OBJECTS) ]\n");
  fprintf(stdout, "                 [ -x key names for (pixel) object positions (Xpos, Ypos) ]\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION: \n");
  fprintf(stdout, "      The program 'anisotropy' fits a two dimensional polynomial to\n");
  fprintf(stdout, "      the (Kaiser, Squires and Broadhurst) ellipticities of stars for\n");
  fprintf(stdout, "      lensing studies. It works on the basis of LDAC catalogues.  The\n");
  fprintf(stdout, "      keynames for elliticities must be 'e1' and 'e2'. Keynames for\n");
  fprintf(stdout, "      (pixel) positions can be set via '-x'. The program first fits e1\n");
  fprintf(stdout, "      and e2 and then rejects objects objects having\n");
  fprintf(stdout, "      (de1**2/sigma_e1**2)+(de2**2/sigma_e2**2) > threshhold. This\n");
  fprintf(stdout, "      process is repeated 'iterations' times. Also e1/Psm and e2/Psm\n");
  fprintf(stdout, "      are fittet. It is configurable whether for Psm the single Psm11\n");
  fprintf(stdout, "      and Psm22 components are taken or whether the mean of this is\n");
  fprintf(stdout, "      used.  All information is stored in a 'STARS' table.  The\n");
  fprintf(stdout, "      program also make the anisotropy correction and stores the\n");
  fprintf(stdout, "      result as e?anisocorr (?=1,2).  Stars that were preselected by\n");
  fprintf(stdout, "      the user (option -c) get a cl=1. Stars that survived the final\n");
  fprintf(stdout, "      fitting get a cl=2.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "EXAMPLES:\n");
  fprintf(stdout, "      anisotropy -i in.cat -o out.cat -c rh 1.5 2.0 mag 17.0 20.0 -e 3\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "      Fit third order, two-dimensional polynomials for e1 and e2 to\n");
  fprintf(stdout, "      all objects (stars) preselected with 1.5 < rh < 2.0 and \n");
  fprintf(stdout, "      17.0 < mag < 20.0.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHORS:\n");
  fprintf(stdout, "      Thomas Erben    (terben@astro.uni-bonn.de)\n");
}

void build_equations(float **acov, float **ab,
                     float *ax, float *ay,
                     float *ae1, float *ae2,
                     int *ainext, int aistart, int anobjects,
                     int adegree)
{
  int i, j, k, l, m;
  int offsetin = 0, offsetout = 0;
  int pointer;
  int coeffs = (adegree + 2) * (adegree + 1) / 2;

  VPRINTF(DEBUG, "building equations\n");

  /* initialise all values to zero */
  for (i = 0; i < coeffs; i++) {
    for (j = 0; j < coeffs; j++) {
      acov[i][j] = 0.0;
    }
  }

  for (i = 0; i < 2; i++) {
    for (j = 0; j < coeffs; j++) {
      ab[i][j] = 0.0;
    }
  }

  /* now build the equations */
  for (j = adegree; j >= 0; j--) {
    for (i = 0; i <= j; i++) {
      offsetin = 0;
      for (k = adegree; k >= 0; k--) {
        for (l = 0; l <= k; l++) {
          pointer = aistart;
          for (m = 0; m < anobjects; m++) {
            acov[offsetin + l][offsetout + i] += pow(ax[pointer], (j - i)) *
              pow(ay[pointer], i) *
              pow(ax[pointer], (k - l)) * pow(ay[pointer], l);

            pointer = ainext[pointer];
          }
        }
        offsetin += k + 1;
      }
    }
    offsetout += j + 1;
  }
  offsetin = 0;

  for (j = adegree; j >= 0; j--) {
    for (i = 0; i <= j; i++) {
      pointer = aistart;
      for (m = 0; m < anobjects; m++) {
        ab[0][offsetin + i] += ae1[pointer] * pow(ax[pointer], (j - i)) *
          pow(ay[pointer], i);
        ab[1][offsetin + i] += ae2[pointer] * pow(ax[pointer], (j - i)) *
          pow(ay[pointer], i);
        pointer = ainext[pointer];
      }
    }
    offsetin += j + 1;
  }
}

float evaluatepolynomial(const float *aa, int an, float ax, float ay)
{
  int i, j;
  int Offset = 0;
  float Result = 0;

  for (j = an; j >= 0; j--) {
    for (i = 0; i <= j; i++) {
      Result += aa[Offset + i] * pow(ax, (j - i)) * pow(ay, i);
    }
    Offset += j + 1;
  }
  return(Result);
}

void printmatrix(float **am, int ax, int ay)
{
  int i, j;

  for (j = 0; j < ay; j++) {
    for (i = 0; i < ax; i++) {
      printf("%f ", am[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}
