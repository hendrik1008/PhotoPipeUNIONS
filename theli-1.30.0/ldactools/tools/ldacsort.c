#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "fitscat.h"
#include "ldacsort_globals.h"
#include "lists_globals.h"
#include "tabutil_globals.h"
#include "obj_obj_util.h"

/*
 *    Global variables do not take as much memory
 */

/*
   23.01.01:
   fixed a bug in save_catalogs: the variable noutkeys
   has to be initialised to 0

   04.01.05:
   removed compiler warnings (gcc with -O3 optimisation)

   11.03.2005:
   - function save_catalog:
     changes to reflect the new syntex of init_catalogs

   - changes to avoid compiler warnings with '-W -Wall -pedantic'

   27.03.2006:
   - I renamed sorting and indexing functions to the new naming scheme
     described in the sort_globals.h file
   - I corrected a bug if the sorting key is of type LONG. It was treated
    as an int in this case which is wrong on 64-bit machines.

   28.03.2006:
   I removed unnecesaary #include directives.
*/

static control_struct control;

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
    fprintf(stderr,"	-o outcat1\n");
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
         case 'o': strcpy(c->outfile,argv[++i]);
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


void save_catalog(catstruct *incat, int *idxpos, int n,
                  char *filename, char *table_name, int argc,
                  char **argv)
/****** save_catalog ***************************************************
 *
 *  NAME	
 *      save_catalog - Copy the input to output
 *
 *  SYNOPSIS
 *      void save_catalog(catstruct *incat, int *idxpos,
 *                        int n, char *filename, char *table_name)
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
 *      idxpos     - The index that sorts the input catalog on position
 *      n          - Number of input catalog objects
 *      filename   - Name of the output catalog
 *      table_name - Name of the table to recieve bi_col and that needs sorting
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
    catstruct	*outcat;
    tabstruct	*tab, *intab, *outtab;
    keystruct	*key, *okey, **outkeys=NULL;
    int		i, j, k, noutkeys=0;

    init_catalogs(incat, &outcat, filename,
                  table_name, NULL, NULL, 0,
                  outkeys, noutkeys, &intab, &outtab,
                  "ldacsort", argc, argv);
    tab = name_to_tab(incat, table_name, 0);
/*
 *  Start the copy process
 */
    for (i=0;i<n;i++) {
       if (control.ascending == NO)
         k = n - 1 - i;
       else
         k = i;
       read_obj_at(intab, tab, idxpos[k]);
/*
 *     Because the order of the output table's keys is the same as that
 *     for the input (see above) we can walk both key chains in parallel
 */
       key  = intab->key;
       okey = outtab->key;
       for (j=0;j<intab->nkey;j++) {
          memcpy(okey->ptr, key->ptr, key->nbytes);
          key = key->nextkey;
          okey = okey->nextkey;
       }
       if ((i) % 2048 == 0)
          VPRINTF(NORMAL,"Objects processed: %d from %d\n", i , n);
       write_obj(outtab);
    }
    end_writeobj(outcat, outtab);
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
     int	POSKEYS = 1, n;
     int	*idxpos;
     int	i;
     catstruct	*incat;
     tabstruct	*tab;
     keystruct	**keys;
     char	**keynames;
     void	*sort_col;
     char       header[MAXCHAR];

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
     VPRINTF(NORMAL, "Reading position key\n");
     ED_CALLOC(keys,     "make_pairing", keystruct *, POSKEYS);
     ED_CALLOC(keynames, "make_pairing", char *,      POSKEYS);
     for (i=0;i<POSKEYS;i++)
        ED_CALLOC(keynames[i], "make_pairing", char, MAXCHAR);
     strcpy(keynames[0], control.sort_col);
     read_keys(tab, keynames, keys, POSKEYS, NULL);
     for (i=0;i<POSKEYS;i++)
        if (!keys[i])
          syserr(FATAL,"make_pairing", "No key %s in table\n", keynames[i]);
     n = keys[0]->nobj;
     ED_CALLOC(idxpos, "main", int, n);
     VPRINTF(NORMAL, "Making the index\n");
     switch (keys[0]->ttype) {
     case T_SHORT:
       ED_COPY_PTR(sort_col, "make_pairing", keys[0],
                   T_SHORT, "short");
       sindex(n, (short *)sort_col, idxpos);
       break;
     case T_LONG:
       ED_COPY_PTR(sort_col, "make_pairing", keys[0],
                   T_LONG, "long");
       lindex(n, (LONG *)sort_col, idxpos);
       break;
     case T_FLOAT:
       ED_COPY_PTR(sort_col, "make_pairing", keys[0],
                   T_FLOAT, "float");
       findex(n, (float *)sort_col, idxpos);
       break;
     case T_DOUBLE:
       ED_COPY_PTR(sort_col, "make_pairing", keys[0],
                   T_DOUBLE, "double");
       dindex(n, (double *)sort_col, idxpos);
       break;
     default:
       syserr(FATAL,"main","cannot sort key of type T_STRING or T_BYTE\n");
       break;
     }
/*
 *   Remove the data values from the memory to allow save_catalog to read
 *   in object-by-object mode
 */
     ED_FREE(keys[0]->ptr);
     save_catalog(incat, idxpos, n,
                  control.outfile, control.table_name,
                  argc, argv);

     VPRINTF(NORMAL,"Done\n");
     return(0);
}
