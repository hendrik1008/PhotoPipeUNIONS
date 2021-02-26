#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fitscat.h"
#include "utils_globals.h"
#include "field_desc.h"

/*
 * 15.07.01:
 * included "#include<string.h>" to avoid compiler warnings
 *
 * 20.07.2004:
 * I removed compiler warnings due to possibly
 * undefined variables (compilation with optimisation
 * turned on)
 *
 */

field_params **load_field_description(tabstruct *fields_tab, int *nfields,
               char *ccrval1, char *ccrval2, char *ccdelt1, char *ccdelt2,
               char *ccrpix1, char *ccrpix2, char *cnaxis1, char *cnaxis2)
{
    field_params	**p;
    keystruct           **keys;
    char                **keynames;
    int                 i, POSKEYS = 7;
    double		*crval1, *crval2,
                        *cdelt1 = NULL, *cdelt2 = NULL,
                        *crpix1, *crpix2;
    double		*cd11, *cd12, *cd21, *cd22;
    int			*xwid, *ywid;
    long int		*chan_nr;

    ED_CALLOC(keys,     "load_field_description", keystruct *, (POSKEYS+6));
    ED_CALLOC(keynames, "load_field_description", char *,      (POSKEYS+6));
    for (i=0;i<POSKEYS+6;i++)
       ED_CALLOC(keynames[i], "load_field_description", char, MAXCHAR);
    strcpy(keynames[0], ccrval1);
    strcpy(keynames[1], ccrval2);
    strcpy(keynames[2], ccrpix1);
    strcpy(keynames[3], ccrpix2);
    strcpy(keynames[4], cnaxis1);
    strcpy(keynames[5], cnaxis2);
    strcpy(keynames[6], "CHANNEL_NR");
    strcpy(keynames[7], ccdelt1);
    strcpy(keynames[8], ccdelt2);
    strcpy(keynames[9],  "CD1_1");
    strcpy(keynames[10], "CD2_2");
    strcpy(keynames[11], "CD1_2");
    strcpy(keynames[12], "CD2_1");

    read_keys(fields_tab, keynames, keys, (POSKEYS+6), NULL);
    for (i=0;i<POSKEYS;i++)
      if (!keys[i])
        syserr(FATAL,"load_field_description", "No key %s in table\n",
           keynames[i]);
    if (!keys[7] && !keys[9])
      syserr(FATAL,"load_field_description", "No %s or CD1_1 in table\n",
          ccdelt1);
    if (!keys[8] && !keys[10])
      syserr(FATAL,"load_field_description", "No %s or CD2_2 in table\n",
          ccdelt2);
    *nfields = keys[0]->nobj;
    ED_COPY_PTR(crval1, "load_field_description", keys[0], T_DOUBLE, "float");
    ED_COPY_PTR(crval2, "load_field_description", keys[1], T_DOUBLE, "float");
    ED_COPY_PTR(crpix1, "load_field_description", keys[2], T_DOUBLE, "float");
    ED_COPY_PTR(crpix2, "load_field_description", keys[3], T_DOUBLE, "float");
    ED_COPY_PTR(xwid,   "load_field_description", keys[4], T_LONG,  "long int");
    ED_COPY_PTR(ywid,   "load_field_description", keys[5], T_LONG,  "long int");
    ED_COPY_PTR(chan_nr,"load_field_description", keys[6], T_LONG,  "long int");
    if (keys[7])
      ED_COPY_PTR(cdelt1, "load_field_description", keys[7], T_DOUBLE, "float");
    if (keys[8])
      ED_COPY_PTR(cdelt2, "load_field_description", keys[8], T_DOUBLE, "float");
    if (keys[9]) {
      ED_COPY_PTR(cd11, "load_field_description", keys[ 9], T_DOUBLE, "float");
    } else {
       ED_CALLOC(cd11, "load_field_description", double, (*nfields));
    }
    if (keys[10]) {
      ED_COPY_PTR(cd22, "load_field_description", keys[10], T_DOUBLE, "float");
    } else {
       ED_CALLOC(cd22, "load_field_description", double, (*nfields));
    }
    if (keys[11]) {
      ED_COPY_PTR(cd12, "load_field_description", keys[11], T_DOUBLE, "float");
    } else {
       ED_CALLOC(cd12, "load_field_description", double, (*nfields));
    }
    if (keys[12]) {
      ED_COPY_PTR(cd21, "load_field_description", keys[12], T_DOUBLE, "float");
    } else {
       ED_CALLOC(cd21, "load_field_description", double, (*nfields));
    }
    for (i=0;i<POSKEYS+6;i++)
       ED_FREE(keynames[i]);
    ED_FREE(keynames);
    ED_CALLOC(p, "load_field_description", field_params *, (*nfields));
    for (i=0;i<*nfields;i++) {
       ED_CALLOC(p[i], "load_field_description", field_params, 1);
       p[i]->crval1 = crval1[i];
       p[i]->crval2 = crval2[i];
       p[i]->crpix1 = crpix1[i];
       p[i]->crpix2 = crpix2[i];
       p[i]->xwid   = xwid[i];
       p[i]->ywid   = ywid[i];
       p[i]->chan_nr= chan_nr[0];
       if (keys[7])
         p[i]->cdelt1 = cdelt1[i];
       else {
         if (fabs(cd11[i]) > fabs(cd12[i])) {
            p[i]->cdelt1 = cd11[i];
         } else {
            p[i]->cdelt1 = cd12[i];
         }
       }
       if (keys[8])
         p[i]->cdelt2 = cdelt2[i];
       else {
         if (fabs(cd22[i]) > fabs(cd21[i])) {
            p[i]->cdelt2 = cd22[i];
         } else {
            p[i]->cdelt2 = cd21[i];
         }
       }
       if (keys[9]) {
         p[i]->pc11 = cd11[i]/p[i]->cdelt1;
         if (p[i]->pc11 != 1.0) syserr(WARNING,"load_field", "CD1_1 != CDELT1");
       } else
         p[i]->pc11 = 1.0;
       if (keys[10])  {
         p[i]->pc22 = cd22[i]/p[i]->cdelt2;
         if (p[i]->pc22 != 1.0) syserr(WARNING,"load_field", "CD2_2 != CDELT2");
       } else
         p[i]->pc22 = 1.0;
       if (keys[11])
         p[i]->pc12 = cd12[i]/p[i]->cdelt1;
       else
         p[i]->pc12 = 0.0;
       if (keys[12])
         p[i]->pc21 = cd21[i]/p[i]->cdelt2;
       else
         p[i]->pc21 = 0.0;
    }
    if (!keys[9])
       ED_FREE(cd11);
    if (!keys[10])
       ED_FREE(cd22);
    if (!keys[11])
       ED_FREE(cd12);
    if (!keys[12])
       ED_FREE(cd21);
/*    for (i=0;i<POSKEYS+6;i++)
       if (keys[i]) free_key(keys[i]); */
    ED_FREE(keys);
    return p;
}

void free_field_desc(field_params **p, int n)
{
    int         i;

    for (i=0;i<n;i++)
      ED_FREE(p[i]);
    ED_FREE(p);
}
