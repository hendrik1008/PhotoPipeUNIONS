/*----------------------------------------------------------------------------

   File name    :   ldacaddmask.c
   Author       :   Thomas Erben
   Created on   :   22.03.2007
   Description  :   marks objects lying within ds9/saoimage polygons

 ---------------------------------------------------------------------------*/

/*
	$Id: ldacaddmask.c,v 1.8 2018/03/28 12:06:15 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:15 $
	$Revision: 1.8 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
   19.06.2007:
   - The code now accepts the term 'POLYGON' written in
     any combination of minuscules and majuscules.
   - The number of polygons is now allocated and adapted
     dynamically
   - The program output (documentation messages etc.) were
     improved.

   18.07.2007:
   - I removed some DEBUG information
   - I removed compiler warnings (signed - unsigned issues)
   - I changed all double declrations to floats
     Floats are sufficient everywhere and do not consume
     as much memory

   27.02.2008:
   I made the code more robust to polygon lines having a comment
   at the end.

   08.04.2008:
   - During polygon reading empty lines could lead to a core dump!
   - The command line value for the masks was not used. Masked objects
     were always given a value of '1'!

   26.09.2008:
   I substituted the algorithm to check whether a point lies
   within a polygon; the new algorithm is considerably shorter
   and simplifies/shortens the program considerably; the total
   speed gain is about 10 percent.

   05.12.2008:
   I largely rewrote the code to speed it up significantly. This
   is achieved by splitting the data field in smaller boxes and
   scanning, for each polygon, only the boxes where candidate galaxies
   for that polygon can be located (see below).
 */

/*----------------------------------------------------------------------------
   Explanation for searching galaxies which might lie within a polygon
 ---------------------------------------------------------------------------*/

/*
   - We subdivide our data field in boxes of size 'boxsize'.
     Boxes are labelled from lower left to upper right by going
     from left to right in a row and continuing with the left-most
     box in the next line, i.e. our data field is divided as:

     6 7 8
     3 4 5
     0 1 2

   - When reading in polygons we register the minimum/maximum x and
     y values of the polygon. This allows us to easily find out in which
     boxes galaxies, that might lie within that polygon, must be located.

   - When reading galaxy we register the box that galaxy is in.

   - When checking whether galaxies lie within a box we go through
     the polygons and regard, for each polygon, only the boxes in which
     candidate galaxies can  be located!
*/

/*----------------------------------------------------------------------------
                                Includes and Defines
 ---------------------------------------------------------------------------*/

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include "utils_macros.h"
#include "utils_globals.h"
#include "lists_globals.h"
#include "tabutil_globals.h"
#include "tsize.h"
#include "fitscat.h"

#ifndef MAXSTRINGLENGTH
#define MAXSTRINGLENGTH    256
#endif

#ifndef MAXPOLYCHAR
#define MAXPOLYCHAR    4096
#endif

#define __PROGRAMVERSION__    "2.0.0"
#define __PROGRAMNAME__       "ldacaddmask"


/*----------------------------------------------------------------------------
                                Function prototypes
 ---------------------------------------------------------------------------*/

/* 'clever and VERY short' algorithm to check whether a point lies within a
   polygon; (c) by  W. Randolph Franklin;
   see http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
   for details!

   The function returns 0 if the point is OUTSIDE the polygon; 1 otherwise
 */
int pnpoly(float testx,  /* x-coordinate of test points */
           float testy,  /* y-coordinate of test points */
           float *vertx, /* x-coordinates of the polygon's vertices */
           float *verty, /* y-coordinates of the polygon's vertices */
           int nvert     /* Number of vertices in the polygon (the first
                            vertext DOES NOT NEED to be repeated at the end) */
           );

/* This function only gives the usage message for the program */
void printUsage(void);

/*----------------------------------------------------------------------------
                                Global Variables
 ---------------------------------------------------------------------------*/

static const char vectok[] = { ",() \t\n\r" };

/*----------------------------------------------------------------------------
                            Main code
 ---------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  char  incatname[MAXSTRINGLENGTH];
  char  outcatname[MAXSTRINGLENGTH]   = "default_addmask.cat";
  char  maskfilename[MAXSTRINGLENGTH] = "masks.asc";
  char  tablename[MAXSTRINGLENGTH]    = "OBJECTS";
  char  xposkeyname[MAXSTRINGLENGTH]  = "Xpos";
  char  yposkeyname[MAXSTRINGLENGTH]  = "Ypos";
  char  maskname[MAXSTRINGLENGTH]     = "MASK";
  char  str[MAXPOLYCHAR];
  char *str2;
  char *comment;
  FILE *maskfile;

  short maskvalue = 1;

  /* If the following number is changed we also need to modify
     the index in the declaration of 'keypointer'
   */
  const int nkeys = 2;  /* we need to read two keys identifying the positions */

  keystruct **keys;
  char **     keynames;

  /* the two in the following array is the same number as
     given for 'nkeys' above. C90 does not allow any more
     to give the 'const' value of nkeys as array index !!
   */
  float *keypointer[2];
  float *xpos, *ypos;
  short *mask;

  /* variables for dividing our data filed in boxes */
  int            nboxes;           /* how many boxes are there in total */
  unsigned short nxboxes, nyboxes; /* number of vertical and horizontal
                                      field boxes */
  unsigned short *ngalinbox;       /* how many galaxies are in a certain box */
  unsigned short *maxgalinbox;     /* the maximum array index for the number of
                                      galaxies in a box */
  unsigned int **galinbox;      /* which galaxies are in a certain box */
  unsigned int   boxsize = 100; /* size of a box in coordinate units */
                                /* default for now is 100 pixels */
  int currbox; /* the current box under consideration */
  int currgal; /* the current galaxy under consideration */
  int nxsearchboxes, nysearchboxes; /* the number of boxes to search
                                       in x and y for a certain polygon */
  int startsearchbox;  /* the first box to search for galaxies (for a given
                          polygon) */
  int startxsearchbox, startysearchbox; /* start boxes in x and y to search
                                           for galaxies */

  /* other variables: */
  int        i, j, k, l;
  int        ngal;
  int        ngalinpoly = 0;
  float      xposmin    = 10000.0, xposmax = -10000.0;
  float      yposmin    = 10000.0, yposmax = -10000.0;
  catstruct *incat, *outcat;
  tabstruct *tab;
  int        nline = 0;

  /* variables for the polygons */
  short   nmaxpolypoints = 500;
  long    nmaxpolygons   = 1000;
  float **xpoly;
  float **ypoly;
  float * xpolymin, *xpolymax;
  float * ypolymin, *ypolymax;

  int  npoly = 0;
  int  npolypoints;
  int *nsegment;

  /* get the command line arguments: */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      printUsage();
      exit(0);
    }
    if ((argv[i][0] == '-') && i < (argc - 1)) {
      switch (tolower(argv[i][1])) {
      case 'x': strncpy(xposkeyname, argv[++i], MAXSTRINGLENGTH - 1);
        strncpy(yposkeyname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'i': strncpy(incatname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'o': strncpy(outcatname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'f': strncpy(maskfilename, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 't': strncpy(tablename, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'n': strncpy(maskname, argv[++i], MAXSTRINGLENGTH - 1);
        break;
      case 'm': maskvalue = (int)atoi(argv[++i]);
        break;
      default:
        break;
      }
    } else {
      syserr(FATAL, "main",
             "Wrong Command line syntax (type 'ldacaddmask -h' for help)\n");
    }
  }

  /* print welcome message: */
  printf("\n%s V%s (%s; %s)\n\n", __PROGRAMNAME__, __PROGRAMVERSION__,
         __DATE__, __TIME__);

  if (argc < 4) {
    printUsage();
    exit(1);
  }

  /* read catalog and quantities: */
  if ((incat = read_cat(incatname)) == NULL) {
    syserr(FATAL, "main", "Error opening input catalog\n");
  }

  outcat = new_cat(1);
  inherit_cat(incat, outcat);
  historyto_cat(outcat, "ldacaddmask", argc, argv);
  copy_tabs(incat, outcat);

  if ((tab = name_to_tab(outcat, tablename, 0)) == NULL) {
    syserr(FATAL, "main", "No %s table in catalog\n", tablename);
  }

  ED_CALLOC(keynames, "main", char *, nkeys);
  for (i = 0; i < nkeys; i++) {
    ED_CALLOC(keynames[i], "main", char, MAXSTRINGLENGTH);
  }

  strncpy(keynames[0], xposkeyname, MAXSTRINGLENGTH - 1);
  strncpy(keynames[1], yposkeyname, MAXSTRINGLENGTH - 1);

  /* read keys: */
  ED_CALLOC(keys, "main", keystruct *, nkeys);

  read_keys(tab, keynames, keys, nkeys, NULL);

  for (j = 0; j < nkeys; j++) {
    if (!keys[j]) {
      syserr(FATAL, "main", "No key %s in table\n", keynames[j]);
    }

    switch (keys[j]->ttype) {
    case T_FLOAT:
      ED_GETVEC(keypointer[j], "main", keys[j],
                keys[j]->name, float, float);
      break;
    case T_DOUBLE:
      ED_GETVEC(keypointer[j], "main", keys[j],
                keys[j]->name, float, double);
      break;
    default:
      syserr(FATAL, "main", "Only float and double key types supported\n");
      break;
    }
  }


  xpos = keypointer[0];
  ypos = keypointer[1];

  ngal = keys[0]->nobj;

  printf("\nInput catalogue read.\n");

  /* free unnecessary memory */
  for (i = 0; i < nkeys; i++) {
    ED_FREE(keys[i]->ptr);
  }

  /*
     determine min. and max of galaxy positions;
     necessary for determinig how many boxes we need
     for our field.
   */
  for (i = 0; i < ngal; i++) {
    if (xpos[i] > xposmax) {
      xposmax = xpos[i];
    }
    if (xpos[i] < xposmin) {
      xposmin = xpos[i];
    }
    if (ypos[i] > yposmax) {
      yposmax = ypos[i];
    }
    if (ypos[i] < yposmin) {
      yposmin = ypos[i];
    }
  }

  printf("\nnumber of galaxies in input catalogue %s: %d\n", incatname, ngal);

  /* put galaxies in field boxes */
  nxboxes = (int)((xposmax / boxsize)) + 1;
  nyboxes = (int)((yposmax / boxsize)) + 1;
  nboxes  = nxboxes * nyboxes;

  ED_CALLOC(ngalinbox, "main", unsigned short, nboxes);
  ED_CALLOC(maxgalinbox, "main", unsigned short, nboxes);
  ED_CALLOC(galinbox, "main", unsigned int *, nboxes);

  for (i = 0; i < nboxes; i++) {
    maxgalinbox[i] = 20;
    ED_CALLOC(galinbox[i], "main", unsigned int, maxgalinbox[i]);
  }

  for (i = 0; i < ngal; i++) {
    currbox = (int)(xpos[i] / boxsize) + nxboxes * (int)(ypos[i] / boxsize);

    galinbox[currbox][ngalinbox[currbox]] = i;
    ngalinbox[currbox]++;

    if (ngalinbox[currbox] == maxgalinbox[currbox]) {
      maxgalinbox[currbox] *= 2;
      ED_REALLOC(galinbox[currbox], "main", unsigned int,
                 maxgalinbox[currbox]);
    }
  }

  /* prepare the polygon searching */
  ED_CALLOC(mask, "main", short, ngal);
  ED_CALLOC(xpoly, "main", float *, nmaxpolygons);
  ED_CALLOC(ypoly, "main", float *, nmaxpolygons);
  ED_CALLOC(xpolymin, "main", float, nmaxpolygons);
  ED_CALLOC(ypolymin, "main", float, nmaxpolygons);
  ED_CALLOC(xpolymax, "main", float, nmaxpolygons);
  ED_CALLOC(ypolymax, "main", float, nmaxpolygons);
  ED_CALLOC(nsegment, "main", int, nmaxpolygons);

  for (i = 0; i < nmaxpolygons; i++) {
    ED_CALLOC(xpoly[i], "main", float, nmaxpolypoints);
    ED_CALLOC(ypoly[i], "main", float, nmaxpolypoints);
  }

  /* read in polygon file */
  printf("Reading polygon file '%s' \n", maskfilename);

  if (!(maskfile = fopen(maskfilename, "r"))) {
    syserr(FATAL, "main", "File %s not found\n", maskfilename);
  }

  while (fgets(str, MAXPOLYCHAR, maskfile)) {
    if (!nline) {
      if (*str) {
        str[strlen(str) - 1] = (char)'\0';
      }
      nline = 1;
    }

    /* check for comments: */
    comment = strchr(str, '#');
    if (comment != NULL) {
      *comment = (char)'\0';
    }

    str2 = strtok(str, vectok);

    /* test for empty string ! */
    if (str2 == NULL) {
      continue;
    }

    if (strcmpcu(str2, "POLYGON") && strcmpcu(str2, "-POLYGON")) {
      continue;
    }
    npolypoints     = 0;
    xpolymin[npoly] = 1.0e10;
    ypolymin[npoly] = 1.0e10;
    xpolymax[npoly] = -1.0e10;
    ypolymax[npoly] = -1.0e10;

    while (1) {
      if (!(str2 = strtok(NULL, vectok))) {
        break;
      }

      if (npolypoints >= nmaxpolypoints) {
        syserr(FATAL, "main", "Too many segments in current polygon");
      } else {
        xpoly[npoly][npolypoints] = atof(str2) - 1.0;

        if (xpoly[npoly][npolypoints] < 0.0) {
          xpoly[npoly][npolypoints] = 0.001;
        }

        if (xpoly[npoly][npolypoints] < xpolymin[npoly]) {
          xpolymin[npoly] = xpoly[npoly][npolypoints];
        }

        if (xpoly[npoly][npolypoints] > xpolymax[npoly]) {
          xpolymax[npoly] = xpoly[npoly][npolypoints];
        }

        if (!(str2 = strtok(NULL, vectok))) {
          syserr(FATAL, "main", "Malformed POLYGON in %s", maskfilename);
        }

        ypoly[npoly][npolypoints] = atof(str2) - 1.0;

        if (ypoly[npoly][npolypoints] < 0.0) {
          ypoly[npoly][npolypoints] = 0.001;
        }

        if (ypoly[npoly][npolypoints] < ypolymin[npoly]) {
          ypolymin[npoly] = ypoly[npoly][npolypoints];
        }

        if (ypoly[npoly][npolypoints] > ypolymax[npoly]) {
          ypolymax[npoly] = ypoly[npoly][npolypoints];
        }

        npolypoints++;
      }
    }
    nsegment[npoly] = npolypoints;

    npoly++;

    if (npoly >= nmaxpolygons) {
      nmaxpolygons *= 2;
      ED_REALLOC(xpoly, "main", float *, nmaxpolygons);
      ED_REALLOC(ypoly, "main", float *, nmaxpolygons);
      ED_REALLOC(xpolymin, "main", float, nmaxpolygons);
      ED_REALLOC(ypolymin, "main", float, nmaxpolygons);
      ED_REALLOC(xpolymax, "main", float, nmaxpolygons);
      ED_REALLOC(ypolymax, "main", float, nmaxpolygons);
      ED_REALLOC(nsegment, "main", int, nmaxpolygons);

      for (i = npoly; i < nmaxpolygons; i++) {
        ED_CALLOC(xpoly[i], "main", float, nmaxpolypoints);
        ED_CALLOC(ypoly[i], "main", float, nmaxpolypoints);
      }
    }
  }

  printf("\npolygons in region file %s: %d\n\n", maskfilename, npoly);

  /* now do the actual testing */
  for (i = 0; i < npoly; i++) {
    startxsearchbox = (int)(xpolymin[i] / boxsize);
    startysearchbox = (int)(ypolymin[i] / boxsize);
    startsearchbox  = startxsearchbox + nxboxes * startysearchbox;

    nxsearchboxes = (int)((xpolymax[i] - xpolymin[i]) / boxsize) + 2;
    nysearchboxes = (int)((ypolymax[i] - ypolymin[i]) / boxsize) + 2;

    if ((startxsearchbox + nxsearchboxes) > nxboxes) {
      nxsearchboxes = (nxboxes - startxsearchbox) + 1;
    }

    if ((startysearchbox + nysearchboxes) > nyboxes) {
      nysearchboxes = (nyboxes - startysearchbox) + 1;
    }

    for (j = 0; j < nysearchboxes; j++) {
      for (k = 0; k < nxsearchboxes; k++) {
        currbox = startsearchbox + k + (j * nxboxes);
        if (currbox >= 0 && currbox < nboxes) {
          for (l = 0; l < ngalinbox[currbox]; l++) {
            currgal = galinbox[currbox][l];

            /* the galaxy might have been masked by another polygon
               already
            */
            if (mask[currgal] == 0) {
              mask[currgal] = pnpoly(xpos[currgal], ypos[currgal],
                                     xpoly[i], ypoly[i], nsegment[i]);

              if (mask[currgal] != 0) {
                ngalinpoly++;
                mask[currgal] = maskvalue;
              }
            }
          }
        }
      }
    }
    if ((i + 1) % 10 == 0) {
      printf("%d/%d polygons processed\r", i + 1, npoly);
    }
  }

  printf("\n\n%d galaxies lie within a polygon\n", ngalinpoly);
  printf("\nGalaxy processing done\n");
  printf("\nWriting mask key to output catalogue\n");

  add_key_to_tab(tab, maskname, ngal, mask, T_SHORT,
                 1, "", "mask value (addmask)");

  /* free memory and bye */
  save_cat(outcat, outcatname);

  free_cat(incat, 1);
  free_cat(outcat, 1);

  for (i = 0; i < nboxes; i++) {
    ED_FREE(galinbox[i]);
  }
  ED_FREE(galinbox);
  ED_FREE(maxgalinbox);
  ED_FREE(ngalinbox);

  for (i = 0; i < nkeys; i++) {
    ED_FREE(keynames[i]);
  }
  ED_FREE(keynames);

  for (i = 0; i < nmaxpolygons; i++) {
    ED_FREE(xpoly[i]);
    ED_FREE(ypoly[i]);
  }
  ED_FREE(xpoly);
  ED_FREE(ypoly);
  ED_FREE(xpolymin);
  ED_FREE(ypolymin);
  ED_FREE(xpolymax);
  ED_FREE(ypolymax);
  ED_FREE(nsegment);

  printf("\nAll Done\n");

  return(0);
}

/* 'clever and VERY short' algorithm to check whether a point lies within a
   polygon; (c) by  W. Randolph Franklin;
   see http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
   for details!
 */
int pnpoly(float testx,  /* x-coordinate of test points */
           float testy,  /* y-coordinate of test points */
           float *vertx, /* x-coordinates of the polygon's vertices */
           float *verty, /* y-coordinates of the polygon's vertices */
           int nvert     /* Number of vertices in the polygon (the first
                            vertext DOES NOT NEED to be repeated at the end) */
           )
{
  int i, j, c = 0;
  for (i = 0, j = nvert - 1; i < nvert; j = i++) {
    if (((verty[i] > testy) != (verty[j] > testy)) &&
        (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i])) {
      c = !c;
    }
  }

  return(c);
}

void printUsage(void)
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "      ldacaddmask - marks objects lying within ds9/saoimage polygons\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "      ldacaddmask -i input catalog\n");
  fprintf(stdout, "                  -o output catalog\n");
  fprintf(stdout, "                  -f file containing the masks (Saoimage POLYGON format)\n");
  fprintf(stdout, "                [ -m mask value (1) ]\n");
  fprintf(stdout, "                [ -t table with objects (OBJECTS) ]\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "                [ -n name of the mask key (MASK) ]\n");
  fprintf(stdout, "                [ -x position quantities (Xpos, Ypos) ]\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION\n");
  fprintf(stdout, "   The program takes SAOimage polygon files as input and marks objects\n");
  fprintf(stdout, "   falling into these polygons.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   The polygon file has to has to cntain one SAOimage/ds9 polygon\n");
  fprintf(stdout, "   on each line. The format is:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   .\n");
  fprintf(stdout, "   .\n");
  fprintf(stdout, "   POLYGON(4458,223,4552,223,4552,317,4458,317,4458,223)\n");
  fprintf(stdout, "   POLYGON(3196,0,3291,0,3291,47,3196,47,3196,0)\n");
  fprintf(stdout, "   .\n");
  fprintf(stdout, "   .\n");
  fprintf(stdout, "   \n");
  fprintf(stdout, "   The polyogn file can contain comment lines starting with '#'.\n");
  fprintf(stdout, "   Polygon lines can finish with a comment as well.\n");
  fprintf(stdout, "   \n");
  fprintf(stdout, "EXAMPLE:\n");
  fprintf(stdout, "   ldacaddmask -i in.cat -o out.cat -m 2 -x X Y\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR\n");
  fprintf(stdout, "   Thomas Erben (terben@astro.uni-bonn.de)\n");
  fprintf(stdout, " \n");
}
