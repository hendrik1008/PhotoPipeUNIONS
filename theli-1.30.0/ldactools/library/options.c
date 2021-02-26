/*
 *
 * History
 *
 * 03.01.2000 (TE):
 * copy_textvec: the first ED_CALLOC is only done
 *               is the number of elements to be allocated
 *               is larger than zero.
 *
 * 15.06.2001: removed the #define constructs around VPRINTF
 *             keeping only the first definition works on all
 *             platforms so far.
 *             removed compiler warning under DEC
 *
 * 20.07.2004:
 * I removed compiler warnings due to possibly
 * undefined variables (compilation with optimisation
 * turned on)
 *
 * 25.09.2009:
 * function read_options:
 * When reading options and their values, spaces are discarded
 * at the beginning and the end of character strings
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils_globals.h"
#include "fitscat.h"
#include "options_globals.h"
#include "lists_globals.h"

/*
 *  Local function prototypes
 */

void	set_option(char *, char *),
		read_options(char *),
		set_verbosity();

static int verb_level(char *);
/*
 * Local variables and definitions
 */
#define VERB_LEVS 4

static int	nopts,		/* # options in final merged list */
		verbose_level,	/* Current set verbosity level */
                have_read = FALSE; /* Has the command file been read? */
static option	*opts;		/* Merged options list */
static char	verbose[80],	/* Verbosity option container */
		*levs[VERB_LEVS] = {"NONE", "NORMAL", "VERBOSE", "DEBUG"};

/*
 *  First the public routines
 */

void altverbose(char *verbo)
{
    strcpy(verbose, verbo);
    set_verbosity();
}

/****** VPRINTF ***************************************************
 *
 *  NAME	
 *      VPRINTF - printf-like command for verbosity limited info on stdout
 *
 *  SYNOPSIS
 *     void VPRINTF(verbo, va_alist)
 *
 *  FUNCTION
 *     Print the va_alist strings with argument subsititution to the stdout
 *     based on the current vebosity level. The verbo argument may consist
 *     out of "NONE", "NORMAL", "VERBOSE", or "DEBUG". Based on the current
 *     setting of the verbosity level (one of the above values) the string
 *     and its associated arguments are printed. The verbosity level is an
 *     increasing number from NONE to DEBUG. When the verbo argument is
 *     associated with a level higher or equal to the verbosity level the
 *     va_alist is printed.
 *
 *  INPUTS
 *     verbo    - Verbosity level ("NONE", "NORMAL", "VERBOSE", or "DEBUG")
 *     va_alist - A variable argument list similar to that of printf
 *
 *  RESULTS
 *     None.
 *
 *  RETURNS
 *     None.
 *
 *  MODIFIED GLOBALS
 *     None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 01-02-1996
 *
 ******************************************************************************/
void VPRINTF(int verbo, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    if (verbo <= verbose_level) {
       if (verbo == NORMAL)
          (void)fprintf(stderr, "\33[1A\33[2K");

       (void)vfprintf(stderr, fmt, args);
       va_end(args);
    }
}

void parse_options(char *file)
{
/****** parse_options ***************************************************
 *
 *  NAME	
 *      parse_options - Parse to configuration file for user options
 *
 *  SYNOPSIS
 *      void parse_options(int n, char *file, option *user_opts)
 *
 *  FUNCTION
 *      This routine opens, reads, interprets, and closes the configuration
 *      file for user configurable parameters. The resulting interpreted
 *      values are put into the user_opts options structure.
 *
 *      See for more detail on how to specify the option structure the
 *      documentation in options_globals.h file.
 *
 *  INPUTS
 *      n         - Number of options in the option structure.
 *      file      - Filename of the configuration file
 *
 *  RESULTS
 *      user_opts - The option structure to be filled from the file.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      When an error is encountered during the interpretation of the
 *      configuration file, the routines exits with an error message.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 25-09-1994   (Original version)
 *                   dd 21-02-1995   (Loosely decouppled version)
 *                   dd 16-02-1996   (Complete separation in function)
 *
 ******************************************************************************/
    read_options(file);
    set_verbosity();
}

int verbosity(int v)
{
/****** verbosity ***************************************************
 *
 *  NAME	
 *      verbosity - Check if argument verbosity exceeds current level
 *
 *  SYNOPSIS
 *      int verbosity(int v)
 *
 *  FUNCTION
 *      Obtain information on the fact if the provided verbosity character
 *      string exceeds the currently set verbosity.
 *
 *  INPUTS
 *      v     - Verbosity level for which to check
 *
 *  RESULTS
 *
 *  RETURNS
 *      FALSE - When the current verbosity level does not exceed the
 *              arguments verbosity.
 *      TRUE  - When the current verbosity level does exceed the arguments
 *              verbosity.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 25-09-1994
 *
 ******************************************************************************/
    int retcode = FALSE;

    if (v <= verbose_level) retcode = TRUE;

    return retcode;
}

void set_command_option(char *opt, char *optval)
{
    char tmp_optval[MAXCHAR];

    strcpy(tmp_optval, optval);
    set_option(&opt[1], tmp_optval);
}

/*
 * Now the local funtions
 */

void set_verbosity()
{
/****i* set_verbosity ***************************************************
 *
 *  NAME	
 *      set_verbosity - Set the current verbosity level
 *
 *  SYNOPSIS
 *      static void set_verbosity()
 *
 *  FUNCTION
 *      Set the current verbosity level to the one obtained from the user
 *      as stored in the user_opts option structure. Note that the user
 *      does not provide an VERBOSE option into the user_opts structure.
 *      This is always added to the user provided list.
 *
 *  INPUTS
 *      None. Except that the global variable verbose needs to have been
 *            set.
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      The global variable verbose_level is set.
 *
 *  NOTES
 *      The verbose global variable is set by the parse_options routine
 *      during interpretation of the users configuraion file.
 *
 *  BUGS
 *      If verbose has not been set this leads to undefined behaviour.
 *
 *  AUTHOR
 *      E.R. Deul  - dd 25-09-1994
 *
 ******************************************************************************/
     verbose_level=verb_level(verbose);
}

static int verb_level(char *level)
{
/****i* verb_level **********************************************************
 *
 *  NAME	
 *      verb_level - Obtain the level value of the provided character string
 *
 *  SYNOPSIS
 *      static int verb_level(char *level)
 *
 *  FUNCTION
 *      This routine steps through the allowed verbosity level stringsarray
 *      and determines the index in that array of the provided verbosity
 *      string.
 *
 *  INPUTS
 *      level - Verbosity string for which to obtain the level
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      The integer value of the verbosity level associated with the
 *      arguments verbosity.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *      If the argument given is not in the array of allowed verbosity
 *      strings, the resulting level will be the maximum allowed level+1.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 25-09-1994
 *
 ******************************************************************************/
     int i ,j = VERB_LEVS;

     for (i=0;i<VERB_LEVS;i++) {
        if (strcmpcu(levs[i],level) == 0)
        {
           j = i;
        }
     }
     return j;
}


static void copy_option(option *o_opts, option i_opts)
{
/****i* copy_option ***************************************************
 *
 *  NAME	
 *      copy_option - Copy the content of an option element
 *
 *  SYNOPSIS
 *      static void copy_option(option *o_opts, option i_opts)
 *
 *  FUNCTION
 *      Copy all the elements of the option structure from the input to
 *      the output option strcuture element.
 *
 *  INPUTS
 *      i_opts - Input option element to be copied
 *
 *  RESULTS
 *      o_opts - Output option element to which to copy
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
    int k;

    strcpy(o_opts->name, i_opts.name);
    o_opts->expl = i_opts.expl;
    o_opts->type = i_opts.type;
    o_opts->ptr = i_opts.ptr;
    o_opts->imin = i_opts.imin;
    o_opts->imax = i_opts.imax;
    o_opts->dmin = i_opts.dmin;
    o_opts->dmax = i_opts.dmax;
    o_opts->ival = i_opts.ival;
    o_opts->dval = i_opts.dval;
    o_opts->bval = i_opts.bval;
    strcpy(o_opts->cval, i_opts.cval);
    for (k=0;k<MAXOPTS;k++) {
       strcpy(o_opts->list[k], i_opts.list[k]);
    }
}

void init_options(int n, option *user_opts)
{
/****** init_options ***************************************************
 *
 *  NAME	
 *      init_options - Initialize the options package
 *
 *  SYNOPSIS
 *      static void init_options(int n, option *user_opts)
 *
 *  FUNCTION
 *      This function copies the user provided option structure to the
 *      local option structure and appends the VERBOSE option the that.
 *
 *  INPUTS
 *      n         - Number of options in the user provided option structure
 *      user_opts - Pointer to the user option structure
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      None.
 *
 *  NOTES
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 16-02-1996
 *
 ******************************************************************************/
   int		i, len;
   char		*t;

   i = 0;
   while (user_opts[i].name[0] != '\0') {
      switch(user_opts[i].type) {
      case BOOLEAN: *(bool *)(user_opts[i].ptr) = user_opts[i].bval; break;
      case INT    : *(int *)(user_opts[i].ptr) = user_opts[i].ival; break;
      case FLOAT  : *(double *)(user_opts[i].ptr) = user_opts[i].dval; break;
      case STRING : strcpy(user_opts[i].ptr, user_opts[i].cval); break;
      case TEXT   : len = strlen(user_opts[i].cval);
                    ED_MALLOC(t, "init_option", char, (len+2));
                    strcpy(t, user_opts[i].cval);
                    t[len+1]=user_opts[i].nelem;
                    memcpy(user_opts[i].ptr,t,len+2);
                    ED_FREE(t);
                    break;
      case TEXTVEC: strcpy(user_opts[i].ptr, user_opts[i].cval);
                    break;
      default     : break;
      }
      i++;
   }
   n = i;
   nopts = n+1;
   ED_CALLOC(opts, "init_option", option, nopts);
   for (i=0;i<n;i++) {
      copy_option(&opts[i], user_opts[i]);
   }
   strcpy(opts[n].name, "VERBOSE");
   opts[n].type = STRING;
   opts[n].ptr = verbose;
   strcpy(opts[n].ptr, "NORMAL");
   for (i=0;i<VERB_LEVS;i++) {
      strcpy(opts[n].list[i],levs[i]);
   }
   ED_CALLOC(opts[n].expl, "init_option", char, MAXCHAR);
   strcpy(opts[n].expl, "Verbosity of the program");
}

void print_options(char *me)
{
   int		i, j, k;
   char		*tptr;

   i = 0;
   printf("# Configuration parameter file for:\n# %s\n# \n",me);
   while (opts[i].name[0] != '\0') {
      printf("# %s\n", opts[i].expl);
      switch(opts[i].type) {
      case BOOLEAN: printf("# Keyword type is BOOLEAN allowing values YES or NO\n%s = ", opts[i].name);
                    if (*(bool *)(opts[i].ptr) == YES)
                      printf("YES");
                    else
                      printf("NO");
                    break;
      case INT    : printf("# Keyword type is INT limits (%d,%d)\n%s = ",
                    opts[i].imin,opts[i].imax, opts[i].name);
                    printf("%d", *(int *)(opts[i].ptr));
                    break;
      case FLOAT  : printf("# Keyword type is FLOAT limits (%f,%f)\n%s = ",
                    opts[i].dmin, opts[i].dmax,opts[i].name);
                    printf("%f", *(double *)(opts[i].ptr));
                    break;
      case FARRAY :{printf("# Keyword type is FLOATING ARRAY\n%s = ",
                           opts[i].name);
                    tptr = opts[i].ptr;
                    for (k=0;k<10;k++) {
                       if (*(double *)tptr != 0.0) {
                          printf("%f ", *(double *)tptr);
                          tptr += sizeof(double);
                       }
                    }
                   }
                    break;
      case STRING : printf("# Keyword type is STRING allowing only:\n# ");
                    for (j=0;j<MAXOPTS;j++)
                       if (opts[i].list[j][0] != '\0')
                          printf("%s ",opts[i].list[j]);
                    printf("\n%s = ", opts[i].name);
                    printf("%s", (char *)opts[i].ptr);
                    break;
      case TEXT   : printf("# Keyword type is TEXT allowing any string\n");
                    printf("%s = ", opts[i].name);
                    printf("%s", (char *)opts[i].ptr);
                    break;
      case TEXTVEC: printf("# Keyword type is TEXTVEC allowing any string\n");
                    printf("# which could be a repetative keyword with others\n");
                    printf("%s = ", opts[i].name);

      default     : break;
      }
      i++;
      printf("\n#\n");
   }

}
void finish_options(char *file)
{
   parse_options(file);
   ED_FREE(opts[nopts-1].expl);
   ED_FREE(opts);
}


void set_option(char *n, char *s)
{
/****i* set_option ***************************************************
 *
 *  NAME	
 *      set_option - Set the option n to the value of s in the global structure
 *
 *  SYNOPSIS
 *      static void set_option(char *n, char *s)
 *
 *  FUNCTION
 *      This function sets the option named n to the value specified in s.
 *      THe global opts structure is modified accoriding to the arguments.
 *
 *  INPUTS
 *      n   - Character string giving the name of the option
 *      s   - Character string representing the value(s) for the option
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      The global opts structure is updated with the information provided
 *      by the arguments to this routine.
 *
 *  NOTES
 *      If the specified option is not available in the option structure
 *      an error message is produced and the routine exits.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 25-09-1994
 *
 ******************************************************************************/
   int    i, j, ival, len;
   double dval;
   bool   bval = NO;
   char   *t, *ptr;
   double *d = NULL;

   for (i=0;i<nopts;i++) {
      if (strcmpcu(n,opts[i].name) == 0) {
         switch (opts[i].type) {
         case FARRAY: j = 0;
              t = strtok(s," ");
              while (t != NULL) {
                 dval = atof(t);
                 if (j == 0) {
                    if (!(d = (double *)malloc(sizeof(double))))
                       syserr(FATAL,"set_option",
                              "Error allocating memory in set_option\n");
                 } else {
                   if (!(d = (double *)realloc(d,sizeof(double)*(j+1))))
                      syserr(FATAL,"set_option",
                             "Error reallocating memory in set_option\n");
                 }
                 d[j++] = dval;
                 t = strtok(NULL," ");
              }
              memcpy(opts[i].ptr,(char *)d,j*sizeof(double));
              free(d);
              goto done;
         case BOOLEAN:
              s = strtok(s," ");
              if (strcmpcu(s,opts[i].list[0]) == 0)
                 bval = YES;
              else { if (strcmpcu(s,opts[i].list[1]) == 0)
                 bval = NO;
              else syserr(FATAL,"set_option",
                       "Option %s on keyword %s should be either YES or NO\n",
                             s,opts[i].name);
              }
              *(bool *)(opts[i].ptr) = bval;
              goto done;
         case FLOAT: dval = atof(s);
              if (dval < opts[i].dmin || dval > opts[i].dmax) {
                 syserr(INFO, "set_option", "Option %s out of bounds at\n",
                              opts[i].name);
                 syserr(FATAL,"set_option",
                              "allowed minimum: %f, and maximum %f\n",
                              opts[i].dmin,opts[i].dmax);
              }
              *(double *)(opts[i].ptr) = dval;
              goto done;
         case INT:   ival = atoi(s);
              if (ival < opts[i].imin || ival > opts[i].imax) {
                 syserr(INFO, "set_option", "Option %s out of bounds at %s",
                              opts[i].name,s);
                 syserr(FATAL,"set_option",
                              "allowed minimum: %d, and maximum %d\n",
                              opts[i].imin,opts[i].imax);
              }
              *(int *)(opts[i].ptr) = ival;
              goto done;
         case STRING: s = strtok(s," ");
              for (j=0;j<MAXOPTS;j++)
                 if (opts[i].list[j][0] != '\0')
                    if (strcmpcu(s,opts[i].list[j]) ==  0) {
                       strcpy(opts[i].ptr,s);
                       if (strcmpcu(opts[i].name,"VERBOSE") == 0) {
                          altverbose(s);
                       }
                       goto done;
                    }
              syserr(INFO,"set_option",
                          "Unknown option %s for keyword %s must be one of\n",
                          s,n);
              for (j=0;j<MAXOPTS;j++)
                 if (opts[i].list[j][0] != '\0')
                    syserr(INFO,"set_option", "%s\n",opts[i].list[j]);
              goto done;
         case TEXT: s = strtok(s,"( ");
              len = strlen(s); t = s;
              if ((t = strtok(NULL, ") ")) != NULL) {
                 s[len+1] = atoi(t);
              } else {
                 s[len+1] = opts[i].nelem;
              }
              memcpy(opts[i].ptr,s,len+2);
              goto done;
         case TEXTVEC: if (strtok(s,"\"") != NULL)
                           s = strtok(s,"\"");
              j=0;
              ptr=(char *)opts[i].ptr;
              while (*ptr != '\0' && opts[i].nelem != 0) {
                 ptr+=OPTLEN;
                 j++;
                 if (j>=TEXTVECLEN/OPTLEN)
                    syserr(FATAL,"set_option", "Too many %s entries (%d>%d)\n",
                         opts[i].name,j, TEXTVECLEN/OPTLEN);
              }
              memcpy(ptr,s,OPTLEN);
              opts[i].nelem++;
              goto done;
         default: break;
         }
      }
   }
   syserr(FATAL,"set_option", "Unknown keyword %s\n",n);

done:
   return;
}


void read_options(char *file)
/****i* read_options ***************************************************
 *
 *  NAME	
 *      read_options - Interpret the configuration file and set the options
 *
 *  SYNOPSIS
 *      static void read_options(char *file)
 *
 *  FUNCTION
 *      Read the configuration file, interpret the lines and provided
 *      information. The information is storedin the global opts structure.
 *      The configuration file syntax is described in detail in the include
 *      file: options_globals.h.
 *
 *  INPUTS
 *      file - Filename of the configuration file
 *
 *  RESULTS
 *      None.
 *
 *  RETURNS
 *      None.
 *
 *  MODIFIED GLOBALS
 *      The global opts structure is updated with the values provided by
 *      the user in the configuration file.
 *
 *  NOTES
 *      When an interpretation error is encountered the routine exits with
 *      an error message.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 25-09-1994  (Original version)
 *      E.R. Deul  - dd 16-02-1996  (Decouppled version)
 *
 ******************************************************************************/
{
  char user_opt[MAXCHAR], optio[OPTLEN], line[MAXCHAR];
  char *tmpchr1, *tmpchr2;
  int  len, i, j;
  FILE *fd;

  if (!have_read) {
    if (ED_FCHECK("read_options", file) == 0) {
       ED_FOPEN(fd, "read_options", file, "rt");
       while (fgets(line,MAXCHAR,fd) != NULL) {
         if (line[0] != '#' && line[0] != '\n') {

            tmpchr1 = strtok(line,"=");
            if ((tmpchr2 = strtok(NULL, "=\n")) == NULL) {
              syserr(FATAL, "read_options",
                     "No values for keyword %s\n", tmpchr1);
            }

            len = strlen(tmpchr1);
            for (i = 0; i < len; i++) {
               if (tmpchr1[i] != ' ' && tmpchr1[i] != '\t') break;
            }

            for (j = len - 1; (j - i) > 0; j--) {
               if (tmpchr1[j] != ' ' && tmpchr1[j] != '\t') break;
            }

            strcpy(optio, &tmpchr1[i]);
            optio[j + 1 - i] = '\0';

            len = strlen(tmpchr2);

            for (i = 0; i < len; i++) {
               if (tmpchr2[i] != ' ' && tmpchr2[i] != '\t') break;
            }

            for (j = len - 1; (j - i) > 0; j--) {
               if (tmpchr2[j] != ' ' && tmpchr2[j] != '\t') break;
            }

            strcpy(user_opt, &tmpchr2[i]);
            user_opt[j + 1 - i] = '\0';

            if (user_opt[0] == '\0')
              syserr(FATAL, "read_options",
                     "No values for keyword %s\n", tmpchr1);

            set_option(optio, user_opt);
         } else {
            if (line[0] == '\n') {
              syserr(WARNING,"read_options",
                     "Line in configuration file without option: %s\n",
                     line);
           }
         }
       }
       have_read = TRUE;
       ED_FCLOSE(fd, "read_options");
    } else {
      syserr(WARNING,"read_options",
            "Using default values for configuration parameters\n\n");
    }
  }
}

int save_options(tabstruct *tab, option *opts, int nopts)
{
    int         endpos, headpos, n, j, nlines;
    char        *key_ptr, str[82];

    endpos = fitsfind(tab->headbuf, "END     ");
/*
 *  Check that the new parameters will fit in the allocated memory space
 */
    nlines = nopts;
    j = 0;
    while (opts[j].name[0] != '\0') {
       if (opts[j].type == FARRAY) nlines += 10;
       else nlines++;
       j++;
    }
    endpos += nlines;

    if (endpos >= (n=tab->headnblock)*(FBSIZE/80)) {
       ED_REALLOC(tab->headbuf, "preastrom_table_create", char, (n+1)*FBSIZE);
       memset(tab->headbuf + n*FBSIZE, ' ', FBSIZE);
       tab->headnblock++;
    }

    j = 0;

    while (opts[j].name[0] != '\0') {
       headpos = fitsfind(tab->headbuf, "END     ");
       key_ptr = tab->headbuf+80*headpos;
       memcpy(key_ptr+80, key_ptr, 80);
       switch (opts[j].type) {
       case BOOLEAN:
          if (*(bool *)opts[j].ptr == YES) {
             sprintf(str, "HISTORY %-20.20s                    YES %-28.28s",
                  opts[j].name, " ");
          } else {
            sprintf(str, "HISTORY %-20.20s                     NO %-28.28s",
                  opts[j].name, " ");
          }
          memcpy(key_ptr, str, 80);
          break;
       case FLOAT:
          sprintf(str, "HISTORY %-20.20s       %16.9e %-28.28s",opts[j].name,
                  *(double *)opts[j].ptr, " ");
          memcpy(key_ptr, str, 80);
          break;
       case INT:
#ifdef INT64
          sprintf(str, "HISTORY %-20.20s       %16d %-28.28s",opts[j].name,
                  *(LONG *)opts[j].ptr, " ");
#else
          sprintf(str, "HISTORY %-20.20s       %16ld %-28.28s",opts[j].name,
                  *(LONG *)opts[j].ptr, " ");
#endif
          memcpy(key_ptr, str, 80);
          break;
       case TEXT:
          sprintf(str, "HISTORY %-20.20s '%-20.20s' %-28.28s",opts[j].name,
                  (char *)opts[j].ptr, " ");
          memcpy(key_ptr, str, 80);
          break;
       case STRING:
          sprintf(str, "HISTORY %-20.20s  %-20.20s  %-28.28s",opts[j].name,
                  (char *)opts[j].ptr, " ");
          memcpy(key_ptr, str, 80);
          break;
       case FARRAY:
          {
          char *tptr;
          int  k;
          tptr = opts[j].ptr;
          for (k=0;k<10;k++) {
             if (*(double *)tptr != 0.0) {
                if (k != 0) {
                   headpos = fitsfind(tab->headbuf, "END     ");
                   key_ptr = tab->headbuf+80*headpos;
                   memcpy(key_ptr+80, key_ptr, 80);
                }
                sprintf(str, "HISTORY %-20.20s       %16.9e %-28.28s",
                     opts[j].name, *(double *)tptr, " ");
                tptr += sizeof(double);
                memcpy(key_ptr, str, 80);
             }
          }}
          break;
       case IARRAY:
       case TEXTVEC:
          break;
       }
       j++;
    }

    return j;
}

char **copy_textvec(char *textvec, int *n)
{
    char 	*ptr;
    char	**string_list = NULL;
    int		i;

    ptr = textvec;
    for (i=0;i<TEXTVECLEN/OPTLEN;i++) {
      if (*ptr == '\0') break;
      ptr += OPTLEN;
    }
    *n = i;

    if((*n) > 0)
    {
      ED_CALLOC(string_list, "copy_textvec", char *, (*n));
    }

    ptr = textvec;
    for (i=0;i<(*n);i++)
    {
      ED_CALLOC(string_list[i], "copy_textvec", char, MAXCHAR);
      strncpy(&string_list[i][0] , ptr,  OPTLEN);
      ptr += OPTLEN;
   }

   return string_list;
}
