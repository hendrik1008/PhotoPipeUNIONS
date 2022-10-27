#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "fitscat.h"
#include "select_overlaps.h"
#include "make_perms_globals.h"
#include "utils_globals.h"
#include "graph_globals.h"
#include "sort_globals.h"
#include "tabutil_globals.h"
#include "check_overlap_globals.h"

#define MAXFRAMES 20000
#define VERS "0.2"
#define NAME "Select Overlaps"
#define DATE "16-01-2002"
#define AUTHOR "E.R. Deul & Thomas Erben"

/*
   the program assigns overlapping frames in a "files" catalog
   (created with the script make_files_cat.sh) a unique number.
*/

/*
   HISTORY:
   07.03.2006:
   I removed compiler warnings connected with the opts structure.

   27.04.2007:
   I increased the allowed ranges of some configuration parameters
   (?WID, CRPIX?)
*/
control_struct control;

option opts[NOPTS] = {
   {"XPIXSIZE", FLOAT, &control.xpixsize, 0, 0, 1.0e-10, 1.0e10,
     0, 1.0e-10, NO, "",
     {"", "", "", "", ""}, 0, ""},
   {"YPIXSIZE", FLOAT, &control.ypixsize, 0, 0, 1.0e-10, 1.0e10,
     0, 1.0e-10, NO, "",
     {"", "", "", "", ""}, 0, ""},
   {"MIN_OVER", FLOAT, &control.min_over, 0, 0, 0.0, 1024.0,
     0, 100.0, NO, "",
     {"", "", "", "", ""}, 0, ""},
   {"CRVAL1", TEXT, &control.crval1, 0, 0, 0.0, 0.0,
     0, 0.0, NO, "Ra",
     {"", "", "", "", ""}, 0, ""},
   {"CRVAL2", TEXT, &control.crval2, 0, 0, 0.0, 0.0,
     0, 0.0, NO, "Dec",
     {"", "", "", "", ""}, 0, ""},
   {"CRPIX1", FLOAT, &control.crpix1, 0, 0, -10000.0, 10000.0,
     0, 9000.0, NO, "CRPIX1",
     {"", "", "", "", ""}, 0, ""},
   {"CRPIX2", FLOAT, &control.crpix2, 0, 0, -10000.0, 10000.0,
     0, 9000.0, NO, "CRPIX2",
     {"", "", "", "", ""}, 0, ""},
   {"CDELT1", FLOAT, &control.cdelt1, 0, 0, -1.0, 1.0,
     0, -0.000061, NO, "CDELT1",
     {"", "", "", "", ""}, 0, ""},
   {"CDELT2", FLOAT, &control.cdelt2, 0, 0, -1.0, 1.0,
     0, 0.000061, NO, "CDELT2",
     {"", "", "", "", ""}, 0, ""},
   {"XWID", FLOAT, &control.xwid, 0, 0, 0.0, 20000.0,
     0, 8000.0, NO, "CDELT1",
     {"", "", "", "", ""}, 0, ""},
   {"YWID", FLOAT, &control.ywid, 0, 0, 0.0, 20000.0,
     0, 8000.0, NO, "CDELT2",
     {"", "", "", "", ""}, 0, ""},
   {"CHECK_COND", STRING, &control.check_cond_string, 0, 0, 0.0, 0.0,
     0, 0.0, NO, "GENERAL_COND",
     {"DENIS_COND", "GENERAL_COND", "", "", ""}, 0, ""},
   {"", BOOLEAN, NULL, 0, 0, 0.0, 0.0, 0, 0.0, NO, "",
     {"YES", "NO", "", "", ""}, 0,
      "dummy key to end structure my_opts"}

};

void usage(char *me)
{
    fprintf(stderr,"Usage: %s [-c configuration file]\n",me);
    fprintf(stderr,"                 -i incat1\n");
    fprintf(stderr,"                 [-OPTION1 OPTVAL1 [-OPTION OPTVAL2] ...]\n");
    exit(1);
}

void init(int argc, char *argv[])
{
/*
 *  Default file name is calling name with .conf extension
 */
   int		i, j, k;

   sprintf(control.conffile,"select_overlaps.conf");
   sprintf(control.tablename,"FILES");

   if (argc == 1) usage(argv[0]);
   control.outfile[0] = '\0';
   init_options(NOPTS, opts);
   for (i=1; i<argc; i++) {
     if (argv[i][0] == '-') {
        switch(tolower((int)argv[i][1])) {
        case 'c': strcpy(control.conffile,argv[++i]);
                  break;
        case 'i': strcpy(control.listfile,argv[++i]);
                  break;
        case 'o': strcpy(control.outfile,argv[++i]);
                  break;
        case 't': strcpy(control.tablename,argv[++i]);
                  break;
        case '@': print_options(argv[0]);
                  exit(0);
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
  if (strcmp(control.check_cond_string,"DENIS_COND") == 0) {
     control.check_cond = DENIS_COND;
  } else {
     control.check_cond = GENERAL_COND;
  }
}

int main(int argc, char *argv[])

/****** select_overlaps ***************************************************
 *
 *  NAME	
 *      select_overlaps - Determine the layout of a set of frames
 *
 *  SYNOPSIS
 *      select_overlaps -i incat1
 *               [-c configfile]
 *
 *  FUNCTION

 *  INPUTS
 *      -i incat1 incat2 ...  - The list of input catalog files
 *     [-c configfile]        - The name of the cofiguration file. The
 *                              default name is /denisoft/conf/select_overlaps.conf
 *     [CONFOPTION]           - Pairs of configuration VARIABLE VALUE
 *                              sequences that allow dynamically
 *                              overwritting of configuration file values.
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
 *      E.R. Deul - dd 17-02-1997
 *
 ******************************************************************************/
{
     char header[MAXCHAR];
     double	*ra, *dec;
     double	cdelt1, cdelt2, crpix1, crpix2;

     int	*id;
     int	graph;
     int	i, j, nperms, operms, nfields;
     int        curr_set_number;
     short      *set;

     int        xwid, ywid;
     perm_struct *l_perms, *c_perms;

     catstruct	*incat, *outcat;
     tabstruct	*tab;
     keystruct	*key;

     init(argc,argv);

     sprintf(header,"\n   %s Version: %s (%s)\n\n",NAME,VERS,DATE);
     VPRINTF(NORMAL,header);

     ED_CALLOC(incat, "select_overlaps", catstruct, 1);
     ED_CALLOC(outcat,"select_overlaps", catstruct, 1);

     if ((incat = read_cat(control.listfile)) == NULL)
     {
        syserr(FATAL, "main", "Could not read catalog: %d\n",
               control.listfile);
     }
     outcat = new_cat(1);
     inherit_cat(incat, outcat);
     copy_tabs(incat, outcat);

     tab = name_to_tab(outcat, control.tablename, 0);

     ED_GETKEY(tab, key, control.crval1,   ra ,    nfields, "select_overlaps");

     /* nothing to do if there is only one field */
     if(nfields < 2)
     {
       syserr(FATAL, "main", "Input catalog contains only one field: nothing to do\n");
     }
     ED_GETKEY(tab, key, control.crval2,   dec,    nfields, "select_overlaps");

     cdelt1 = control.cdelt1;
     cdelt2 = control.cdelt2;
     crpix1 = control.crpix1;
     crpix2 = control.crpix2;
     xwid   = control.xwid;
     ywid   = control.ywid;

     ED_CALLOC(id, "select_overlaps", int, nfields);

     for (i=0;i<nfields;i++)
     {
       id[i] = i+1;
     }


     check_overlaps(&graph, ra, dec, nfields,
                    (float)cdelt1, (float)cdelt2,
                    (float) control.min_over, xwid, ywid,
                    control.check_cond);

     if (verbosity(DEBUG)) display_graph(graph);
     l_perms = NULL;
     ED_CALLOC(set, "select_overlaps", short, nfields+1);
     ED_CALLOC(c_perms, "select_overlaps", perm_struct, nfields);

     curr_set_number = 1;

     if (nfields > 1)
     {
        operms = 0;
        for (i=0;i<nfields;i++)
        {
	  if(set[i] == 0)
	  {
 	    nperms = largest_perms(graph, nfields, c_perms, i);

	    if(nperms == 0)
	    {
	      set[i] = curr_set_number;
	    }
	    else
	    {
  	      for(j=0; j<nperms; j++)
  	      {
  		set[c_perms[j].from] = curr_set_number;
  		set[c_perms[j].to] = curr_set_number;
  	      }

  	      if (operms < nperms) {
  		 ED_FREE(l_perms);
  		 l_perms = c_perms;
  		 ED_CALLOC(c_perms, "select_overlaps", perm_struct, nfields);
  		 operms  = nperms;
  	      }
	    }
	    curr_set_number++;
	  }
        }

        if (verbosity(DEBUG)) {
           printf("Nperms: %d\n", operms);
           for (i=0;i<operms;i++) {
	       printf("From: %d  to: %d\n", id[l_perms[i].from],
                                            id[l_perms[i].to]);
           }
        }
        remove_graph(graph);
     }

     add_key_to_tab(tab, "POSSET", nfields, set, T_SHORT, 1, "" , "");

     save_cat(outcat, control.outfile);

     ED_FREE(l_perms);
     return 0;
}







