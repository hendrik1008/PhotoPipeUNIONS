/*
   05.05.03:
   included the tsize.h file

   24.01.05:
   I changed char * arguments in 'add_key_to_tab'
   to const char * to avoid warnings under C++
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fitscat.h"
#include "tsize.h"
#include "tabutil_globals.h"
#include "options_globals.h"
#include "utils_globals.h"

void clear_keys(tabstruct *tab)
{
   keystruct    *key;
   int          i;

   if (!(key=tab->key))
     syserr(FATAL, "clear_keys", "tab structure has no keys");

   for (i=0; i<tab->nkey; i++) {
      ED_FREE(key->ptr);
      key=key->nextkey;
   }
   return;
}

keystruct *add_key_to_tab(tabstruct *tab,
                          const char *keyname, int nobj, void *ptr,
                          t_type ttype, int depth, const char *unit,
                          const char *comment)
{
    keystruct *key;

    key = new_key(keyname);
    key->nobj   = nobj;
    key->ptr    = ptr;
    key->ttype  = ttype;
    if (ttype == T_STRING) {
       key->htype = H_STRING;
    }
    key->naxis  = 1;
    ED_MALLOC(key->naxisn, "add_key_to_tab", int, key->naxis);
    key->naxisn[0] = depth;
    key->nbytes = t_size[ttype] * depth;
    strncpy(key->unit,unit,79);
    strncpy(key->comment,comment,79);
    if (add_key(key, tab, 0) == RETURN_ERROR) {
       syserr(FATAL,"main","Error adding key %s to table %s\n",
              keyname, tab->extname);
    }

    return key;

}

static tabstruct	*chan_nr_tab = NULL, *chan_name_tab = NULL;
static int		chan_nr;
static char		chan_name[MAXCHAR];

int get_channel_number(tabstruct *tab)
{
    keystruct	*key;
    int		*nchan;

    if (tab != chan_nr_tab) {
       if ((key = read_key(tab, "CHANNEL_NR"))) {
          nchan = key->ptr;
          chan_nr = nchan[0];
          chan_nr_tab = tab;
       } else {
          chan_nr = 0;
          syserr(WARNING,"get_channel_number", "No CHANNEL_NR! Please rerun ldacconv\n");
       }
    }
    return (chan_nr);
}

char *get_channel_name(tabstruct *tab)
{
    keystruct	*key;
    char	*nchan;

    if (tab != chan_name_tab) {
       if ((key = read_key(tab, "FILTER_NAME"))) {
          nchan = key->ptr;
          strcpy(chan_name, &nchan[0]);
          chan_name_tab = tab;
       } else {
          strcpy(chan_name, "Unknown");
          syserr(WARNING,"get_channel_name", "No FILTER_NAME! Please rerun ldacconv\n");
       }

    }

    return (chan_name);

}
