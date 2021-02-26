#include "ssc_defs.h"
#include "ssc_types.h"
#include "obj_sets_globals.h"

/* 21.02.01:
   substituted all long int by the LONG macro

   23.01.2005:
   I moved definitions from global variables
   (col_name, col_input, col_merge, col_chan)
   to avoid double definitions in the linking
   process.

   10.03.2005:
   new functions min and max to introduce MIN and MAX
   possibilities in channel merging.
*/

#ifndef __SSC_GLOBALS_H__
#define __SSC_GLOBALS_H__

extern LONG *last_best_fpos;
extern keystruct **best_key;
extern keystruct **crval1,**crval2,**cdelt1,**cdelt2,**crpix1,**crpix2;
extern int	weighted(keystruct *, keystruct **, int, set_struct *),
		average_regular(keystruct *, keystruct **, int, set_struct *),
		average_error(keystruct *, keystruct **, int, set_struct *),
		average_phot_error(keystruct *, keystruct **, int, set_struct *),
		average_flag(keystruct *, keystruct **, int, set_struct *),
		average_photometry(keystruct *, keystruct **, int, set_struct *),
		min(keystruct *, keystruct **, int, set_struct *),
		max(keystruct *, keystruct **, int, set_struct *),
		best(keystruct *, keystruct **, int, set_struct *),
		detected(keystruct *, keystruct **, int, set_struct *),
		if_used(int, int),
		get_fpos(int, int);
extern void	set_used(int, int),
		make_indices(),
		init_count_status(int, int),
		add_count_status(int, int);
extern LONG get_count_status(int, int);

extern char **col_name;
extern char **col_input;
extern char **col_merge;
extern char **col_chan;

#endif
