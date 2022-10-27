#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fitscat.h"
#include "tsize.h"
#include "options_globals.h"
#include "utils_globals.h"
#include "tabutil_globals.h"

/*
   02.12.2002:
   function init_catalogs:
   introduced a check whether a table exists in a catalogs before
   readobj from this table is asked for (not doing this produced a core
   dump if the table didn't exist)

   05.05.03:
   included the tsize.h file

   20.07.2004:
   I removed compiler warnings due to possibly
   undefined variables (compilation with optimisation
   turned on)

   29.01.2005:
   function init_catalogs:
   I adapted the use of addhistoryto_cat to the new syntax
   of this routine (including a flag for a time stamp).

   11.03.2005:
   function init_catalogs:
   I changed the call to addhistoryto_cat by one to
   historytocat and changed the argument list accordingly.
*/

void init_catalogs(catstruct *lincat, catstruct **outcat,
                   char *outfile, char *table_name,
                   keystruct **inkeys, char **keynames, int ninkeys,
                   keystruct **outkeys, int noutkeys,
                   tabstruct **in_objtab, tabstruct **out_objtab,
                   char *programname, int argc, char **argv)
/****** init_catalogs ***************************************************
 *
 *  NAME	
 *      init_catalogs - Initialize the object_by_object processing
 *
 *  SYNOPSIS
 *
 *  FUNCTION
 *
 *  INPUTS
 *      lincat     - Pointer to the input catalog
 *      outcat     - Pointer to the output catalog
 *      outfile    - Name of the output catalog
 *      header     - String to add to catalog history list
 *      table_name - Name of the object table
 *      key_names  - List of keys to which inkeys must point
 *      inkeys     - List of key pointers for immediate colum element reading
 *      ninkeys    - Number of keys in the inkeys list
 *      outkeys    - List of key pointer to add to output table
 *      noutkeys   - Number of keys in the outkeys list
 *      in_objtab  - Pointer to object_by_object input table
 *      out_objtab - Pointer to object_by_object output table
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      Pointer to the opened output catalog
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 27-05-1998
 *
 ******************************************************************************/
{
    catstruct	*loutcat;
    tabstruct	*tab, *intab, *outtab = NULL;
    keystruct	*key, *okey;
    char	**tabnames, str[MAXCHAR];
    int		j, k, ntabs;
    char	*ptr;

    loutcat = new_cat(1);
    inherit_cat(lincat, loutcat);
    historyto_cat(loutcat, programname, argc, argv);
    strcpy(loutcat->filename, outfile);
    if (open_cat(loutcat, WRITE_ONLY) != RETURN_OK)
      syserr(FATAL, "init_catalogs","Cannot open output catalog %s\n",
             outfile);
/*
 *  First write the Primary FITS header unit
 */
    save_tab(loutcat, loutcat->tab);
    tabnames = tabs_list(lincat, &ntabs);
/*
 *  Then copy and save all unaltered tables
 */
    for (k=1;k<ntabs;k++) {
       if (strcmp(tabnames[k], table_name) != 0) {
          copy_tab(lincat, tabnames[k], 0, loutcat, 0);
          save_tab(loutcat, name_to_tab(loutcat, tabnames[k], 0));
       }
    }
    for (k=1;k<ntabs;k++) {
       if (strcmp(tabnames[k], table_name) == 0) {
          tab = name_to_tab(lincat, table_name, 0);
          outtab = new_tab(table_name);
          key = tab->key;
          for (j=0;j<tab->nkey;j++) {
             copy_key(tab, key->name, outtab, 0);
             okey = name_to_key(outtab, key->name);
             okey->nobj   = 1;
             ED_CALLOC(okey->ptr, "save_catalog", char, (key->nbytes));
             key = key->nextkey;
          }
/*
 *        Add the new columns if they are not already there
 */
          for (j=0;j<noutkeys;j++) {
             if (outkeys[j]) {
                if (!(key = name_to_key(outtab, outkeys[j]->name))) {
                  key = outkeys[j];
                  switch (key->ttype) {
                  case T_SHORT:
                  case T_LONG:
                  case T_FLOAT:
                  case T_DOUBLE:
                  case T_BYTE:
                     ED_CALLOC(ptr, "save_catalog", char, t_size[key->ttype]);
                     add_key_to_tab(outtab, key->name, 1, ptr, key->ttype,
                                    1, key->unit, key->comment);
                     break;
                  case T_STRING:
                     ED_CALLOC(ptr, "save_catalog", char, key->nbytes);
                     add_key_to_tab(outtab, key->name, key->nbytes, ptr,
                                    key->ttype, 1, key->unit, key->comment);
                     break;
                  }
                }
             }
          }
          outtab->cat = loutcat;
          init_writeobj(loutcat, outtab);
       }
    }
    if(!(tab = name_to_tab(lincat, table_name, 0)))
    {
       syserr(FATAL, "init_catalogs", "No table %s in input catalog %s\n",
              table_name, lincat->filename);
    }
    intab = init_readobj(tab);
    for (j=0;j<ninkeys;j++)
       if (!(inkeys[j] = name_to_key(intab, keynames[j])))
          syserr(FATAL, "init_catalogs", "No key %s in input table %s\n",
               keynames[j], table_name);
    for (j=0;j<noutkeys;j++) {
        strcpy(str, outkeys[j]->name);
        ED_FREE(outkeys[j]);
        outkeys[j] = name_to_key(outtab, str);
    }
    *in_objtab  = intab;
    *out_objtab = outtab;
    *outcat     = loutcat;
    for (j=0;j<ntabs;j++)
       ED_FREE(tabnames[j]);
    ED_FREE(tabnames);
}

