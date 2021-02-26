#include <stdio.h>
#include <string.h>
#include "filter_globals.h"
#include "fitscat.h"
#include "parse_globals.h"
#include "options_globals.h"
#include "utils_globals.h"
#include "utils_macros.h"
#include "tabutil_globals.h"
/*
   14.07.01:
   fixed a bug that caused a NULL operation of ldacfilter if the
   examined catalog only contained ONE table. The index of the loop
   writing the tables of the output cat has to start with 0, not 1

   21.03.02:
   updated the program so that it can mask filtered
   objects instead of deleting them

   02.03.2004:
   a free command at the end of the filter function had to
   be executed only if the filter condition was true.

   04.01.05:
   removed compiler warnings (gcc with -O3 optimisation)

   29.01.2005:
   - I added history information to the output catalog
   - I added argc and argv to the argument list of filter
     as they are needed for writing the HISTORY to the
     output catalog.

   06.12.2010:
   Bug fix (change) in fuction filter:
   The code line 'if (nobj_pos[nfobjs-1] == 1 && nflines != 1)'
   produced a core dump. Inspection of the code suggests that
   'nfobjs' needs to be replaced by 'nflines' but I am not sure
   about this. It is however clear that 'nfobjs' can lead to
   an access of not allocated memory!
*/

void filter(char *inname, char *outname, char *table_name, char *line,
            int code, int argc, char **argv, int filter, char* maskname)
{
    int		i, j, jj, k, nf, nnf, nobjs, nfobjs, nfk, nlines, ntabs;
    int		*obj_count=NULL, *obj_pos=NULL;
    int		*nobj_count, *nobj_pos, nflines;
    char	**keynames, **req_keynames;
    unsigned char	 *ptr;
    char	**fkeynames;
    char	**tabnames=NULL;
    unsigned char *mask_objs, *mask_fields;
    short         *mask_objs_short;

    tree_struct	*tr;
    desc_struct	*description;
    rec_struct	*data;
    catstruct	*i_cat, *o_cat;
    tabstruct	*i_tab, *i_ftab, *o_tab, *o_ftab, *w_tab;
    keystruct	*key, **i_keys, **o_keys;
    keystruct	*fkey, **i_fkeys, **o_fkeys;
    int		special,depth;

    if (!(i_cat = read_cat(inname)))
      error(EXIT_FAILURE, "*Error*: catalog not found: ", inname);
    if (!(i_tab = name_to_tab(i_cat, table_name, 0)))
      error(EXIT_FAILURE, "*Error*: found no such table: ", table_name);
    if (!(key = i_tab->key))
      error(EXIT_FAILURE, "*Error*: found no key in ", table_name);
    nf = i_tab->nkey;
    ED_CALLOC(keynames,    "filter", char *,      nf);
    ED_MALLOC(i_keys,      "filter", keystruct *, nf);
    ED_MALLOC(o_keys,      "filter", keystruct *, nf);
    for (i=0;i<nf;i++) {
       ED_CALLOC(keynames[i], "filter", char,      MAXCHAR);
       ED_CALLOC(i_keys[i],   "filter", keystruct, 1);
       ED_CALLOC(o_keys[i],   "filter", keystruct, 1);
       strcpy(keynames[i], key->name);
       key = key->nextkey;
    }

    if ((tr = parse_line(line, keynames, nf, "condition")) == NULL)
    {
       exit(1);
    }
    req_keynames = find_idents(tr, &nnf);

    if (nnf == 0)
       syserr(FATAL,"filter","No column names in filter condition\n");
/*
 *  We do always need the FIELD_POS column in case of the OBJECTS table
 */
    if (strcmp(table_name, "OBJECTS") == 0) {
       nnf++;
       ED_REALLOC(req_keynames,     "filter", char *, nnf);
       ED_MALLOC(req_keynames[nnf-1], "filter", char,   MAXCHAR);
       strcpy(req_keynames[nnf-1], "FIELD_POS");
    }
    VPRINTF(NORMAL, "Reading catalog %s\n", inname);
    read_keys(i_tab, req_keynames, i_keys, nnf, NULL);
    for (j=0;j<nnf;j++)
    {
       if (!(i_keys[j]))
       {
         syserr(FATAL, "filter", "No key %s in table %s\n",
           req_keynames[j], i_tab->extname);
       }
    }
    nobjs = i_keys[0]->nobj;
    ED_MALLOC(description, "filter", desc_struct, nnf);
    ED_MALLOC(data,        "filter", rec_struct,  nnf);
    ED_CALLOC(mask_objs,   "filter", unsigned char, nobjs);
    ED_CALLOC(mask_objs_short,   "filter", short, nobjs);
    nlines = 0;
    VPRINTF(NORMAL, "Filtering catalog\n");
    for (i=0;i<nobjs;i++)
    {
       for (j=0;j<nnf;j++)
       {
          strcpy(description[j].name, i_keys[j]->name);
          if (i_keys[j]->naxis > 0)
             depth = i_keys[j]->naxisn[0];
          else
             depth = 1;
          description[j].type = i_keys[j]->ttype;
          description[j].depth= depth;
          ptr = i_keys[j]->ptr;
          switch (i_keys[j]->ttype) {
          case T_BYTE:
             ED_CALLOC(data[j].b, "filter", char, depth);
             for (k=0;k<depth;k++)
                data[j].b[k] = ptr[i*depth+k];
             break;
          case T_SHORT:
             ED_CALLOC(data[j].i, "filter", short int, depth);
             for (k=0;k<depth;k++)
                data[j].i[k] = ((short int *)ptr)[i*depth+k];
             break;
          case T_LONG:
             ED_CALLOC(data[j].l, "filter", LONG, depth);
             for (k=0;k<depth;k++)
                data[j].l[k] = ((LONG *)ptr)[i*depth+k];
             break;
          case T_FLOAT:
             ED_CALLOC(data[j].f, "filter", float, depth);
             for (k=0;k<depth;k++)
                data[j].f[k] = ((float *)ptr)[i*depth+k];
             break;
          case T_DOUBLE:
             ED_CALLOC(data[j].d, "filter", double, depth);
             for (k=0;k<depth;k++)
                data[j].d[k] = ((double *)ptr)[i*depth+k];
             break;
          case T_STRING:
             strncpy(data[j].s,&((char *)ptr)[i],i_keys[j]->nbytes);
             break;
          }
       }

       if (question(tr, description, data))
       {
          if (code)
          {
             mask_objs[i] = FALSE;
          } else {
             mask_objs[i] = TRUE; nlines++;
          }
       }
       else
       {
          if (code)
          {
             mask_objs[i] = TRUE; nlines++;
          } else {
             mask_objs[i] = FALSE;
          }
       }

       for (j=0;j<nnf;j++)
          ED_FREE(data[j].b);
       if ((nlines+1) % 2048 == 0)
          VPRINTF(NORMAL,"Objects selected: %d\n", nlines-1);
    }
    for (i=0;i<nnf;i++) {
       ED_FREE(req_keynames[i]);
    }

    if (nlines == 0)
       syserr(FATAL, "filter", "No output lines\n");
    o_cat = new_cat(1);
    inherit_cat(i_cat, o_cat);
    historyto_cat(o_cat, "ldacfilter", argc, argv);

    if(filter == 0)
    {
      copy_tabs(i_cat, o_cat);

      if (!(w_tab = name_to_tab(o_cat, table_name, 0)))
      {
        error(EXIT_FAILURE, "*Error*: found no such table: ", table_name);
      }

      for(i=0; i<nobjs; i++)
      {
	mask_objs_short[i] = mask_objs[i];
      }
      add_key_to_tab(w_tab, maskname, nobjs, mask_objs_short,
  		     T_SHORT, 1, "", "");
    }
    else
    {
      tabnames = tabs_list(i_cat, &ntabs);
      special = FALSE;
      if (strcmp(table_name,"DISTORTIONS") == 0)
      {
  	 syserr(WARNING, "filter", "Cannot filter DISTORTIONS table\n");
      }

      for (k=0;k<ntabs;k++) {
  	 if ((strcmp(tabnames[k], table_name) != 0 &&
  	      strcmp(table_name, "OBJECTS") != 0 &&
  	      strcmp(table_name, "FIELDS") != 0) ||
  	    ((strcmp(table_name, "OBJECTS") == 0 ||
  	      strcmp(table_name, "FIELDS") == 0) &&
  	      strcmp(tabnames[k], "OBJECTS") != 0 &&
  	      strcmp(tabnames[k], "FIELDS") != 0)) {
  /*
   *        This is a table that can be copied straight, it is not the
   *        filtered table, and it is not FIELDS and not OBJECTS
   */
  	    if (strcmp(tabnames[k], "DISTORTIONS") == 0 &&
  		strcmp(table_name, "FIELDS"))
  	       syserr(WARNING, "filter",
  		 "Cannot filter FIELDS table with existing DISTORTIONS table\n");

  	    copy_tab(i_cat, tabnames[k], 0, o_cat, 0);
  	 } else {
  	     if (strcmp(tabnames[k], "OBJECTS") != 0 &&
  		 strcmp(tabnames[k], "FIELDS") != 0 &&
  		 strcmp(table_name, "OBJECTS") != 0 &&
  		 strcmp(table_name, "FIELDS") != 0) {
  /*
   *            This is the filtered table not FIELDS and not OBJECTS
   */
  		i_tab = name_to_tab(i_cat, tabnames[k], 0);
  		read_keys(i_tab, keynames, i_keys, nf, mask_objs);
  		o_tab = new_tab(tabnames[k]);
  		for (jj=0;jj<i_tab->nkey;jj++) {
  		   copy_key(i_tab, i_keys[jj]->name, o_tab, 0);
  		   key = name_to_key(o_tab, i_keys[jj]->name);
  		   key->ptr = i_keys[jj]->ptr;
  		}
  		add_tab(o_tab, o_cat, 0);
  	     } else {
  /*
   *             This is either the FIELDS or the OBJECTS table
   */
  		 special = TRUE;
  	     }
  	 }
      }
      if (special) {
  	 if (strcmp(table_name, "FIELDS") == 0) {
  /*
   *        We filtered the FIELDS table, copy first the filtered info
   *        to the output catalog and then correctly filter the
   *        OBJECTS table
   */
  	    short int	*fpos=NULL;
  	    int		nobj, nkeys;
  	    keystruct	**li_keys, **lo_keys, *lkey;
  	    tabstruct	*lo_tab, *li_tab;
  	    char		**lkeynames;
  	    unsigned char	*mask_object;

  	    o_tab = new_tab(table_name);
  	    read_keys(i_tab, keynames, i_keys, nf, mask_objs);
  	    for (jj=0;jj<i_tab->nkey;jj++) {
  	       copy_key(i_tab, i_keys[jj]->name, o_tab, 0);
  	       o_keys[jj] = name_to_key(o_tab, i_keys[jj]->name);
  	       o_keys[jj]->nobj = nlines;
  	       o_keys[jj]->ptr = i_keys[jj]->ptr;
  	       if (strcmp(o_keys[jj]->name, "OBJECT_COUNT") == 0) {
  		  obj_count = o_keys[jj]->ptr;
  	       }
  	       if (strcmp(o_keys[jj]->name, "OBJECT_POS") == 0) {
  		  obj_pos = o_keys[jj]->ptr;
  	       }
  	    }
  /*
   *        Fix the obj_pos parameter and save the table
   */
  	    obj_pos[0] = 1;
  	    for (i=1;i<nlines;i++)
  	       obj_pos[i] = obj_pos[i-1] + obj_count[i-1];
  	    update_tab(o_tab);
  	    add_tab(o_tab, o_cat, 0);
  /*
   *        Now filter the OBJECTS table
   */
  	    if (!(li_tab = name_to_tab(i_cat, "OBJECTS", 0)))
  	       error(EXIT_FAILURE, "*Error*: found no such table","OBJECTS");
  	    if (!(lkey = li_tab->key))
  	       error(EXIT_FAILURE, "*Error*: found no key in table", "OBJECTS");
  	    nkeys = li_tab->nkey;
  	    ED_CALLOC(lkeynames,    "filter", char *,        nkeys);
  	    ED_MALLOC(li_keys,      "filter", keystruct *,   nkeys);
  	    ED_MALLOC(lo_keys,      "filter", keystruct *,   nkeys);
  	    for (i=0;i<nkeys;i++) {
  	       ED_CALLOC(lkeynames[i], "filter", char,      MAXCHAR);
  	       ED_CALLOC(li_keys[i],   "filter", keystruct, 1);
  	       ED_CALLOC(lo_keys[i],   "filter", keystruct, 1);
  	       strcpy(lkeynames[i], lkey->name);
  	       lkey = lkey->nextkey;
  	    }
  	    ED_GETKEY(li_tab, lkey, "FIELD_POS", fpos, nobj, "filter");
  	    ED_CALLOC(mask_object,  "filter", unsigned char, nobj);
  	    nlines = 0;
  	    for (i=0;i<nobj;i++) {
  	       mask_object[i] = mask_objs[fpos[i]-1];
  	       if (mask_object[i]) nlines++;
  	    }
  	    lkey = li_tab->key;
  	    for (i=0;i<nkeys;i++)
  	    {
  	       if (strcmp(lkey->name, "FIELD_POS") == 0) {
  		  lkey->ptr = NULL;
  		  break;
  	       }
  	    }
  	    lo_tab = new_tab("OBJECTS");
  	    read_keys(li_tab, lkeynames, li_keys, nkeys, mask_object);
  	    for (jj=0;jj<li_tab->nkey;jj++)
  	    {
  	       copy_key(li_tab, li_keys[jj]->name, lo_tab, 0);
  	       lo_keys[jj] = name_to_key(lo_tab, li_keys[jj]->name);
  	       lo_keys[jj]->nobj = nlines;
  	       lo_keys[jj]->ptr = li_keys[jj]->ptr;
  	       if (strcmp(lo_keys[jj]->name, "FIELD_POS") == 0)
  	       {
  		  fpos = lo_keys[jj]->ptr;
  	       }
  	    }
  /*
   *        Fix the field_pos parameter
   */
  	    j = fpos[0]; jj = 1;
  	    for (i=0;i<nlines;i++)
  	    {
  		if (fpos[i] != j)
  		{
  		   j = fpos[i];
  		   jj++;
  		}
  		fpos[i] = jj;
  	    }
  	    update_tab(lo_tab);
  	    add_tab(lo_tab, o_cat, 0);
  	 }
  	 if (strcmp(table_name, "OBJECTS") == 0)
  	 {

  /*
   *        We filtered the OBJECTS table, copy first the filtered info
   *        to the output catalog and then correctly filter the
   *        FIELDS table
   */
  	    short int	*fpos=NULL;
  /*
   *        Copy the filtered information to the output catalog and retain
   *        a pointer to the FIELD_POS for the OBJECTS table
   */
  	    o_tab = new_tab(table_name);
  	    read_keys(i_tab, keynames, i_keys, i_tab->nkey, mask_objs);
  	    for (jj=0;jj<i_tab->nkey;jj++)
  	    {
  	       copy_key(i_tab, i_keys[jj]->name, o_tab, 0);
  	       o_keys[jj] = name_to_key(o_tab, i_keys[jj]->name);
  	       o_keys[jj]->nobj = nlines;
  	       o_keys[jj]->ptr = i_keys[jj]->ptr;
  	       if (strcmp(o_keys[jj]->name, "FIELD_POS") == 0)
  	       {
  		  fpos = o_keys[jj]->ptr;
  	       }
  	    }
  /*
   *        Copy the FIELDS table and update the pointing information
   */
  	    i_ftab = name_to_tab(i_cat, "FIELDS", 0);
  /*
   *        Check for the direct access columns
   */
  	    if (!(key = name_to_key(i_ftab, "OBJECT_COUNT")))
  	       syserr(FATAL, "filter",
  		      "No column OBJECT_COUNT in table FIELDS\n");
  	    if (!(key = name_to_key(i_ftab, "OBJECT_POS")))
  	       syserr(FATAL, "filter",
  		      "No column OBJECT_POS in table FIELDS\n");
  	    nfobjs = key->nobj;
  	    ED_CALLOC(obj_count,   "filter", int,           nfobjs);
  	    ED_CALLOC(obj_pos,     "filter", int,           (nfobjs+1));
  	    ED_CALLOC(mask_fields, "filter", unsigned char, nfobjs);
  /*
   *        Make a new direct access pointer set. For those `frames'
   *        resulting in zero objects just keep the header info. First
   *        find the reference column in the OBJECTS table
   */
  /*
   *        Do the counting
   */
  	    for (i=0;i<nlines;i++)
  	    {
  	       obj_count[fpos[i]-1]++;
  	    }
  	    nflines = 0;
  	    for (i=0;i<nfobjs;i++) {
  	       if (obj_count[i] > 0) {
  		  mask_fields[i] = TRUE;
  		  nflines++;
  	       }
  	    }
  	    ED_CALLOC(nobj_count, "filter", int, nflines);
  	    ED_CALLOC(nobj_pos,   "filter", int, (nflines+1));
  	    j = 0;
  	    nobj_pos[0] = 1;
  	    for (i=0;i<nfobjs;i++) {
  	       if (mask_fields[i]) {
  		  nobj_count[j] = obj_count[i];
  		  if (j > 0)
  		     nobj_pos[j]   = nobj_pos[j-1] + nobj_count[j-1];
  		  j++;
  	       }
  	    }
  	    if (nobj_pos[nflines-1] == 1 && nflines != 1)
  	       syserr(FATAL, "filter", "Table OBJECTS has no rows!\n");
  /*
   *        Fix the field_pos parameter and save the OBJECTS table
   */
  	    j = fpos[0]; jj = 1;
  	    for (i=0;i<nlines;i++) {
  		if (fpos[i] != j) {
  		   j = fpos[i];
  		   jj++;
  		}
  		fpos[i] = jj;
  	    }
  	    add_tab(o_tab, o_cat, 0);
  /*
   *        Create the output FIELDS table and save the new direct access
   *        pointing information into it. Also copy all refiltering keys
   *        ofcourse.
   */
  	    nfk = i_ftab->nkey;
  	    key = i_ftab->key;
  	    ED_CALLOC(fkeynames,    "filter", char *,      nfk);
  	    ED_MALLOC(i_fkeys,      "filter", keystruct *, nfk);
  	    ED_MALLOC(o_fkeys,      "filter", keystruct *, nfk);
  	    for (i=0;i<nfk;i++) {
  	       ED_CALLOC(fkeynames[i], "filter", char,      MAXCHAR);
  	       ED_CALLOC(i_fkeys[i],   "filter", keystruct, 1);
  	       ED_CALLOC(o_fkeys[i],   "filter", keystruct, 1);
  	       strcpy(fkeynames[i], key->name);
  	       key = key->nextkey;
  	    }
  /*
   *        Copy the filtered information to the output catalog
   */
  	    o_ftab = new_tab("FIELDS");
  	    read_keys(i_ftab, fkeynames, i_fkeys, nfk, mask_fields);
  	    key = i_ftab->key;
  	    for (jj=0;jj<i_ftab->nkey;jj++) {
  	       copy_key(i_ftab, i_fkeys[jj]->name, o_ftab, 0);
  	       o_fkeys[jj] = name_to_key(o_ftab, i_fkeys[jj]->name);
  	       o_fkeys[jj]->nobj = nflines;
  	       o_fkeys[jj]->ptr = i_fkeys[jj]->ptr;
  /*
   *           Make the local information arrays known to the final save_cat
   */
  	       fkey = name_to_key(o_ftab, key->name);
  	       if (strcmp(key->name,"OBJECT_COUNT") == 0)
  		  fkey->ptr = nobj_count;
  	       if (strcmp(key->name,"OBJECT_POS") == 0)
  		  fkey->ptr = nobj_pos;
  	       key = key->nextkey;
  	    }
  	    update_tab(o_ftab);
  	    add_tab(o_ftab, o_cat, 0);
  	 }
      }
    }
    VPRINTF(NORMAL, "Saving catalog %s\n", outname);
    save_cat(o_cat, outname);

    if(filter == 1)
    {
      for (i=0;i<ntabs;i++)
      {
        ED_FREE(tabnames[i]);
      }
      ED_FREE(tabnames);
    }
    ED_FREE(description);
    ED_FREE(data);
    ED_FREE(mask_objs);
    ED_FREE(mask_objs_short);
    for (i=0;i<nf;i++) ED_FREE(keynames[i]);
    ED_FREE(keynames);
}

