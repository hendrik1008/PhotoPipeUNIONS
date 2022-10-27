#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "fitscat.h"
#include "ldacsort_globals.h"
#include "lists_globals.h"
#include "tabutil_globals.h"

/* 10.01.03:
 * resolved compiler warnings under Linux
 *
 * 24.01.05:
 * decreased a LON constant to avoid promoting
 * of this variable to a new type (SunOS compiler).
 *
 * 07.03.2006:
 * I removed compiler warnings connected with the opts structure
 *
 * 28.03.2006:
 * I removed unnecessary #include directives.
 */

/*
 *    Global variables do not take as much memory
 */
static control_struct control;

char header[MAXCHAR];

option my_opts[] = {
            {"SORT_COL", TEXT, control.sort_col, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "Ra",
              {"", "", "", "", ""}, 0,
              "The name of the table column containing the sorting key"},
            {"", BOOLEAN, NULL, 0, 0, 0.0, 0.0, 0, 0.0, NO, "",
             {"YES", "NO", "", "", ""}, 0,
              "dummy key to end structure my_opts"}
            };

void usage(char *me)
{
    fprintf(stderr,"Usage: %s \n", me);
    fprintf(stderr,"	-i incat1\n");
    fprintf(stderr,"	[-c conf_file] \n");
    fprintf(stderr,"	[-t table_name] \n");
    fprintf(stderr,"	[-r] \n\n");
    exit(1);
}


void init(int argc, char *argv[], control_struct *c)
{
/*
 *  Default file name is calling name with .conf extension
 */
    int i, j, k;

    sprintf(c->table_name,"OBJECTS");
    c->ascending = YES;
    if (argc == 1) usage(argv[0]);
    init_options(0, my_opts);
    for (i=1; i<argc; i++) {
      if (argv[i][0] == '-') {
         switch(argv[i][1]) {
         case 'r': c->ascending = NO;
                   break;
         case 'c': strcpy(c->conffile,argv[++i]);
                   break;
         case 't': strcpy(c->table_name,argv[++i]);
                   break;
         case 'i': strcpy(c->listfile,argv[++i]);
                   break;
         case '@': print_options(argv[0]);
                   exit(0);
         default:  parse_options(c->conffile);
                   j = i++; k = i;
                   set_command_option(argv[j], argv[k]);
                   break;
         }
      } else {
         syserr(WARNING,"init","Command line syntax error\n");
         usage(argv[0]);
      }
   }
   finish_options(c->conffile);
}


void examine_catalog(catstruct *incat, char *table_name)
/****** save_catalog ***************************************************
 *
 *  NAME	
 *      save_catalog - Copy the input to output
 *
 *  SYNOPSIS
 *      void save_catalog(catstruct *incat, char *table_name)
 *
 *  FUNCTION
 *      This routine copies the input catalog to the output catalog
 *      adding the bit interleave value to the required output table.
 *      The copying is done on an object by object bases while using
 *      index that sorts the input catalog on position. Therefore the
 *      output catalog is position sorted.
 *
 *  INPUTS
 *      incat      - Pointer to teh input catalog
 *      table_name - Name of the table to be examined
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
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
    tabstruct	*tab, *intab;
    keystruct	*key;
    int		i;
    typedef	union {
                 short int s;
                 LONG  l;
                 float     f;
                 double    d;
                } tstore;
    tstore	*min, *max;

    tab = name_to_tab(incat, table_name, 0);
    intab = init_readobj(tab);
/*
 *  Start the copy process
 */
   ED_MALLOC(min, "ldacminmax", tstore, tab->nkey);
   ED_MALLOC(max, "ldacminmax", tstore, tab->nkey);
   for (i=0,key=tab->key;i<tab->nkey;i++,key=key->nextkey) {
      switch (key->ttype) {
         case T_SHORT: min[i].s = 32767;
                       max[i].s = -32767;
                       break;
         case T_LONG:  min[i].l = 2147483647L;
                       max[i].l = -2147483647L;
                       break;
         case T_FLOAT: min[i].f = 1e30;
                       max[i].f = -1e30;
                       break;
         case T_DOUBLE:min[i].d = 1e132;
                       max[i].d = -1e132;
                       break;
         default:
           syserr(WARNING,"examine_catalog","skip key of type BYTE or STRING\n");
	   break;
      }
   }
   while (read_obj(intab, tab)) {
      for (i=0,key=tab->key;i<tab->nkey;i++,key=key->nextkey) {
      switch (key->ttype)
         {
         case T_SHORT: if (*(short int *)key->ptr < min[i].s)
                         min[i].s = *(short int *)key->ptr;
                       if (*(short int *)key->ptr > max[i].s)
                         max[i].s = *(short int *)key->ptr;
                       break;
         case T_LONG:  if (*(LONG *)key->ptr < min[i].l)
                         min[i].l = *(LONG *)key->ptr;
                       if (*(LONG *)key->ptr > max[i].l)
                         max[i].l = *(LONG *)key->ptr;
                       break;
         case T_FLOAT: if (*(float *)key->ptr < min[i].f)
                         min[i].f = *(float *)key->ptr;
                       if (*(float *)key->ptr > max[i].f)
                         max[i].f = *(float *)key->ptr;
                       break;
         case T_DOUBLE:if (*(double *)key->ptr < min[i].d)
                         min[i].d = *(double *)key->ptr;
                       if (*(double *)key->ptr > max[i].d)
                         max[i].d = *(double *)key->ptr;
                       break;
         default:
           syserr(WARNING,"examine_catalog","skip key of type BYTE or STRING\n");
	   break;
	
         }
       }
       if ((i) % 2048 == 0)
          VPRINTF(NORMAL,"Objects processed: %d \n", i);
    }
   for (i=0,key=tab->key;i<tab->nkey;i++,key=key->nextkey)
      switch (key->ttype) {
         case T_SHORT: printf(" %s min: %d, max %d\n", key->name, min[i].s, max[i].s);
                       break;
         case T_LONG:
#ifdef INT64
                       printf(" %s min: %d, max %d\n", key->name, min[i].l, max[i].l);
#else
                       printf(" %s min: %ld, max %ld\n", key->name, min[i].l, max[i].l);
#endif
                       break;
         case T_FLOAT: printf(" %s min: %f, max %f\n", key->name, min[i].f, max[i].f);
                       break;
         case T_DOUBLE:printf(" %s min: %g, max %g\n", key->name, min[i].d, max[i].d);
                       break;
         default:
           syserr(WARNING,"examine_catalog","skip key of type BYTE or STRING\n");
	   break;
         }
}

int main(int argc, char *argv[])

/****** ldacsort ***************************************************
 *
 *  NAME	
 *      ldacsort - Sort on any key
 *
 *  SYNOPSIS
 *      ldacsort -i incat1 -o outcat1
 *                [-c configfile] [CONFOPTION]
 *
 *  FUNCTION
 *      This program sort a specified table according to the sorting key
 *
 *  INPUTS
 *      -i incat1      - The name of input catalog
 *      -o outcat1     - The name of output catalogs
 *     [-c configfile] - The name of the configuration file.
 *     [-t table_name] - The name of the table to be sorted
 *     [-r]            - Descending instead of ascending order
 *     [CONFOPTION]    - Pairs of configuration VARIABLE VALUE
 *                       sequences that allow dynamically
 *                       overwritting of configuration file values.
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
 *      E.R. Deul - dd 26-05-1998
 *
 ******************************************************************************/
{
     catstruct	*incat;
     tabstruct	*tab;

     init(argc, argv, &control);

     sprintf(header,"%s Version: %s (%s)",PROGRAM,VERS,DATE);
     VPRINTF(NORMAL,"\n   %s\n\n",header);
/*
 *   Open the table to be sorted and read the position keys
 */
     if (!(incat = read_cat(control.listfile)))
         syserr(FATAL, "ldacsort", "Could not open catalog %s\n",
                   control.listfile);
     if (!(tab = name_to_tab(incat,control.table_name, 0)))
         syserr(FATAL, "ldacsort", "Could not find table %s\n",
                   control.table_name);
     examine_catalog(incat, control.table_name);

     VPRINTF(NORMAL,"Done\n");
     return(0);
}
