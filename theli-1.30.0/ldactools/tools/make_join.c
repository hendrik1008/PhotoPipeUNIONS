#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "make_join_globals.h"
#include "lists_globals.h"
#include "tabutil_globals.h"
#include "tsize.h"

/* 23.02.01:
   substituted all "long int" by LONG

   05.05.03:
   included the tsize.h file

   09.06.2004:
   The reference column has to contain ascending
   numbers starting with 1. If possible we now give
   an error message and quit if this is not the case.

   29.01.2005:
   I added HISTORY Information to the output catalog.

   06.09.2005:
   - I got rid of thr restriction that the reference column
     has to contain ascending numbers starting with 1.
     (see also chanegs from 09.06.2004).
   - I removed compiler warnings.

   23.01.2006:
   I took back all changes made on 06/09/2005. These changes were
   incompatible to the previous program behaviour.

   01.05.2006:
   I removed the superfluous inclusion of precess_defs.h

   02.05.2006:
   A function call to 'close_catalogs' had too many arguments
*/

/*
 *    Global variables do not take as much memory
 */

static tabstruct	*main_tab, *ref_tab, *outtab;
static tabstruct	*main_itab, *ref_itab;
static catstruct	*incat, *outcat;
static int		ncols;

static control_struct control;

char header[MAXCHAR];

option my_opts[] = {
            {"COL_REF", TEXT, control.col_ref, 0, 0, 0.0, 0.0,
              0, 1.0, NO, "",
             {"", "", "", "", ""}, 0,
             "Name of the reference column to use as index"},
            {"COL_NAME", TEXTVEC, control.name, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""}, 0,
             "Name of the output column"},
            {"COL_INPUT", TEXTVEC, control.input, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""},0,
              "Name of the input column"},
            {"", BOOLEAN, NULL, 0, 0, 0.0, 0.0, 0, 0.0, NO, "",
             {"YES", "NO", "", "", ""}, 0,
              "dummy key to end structure my_opts"}
            };

void usage(char *me)
{
    fprintf(stderr,"Usage: %s \n", me);
    fprintf(stderr,"	-i incat\n");
    fprintf(stderr,"	-o outcat\n");
    fprintf(stderr,"	-c conf_file \n");
    fprintf(stderr,"	[-m table_name_main] \n");
    fprintf(stderr,"	[-r table_name_ref] \n");
    fprintf(stderr,"    [-OPTION VALUE [-OPTION VALUE ...]]\n\n");
    exit(1);
}


void init(int argc, char *argv[], control_struct *c)
{
/*
 *  Default file name is calling name with .conf extension
 */
    int i, j, k;

    sprintf(c->conffile,"make_join.conf");
    sprintf(c->table_name_main,"OBJECTS");
    sprintf(c->table_name_ref,"FIELDS");
    if (argc == 1) usage(argv[0]);
    init_options(0, my_opts);
    for (i=1; i<argc; i++) {
      if (argv[i][0] == '-') {
         switch(argv[i][1]) {
         case 'c': strcpy(c->conffile,argv[++i]);
                   break;
         case 'm': strcpy(c->table_name_main,argv[++i]);
                   break;
         case 'r': strcpy(c->table_name_ref,argv[++i]);
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
   col_name = copy_textvec(control.name, &ncols);
   if (ncols == 0)
      syserr(FATAL,"init","No output columns defined for %s\n",control.outfile);
   col_input = copy_textvec(control.input, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_INPUT keys differs from COL_NAME\n");
}

int init_catalogs(control_struct c)
{
    int 		i, j, ntabs, nobj;
    char		**tabnames;
    keystruct		*key, *inkey, *outkey;
    tabstruct		*tab;

/*
 *  Open the input tables:   Main read_obj    sequential
 *                           Ref  read_obj_at direct access
 *  Open the output catalog: write_obj        sequential
 */
    if (!(incat = read_cat(c.listfile)))
       syserr(FATAL, "init_catalogs", "Could not read catalog %s\n",
          c.listfile);
    outcat = new_cat(1);
    inherit_cat(incat, outcat);
/*
 *  First write the Primary FITS header unit
 */
    strcpy(outcat->filename, c.outfile);
    if (open_cat(outcat, WRITE_ONLY) != RETURN_OK)
       syserr(FATAL, "init_catalogs","Cannot open output catalog %s\n",
          c.outfile);
    save_tab(outcat, outcat->tab);
/*
 *  Copy all tables except the c.table_name_main table and save them
 */
    tabnames = tabs_list(incat, &ntabs);
    for (i=1;i<ntabs;i++) {
       if (strcmp(tabnames[i],c.table_name_main) != 0) {
          copy_tab(incat, tabnames[i], 0, outcat, 0);
          tab = name_to_tab(outcat, tabnames[i], 0);
          save_tab(outcat, tab);
       }
       ED_FREE(tabnames[i]);
    }
    ED_FREE(tabnames);
/*
 *  Create the table description for objectwise writing of the main
 *  table. First copy all the main tables keys and make nobj = 1
 */
    if (!(outtab = new_tab(c.table_name_main)))
       syserr(FATAL, "init_catalogs", "Could not create table %s!\n",
            c.table_name_main);
    if (!(tab = name_to_tab(incat, c.table_name_main, 0)))
       syserr(FATAL, "init_catalogs", "No table %s in catalog %s!\n",
            c.table_name_main, incat->filename);
    key = tab->key;
    nobj = key->nobj;
    for (i=0;i<tab->nkey;i++) {
       copy_key(tab, key->name, outtab, 0);
       outkey = name_to_key(outtab, key->name);
       outkey->nobj = 1;
       outkey->nobj = nobj;
       ED_CALLOC(outkey->ptr, "init_catalogs", char, outkey->nbytes);
       key = key->nextkey;
    }
/*
 *  Get the description of reference table keys to be joined to the
 *  main table and make nobj = 1
 */
    if (!(tab = name_to_tab(incat, c.table_name_ref, 0)))
       syserr(FATAL, "init_catalogs", "No table %s in catalog %s!\n",
            c.table_name_main, incat->filename);

    for (i=0;i<ncols;i++) {
/*
 *     Make the keys for the output table based on the info from the input key
 *     from the reftab
 */
       key = new_key(col_name[i]);
       if ((inkey = name_to_key(tab, col_input[i])) == NULL)
          syserr(FATAL,"init_catalogs", "No key: %s in input catalog\n",
                       col_input[i]);
       *key = *inkey;
       strcpy(key->name, col_name[i]);
       strcpy(key->comment,inkey->comment);
       strcpy(key->printf,inkey->printf);
       strcpy(key->unit,inkey->unit);
       key->nobj = 1;
       key->nobj = nobj;
       key->nextkey = key->prevkey = NULL;
       if (key->naxis != 0) {
          ED_MALLOC(key->naxisn, "init_catalogs", int, key->naxis);
          for (j=0;j<key->naxis;j++) key->naxisn[j] = inkey->naxisn[j];
       }
       key->nbytes = t_size[key->ttype];
       for (j=0;j<key->naxis;j++)
          key->nbytes *= key->naxisn[j];
       key->ptr = NULL;
       ED_MALLOC(key->ptr, "init_catalogs", char, key->nbytes);
       if (add_key(key, outtab, 0)==RETURN_ERROR)
          syserr(FATAL, "create_ssc_table", "Error adding key %s to table %s\n",             key->name, c.table_name_ref);
    }
    update_tab(outtab);

    outtab->cat = outcat;
/*
 *  Setup direct access read system
 */
    if (!(main_tab = name_to_tab(incat, control.table_name_main, 0)))
          syserr(FATAL, "init_catalogs", "No table %s in catalog %s\n",
                 control.table_name_main, control.listfile);
    main_itab = init_readobj(main_tab);
    if (!(ref_tab = name_to_tab(incat, control.table_name_ref, 0)))
          syserr(FATAL, "init_catalogs", "No table %s in catalog %s\n",
                 control.table_name_ref, control.listfile);
    ref_itab  = init_readobj(ref_tab);
    init_writeobj(outcat, outtab);

    return nobj;
}
void save_row()
{
    keystruct	*key, *outkey;
    int		i, j;
    LONG	row_nr=0;
    char	*fptr, *tptr;

/*
 *  First get the reference key value
 */
    if (!(key = name_to_key(main_itab, control.col_ref)))
       syserr(FATAL, "save_row", "No key %s in table %s\n",
              control.col_ref,control.table_name_main);
    switch (key->ttype) {
    case T_SHORT: row_nr = (LONG) *((short int *)key->ptr);
                  break;
    case T_LONG:  row_nr = *((LONG *)key->ptr);
                  break;
    default:
           syserr(FATAL, "save_row",
              "Cannot make_join on column %s with wrong type\n  (Allowed types: SHORT, LONG)\n",
              key->name);
    }
/*
 *  Copy the main table information to the output table
 */
    key = main_itab->key;
    for (i=0;i<main_itab->nkey;i++) {
       outkey = name_to_key(outtab, key->name);
       tptr = outkey->ptr;
       fptr = key->ptr;
       for (j=0;j<key->nbytes;j++)
          *(tptr++) = *(fptr++);
       key = key->nextkey;
    }
/*
 *  Append the required information to the output table
 */
    if(read_obj_at(ref_itab, ref_tab, row_nr-1) == RETURN_ERROR)
    {
           syserr(FATAL, "save_row",
              "Cannot read value of referenced key(s) at position %d in the reference table.\n",
              row_nr);
    }
    for (i=0;i<ncols;i++)
    {
       key    = name_to_key(ref_itab, col_input[i]);
       outkey = name_to_key(outtab,   col_name[i]);
       tptr = outkey->ptr;
       fptr = key->ptr;
       for (j=0;j<key->nbytes;j++)
          *(tptr)++ = *(fptr++);
       key = key->nextkey;
    }
/*
 *  And save it to disk
 */
    write_obj(outtab);
}

void close_catalogs()
{
    end_writeobj(outcat, outtab);
    end_readobj(main_itab, main_tab);
}

int main(int argc, char *argv[])
/****** make_join ***************************************************
 *
 *  NAME	
 *      make_joins - Perform row-by-row joining
 *
 *  SYNOPSIS
 *      make_join -i incat1 incat2 -o outcat1 outcat2
 *                -t table1 table2 [-c configfile] [CONFOPTION]
 *
 *  FUNCTION
 *      This program makes joining of two tables based on the equality of
 *      a common column.
 *
 *  INPUTS
 *      -i incat1 incat2     - The list of input catalogs
 *      -o outcat            - The list of output catalogs
 *      -t table1 table2     - The name of the input tables
 *     [-c configfile]       - The name of the configuration file.
 *     [CONFOPTION]          - Pairs of configuration VARIABLE VALUE
 *                             sequences that allow dynamically
 *                             overwritting of configuration file values.
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
 *      E.R. Deul - dd 27-11-1997
 *
 ******************************************************************************/
{
     int       i, n;

     init(argc, argv, &control);

     sprintf(header,"%s Version: %s (%s)",PROGRAM,VERS,DATE);
     VPRINTF(NORMAL,"\n   %s\n\n",header);

/*
 *   Loop over the main table and find the joining line from the ref table
 *   Read line by line from the main table and allow direct access read
 *   for the ref table
 */
     n = init_catalogs(control);

     for (i=0;i<n;i++) {
        read_obj_at(main_itab, main_tab, i);
        save_row();
     }

/* add HISTORY information to the output catalog */
     historyto_cat(outcat, "make_join", argc, argv);

     close_catalogs();
     VPRINTF(NORMAL,"Done\n");
     exit(0);
}
