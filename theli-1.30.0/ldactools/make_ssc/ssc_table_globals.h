/*
  29.01.2005:
  I adapted the arguments of open_ssc_cat for the inclusion
  of HISTORY information to the output catalogs
*/

#ifndef __SSC_TABLE_GLOBALS_H__
#define __SSC_TABLE_GLOBALS_H__

extern void 	close_ssc_cat(tabstruct *),
		save_catalog(char *, char *),
		add_ssc_object(double, double, float, float *, float *,
			float *, float *, float *, unsigned char *, char),
		ssc_put_pairs(keystruct **, set_struct *, tabstruct *),
		ssc_put_row(keystruct **, set_struct *, tabstruct *);

extern tabstruct
                *open_ssc_cat(char *, tabstruct **, tabstruct **,
                              int, int, char **);

#endif
