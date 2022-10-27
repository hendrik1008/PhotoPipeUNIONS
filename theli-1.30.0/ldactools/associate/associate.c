#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "assoc_globals.h"
#include "assoc_vers.h"
#include "lists_globals.h"
#include "tsize.h"
#include "tabutil_globals.h"
#include "global_vers.h"

/* 23.02.01:
   substituted long and long int by the LONG macro;
   cosmetic changes to avoid compiler warnings

   02.03:
   the changes from 23.02. introduced some newline in
   string constants that SUNOS complained about.

   19.06.01:
   routine "read_channels":
   I test whether there are objects with 0 or negative
   major/minor axis in the catalogs

   05.05.03:
   included the tsize.h file

   10.01.05:
   I removed compiler warnings under gcc with optimisation
   enabled (-O3).

   29.01.2005:
   - I added history information to the output catalog
   - I added argc and argv to the arguments of function
     save_channels to write HISTORY info to output catalogs.

   09.03.2006:
   I removed compiler warnings (-Wall -W -pedantic compiler flags)

   27.03.2006:
   I renamed sorting and indexing functions to the new naming scheme
   described in the sort_globals.h file

   01.05.2006:
   I removed the inclusion of precess_defs.h and inserted
   the missing definition of DEG2RAD directly into the code

   17.11.2007:
   I made command line reading a bit more robust

   31.08.2010:
   I check that the keytype for the object SeqNr. is integer type.
*/

/* definitions */
#ifndef DEG2RAD
#define DEG2RAD 0.017453292         /* degree/radian conversion */
#endif

/*
 *    Global variables do not take as much memory
 */
static double	*ra, *dec;
static float	*a, *aa, *b, *t, *e;
static int	*id;
static short int *f, **ex_f;
static short int *flag;
static LONG     *fpos;
static LONG	*nobjs, *sumobjs;
static int	*chan_nr;
static char	**chan_name;

static float	global_tol;

static int	***p;
static int	**cn, **rn;
static int	cluster_nr = 0, richness;

static control_struct control;

char header[30000];

option my_opts[] = {
            {"INTER_COLOR_TOL", FLOAT, &control.inter_tol, 0, 0, 0.0, 1000.0,
              0, 1.0, NO, "NORMAL",{"", "", "", "", ""}, 0,
             "The tolerance facter for association between catalogs"},
            {"ISO_COLOR_TOL", FLOAT, &control.iso_tol, 0, 0, 0.0, 1000.0,
              0, 1.0, NO, "NORMAL",{"", "", "", "", ""}, 0,
             "The tolerance factor for association within a catalog"},
            {"MASK", INT, &control.mask, 0, 64768, 0.0, 0.0,
              248, 0, NO, "",{"", "", "", "", ""}, 0,
             "The value of the mask to AND the FLAG with. If result <> 0 then don't associate"},
            {"MIN_ELLIPS_DELTA", FLOAT, &control.min_el_d, 0, 0, 0.0, 1.0,
              0, 1.0, NO, "",{"", "", "", "", ""}, 0,
             "The minimum difference between object ellipse to allow association"},
            {"EQUAL_MERGE_FLAG", BOOLEAN, &control.equal_flag, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "", {"YES", "NO", "", "", ""}, 0,
             "Must FLAGs be equal for association or not"},
            {"ALLOW_EQUAL_FIELD_POS", BOOLEAN, &control.equal_frame, 0, 0, 0.0, 0.0,
              0, 0.0, YES, "", {"YES", "NO", "", "", ""}, 0,
             "Will associate including equal frame associateion"},
            {"PAIR_COLS", BOOLEAN, &control.pair_cols, 0, 0, 0.0, 0.0,
              0, 0.0, YES, "", {"YES", "NO", "", "", ""}, 0,
             "Will associate including equal frame associateion"},
            {"FIELD_POS", TEXT, control.fpos, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "FIELD_POS", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the field position"},
            {"SEQNR", TEXT, control.seqnr, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "SeqNr", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the sequence number"},
            {"RA", TEXT, control.ra, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "Ra", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the right ascension"},
            {"DEC", TEXT, control.dec, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "Dec", {"", "", "", "", ""}, 0,
             "THe table OBJECTS column name storing the declination"},
            {"FLAG", TEXT, control.flag, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "Flag", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the flag value on which to check prior to association"},
            {"A_WCS", TEXT, control.a, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "A_WCS", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the major axis in WCS"},
            {"B_WCS", TEXT, control.b, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "B_WCS", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the minor axis in WCS"},
            {"THETAWCS", TEXT, control.theta, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "THETAWCS", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the position angle in WCS"},
            {"E_WCS", TEXT, control.e, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "E_WCS", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name storing the ellipticity in WCS"},
            {"ASSOC_FLAG", TEXT, control.assoc_flag, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "A_FLAG", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name recieving the association flag.  0=No pairing, 1=Type 1 pairing, 2=Equal flag pairing"},
            {"CLUSTER_NR", TEXT, control.cluster_nr, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "CLUSTER_NR", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name recieving the cluster sequence number"},
            {"RICHNESS", TEXT, control.richness, 0, 0, 0.0, 0.0,
              0, 0.0, NO, "RICHNESS", {"", "", "", "", ""}, 0,
             "The table OBJECTS column name recieving the cluster richness"},
            {"", BOOLEAN, NULL, 0, 0, 0.0, 0.0, 0, 0.0, NO, "",
             {"YES", "NO", "", "", ""}, 0,
              "dummy key to end structure my_opts"}
            };

void usage(char *me, char *head)
{
    fprintf(stderr,"%s \n\n", head);
    fprintf(stderr,"Usage: %s \n", me);
    fprintf(stderr,"		-i incat1 [incat2 [incat3 [...]]]	(input catalogs)\n");
    fprintf(stderr,"		-o outcat1 [outcat2 [outcat3 [...]]]	(output catalogs)\n");
    fprintf(stderr,"		[-t table_name] 			(table name of objects)\n");
    fprintf(stderr,"		[-c conf_file] \n");
    fprintf(stderr,"		[-OPTION1 OPTVAL1 [-OPTION OPTVAL2] ...]\n\n");

    exit(1);
}


void init(int argc, char *argv[], char *head, control_struct *c)
{
/*
 *  Default file name is calling name with .conf extension
 */
    int i, j, k;
    int noutcats = 0;

    sprintf(c->table_name,"OBJECTS");
    c->in_pix = NO;
    if (argc == 1) usage(argv[0], head);
    init_options(0, my_opts);
    for (i=1; i<argc; i++)
    {
      if (argv[i][0] == '-')
      {
        switch(argv[i][1]) {
        case 'c': strcpy(c->conffile,argv[++i]);
                  break;
        case '@': print_options(argv[0]);
                  exit(0);
        case 't': strcpy(c->table_name,argv[++i]);
                  break;
        case 'p': c->in_pix = YES;
                  break;
        case 'i': c->ilist = 0;
                  while (i+1 < argc && argv[i+1][0] != '-'
                         && c->ilist < MAXCHAN)
                  {
                     strcpy(c->listfile[c->ilist++],argv[++i]);
                  }
                  if ((c->ilist) == (MAXCHAN - 1) && (i + 1) < argc
                      && argv[i + 1][0] != '-')
                  {
                     syserr(FATAL, "init", "Too many input catalogs %d>=%d\n",
                      c->ilist,MAXCHAN);
                  }
                  break;
        case 'o': j = 0;
                  while (i+1<argc && argv[i+1][0] != '-' && j < MAXCHAN)
                  {
                     strcpy(c->outfile[j++],argv[++i]);
                  }
                  noutcats = j;
                  break;
        default:  parse_options(c->conffile);
                  j = i++; k = i;
                  if (argv[j] && argv[k])
                    set_command_option(argv[j], argv[k]);
                  else
                    usage(argv[0], head);
                  break;
        }
      }
      else
      {
         syserr(WARNING,"init","Command line syntax error\n");
         usage(argv[0], head);
      }
    }

    /* sanity checks */
    if(c->ilist < 1)
    {
      syserr(FATAL, "init", "no input catalogue given. Aborting !!\n");
    }

    if(noutcats != c->ilist)
    {
      syserr(FATAL, "init", "Input & Output lists have different length\n");
    }

    finish_options(c->conffile);
    control.shortmask = control.mask;
    VPRINTF(NORMAL,"%s\n\n",head);
    control.use_ellip = YES;
    if ((1.0 - control.min_el_d) < 1e-6) control.use_ellip = NO;
}

#define NKEYS 9
catstruct  **incat;

int read_channels(control_struct c, int *n)
{
    tabstruct  *tab;
    keystruct  **keys;
    char       **keynames;
    int        num, prev_num;
    double     *t_ra=NULL, *t_dec=NULL;
    float      *t_a, *t_b, *t_t, *t_e=NULL;
    int        *t_id;
    short int  *t_f;
    LONG       *t_fp=NULL;
    int        i, j, equal_chan;

    num = 0;
    ED_CALLOC(incat,    "read_channels", catstruct *, c.ilist);
    ED_CALLOC(nobjs,    "read_channels", LONG,        c.ilist+1);
    ED_CALLOC(sumobjs,  "read_channels", LONG,        c.ilist+1);
    ED_CALLOC(chan_nr,  "read_channels", int,         c.ilist);
    ED_CALLOC(chan_name,"read_channels", char *,      c.ilist);
    for (i=0;i<c.ilist;i++) {
        VPRINTF(NORMAL,"Reading file %s\n",c.listfile[i]);
        if ((incat[i] = read_cat(c.listfile[i])) == NULL) {
           syserr(FATAL, "read_channels", "Could not read catalog: %s\n",
                  c.listfile[i]);
        }
/*
 *      In case of no FIELDS table assume chan_nr is the same as the
 *      order in which the catalogs are given to the program
 */
        ED_CALLOC(chan_name[i], "read_channels", char, MAXCHAR);
        if (!(tab = name_to_tab(incat[i], "FIELDS",0))) {
           syserr(WARNING, "read_channels", "No FIELDS table, assuming channel numbers\n");
           chan_nr[i] = i;
           strcpy(chan_name[i], c.listfile[i]);
        } else {
           chan_nr[i] = get_channel_number(tab);
           strcpy(chan_name[i], get_channel_name(tab));
        }

        if (!(tab = name_to_tab(incat[i], c.table_name,0)))
           syserr(FATAL, "read_channels", "No table %s in catalog %s\n",
              c.table_name, c.listfile[i]);
        ED_CALLOC(keys,     "read_channels", keystruct *, NKEYS);
        ED_CALLOC(keynames, "read_channels", char *, NKEYS);
        for (j=0;j<NKEYS;j++) {
           ED_CALLOC(keynames[j], "read_channels", char, MAXCHAR);
        }
        strcpy(keynames[0], control.seqnr);
        strcpy(keynames[1], control.ra);
        strcpy(keynames[2], control.dec);
        strcpy(keynames[3], control.flag);
        strcpy(keynames[4], control.a);
        strcpy(keynames[5], control.b);
        strcpy(keynames[6], control.theta);
        if (control.equal_frame == NO)
          strcpy(keynames[7], control.fpos);
        else
          strcpy(keynames[7], "");
        if (control.use_ellip == YES)
          strcpy(keynames[8], control.e);
        else
          strcpy(keynames[8], "");
        read_keys(tab, keynames, keys, NKEYS, NULL);
        for (j=0;j<NKEYS-2;j++) {
           if (!keys[j])
              syserr(FATAL,"read_channels", "No key %s in table\n",keynames[j]);
        }

        /* Ra and Dec can be stored as floats or as doubles. Internally
           we treat them always as doubles
        */
        switch (keys[1]->ttype)
        {
        case T_FLOAT:
              ED_GETVEC(t_ra, "read_channels",keys[1], keys[1]->name,
                        double, float);
              break;
        case T_DOUBLE:
              ED_GETVEC(t_ra, "read_channels",keys[1], keys[1]->name,
                        double, double);
              break;
	default:
              syserr(FATAL, "Only key types FLOAT and DOUBLE allowed for %s",
                     keys[1]->name);
              break;
        }
        switch (keys[2]->ttype)
        {
        case T_FLOAT:
              ED_GETVEC(t_dec, "read_channels",keys[2], keys[2]->name,
                        double, float);
              break;
        case T_DOUBLE:
              ED_GETVEC(t_dec, "read_channels",keys[2], keys[2]->name,
                        double, double);
              break;
	default:
              syserr(FATAL, "Only key types FLOAT and DOUBLE allowed for %s",
                     keys[2]->name);
              break;
        }
        ED_FREE(keys[1]->ptr);
        ED_FREE(keys[2]->ptr);

        /* Check keytypes; the SeqNr can be SHORT or LONG */
        if ((keys[0]->ttype != T_SHORT) && (keys[0]->ttype != T_LONG)) {
            syserr(FATAL, "read_channels",
                   "Key %s should be of type SHORT or LONG\n", keys[0]->name);
        }
        ED_CHECK_TYPE(keys[3], "read_channels", T_SHORT, "short int");
        ED_CHECK_TYPE(keys[4], "read_channels", T_FLOAT, "float");
        ED_CHECK_TYPE(keys[5], "read_channels", T_FLOAT, "float");
        ED_CHECK_TYPE(keys[6], "read_channels", T_FLOAT, "float");
        t_id   = keys[0]->ptr;
        t_f    = keys[3]->ptr;
        t_a    = keys[4]->ptr;
        t_b    = keys[5]->ptr;
        t_t    = keys[6]->ptr;
        if (keys[7])
        {
           switch (keys[7]->ttype)
           {
              case T_SHORT:
              ED_GETVEC(t_fp, "read_channels",keys[7], keys[7]->name,
                        LONG, short int);
              break;
              case T_LONG:
              ED_GETVEC(t_fp, "read_channels",keys[7], keys[7]->name,
                        LONG, LONG);
              break;
              default:
                syserr(FATAL, "read_channels",
                    "Key %s must be of type T_SHORT or T_LONG\n",
                     keys[7]->name);
              break;
           }
        }
        /* I have no idea what the following two lines
           should do or what their intenstion is (TE: 23.02.01)
        */
        /*else
           control.equal_frame == YES;
        */
        if (control.use_ellip == YES && !keys[8])
           syserr(FATAL, "read_channels", "No ellipse key %s in table\n",
             control.e);
        if (keys[8]) {
           ED_CHECK_TYPE(keys[8], "read_channels", T_FLOAT, "float");
           t_e    = keys[8]->ptr;
        }
        nobjs[i+1] = keys[0]->nobj;
        prev_num = num;
        num += nobjs[i+1];
        sumobjs[i+1] = num;
        num++;
        if (i == 0) {
           ED_MALLOC(id,     "read_channels", int,       num);
           ED_MALLOC(ra,     "read_channels", double,    num);
           ED_MALLOC(dec,    "read_channels", double,    num);
           ED_MALLOC(f,      "read_channels", short int, num);
           ED_MALLOC(a,      "read_channels", float,     num);
           ED_MALLOC(b,      "read_channels", float,     num);
           ED_MALLOC(t,      "read_channels", float,     num);
           ED_MALLOC(flag,   "read_channels", short int, num);
           if (control.use_ellip == YES)
             ED_MALLOC(e,      "read_channels", float,   num);
           if (control.equal_frame == NO)
             ED_MALLOC(fpos,   "read_channels", LONG, num);
        } else {
           ED_REALLOC(id,     "read_channels", int,       num);
           ED_REALLOC(ra,     "read_channels", double,    num);
           ED_REALLOC(dec,    "read_channels", double,    num);
           ED_REALLOC(f,      "read_channels", short int, num);
           ED_REALLOC(a,      "read_channels", float,     num);
           ED_REALLOC(b,      "read_channels", float,     num);
           ED_REALLOC(t,      "read_channels", float,     num);
           ED_REALLOC(flag,   "read_channels", short int, num);
           if (control.use_ellip == YES)
             ED_REALLOC(e,      "read_channels", float,   num);
           if (control.equal_frame == NO)
             ED_REALLOC(fpos,   "read_channels", LONG,num);
        }
        VPRINTF(NORMAL,"Read %d objects\n",nobjs[i+1]);
        num--;
        for (j=prev_num;j<num;j++) {
           id[j]     = (int)(t_id[j-(prev_num)]);
           ra[j]     = t_ra[j-(prev_num)];
           dec[j]    = t_dec[j-(prev_num)];
           f[j]      = t_f[j-(prev_num)];
           a[j]      = t_a[j-(prev_num)];
           b[j]      = t_b[j-(prev_num)];
           if((a[j] <=0.0) || (b[j] <= 0.0))
	   {
             syserr(FATAL, "read_channels",
         "There are objects with 0 or negative semi minor/major axis in %s\n",
                    c.listfile[i]);
	   }
           t[j]      = t_t[j-(prev_num)] * DEG2RAD;
           flag[j]   = i+1;
           if (control.use_ellip == YES) e[j]      = t_e[j-(prev_num)];
           if (control.equal_frame == NO) fpos[j]   = t_fp[j-(prev_num)];
        }
        ED_FREE(t_id);
        ED_FREE(t_ra);
        ED_FREE(t_dec);
        ED_FREE(t_f);
        ED_FREE(t_a);
        ED_FREE(t_b);
        ED_FREE(t_t);
        if (control.use_ellip == YES) ED_FREE(t_e);
        if (control.equal_frame == NO) ED_FREE(t_fp);
        for (j=0;j<NKEYS;j++) {
           ED_FREE(keynames[j]);
        }
        ED_FREE(keynames);
        ED_FREE(keys);
       close_cat(incat[i]);
    }
    *n    = num;
    ED_CALLOC(cn, "read_channels", int *, c.ilist);
    ED_CALLOC(rn, "read_channels", int *, c.ilist);
    ED_CALLOC(ex_f, "read_channels", short int *, c.ilist);
    for (i=0;i<c.ilist;i++) {
      ED_CALLOC(ex_f[i], "read_channels", short int, nobjs[i+1]);
      ED_CALLOC(cn[i],   "read_channels", int,       nobjs[i+1]);
      ED_CALLOC(rn[i],   "read_channels", int,       nobjs[i+1]);
    }
    if (control.pair_cols == YES) {
      ED_CALLOC(p,  "read_channels", int **, c.ilist);
      for (i=0;i<c.ilist;i++) {
         ED_CALLOC(p[i], "read_channels", int *, c.ilist);
         for (j=0;j<c.ilist;j++) {
            ED_CALLOC(p[i][j], "read_channels", int, nobjs[i+1]);
         }
      }
    }
    equal_chan = FALSE;
    for (i=0;i<c.ilist;i++)
       for (j=i;j<c.ilist;j++)
          if (chan_nr[i] == chan_nr[j]) equal_chan = TRUE;
    if (equal_chan)
       for (i=0;i<c.ilist;i++)
          chan_nr[i] = i;
    return i;
}

void save_channels(control_struct c, int argc, char **argv)
{
    catstruct	**outcat;
    tabstruct	*tab, *outtab;
    keystruct	*key, *outkey;
    int		i, j, jj, ntabs;
    char	**tabnames, *s, *t;
    int		*p_saved, assoc_flag;

    ED_CALLOC(outcat ,"save_channels", catstruct *, c.ilist);
    ED_CALLOC(s,      "save_channels", char,        MAXCHAR);
    ED_CALLOC(t,      "save_channels", char,        MAXCHAR);
    ED_CALLOC(p_saved,"save_channels", int,         c.ilist);
    for (i=0;i<c.ilist;i++) {
       for (j=0;j<c.ilist;j++) {
          p_saved[j] = 0;
       }
       assoc_flag = 0;
       outcat[i] = new_cat(1);
       if ((incat[i] = read_cat(c.listfile[i])) == NULL) {
          syserr(FATAL, "read_channels", "Could not read catalog: %s\n",
                 c.listfile[i]);
        }
       inherit_cat(incat[i], outcat[i]);
       historyto_cat(outcat[i], "associate", argc, argv);
/*
 *     Copy all tables except the c.table_name table
 */
       tabnames = tabs_list(incat[i], &ntabs);
       ED_FREE(tabnames[0]);
       for (j=1;j<ntabs;j++) {
          if (strcmp(tabnames[j],c.table_name) != 0) {
             copy_tab(incat[i], tabnames[j], 0, outcat[i], 0);
          }
          ED_FREE(tabnames[j]);
       }
       ED_FREE(tabnames);
       if (!(outtab = new_tab(c.table_name)))
	  syserr(FATAL, "save_channels", "No table %s in catalog!\n", c.table_name);

       tab = name_to_tab(incat[i], c.table_name, 0);
       key = tab->key;
       for (jj=0;jj<tab->nkey;jj++) {
          copy_key(tab, key->name, outtab, 0);
/*
 *        Make the local information arrays known to the final save_cat
 */
          outkey = name_to_key(outtab, key->name);
          if (control.pair_cols == YES) {
/*
 *          Find the key to put the pairing info in. Actually preastrom
 *          should have made these keys already.
 */
            for (j=0;j<c.ilist;j++) {
               sprintf(s,"Pair_%-d",chan_nr[j]);
               if (strcmp(key->name,s) == 0) {
                  outkey->ptr = p[i][j];
                  p_saved[j] = 1;
               }
            }
          }
          if (strcmp(key->name,control.assoc_flag) == 0) {
             outkey->ptr = ex_f[i];
             assoc_flag = 1;
          }
          key = key->nextkey;
       }
       if (control.pair_cols == YES) {
/*
 *       Check if we have saved them all. If not shout and add
 */
         for (j=0;j<c.ilist;j++) {
            if (p_saved[j] == 0) {
               VPRINTF(NORMAL, "No Pair_%-d key found! Adding it now\n",
                       chan_nr[j]);
               sprintf(s,"Pair_%-d",chan_nr[j]);
               sprintf(t,"Pairing with catalog %s",chan_name[j]);
               add_key_to_tab(outtab, s, nobjs[i+1], p[i][j], T_LONG, 1, "", t);
            }
         }
       }
       if (!assoc_flag) {
          add_key_to_tab(outtab, control.assoc_flag, nobjs[i+1], ex_f[i],
                         T_SHORT, 1, "", "Association flag");
       }
       if (!(key = name_to_key(outtab, control.cluster_nr)))
          add_key_to_tab(outtab, control.cluster_nr, nobjs[i+1], cn[i],
                      T_LONG, 1, "", "Cluster Sequence Number");
       else
          key->ptr = cn[i];
       if (!(key = name_to_key(outtab, control.richness)))
          add_key_to_tab(outtab, control.richness, nobjs[i+1], rn[i],
                      T_LONG, 1, "", "Cluster Richness");
       else
          key->ptr = rn[i];
       add_tab(outtab, outcat[i], 0);
       VPRINTF(NORMAL,"Saving catalog %s\n", c.outfile[i]);
       save_cat(outcat[i], c.outfile[i]);
       free_cat(incat[i], 1);
    }
    ED_FREE(outcat);
    ED_FREE(s);
    ED_FREE(t);
    ED_FREE(p_saved);
}

void free_pairs(control_struct c)
{
    int i;

    if (control.pair_cols == YES) {
      for (i=0;i<c.ilist;i++) {
        ED_FREE(p[i]);
      }
      ED_FREE(p);
    }
}

int assoc(int i, int n, int *idxpos, int *posidx, int graph, int level)
{
     int       jj, jj_middle;
     int       id_ref, id_cur;
     short int ref_chan, curr_chan;
     double    rra_ref, ra_ref, ddec_ref, dec_ref, ra_cur, dec_cur;
     double    ra_curf, dec_curf, ra_reff, dec_reff;
     double    box_ref, box_cur, ref_corr = 1.0, cur_corr = 1.0;
     double    box_siz;
     double    sinref,cosref,sincur,coscur,temc,temr;
     float     ref_a, ref_b, curr_a, curr_b;
     int       retcode = FALSE, up_walk = TRUE, down_walk = FALSE, iter = 1;

     if (flag[i] >= 0 && !(f[i]&control.shortmask)) {
        ra_ref   = ra[i];
        dec_ref  = dec[i];
        id_ref   = id[i];
        ref_chan = flag[i];
        if (control.in_pix == NO) ref_corr = 1.0/ cos(dec_ref * DEG2RAD);
        if (level == 0) global_tol = MAX(control.inter_tol, control.iso_tol) *
                                     2.0 * aa[i];
        VPRINTF(DEBUG,"Checking out %d %d %f\n", ref_chan, id_ref, global_tol);
        jj_middle = posidx[i];
        jj = jj_middle;
        while (up_walk || down_walk) {
          if (fabs(dec[idxpos[jj]]-dec_ref)>global_tol ||
            jj == n-1 || jj == 0) {
            if (up_walk) {
              up_walk = FALSE; down_walk = TRUE;
              iter = -1; jj = jj_middle;
              if (jj_middle == 0)
                 return retcode;
            } else {
              if (down_walk)
                 return retcode;
            }
          }
          /*VPRINTF(DEBUG,"%d %f\n", jj, dec[idxpos[jj]]);*/
          jj += iter;
           id_cur    = id[idxpos[jj]];
           if (flag[idxpos[jj]] > 0) {
             curr_chan = (short int) flag[idxpos[jj]];
           box_ref=((ref_chan==curr_chan)?control.iso_tol:control.inter_tol)*
                   aa[i];
           ddec_ref = dec_ref;
/*
 *         Not the same object
 */
           if (!(id_ref == id_cur && ref_chan == curr_chan)) {
              ra_cur   = ra[idxpos[jj]];
              dec_cur  = dec[idxpos[jj]];
              if (control.in_pix == NO) cur_corr = 1.0/ cos(dec_ref * DEG2RAD);
              box_cur=((ref_chan==curr_chan)?control.iso_tol:control.inter_tol)
                      * aa[idxpos[jj]];
/*
 *            Outside enclosing box then continue
 */
              if ((((dec_ref - box_ref) > dec_cur) &&
                   ((dec_cur + box_cur) < dec_ref)) ||
                  (((dec_ref + box_ref) < dec_cur) &&
                   ((dec_cur - box_cur) > dec_ref)))
                    continue;
/*
 *            Inside enclosing box
 */
              box_siz = MAX(box_ref,box_cur);
              rra_ref = ra_ref+(((ra_ref-box_siz*ref_corr) < 0.0)?360.0:0.0);
              if ((ra_cur - box_siz*cur_corr) < 0.0) {
                 ra_cur += 360.0;
              }
              if ((((rra_ref + box_ref*ref_corr) >= ra_cur)  &&
                   ((rra_ref - box_ref*ref_corr) <= ra_cur)) ||
                  (((ra_cur  - box_cur*cur_corr) <= rra_ref)  &&
                   ((ra_cur  + box_cur*cur_corr) >= rra_ref))) {
                 cosref = cos(t[i]);
                 sinref = sin(t[i]);
                 coscur = cos(t[idxpos[jj]]);
                 sincur = sin(t[idxpos[jj]]);
                 ra_reff  =-(rra_ref - ra_cur) * sinref / ref_corr +
                            (dec_ref - dec_cur) * cosref;
                 dec_reff = (rra_ref - ra_cur) * cosref / ref_corr +
                            (dec_ref - dec_cur) * sinref;
                 ra_curf  =-(ra_cur - ra_ref) * sincur / cur_corr +
                            (dec_cur - dec_ref) * coscur;
                 dec_curf = (ra_cur - ra_ref) * coscur / cur_corr +
                            (dec_cur - dec_ref) * sincur;
                 if (ref_chan == curr_chan) {
                    curr_a = control.iso_tol * a[idxpos[jj]];
                    ref_a  = control.iso_tol * a[i];
                    curr_b = control.iso_tol * b[idxpos[jj]];
                    ref_b  = control.iso_tol * b[i];
                 } else {
                    curr_a = control.inter_tol * a[idxpos[jj]];
                    ref_a  = control.inter_tol * a[i];
                    curr_b = control.inter_tol * b[idxpos[jj]];
                    ref_b  = control.inter_tol * b[i];
                 }
                 temr = dec_reff*dec_reff/(ref_b*ref_b) +
                        ra_reff*ra_reff/(ref_a*ref_a);
                 temc = dec_curf*dec_curf/(curr_b*curr_b) +
                        ra_curf*ra_curf/(curr_a*curr_a);
                 if (temr <= 1.0 || temc <= 1.0) {
/*
 *                  Does it pass the MASK condition
 */
                    if (f[idxpos[jj]]&control.shortmask)  {
                       ex_f[curr_chan-1][idxpos[jj]-sumobjs[curr_chan-1]] |= 1;
                    }
/*
 *                  Must the FLAGS be equal
 */
                    if (control.equal_flag == YES && f[i] != f[idxpos[jj]]) {
                       ex_f[curr_chan-1][idxpos[jj]-sumobjs[curr_chan-1]] |= 2;
                    }
/*
 *                  Ellipticity difference less then allowed
 */
                    if (control.use_ellip == YES &&
                        fabs(e[idxpos[jj]]-e[i]) > control.min_el_d) {
                       ex_f[curr_chan-1][idxpos[jj]-sumobjs[curr_chan-1]] |= 4;
                    }
/*
 *                  No association within the same frame
 */
                    if (control.equal_frame == NO &&
                        fpos[i] == fpos[idxpos[jj]] &&
                        ref_chan == curr_chan) {
                      ex_f[curr_chan-1][idxpos[jj]-sumobjs[curr_chan-1]] |= 8;
                    }
                    if (ex_f[curr_chan-1][idxpos[jj]-sumobjs[curr_chan-1]])
                      continue;
                    retcode = TRUE;
                    VPRINTF(DEBUG,"Hit %d %d %f %f (%f) -> %d %d %f %f (%f)\n",
                          ref_chan, id_ref, rra_ref, dec_ref, box_ref,
                          curr_chan, id_cur, ra_cur, dec_cur, box_cur);
                    VPRINTF(DEBUG,"r:%f d:%f a:%f b:%f,  r:%f d:%f a:%f b:%f\n",
                         ra_reff, dec_reff, ref_a,ref_b,
                         ra_curf, dec_curf, curr_a, curr_b);
                    flag[i] = -(short int)fabs((double)flag[i]);
                    if (add_graph_side(graph, id_ref, ref_chan,
                                              id_cur, curr_chan)) {
                       assoc(idxpos[jj], n, idxpos, posidx, graph, 1);
                       flag[idxpos[jj]] = -(short int)fabs((double)flag[idxpos[jj]]);
                    }
                 }
              }
           }
        }
/* test changes */
        }
     }
     return retcode;
}

void get_index(int n, int *idxid, int ref_id, int chan, int *idx)
{
     int jj, jj_start, jj_end;

     ibin_search_indexed(n, id, idxid,
                 ref_id, 3, &jj_start, &jj_end, 0);
     for (jj=jj_start;jj<=jj_end;jj++) {
        if (id[idxid[jj]] == ref_id&&
           (int) fabs(flag[idxid[jj]]) == chan) {
           *idx = idxid[jj];
           break;
        }
     }
}

void merge_info(int n, int graph, int *idxid, int nchan)
{
     int	i, j, k, *l;
     int 	*ref_ids, **object_list, from = 0;
     int	offs = 0;

     ED_MALLOC(object_list, "merge_info", int *, nchan);
     ED_MALLOC(l,           "merge_info", int,   nchan);
     richness = 0;
     for (i=0;i<nchan;i++) {
/*
 *      For each channel get all unique objects
 */
        ref_ids = get_all_ids(graph, i+1, &l[i]);
        object_list[i] = uniq_int(ref_ids, &l[i]);
        ED_FREE(ref_ids);
        richness += l[i];
     }

     if (richness > 1) cluster_nr++;
     VPRINTF(DEBUG,"RICHNESS: %d, CLUSTER_NR: %d\n", richness, cluster_nr);
     for (i=0;i<nchan;i++) {
/*
 *      First make a complete linked list in the same color
 */
        offs += nobjs[i];
        for (k=0;k<l[i]-1;k++) {
           get_index(n, idxid, object_list[i][k], i+1, &from);
           VPRINTF(DEBUG,"A) From: (%d,%d), To: (%d,%d) [%d]\n",
                    i,id[from],i,object_list[i][k+1],
                    from-offs);
           if (control.pair_cols == YES)
             p[i][i][from-offs] = object_list[i][k+1];
           if (richness > 1) {
              cn[i][from-offs] = cluster_nr;
              rn[i][from-offs] = richness;
           }
           for (j=0;j<nchan;j++) {
/*
 *            Now make each element of the one color point to the top of the
 *            element of the outer color. In fact any element could do.
 */
              if (i != j) {
                 if (l[j] > 0) {
                    VPRINTF(DEBUG,"B) From: (%d,%d), To: (%d,%d) [%d]\n",
                             i,id[from],j,object_list[j][0],
                             from-offs);
                    if (control.pair_cols == YES)
                      p[i][j][from-offs] = object_list[j][0];
                    if (richness > 1) {
                      cn[i][from-offs] = cluster_nr;
                      rn[i][from-offs] = richness;
                    }
                 }
              }
           }
        }
        if (l[i] >= 1) {
           get_index(n, idxid, object_list[i][l[i]-1], i+1, &from);
           if (l[i] > 1) {
              VPRINTF(DEBUG,"C) From: (%d,%d), To: (%d,%d) [%d]\n",
                      i,id[from],i,object_list[i][0],
                      from-offs);
              if (control.pair_cols == YES)
                p[i][i][from-offs] = object_list[i][0];
              if (richness > 1) {
                 cn[i][from-offs] = cluster_nr;
                 rn[i][from-offs] = richness;
              }
           }
           for (j=0;j<nchan;j++) {
/*
 *            Now make each element of the one color point to the top of the
 *            element of the outer color. In fact any element could do.
 */
              if (i != j) {
                 if (l[j] > 0) {
                    VPRINTF(DEBUG,"D) From: (%d,%d), To: (%d,%d) [%d]\n",
                             i,id[from],j,object_list[j][0],
                             from-offs);
                    if (control.pair_cols == YES)
                      p[i][j][from-offs] = object_list[j][0];
                    if (richness > 1) {
                      cn[i][from-offs] = cluster_nr;
                      rn[i][from-offs] = richness;
                    }
                 }
              }
           }
        }
     }
     for (i=0;i<nchan;i++) {
        ED_FREE(object_list[i]);
     }
     ED_FREE(object_list);
     ED_FREE(l);
}


int main(int argc, char *argv[])

/****** associate ***************************************************
 *
 *  NAME	
 *      associate - Perform object-to-object associations
 *
 *  SYNOPSIS
 *      associate -i incat1 incat2 ... -o outcat1 outcat2 ...
 *                [-c configfile] [CONFOPTION]
 *
 *  FUNCTION
 *      This program associates individual objects from the input catalogs
 *      in terms of their positional and shape information. It solves for
 *      multiple observations within a strip (frame overlap), for
 *      inter-color multiple observations (passband pairing), and for
 *      intra-strip multiple observations (strip overlap). The association
 *      information is stored into the output catalogs in the form of a
 *      sequence number pointing mechanism.
 *
 *  INPUTS
 *      -i incat1 incat2 ...    - The list of input catalogs
 *      -o outcat1 outcat2 ...  - The list of output catalogs
 *     [-c configfile]          - The name of the configuration file. The
 *                                default name is /denisoft/conf/associate.conf
 *     [CONFOPTION]             - Pairs of configuration VARIABLE VALUE
 *                                sequences that allow dynamically
 *                                overwritting of configuration file values.
 *
 *  RESULTS
 *
 *  RETURNS
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *      The intra-strip pairing can be done provided the sequence numbers
 *      of the strips do not overlap. In the current pipeline version this
 *      condition is not yet met, however.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul - dd 05-09-1996
 *
 ******************************************************************************/
{
     int       n;
     int       *idxpos, *posidx, *idxdim, *idxid;
     int       i, graph, modulo = 10;
     float     *fptr, *tptr;
     double    *dptr;


     sprintf(header,"\n%s Version: %s (%s)\n(%s)",
             PROGRAM, VERS, DATE, __PIPEVERS__);

     init(argc, argv, header, &control);

     for (i=1;i<argc;i++)
       sprintf(header, "%s %s", header, argv[i]);

     read_channels(control, &n);
/*
 *   Get the sorting indexes on
 *     position, dimension, and on id
 */
     ED_CALLOC(idxpos, "main", int,   n);
     ED_CALLOC(posidx, "main", int,   n);
     ED_CALLOC(idxdim, "main", int,   n);
     ED_CALLOC(idxid,  "main", int,   n);
     ED_CALLOC(aa,     "main", float, n);
     VPRINTF(NORMAL, "Making the position index\n");
     dindex(n, dec, idxpos);
     iindex(n, idxpos, posidx);
     VPRINTF(NORMAL, "Making the size index\n");
     if (control.in_pix == YES) {
       for (i=0,tptr=aa,fptr=a;i<n;i++)
          *(tptr++) = *(fptr++);
     } else {
       for (i=0,tptr=aa,fptr=a,dptr=dec;i<n;i++)
          *(tptr++) = *(fptr++) / cos(*(dptr++)*DEG2RAD);
     }
     findex(n, aa, idxdim);
     VPRINTF(NORMAL, "Making the sequence number index\n");
     iindex(n, id, idxid);

     graph = init_graph(20);
/*
 *   Find the associations, start with the largest object
 */
     global_tol = aa[idxdim[n-1]];
     VPRINTF(NORMAL,"Processing: %d\n", n);
     for (i=n-1;i>=0;i--) {
        if ((n-i) % modulo == 0) {
           if (modulo < 2000) modulo *= 2;
           VPRINTF(NORMAL,"Objects processed: %d out of %d\n", n-i,n);
        }
        if (assoc(idxdim[i], n, idxpos, posidx, graph, 0)) {
           merge_info(n, graph, idxid, control.ilist);
           clear_graph(graph);
        }
     }
     remove_graph(graph);
     save_channels(control, argc, argv);
     free_pairs(control);
     VPRINTF(NORMAL,"Done\n");
     ED_FREE(idxpos);
     ED_FREE(posidx);
     ED_FREE(idxdim);
     ED_FREE(idxid);
     ED_FREE(aa);

     return 0;
}
