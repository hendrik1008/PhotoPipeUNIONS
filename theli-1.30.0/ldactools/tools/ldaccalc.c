/*
 				ldaccalc.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	LDACcalc
*
*	Author:		E.Deul, DeNIS/LDAC
*
*	Contents:	parsing and main loop.
*
*	Last modify:	20/01/12
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/*
   10.02.01:
   substitued "long int" by the "LONG" macro.

   20.03.02:
   fixed a bug that made the calculation with reference
   tables/cross references impossible (the set_function
   was called before the data arrays were set appropriately).

   TODO:
   make it work with depth !=1 keys

   24.01.05:
   Added an explicit cast in a strcpy command to avoid compiler
   warnings.

   29.01.2005:
   - I added history information to the output catalog
   - function calculate: I added agrc and argv to the arguments
     of this function as they are needed for writing the
     HISTORY to the output catalog.

   27.03.2006:
   I renamed sorting and indexing functions to the new naming scheme
   described in the sort_globals.h file

   02.12.2011:
   Version number goes to 1.0

   20.01.2012:
   I corrected a serious bug in derive_set_func. I also heavily
   'simplified' this function; version number goes to 1.1.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "fitscat.h"
#include "filter_globals.h"
#include "parse_globals.h"
#include "options_globals.h"
#include "utils_globals.h"
#include "tabutil_globals.h"
#include "sort_globals.h"
#include "moments_globals.h"
#include "utils_macros.h"

#define NAME	"Catalog Arithmatic Program"
#define VERS	"1.1"
#define DATE	__DATE__
#define MAXCOND 100

void calculate(char *, char *, char *, char *, char *, char *,
               char [MAXCOND][MAXCHAR],char [MAXCOND][MAXCHAR],
               char [MAXCOND][MAXCHAR],char [MAXCOND][MAXCHAR],
               int, int, char **);


#define		SYNTAX \
"ldaccalc -i Input_catalog \n\
          [-o Output_catalog]\n\
           -c <filter_condition> \n\
           -n <new_column_name> <new_column_comment>\n\
           -k  <new_column_type>\n\
           [-c <filter_condition> \n\
            -n <new_column_name> <new_column_comment>\n\
            -k  <new_column_type> \n\
            [-c <filter_condition> \n\
             -n <new_column_name> <new_column_comment>\n\
             -k  <new_column_type> ]..]\n\
          [-t <data_table_name>]\n\
          [-r <reference_table_name>]\n\
          [-x <cross_reference_column_name>]\n\
          [-s <set_function> (one of median, minimum, maximum\n\
               mean, avedev, stddev, or variance)]\n\
          [options]\n\n\
       Options are:	\
                        -v [verbosity_on]\n"

/********************************** main ************************************/
int main(int argc, char *argv[])

/****** ldaccalc ***************************************************
 *
 *  NAME	
 *      ldaccalc - Use conditional argument to make a new column in a catalog
 *
 *  SYNOPSIS
 *      ldaccalc -i incat [-o outcat] -c condition -n new_col [-t tablename]
 *
 *  FUNCTION
 *      This routine creates a new columns in the specified table using the
 *      arithmatic specified in the condition statement. The new row is
 *      a type double.
 *
 *      The syntax of the condition statement can be written down in the
 *      following  representation, where | is the or character, a string
 *      surrounded by (..)* may exist zero to n times, and -> means resolves
 *      into. Any string or character to be taken literal is surrounded by ''.`
 *
 *      [condition] -> '(' [condition] ')' ( ('AND'|'OR') '(' [condition] ')' )*
 *                      | ident ('='|'<='|'>='|'<'|'>') [expr]
 *                      | [func]
 *
 *      [expr]      -> [term] (('+'|'-') [term])*
 *
 *      [term]      -> [factor] (('*'|'/') [factor])*
 *
 *      [factor]    -> '(' [expr] ')' | [number] | ident
 *
 *      [number]    -> ('+'|'-'|$) (digit)* ('.'|$) (digit)*
 *
 *      [func]      -> (char) (char)* '(' [number] ',' [number]')'
 *
 *      The ident denoted above is in fact the name of an individual column
 *      from the specified table.
 *
 *  INPUTS
 *      -i incat      - The name of the input catalog file
 *     [-o outcat]    - The name of the output catalog file. The default
 *                      output catalog is default_filter.cat
 *      -c condition  - The conditional statement
 *      -n new_col    - The name of the new column
 *     [-t tabename]  - The name of the table for which the condition holds.
 *                      The default table name is OBJECTS.
 *
 *  RESULTS
 *
 *  RETURNS
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 07-04-1998
 *
 ******************************************************************************/
  {
   char		condition[MAXCOND][MAXCHAR], outfilename[MAXCHAR];
   char		datatabname[MAXCHAR], reftabname[MAXCHAR];
   char		refcolname[MAXCHAR], setfunction[MAXCHAR];
   char		infilename[MAXCHAR], header[MAXCHAR];
   char         new_col_name[MAXCOND][MAXCHAR];
   char	        new_col_comment[MAXCOND][MAXCHAR];
   char		new_col_type[MAXCOND][MAXCHAR];
   int		a, ni, nn, nk, nc;

  if (argc<6)
    error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);

/*default parameters */
  sprintf(datatabname, "OBJECTS");
  sprintf(outfilename, "default_calc.cat");

  ni=nc=nn=nk=0;
  altverbose("NORMAL");
  strcpy(reftabname, "");
  strcpy(setfunction, "median");
  for (a=0;a<MAXCOND;a++)
  strcpy(new_col_type[a], "DOUBLE");
  for (a=1; a<argc; a++)
    switch((int)tolower((int)argv[a][1]))
      {
      case 'i':	sprintf(infilename, "%s", argv[++a]);
                ni=1;
	        break;
      case 'c':	sprintf(condition[nc++], "%s", argv[++a]);
	        break;
      case 'n': sprintf(new_col_name[nn], "%s", argv[++a]);	
                sprintf(new_col_comment[nn++], "%s", argv[++a]);
	        break;
      case 'k':	strcpy(new_col_type[nk++], argv[++a]);
	        break;
      case 't':	strcpy(datatabname, argv[++a]);
	        break;
      case 'r':	strcpy(reftabname, argv[++a]);
	        break;
      case 'x':	strcpy(refcolname, argv[++a]);
	        break;
      case 's':	strcpy(setfunction, argv[++a]);
	        break;
      case 'o':	sprintf(outfilename, "%s", argv[++a]);
	        break;
      case 'v':	altverbose(argv[++a]);
	        break;
      default : error(EXIT_FAILURE,"SYNTAX: ", SYNTAX);
      }

  if (!ni || nc != nn || nn != nk)
    error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);

  sprintf(header,"%s Version: %s (%s)",NAME,VERS,DATE);
  VPRINTF(NORMAL,"\n   %s\n\n",header);

  calculate(infilename, outfilename, datatabname,
            reftabname, refcolname, setfunction,
            condition, new_col_name, new_col_comment, new_col_type, nc,
            argc, argv);

  return EXIT_SUCCESS;
  }

void median(float *t, int n, double *med, double *min, double *max)
{
    int		*indx, n2;

    ED_CALLOC(indx, "median", int, n);
    findex(n, t, indx);
    n2=n/2;

    if (2*n2 == n) {
       *med = 0.5*(t[indx[n2-1]]+t[indx[n2]]);
    } else {
       *med = t[indx[n2]];
    }

    *min = t[indx[0]];
    *max = t[indx[n-1]];
    ED_FREE(indx);
}

double *derive_set_func(catstruct *cat, /* input catalogue */
                        int *n,         /* number of entries in input table;
                                           modified at the end of this
                                           function */
                        double *col,
                        char *data_tab_name, /* input table name */
                        char *cross_ref_col_name,  /* cross reference column */
                        char *ref_tab_name, /* reference table name */
                        char *setfunc /* function to use (median etc.) */)
{
  tabstruct *ref_tab, *data_tab;
  keystruct *key;
  double *data;
  double med, min, max;
  float ave, adev, sdev, var, skew, curt;
  float	*temp;
  int i, curr_obj, *indx;
  int nobj, nr; /* nobj: number of objects */
  char *ptr;
  int *ref_col;

  if (!(ref_tab = name_to_tab(cat, ref_tab_name, 0))) {
     syserr(FATAL, "derive_set_func", "No table %s in catalog\n",
                   ref_tab_name);
  }

  nobj = ref_tab->key->nobj;
  ED_CALLOC(data, "derive_set_func", double, nobj);

  if (!(data_tab = name_to_tab(cat, data_tab_name, 0)))
     syserr(FATAL, "derive_set_func", "No table %s in catalog\n",
            data_tab_name);

  ED_GETKEY(data_tab, key, cross_ref_col_name, ptr, nr,
            "derive_set_func");

  if ((*n) != nr) {
    syserr(FATAL, "derive_set_func", "Internal error\n");
  }

  ED_CALLOC(ref_col, "derive_set_func", int, nr);

  for (i = 0; i < *n; i++) {
    if (key->ttype == T_SHORT) {
      ref_col[i] = (int) ((short int *)ptr)[i];
    } else {
      ref_col[i] = (int) ((LONG *)ptr)[i];
    }
  }

  ED_CALLOC(indx, "derive_set_func", int, (*n));
  iindex(*n, ref_col, indx);

  i = 0;
  ED_CALLOC(temp, "derive_set_func", float , (*n));
  while (i < *n) {
    curr_obj = ref_col[indx[i]] - 1;

    if (curr_obj < 0 || curr_obj >= nobj)
      syserr(FATAL, "derive_set_func",
             "Cross reference value %d out of bounds(%d,%d)\n",
             curr_obj, 1, nobj);

    nr = 0;
    while ((i < (*n)) && (ref_col[indx[i]] == (curr_obj + 1))) {
      temp[nr] = col[indx[i]];
      nr++;
      i++;
    }

    median(temp, nr, &med, &min, &max);
    if (strcmp(to_lower(setfunc), "median") == 0) {
      data[curr_obj] = med;
    } else {
      if (strcmp(to_lower(setfunc), "minimum") == 0) {
        data[curr_obj] = min;
      } else {
        if (strcmp(to_lower(setfunc), "maximum") == 0) {
          data[curr_obj] = max;
        } else {
          moments(temp, nr, &ave, &adev, &sdev, &var, &skew, &curt);
          if (strcmp(to_lower(setfunc), "mean") == 0) {
            data[curr_obj] = ave;
          } else {
            if (strcmp(to_lower(setfunc), "avedev") == 0) {
              data[curr_obj] = adev;
            } else {
              if (strcmp(to_lower(setfunc), "stddev") == 0) {
                data[curr_obj] = sdev;
              } else {
                if (strcmp(to_lower(setfunc), "variance") == 0) {
                  data[curr_obj] = var;
                } else {
                  syserr(FATAL, "derive_set_func",
                         "No set function %s\n", setfunc);
                }
              }
            }
          }
        }
      }
    }
  }

  ED_FREE(temp);

  /* modification of the function parameter n ! */
  *n = nobj;

  return (data);
}


void calculate(char *inname, char *outname, char *table_name,
               char *ref_tab_name, char *ref_col_name, char *setfunc,
               char line[MAXCOND][MAXCHAR],
               char new_col_name[MAXCOND][MAXCHAR],
               char new_col_comment[MAXCOND][MAXCHAR],
               char new_col_type[MAXCOND][MAXCHAR], int n,
               int argc, char **argv)
{
  int		i, j, k, l, nf, nnf, nobjs, nlines, ntabs, ocols;
  char	**keynames, **req_keynames;
  unsigned char	*ptr;
  char	**tabnames;
  tree_struct	*tr;
  desc_struct	*description;
  rec_struct	*data;
  catstruct	*i_cat, *o_cat;
  tabstruct	*i_tab, *o_tab;
  keystruct	*key, **i_keys, **o_keys;
  int		depth;
  double	*new_col, *set_col;
  char	*b_col;
  short int	*s_col;
  LONG	*l_col;
  float	*f_col;

  if (!(i_cat = read_cat(inname)))
    error(EXIT_FAILURE, "*Error*: catalog not found: ", inname);

  if (!(i_tab = name_to_tab(i_cat, table_name, 0)))
    error(EXIT_FAILURE, "*Error*: found no such table: ", table_name);

  if (!(key = i_tab->key))
    error(EXIT_FAILURE, "*Error*: found no key in ", table_name);

  nf = i_tab->nkey;
  o_cat = new_cat(1);
  inherit_cat(i_cat, o_cat);
  historyto_cat(o_cat, "ldaccalc", argc, argv);
  tabnames = tabs_list(i_cat, &ntabs);

  for (k = 1; k < ntabs; k++) {
    copy_tab(i_cat, tabnames[k], 0, o_cat, 0);
  }

  for (ocols = 0; ocols < n; ocols++) {
     ED_CALLOC(keynames,    "filter", char *,      nf);
     ED_MALLOC(i_keys,      "filter", keystruct *, nf);
     ED_MALLOC(o_keys,      "filter", keystruct *, nf);
     for (i = 0; i < nf; i++) {
        ED_CALLOC(keynames[i], "filter", char,      MAXCHAR);
        ED_CALLOC(i_keys[i],   "filter", keystruct, 1);
        ED_CALLOC(o_keys[i],   "filter", keystruct, 1);
        strcpy(keynames[i], key->name);
        key = key->nextkey;
     }

     if ((tr = parse_line(line[ocols],
                          keynames, nf, "expression")) == NULL) {
        exit(1);
     }
     req_keynames = find_idents(tr, &nnf);
     if (nnf == 0) {
        syserr(FATAL,"filter","No column names in arithmatic expression");
     }

     VPRINTF(NORMAL, "Reading catalog %s\n", inname);
     read_keys(i_tab, req_keynames, i_keys, nnf, NULL);

     for (i = 0, depth = 0; i < nnf; i++) {
        if (!depth) {
          if (i_keys[i]->naxisn) {
             depth = i_keys[i]->naxisn[0];
          } else {
             depth = 1;
          }
        }

        if ((i_keys[i]->naxisn && depth != i_keys[i]->naxisn[0]) ||
            (depth > 1 && !i_keys[i]->naxisn)) {
           syserr(FATAL,"filter","Not all input columns have equal depth\n");
        }
        ED_FREE(req_keynames[i]);
     }

     nobjs = i_keys[0]->nobj;
     ED_MALLOC(description, "filter", desc_struct, nnf);
     ED_MALLOC(data,        "filter", rec_struct,  nnf);

     for (j = 0; j < nnf; j++) {
        ED_CALLOC(data[j].b, "filter", char, 1);
     }
     ED_MALLOC(new_col, "filter", double, nobjs * depth);
     nlines = 0;
     VPRINTF(NORMAL, "Deriving new column values\n");

     for (i = 0; i < nobjs; i++) {
       for (j = 0; j < nnf; j++) {
         if (i_keys[j]->naxis > 0) {
           depth = i_keys[j]->naxisn[0];
         } else {
           depth = 1;
         }

         description[j].type = i_keys[j]->ttype;
         description[j].depth= depth;
         strcpy(description[j].name, i_keys[j]->name);
         ptr = i_keys[j]->ptr;

         switch (i_keys[j]->ttype) {
         case T_BYTE:
           ED_REALLOC(data[j].b, "filter", char, depth);
           for (k = 0; k < depth; k++) {
             data[j].b[k] = ptr[i * depth + k];
           }
           break;
         case T_SHORT:
           ED_REALLOC(data[j].i, "filter", short int, depth);
           for (k = 0; k < depth; k++) {
             data[j].i[k] = ((short *)ptr)[i * depth + k];
           }
           break;
         case T_LONG:
           ED_REALLOC(data[j].l, "filter", LONG, depth);
           for (k = 0; k < depth; k++) {
             data[j].l[k] = ((LONG *)ptr)[i * depth + k];
           }
           break;
         case T_FLOAT:
           ED_REALLOC(data[j].f, "filter", float, depth);
           for (k = 0; k < depth; k++) {
             data[j].f[k] = ((float *)ptr)[i * depth + k];
           }
           break;
         case T_DOUBLE:
           ED_REALLOC(data[j].d, "filter", double, depth);
           for (k = 0; k < depth; k++) {
             data[j].d[k] = ((double *)ptr)[i * depth + k];
           }
           break;
         case T_STRING:
           strncpy(data[j].s,(const char *)(&ptr[i]),i_keys[j]->nbytes);
           break;
         }
       }
       for (k = 0; k < depth; k++) {
         set_vec_col(tr, k);
         new_col[i * depth + k] = eval(tr, description, data);
       }
       if ((nlines+1) % 2048 == 0) {
         VPRINTF(NORMAL,"Objects selected: %d\n", nlines - 1);
       }
     }


     if (ref_tab_name[0] != '\0') {
	 for (k = 1; k < ntabs; k++) {
	   if (strcmp(tabnames[k], ref_tab_name) == 0) {
           /* note that the following call modifies the
              nobjs variable!
           */
           set_col = derive_set_func(i_cat, &nobjs, new_col, table_name,
                                     ref_col_name, ref_tab_name,
                                     setfunc);

           o_tab = name_to_tab(o_cat, tabnames[k], 0);
           if (!(strcmp(new_col_type[ocols],"BYTE"))) {
             ED_CALLOC(b_col, "calculate", char, nobjs);

             for (l = 0; l < nobjs; l++) {
               b_col[l] = (char) set_col[l];
             }
             add_key_to_tab(o_tab, new_col_name[ocols], nobjs, b_col,
                            T_BYTE, 1, "", new_col_comment[ocols]);
           }

           if (!(strcmp(new_col_type[ocols],"SHORT"))) {
             ED_CALLOC(s_col, "calculate", short int, nobjs);

             for (l = 0; l < nobjs; l++) {
               s_col[l] = (short int) set_col[l];
             }
             add_key_to_tab(o_tab, new_col_name[ocols], nobjs, s_col,
                            T_SHORT, 1, "", new_col_comment[ocols]);
           }

           if (!(strcmp(new_col_type[ocols],"LONG"))) {
             ED_CALLOC(l_col, "calculate", LONG, nobjs);

             for (l = 0; l < nobjs; l++) {
               l_col[l] = (LONG) set_col[l];
             }
             add_key_to_tab(o_tab, new_col_name[ocols], nobjs, l_col,
                            T_LONG, 1, "", new_col_comment[ocols]);
           }
           if (!(strcmp(new_col_type[ocols],"FLOAT"))) {
             ED_CALLOC(f_col, "calculate", float, nobjs);

             for (l = 0; l < nobjs; l++) {
               f_col[l] = (float) set_col[l];
             }
             add_key_to_tab(o_tab, new_col_name[ocols], nobjs, f_col,
                            T_FLOAT, 1, "", new_col_comment[ocols]);
           }
           if (!(strcmp(new_col_type[ocols],"DOUBLE"))) {
             add_key_to_tab(o_tab, new_col_name[ocols], nobjs, set_col,
                            T_DOUBLE, 1, "", new_col_comment[ocols]);
           }
	   }
       }
     } else {
       o_tab = name_to_tab(o_cat, table_name, 0);

	 if (!(strcmp(new_col_type[ocols],"BYTE"))) {
	   ED_CALLOC(b_col, "calculate", char, nobjs * depth);

	   for (l = 0; l < nobjs * depth; l++) {
	     b_col[l] = (char) new_col[l];
         }
	   add_key_to_tab(o_tab, new_col_name[ocols], nobjs, b_col,
			  T_BYTE, depth, "", new_col_comment[ocols]);
	 }
	 if (!(strcmp(new_col_type[ocols],"SHORT"))) {
	   ED_CALLOC(s_col, "calculate", short int, nobjs * depth);

	   for (l = 0; l < nobjs * depth; l++) {
	     s_col[l] = (short int) new_col[l];
         }
	   add_key_to_tab(o_tab, new_col_name[ocols], nobjs, s_col,
			  T_SHORT, depth, "", new_col_comment[ocols]);
	 }
	 if (!(strcmp(new_col_type[ocols],"LONG"))) {
	   ED_CALLOC(l_col, "calculate", LONG, nobjs * depth);

	   for (l = 0; l < nobjs * depth; l++) {
	     l_col[l] = (LONG) new_col[l];
         }
	   add_key_to_tab(o_tab, new_col_name[ocols], nobjs, l_col,
			  T_LONG, depth, "", new_col_comment[ocols]);
	 }
	 if (!(strcmp(new_col_type[ocols],"FLOAT"))) {
	   ED_CALLOC(f_col, "calculate", float, nobjs * depth);

	   for (l = 0; l < nobjs * depth; l++) {
	     f_col[l] = (float) new_col[l];
         }
	   add_key_to_tab(o_tab, new_col_name[ocols], nobjs, f_col,
			  T_FLOAT, depth, "", new_col_comment[ocols]);
	 }
	 if (!(strcmp(new_col_type[ocols],"DOUBLE"))) {
	   add_key_to_tab(o_tab, new_col_name[ocols], nobjs, new_col,
			  T_DOUBLE, depth, "", new_col_comment[ocols]);
	 }
     }

     ED_FREE(description);
     ED_FREE(data);

     for (i = 0; i < nf; i++) {
       ED_FREE(keynames[i]);
     }
     ED_FREE(keynames);
  }

  for (i = 0; i < ntabs; i++) {
    ED_FREE(tabnames[i]);
  }
  ED_FREE(tabnames);

  VPRINTF(NORMAL, "Saving catalog %s\n", outname);
  save_cat(o_cat, outname);
}

