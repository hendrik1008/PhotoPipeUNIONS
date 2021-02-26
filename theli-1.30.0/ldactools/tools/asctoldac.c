/*
   History:
   03.01.2000:
     included outtab = NULL in the beginning of
     the main program. Not doing this led to a
     crash on IRIX system at CITA (beluga)
   28.11.2000:
     included some parantheses in the my_opts initialisation
     to avoid warning messages.
   09.02.2001:
     corrected a bug in the creation of the OBJECTS table.
     In the case of a new objects table the FIELD_POS key was
     only set correctly to one for the first objects. For all
     others it was undefined.
     substituted the "long int" by the "LONG" macro.
   05.05.03:
     included the tsize.h file
   07.03.2006:
     I removed compiler warnings connected with the opts
     structure.
   03.07.2007:
     For converting strings to ints or longs I introduced a
     'clean' distinction between 'atoi' and 'atol' depending
     on architecture (INT64 set or not).
   30.08.2010:
     We observed 'strangely filled' string elements in some
     LDAC catalogue. Unfortunately, I could not reproduce it
     on my machines! A possible reason can be that the initial
     memory for STRING objects was not initialised to zero
     (use of malloc instead of calloc). I changed that.
   03.12.2011:
     I increased the maximum allowed size for input ASCII lines
     to 10240 characters. Catherine came up with lines longer than
     5120 chars.
   15.08.2018:
     I corected a bug in the treatment of key units. If the key was
     given as "" in the config file (no unit), the key actucal was
     assigned a 'unit' consisting of '""'. I corrected this.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "fitscat.h"
#include "fitscat_defs.h"
#include "tsize.h"
#include "utils_globals.h"
#include "options_globals.h"
#include "asctoldac_defs.h"
#include "tabutil_globals.h"

#ifdef SHORT
#undef SHORT
#endif

typedef struct {
   char conffile[MAXCHAR];
   char asciifile[MAXCHAR];
   char infile[MAXCHAR];
   char outfile[MAXCHAR];
   char table_name[MAXCHAR];
   char separator[MAXCHAR];
   char chan_name[MAXCHAR];
   int  chan_nr;
   int  ilist;
   char name[TEXTVECLEN];
   char ttype[TEXTVECLEN];
   char htype[TEXTVECLEN];
   char comm[TEXTVECLEN];
   char unit[TEXTVECLEN];
   char depth[TEXTVECLEN];

} control_struct;

control_struct control;
char **col_name;
char **col_ttype;
char **col_htype;
char **col_comm;
char **col_unit;
char **col_depth;


LONG count;

option my_opts[] = {
            {"SEPARATOR", TEXT, control.separator, 0, 0, 0.0, 0.0,
              0, 0.0, NO, " 	",
             {"", "", "", "", ""}, 0,
             "The separator characters used to separate elements from the ascii file"},
            {"COL_NAME", TEXTVEC, control.name, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""},0,
             "The name of the output column"},
            {"COL_TTYPE", TEXTVEC, control.ttype, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""}, 0,
             "The type of the output column, can be BYTE, SHORT, LONG, FLOAT, DOUBLE, or STRING"},
            {"COL_HTYPE", TEXTVEC, control.htype, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""}, 0,
             "The print type of the output column, can be INT, FLOAT, EXPO, BOOL, or STRING"},
            {"COL_COMM", TEXTVEC, control.comm, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""}, 0,
             "The comment line for the key. Enclose within double quotes"},
            {"COL_UNIT", TEXTVEC, control.unit, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""}, 0,
             "The unit  for the key. Enclose within double quotes"},
            {"COL_DEPTH", TEXTVEC, control.depth, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "",
             {"", "", "", "", ""}, 0,
             "The depth of the key (>1 for vectors or strings)"},
            {"", BOOLEAN, NULL, 0, 0, 0.0, 0.0, 0, 0.0, NO, "",
             {"YES", "NO", "", "", ""}, 0,
              "dummy key to end structure my_opts"}
            };

int	*nobj;
int	ncols, ndepths;

/**/
static void usage(char *me)
{
/****i* usage ***************************************************
 *
 *  NAME
 *      usage - Print usage information to the user
 *
 *  SYNOPSIS
 *      void usage(char *me)
 *
 *  FUNCTION
 *      Print information of command line arguments and general usage
 *      of the program to the user on standard error channel.
 *
 *  INPUTS
 *      me - String containing the calling name of the program
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
 *      E.R. Deul - dd 10-10-1996
 *
 ******************************************************************************/
    fprintf(stderr,"Usage: %s \n", me);
    fprintf(stderr,"                 -a ascii_file\n");
    fprintf(stderr,"                 -i incat\n");
    fprintf(stderr,"                 -o outcat\n");
    fprintf(stderr,"                 -t table_name\n");
    fprintf(stderr,"                 -b channel_number\n");
    fprintf(stderr,"                 -n channel_name\n");
    fprintf(stderr,"                 -c config_file\n\n");
    exit(1);
}
/**/

static void init(int argc, char *argv[])
{
/****i* init ***************************************************
 *
 *  NAME
 *      init - Parse the command line arguments
 *
 *  SYNOPSIS
 *      void init(int argc, char *argv[])
 *
 *  FUNCTION
 *      This routine interprets the command line arguments and fills the
 *      global controls structure with the specified values.
 *
 *  INPUTS
 *      argc - Number of arguments
 *      argv - Array of argument strings
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      The control structure is modified
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul dd - 10-10-1996
 *
 ******************************************************************************/
/*
 *  Default file name is calling name with .conf extension
 */
   int		i, j, k;

   sprintf(control.conffile,"%s.conf",argv[0]);
   sprintf(control.table_name,"OBJECTS");
   sprintf(control.chan_name,"I");
   control.infile[0]= '\0';
   control.asciifile[0]= '\0';
   control.chan_nr = 0;
   if (argc == 1) usage(argv[0]);
   init_options(0, my_opts);
   for (i=1; i<argc; i++) {
     if (argv[i][0] == '-') {
        switch(argv[i][1]) {
        case 'c': strcpy(control.conffile,argv[++i]);
                  break;
        case 't': strcpy(control.table_name,argv[++i]);
                  break;
        case 'a': strcpy(control.asciifile,argv[++i]);
                  break;
        case 'i': strcpy(control.infile,argv[++i]);
                  break;
        case 'o': strcpy(control.outfile,argv[++i]);
                  break;
        case '@': print_options(argv[0]);
                  exit(1);
        case 'b': control.chan_nr = atoi(argv[++i]);
                  break;
        case 'n': strcpy(control.chan_name,argv[++i]);
                  break;
        default:  parse_options(control.conffile);
                  j = i++; k = i;
                  set_command_option(argv[j], argv[k]);
                  break;
        }
     } else {
        syserr(WARNING,"init","Command line syntax error\n");
        usage(argv[0]);
     }
   }
   finish_options(control.conffile);
   if (control.asciifile[0] == '\0') {
     strcpy(control.asciifile, control.infile);
     control.infile[0] = '\0';
   }
   col_name = copy_textvec(control.name, &ncols);
   if (ncols == 0)
      syserr(FATAL,"init","No output columns defined for %s\n",control.outfile);
   col_ttype = copy_textvec(control.ttype, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_TTYPE keys differs from COL_NAME\n");
   col_htype = copy_textvec(control.htype, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_HTYPE keys differs from COL_NAME\n");
   col_comm = copy_textvec(control.comm, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_COMM keys differs from COL_NAME\n");
   col_unit = copy_textvec(control.unit, &k);
   if (k != ncols)
      syserr(FATAL, "init", "Number of COL_UNIT keys differs from COL_NAME\n");
   col_depth = copy_textvec(control.depth, &k);
   if (k != 0 && k != ncols)
      syserr(FATAL, "init", "Number of COL_DEPTH keys differs from COL_NAME\n");
   ndepths = k;
}
/**/

catstruct *open_outcat(char *name, catstruct *incat)
{
    catstruct *outcat;

    outcat = new_cat(1);
    init_cat(outcat);
    if (incat) inherit_cat(incat, outcat);
    strcpy(outcat->filename, name);
    if (open_cat(outcat, WRITE_ONLY) != RETURN_OK)
      syserr(FATAL, "open_outcat","Cannot open output catalog %s\n",
             name);
/*
 *  First write the Primary FITS header unit
 */
    save_tab(outcat, outcat->tab);
    return outcat;
}

tabstruct *create_outtab(catstruct *outcat, char *name, tabstruct *ctab)
{
    int		i, j, found;
    tabstruct	*outtab;
    keystruct	*key;

/*
 *  If an input table exists use it
 */
    if (!ctab) {
       outtab = new_tab(name);
       outtab->cat = outcat;
    } else {
       outtab = ctab;
    }
    for (i=0;i<ncols;i++) {
/*
 *     Make the keys for the output table based on the info from the input key
 */
       key = new_key(col_name[i]);
       found = FALSE;
       if (strcmp(col_htype[i], "INT")==0) {
          found = TRUE;
          key->htype = H_INT;
       }
       if (strcmp(col_htype[i], "FLOAT")==0) {
          found = TRUE;
          key->htype = H_FLOAT;
       }
       if (strcmp(col_htype[i], "EXPO")==0) {
          found = TRUE;
          key->htype = H_EXPO;
       }
       if (strcmp(col_htype[i], "BOOL")==0) {
          found = TRUE;
          key->htype = H_BOOL;
       }
       if (strcmp(col_htype[i], "STRING")==0) {
          found = TRUE;
          key->htype = H_STRING;
       }
       if (!found) syserr(FATAL, "create_outtab", "No such HTYPE: %s for key %s\n",
                         col_htype[i], col_name[i]);
       found = FALSE;
       if (strcmp(col_ttype[i], "BYTE")==0) {
          found = TRUE;
          key->ttype = T_BYTE;
       }
       if (strcmp(col_ttype[i], "SHORT")==0) {
          found = TRUE;
          key->ttype = T_SHORT;
       }
       if (strcmp(col_ttype[i], "LONG")==0) {
          found = TRUE;
          key->ttype = T_LONG;
       }
       if (strcmp(col_ttype[i], "FLOAT")==0) {
          found = TRUE;
          key->ttype = T_FLOAT;
       }
       if (strcmp(col_ttype[i], "DOUBLE")==0) {
          found = TRUE;
          key->ttype = T_DOUBLE;
       }
       if (strcmp(col_ttype[i], "STRING")==0) {
          found = TRUE;
          key->ttype = T_STRING;
       }
       if (!found) syserr(FATAL, "create_outtab",
                          "No such TTYPE: %s for key %s\n",
                          col_ttype[i], col_name[i]);
       if (ndepths != 0 && atoi(col_depth[i]) != 1) {
          key->naxis = 1;
          ED_MALLOC(key->naxisn, "create_outtab", int, key->naxis);
          key->naxisn[0] = atoi(col_depth[i]);
       }
       key->nbytes = t_size[key->ttype];
       for (j=0;j<key->naxis;j++)
          key->nbytes *= key->naxisn[j];

       strcpy(key->comment, col_comm[i]);
       if(strcmp(col_unit[i], "\"\"") !=0 && strcmp(col_unit[i], "\'\'") !=0) {
         strcpy(key->unit,col_unit[i]);
       } else {
         key->unit[0] = '\0';
       }

       key->nobj = 1;
       ED_CALLOC(key->ptr, "create_outtab", char, key->nbytes);
       if (add_key(key, outtab, 0)==RETURN_ERROR)
           syserr(FATAL, "create_outtab",
              "Error adding key %s to output table\n", key->name);
    }
/*
 * Only if we create a new OBJECTS table make sure the FIELD_POS is added
 */
    if (!ctab) {
       if (strcmp(name, "OBJECTS") == 0) {
          key = new_key("FIELD_POS");
          key->htype = H_INT;
          key->ttype = T_SHORT;
          key->nbytes = t_size[key->ttype];
          for (j=0;j<key->naxis;j++)
	  {
            key->nbytes *= key->naxisn[j];
	  }
          strcpy(key->comment, "Reference number to field parameters");
          strcpy(key->unit,"");
          key->nobj = 1;
          ED_MALLOC(key->ptr, "create_outtab", char, key->nbytes);
          *((short int *)key->ptr) = 1;
          if (add_key(key, outtab, 0)==RETURN_ERROR)
           syserr(FATAL, "create_outtab",
              "Error adding key %s to output table\n", key->name);
       }
    } else {
      add_tab(outtab, outcat, 0);
      outtab->cat = outcat;
    }
    init_writeobj(outcat, outtab);
    return outtab;
}

void save_row(tabstruct *tab, tabstruct *rtab, tabstruct *intab, char *line)
{
    int		i, j, k, l;
    char	*t, *ptr;
    keystruct	*key, *okey;

/*
 *   If an input catalog exists copy the info. Ordiring is preserved!
 */
    if (intab) {
       read_obj(rtab, intab);
       key  = rtab->key;
       okey = tab->key;
       for (j=0;j<rtab->nkey;j++) {
          memcpy(okey->ptr, key->ptr, key->nbytes);
          key = key->nextkey;
          okey = okey->nextkey;
       }
       key = okey;
    } else {
       key = tab->key;
    }
/*
 *  Now copy the remaining ascii info information
 */
    for (i=0;i<ncols;i++)
    {
       ptr = key->ptr;
       for (k=0;k<((ndepths != 0) ? atoi(col_depth[i]):1);k++) {
          if (k == 0 && i == 0)
             t = strtok(line, control.separator);
          else
             t = strtok(NULL, control.separator);
          if (!t) syserr(FATAL, "save_row",
                   "Too few columns. Found %d, expected %d\n%s",i,ncols,line);
          for (j=0;t[j] != '\0' && t[j] != '\n'; j++);
          t[j] = '\0';
          switch (key->ttype) {
          case T_BYTE:
                      switch (key->htype) {
                      case H_BOOL:
                         if (t[0] == 't' || t[0] == 'T')
                            *((BYTE *)ptr) = 1;
                         if (t[0] == 'f' || t[0] == 'F')
                            *((BYTE *)ptr) = 0;
                         break;
                      case H_INT:
                         *((BYTE *)ptr) = (BYTE) atoi(t);
                         break;
                      default:
                         syserr(FATAL, "save_row",
                           "Error for %s: BYTE type can only be H_BOOL or H_INT\n",
                                   key->name);
                         break;
                      }
                      break;
          case T_SHORT:
                      *((short int *)ptr) = (short int) atoi(t);
                      break;
          case T_LONG:
#ifdef INT64
                      *((LONG *)ptr) = (LONG) atoi(t);
#else
                      *((LONG *)ptr) = (LONG) atol(t);
#endif
                      break;
          case T_FLOAT:
                      *((float *)ptr) = (float) atof(t);
                      break;
          case T_DOUBLE:
                      *((double *)ptr) = (double) atof(t);
                      break;
          case T_STRING:
                      strncpy(ptr, t, (key->nbytes - 1));
                      break;
          }
          if (key->ttype == T_STRING) break;
          for (l=0;l<t_size[key->ttype];l++)
	  {
             ptr++;
	  }
       }
       key = key->nextkey;
    }
    /* deal with the FIELD_POS key in case we create a new objects table */
    if((strcmp(control.table_name, "OBJECTS") == 0) && !intab)
    {
       ptr = key->ptr;
       *((short int *)ptr) = 1;
    }
    write_obj(tab);
}

void close_outcat(tabstruct *tab)
{
    catstruct *cat;

    cat = tab->cat;
    end_writeobj(cat, tab);
}

void add_fields_tab(catstruct *cat, int count)
{
    tabstruct	*ftab;
    LONG	*obj_pos, *obj_count, *chan_nr;
    char	*chan_name;

    ftab = new_tab("FIELDS");
    ED_CALLOC(obj_pos,   "add_field_tab", LONG, 1);
    ED_CALLOC(obj_count, "add_field_tab", LONG, 1);
    ED_CALLOC(chan_nr,   "add_field_tab", LONG, 1);
    ED_CALLOC(chan_name, "add_field_tab", char    , STRING_LEN);
    obj_pos[0] = 1;
    obj_count[0] = count;
    chan_nr[0] = control.chan_nr;
    strcpy(&chan_name[0], control.chan_name);
    add_key_to_tab(ftab, "OBJECT_POS", 1, obj_pos, T_LONG, 1, "",
                             "Starting position of field");
    add_key_to_tab(ftab, "OBJECT_COUNT", 1, obj_count, T_LONG, 1, "",
                             "Number of objects in field");
    add_key_to_tab(ftab, "CHANNEL_NR", 1, chan_nr, T_LONG, 1, "",
                             "Channel numbers");
    add_key_to_tab(ftab, "CHANNEL_NAME", 1, chan_name, T_STRING, STRING_LEN, "",
                             "Channel name");
    update_tab(ftab);
    if (add_tab(ftab,cat,0) == RETURN_ERROR)
       syserr(FATAL, "add_fields_tab", "Error adding FIELDS table to %s\n",
         cat->filename);
    save_tab(cat, ftab);
}



/**/
int main(int arg, char *argv[])

/****** asctoldac ***************************************************
 *
 *  NAME
 *      asctoldac - Convert an ASCII table into a FITS binary table
 *
 *  SYNOPSIS
 *      asctoldac -i asciifile -o outcat -t table_name
 *                -c configfile [CONFOPTION]
 *
 *  FUNCTION
 *
 *  INPUTS
 *      -i asciifile    - The name of the asciifile
 *      -o outcat       - The name of the output catalog file
 *      -c configfile   - The name of the configuration file. The
 *                        default name is asctoldac.conf
 *
 *  RESULTS
 *
 *  RETURNS
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      Program still under development.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 26-11-1997
 *
 ******************************************************************************/
{
    int		count = 0;
    char	header[MAXCHAR], line[10240];
    FILE	*fd;
    catstruct	*incat, *outcat;
    tabstruct	*tab, *intab, *rtab, *outtab;
    keystruct	*key, *okey;
    int		j, ntabs;
    char	**tabnames;

    sprintf(header,"\n   %s Version: %s (%s)\n\n",PROGRAM,VERS,DATE);
    init(arg, argv);
    VPRINTF(NORMAL,header);

    if (!(control.infile[0])) {
       outcat = open_outcat(control.outfile, NULL);
       intab = rtab = outtab = NULL;
    } else {
       incat = read_cat(control.infile);
       outcat = open_outcat(control.outfile, incat);
/*
 *     Copy all tables except the control.table_name table
 */
       tabnames = tabs_list(incat, &ntabs);
       for (j=1;j<ntabs;j++) {
          if (strcmp(tabnames[j],control.table_name) != 0) {
             copy_tab(incat, tabnames[j], 0, outcat, 0);
             save_tab(outcat, name_to_tab(outcat, tabnames[j], 0));
          }
          ED_FREE(tabnames[j]);
       }
       ED_FREE(tabnames);

       intab = name_to_tab(incat, control.table_name, 0);
       outtab = new_tab(control.table_name);
       key = intab->key;
       for (j=0;j<intab->nkey;j++) {
         copy_key(intab, key->name, outtab, 0);
         okey = name_to_key(outtab, key->name);
         okey->nobj   = 1;
         ED_CALLOC(okey->ptr, "save_catalog", char, (key->nbytes));
         key = key->nextkey;
       }
       rtab = init_readobj(intab);
    }
    tab = create_outtab(outcat, control.table_name, outtab);
    ED_FOPEN(fd, "asctoldac", control.asciifile, "rt");
    while (fgets(line, 10240, fd)) {
       if (line[0] != '#') {
          save_row(tab, rtab, intab, line);
          count++;
       }
    }
    ED_FCLOSE(fd, "asctoldac");
    close_outcat(tab);
    if (!rtab && strcmp(control.table_name, "OBJECTS") == 0) {
       add_fields_tab(outcat, count);
    }
    if (count == 0)
       syserr(FATAL, "asctoldac", "No rows converted!\n");
    VPRINTF(NORMAL,"%d row(s) converted\n", count);
    return(0);
}
