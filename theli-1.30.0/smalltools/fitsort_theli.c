/*----------------------------------------------------------------------------
                                    E.S.O.
   -----------------------------------------------------------------------------
   File name    :   fitsort.c
   Author       :   Nicolas Devillard
   Created on   :   May 1st, 1996
   Language     :   ANSI C
                    Part of ECLIPSE library for Adonis
   Description  :   Sorts out FITS keywords.
                    The input is a succession of FITS headers delivered
                    through stdin by 'dfits'. On the command line, specify
                    which keywords you wish to display, the output contains
                    for each file, the file name and the keyword values
                    in column format.
                    Example:
                    dfits *.fits | fitsort BITPIX NAXIS NAXIS1 NAXIS2 NAXIS3
                    The output would be like:

   File            BITPIX      NAXIS    NAXIS1    NAXIS2    NAXIS3

   image1.fits         32          2       256       256
   image2.fits        -32          3       128       128        40
   ...

                    Using 'fitsort -d ...' would prevent printing the
                    first line (filename and keyword names).

                    Using 'fitsort -s ...' substitutes spaces in header
                    keywords with underscores

                    The output format is simple: values are separated by
                    tabs, records by linefeeds. When no value is present
                    (no keyword in this header), only a tab is printed
                    out.

   Example:
    file1.fits contains NAXIS1=100 NAXIS2=200
    file2.fits contains NAXIS1=20

    dfits file1.fits file2.fits | fitsort NAXIS2 NAXIS1
    would litterally print out (\t stands for tab, \n for linefeed):

   file1.fits\t200\t100\n
   file2.fits\t\t20\n

   ---------------------------------------------------------------------------*/

/*

   $Id: fitsort_theli.c,v 1.3 2017/12/20 17:05:53 thomas Exp $
   $Author: thomas $
   $Date: 2017/12/20 17:05:53 $
   $Revision: 1.3 $
   $Log: fitsort_theli.c,v $
   Revision 1.3  2017/12/20 17:05:53  thomas
   Bug fix introduced in the changes from 21.11.2017: If more than one
   keyword was given as argument, the program did not work anymore correctly.

   Revision 1.2  2017/11/21 17:46:29  thomas
   - I included Unix-like command line parsing with getopt.
   - I removed compiler warnings

   Revision 1.1  2016/12/08 19:52:22  thomas
   I renamed ESO programs to clearly identify them as THELI versions.
   There were too many name clashes with programs installed under the
   same name in system-wide diretories

   Revision 1.6  2015/09/04 15:58:59  thomas
   I added usage information. It originates from the ESO fitsort man page.

   Revision 1.5  2014/12/23 02:27:32  thomas
   Now, also keys containing only of spaces only return 'KEY_EMPTY'. Before,
   only really empty keys returned that value.

   Revision 1.4  2014/09/29 08:06:36  thomas
   small refinement to the usage message. We included the '-s' f;ag
   to substitute spaces in FITS keys by underscores.

   Revision 1.3  2014/09/11 16:46:06  thomas
   I included the possibility to substitute blanks in header keywords
   with underscores (command line option '-s'). This ensures that each
   header keywords exactly produces 'one' output string (parsing in
   shell scripts etc.)

   Revision 1.2  2005/04/15 13:22:37  terben
   I changed the behaviour of the program for non-existing keys or
   'empty' keys. Up to now, in both cases a blank was printed which
   did not allow to distinguish between these cases. Now. for empty
   keywords the string 'KEY_EMPTY' is printed and for keys that do
   not exist in a header 'KEY_N/A' is prompted.

   Revision 1.1  2001/03/13 15:55:34  erben
   first checkin of atandalone external utility programs
   (they consist of single C-files that can be compiled standalone
   without the need for any library etc.)

   Revision 1.4  1999/10/26 11:48:02  ndevilla
   corrected print_hdr feature (used to coredump on Linux)

   Revision 1.3  1999/09/21 15:39:14  ndevilla
   added -d option (forgot to implement it!)

   Revision 1.2  1999/09/20 12:30:18  ndevilla
   added support for non-dfits inputs (missing filenames)
   as a result, it is not possible to do 'fitsort SIMPLE', but that
   should not be too much of a problem... should it?

   Revision 1.1  1999/05/04 14:41:39  isaacp
   Initial revision

   04.09.2015:
   I added usage information. It originates from the ESO fitsort man page.

   21.11.2017:
   - I included Unix-like command line parsing with getopt.
   - I removed compiler warnings

   20.12.2017:
   Bug fix introduced in the changes from 21.11.2017: If more than one
   keyword was given as argument, the program did not work anymore correctly.
 */

/*----------------------------------------------------------------------------
                                Includes
   ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

/*----------------------------------------------------------------------------
                                Defines
   ---------------------------------------------------------------------------*/

#define MAX_STRING    128      /* Maximum string length                    */
#define MAX_KEY       512      /* Maximum number of keywords to search for */

#define FMT_STRING    "%%-%ds\t"

/*----------------------------------------------------------------------------
                                New types
   ---------------------------------------------------------------------------*/

/* This holds a keyword value and a flag to indicate its presence   */

typedef struct _KEYWORD_ {
  char value[MAX_STRING];
  int  present;
} keyword;

/*
 * Each detected file in input has such an associated structure, which
 * contains the file name and a list of associated keywords.
 */

typedef struct _RECORD_ {
  char    filename[MAX_STRING];
  keyword listkw[MAX_KEY];
} record;

/*
 * NB: Everything is deliberately allocated statically. As we are not
 * dealing with huge amounts of data, works fine and makes it easier to
 * read/write/modify.
 */

/*----------------------------------------------------------------------------
                            Function prototypes
                        See function descriptions below
   ---------------------------------------------------------------------------*/

void usage();  /* print usage message */

static int isfilename(char *string);

static void getfilename(char *line, char *word);

static char *expand_hierarch_keyword(
  char *dotkey,
  char *hierarchy
  );

static int isdetectedkeyword(char *line, char *keywords[], int nkeys);

static void getkeywordvalue(char *line, char *word);

static char *trimwhitespace(char *str);

/*----------------------------------------------------------------------------
                            Function codes
   ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  char    curline[MAX_STRING];
  char    word[MAX_STRING];
  char    *word_tmp;
  int     c, i, j;
  int     nfiles;
  record *allrecords;
  int     kwnum;
  int     len;
  int     max_width[MAX_KEY];
  int     max_filnam;
  char    fmt[8];
  int     flag;
  int     printnames;
  int     print_hdr;
  int     subst_spaces ; /* do we substitute spaces in keyword values
                            with underscores? (1=YES) */

  if (argc < 2) {
    usage();
    return(1);
  }

  printnames   = 0;
  print_hdr    = 1;
  subst_spaces = 0;
  nfiles       = 0;
  allrecords   = (record *)calloc(1, sizeof(record));

  while ((c = getopt(argc, argv, "sd")) != -1) {
    switch (c) {
      case 's':
        subst_spaces = 1;
        break;
      case 'd':
        print_hdr = 0;
        break;
      }
  }

  argv += (optind - 1);
  argc -= (optind - 1);
  argv++;

  while (fgets(curline, MAX_STRING, stdin) != (char *)NULL) {
    if ((flag = isfilename(curline))) {
      /* New file entry is detected */
      if (flag == 1) {
        /* New file name is detected, get the new file name */
        printnames = 1;
        getfilename(curline, allrecords[nfiles].filename);
        /* Absorb next line (contains SIMPLE=) */
        if (fgets(curline, MAX_STRING, stdin) == (char *)NULL) {
          fprintf(stderr, "*** Error in reading data!");
          return 1;
        }
      } else {
        /* New SIMPLE=T entry, no associated file name */
        allrecords[nfiles].filename[0] = (char)0;
      }
      nfiles++;

      /*
       * Initialize a new record structure to store input data for
       * this file.
       */

      allrecords = (record *)realloc(allrecords,
                                     (nfiles + 1) * sizeof(record));
      for (i = 0; i < MAX_KEY; i++) {
        allrecords[nfiles].listkw[i].present = 0;
      }
    } else {
      /* Is not a file name, is it a searched keyword?    */
      if ((kwnum = isdetectedkeyword(curline,
                                     argv,
                                     argc - 1)) != -1) {
        /* Is there anything allocated yet to store this? */
        if (nfiles > 0) {
          /* It has been detected as a searched keyword.  */
          /* Get its value, store it, present flag up     */
          getkeywordvalue(curline, word);
          /* first remove leading/trailing whitespace if any */
          word_tmp = trimwhitespace(word);

          if (strlen(word_tmp) == 0) {
            strcpy(allrecords[nfiles - 1].listkw[kwnum].value, "KEY_EMPTY");
          } else {
            /* should we substitute blanks in header keywords with
               underscores? */
            if (subst_spaces == 1) {
              for (j = 0; j < strlen(word_tmp); j++) {
                if (word_tmp[j] == ' ') {
                  word_tmp[j] = '_';
                }
              }
            }
            strcpy(allrecords[nfiles - 1].listkw[kwnum].value, word_tmp);
          }
          allrecords[nfiles - 1].listkw[kwnum].present++;
        }
      }
    }
  }
  for (i = 0; i < argc - 1; i++) {
    max_width[i] = (int)strlen(argv[i]);
  }

  /* Record the maximum width for each column */
  max_filnam = 0;
  for (i = 0; i < nfiles; i++) {
    len = (int)strlen(allrecords[i].filename);
    if (len > max_filnam) {
      max_filnam = len;
    }
    for (kwnum = 0; kwnum < argc - 1; kwnum++) {
      if (allrecords[i].listkw[kwnum].present) {
        len = (int)strlen(allrecords[i].listkw[kwnum].value);
      } else {
        /* the '7' is the length of 'KEY_N/A' which is printed
           for not available keywords */
        len = 7;
      }
      if (len > max_width[kwnum]) {
        max_width[kwnum] = len;
      }
    }
  }

  /* Print out header line */
  if (print_hdr) {
    sprintf(fmt, FMT_STRING, max_filnam);
    if (printnames) {
      printf(fmt, "FILE");
    }
    for (i = 0; i < argc - 1; i++) {
      sprintf(fmt, FMT_STRING, max_width[i]);
      printf(fmt, argv[i]);
    }
    printf("\n");
  }


  /* Now print out stored data    */
  if (nfiles < 1) {
    fprintf(stderr, "*** error: no input data corresponding to dfits output\n");
    return 1;
  }
  for (i = 0; i < nfiles; i++) {
    if (printnames) {
      sprintf(fmt, FMT_STRING, max_filnam);
      printf(fmt, allrecords[i].filename);
    }
    for (kwnum = 0; kwnum < argc - 1; kwnum++) {
      sprintf(fmt, FMT_STRING, max_width[kwnum]);
      if (allrecords[i].listkw[kwnum].present) {
        printf(fmt, allrecords[i].listkw[kwnum].value);
      } else {
        printf(fmt, "KEY_N/A");
      }
    }
    printf("\n");
  }
  free(allrecords);
  return 0;
}

/*----------------------------------------------------------------------------
   Function :   isfilename()
   In       :   dfits output line
   Out      :   integer
                1 if the line contains a valid file name as produced
                by dfits.
                2 if the line starts with 'SIMPLE  ='
                0 else
   Job      :   find out if an input line contains a file name or a
                FITS magic number
   Notice   :
   Filename recognition is based on 'dfits' output.
   ---------------------------------------------------------------------------*/

static int isfilename(char *string)
{
  if (!strncmp(string, "====>", 5)) {
    return 1;
  }
  if (!strncmp(string, "SIMPLE  =", 9)) {
    return 2;
  }
  return 0;
}

/*----------------------------------------------------------------------------
   Function :   getfilename()
   In       :   dfits output line
   Out      :   second argument: file name
   Job      :   returns a file name from a dfits output line
   Notice   :   This is dfits dependent.
   ---------------------------------------------------------------------------*/

static void getfilename(char *line, char *word)
{
  /* get filename from a dfits output */
  sscanf(line, "%*s %*s %s", word);
  return;
}

/*----------------------------------------------------------------------------
   Function :   isdetectedkeyword()
   In       :   FITS line, set of keywords, number of kw in the set.
   Out      :   keyword rank, -1 if unidentified
   Job      :   detects a if a keyword is present in a FITS line.
   Notice   :
                Feed this function a FITS line, a set of keywords in the
 * argv[] fashion (*keywords[]).
                If the provided line appears to contain one of the keywords
                registered in the list, the rank of the keyword in the list
                is returned, otherwise, -1 is returned.
   ---------------------------------------------------------------------------*/
static int isdetectedkeyword(char *line,
                             char *keywords[],
                             int nkeys)
{
  int  i;
  char kw[MAX_STRING];
  char esokw[MAX_STRING];

  /*
   * The keyword is defined as the input line, up to the equal character,
   * with trailing blanks removed
   */
  strcpy(kw, line);
  strtok(kw, "=");
  /* Now remove all trailing blanks (if any) */
  i = (int)strlen(kw) - 1;
  while (kw[i] == ' ') {
    i--;
  }
  kw[i + 1] = (char)0;

  /* Now compare what we got with what's available */
  for (i = 0; i < nkeys; i++) {
    if (strstr(keywords[i], ".") != NULL) {
      /*
       * keyword contains a dot, it is a hierarchical keyword that
       * must be expanded. Pattern is:
       * A.B.C... becomes HIERARCH ESO A B C ...
       */
      expand_hierarch_keyword(keywords[i], esokw);
      if (!strcmp(kw, esokw)) {
        return i;
      }
    } else if (!strcmp(kw, keywords[i])) {
      return i;
    }
  }
  /* Keyword not found    */
  return -1;
}

/*---------------------------------------------------------------------------
   Function :   expand_hierarch_keyword()
   In       :   two allocated strings
   Out      :   char *, pointer to second input string (modified)
   Job      :   from a HIERARCH keyword in format A.B.C expand to
                HIERARCH ESO A B C
   Notice   :
   ---------------------------------------------------------------------------*/

static char *expand_hierarch_keyword(char *dotkey, char *hierarchy)
{
  char *token;
  char  ws[MAX_STRING];

  sprintf(hierarchy, "HIERARCH ESO");
  strcpy(ws, dotkey);
  token = strtok(ws, ".");
  while (token != NULL) {
    strcat(hierarchy, " ");
    strcat(hierarchy, token);
    token = strtok(NULL, ".");
  }
  return(hierarchy);
}

/*----------------------------------------------------------------------------
   Function :   getkeywordvalue()
   In       :   FITS line to process, char string to return result
   Out      :   void, result returned in char *word
   Job      :   Get a keyword value within a FITS line
   Notice   :   No complex value is recognized
   ---------------------------------------------------------------------------*/

static void getkeywordvalue(char *line, char *word)
{
  int   c, w;
  char  tmp[MAX_STRING];
  char *begin, *end;
  int   length;
  int   quote  = 0;
  int   search = 1;

  memset(tmp, (char)0, MAX_STRING);
  memset(word, (char)0, MAX_STRING);
  c = w = 0;

  /* Parse the line till the equal '=' sign is found  */
  while (line[c] != '=') {
    c++;
  }
  c++;

  /* Copy the line till the slash '/' sign is found   */
  /* or the end of data is found.                     */

  while (search == 1) {
    if (c >= 80) {
      search = 0;
    } else if ((line[c] == '/') && (quote == 0)) {
      search = 0;
    }

    if (line[c] == '\'') {
      quote = !quote;
    }

    tmp[w++] = line[c++];
  }

  /* NULL termination of the string   */
  tmp[--w] = (char)0;

  /* Return the keyword only : a difference is made between text fields   */
  /* and numbers.                                                         */

  if ((begin = strchr(tmp, '\'')) != (char *)NULL) {
    /* A quote has been found: it is a string value */
    begin++;
    end    = strrchr(tmp, '\'');
    length = (int)strlen(begin) - (int)strlen(end);
    strncpy(word, begin, length);
  } else {
    /* No quote, just get the value (only one, no complex supported) */
    sscanf(tmp, "%s", word);
  }

  return;
}

/* Note: This function returns a pointer to a substring of the original string.
  If the given string was allocated dynamically, the caller must not overwrite
  that pointer with the returned value, since the original pointer must be
  deallocated using the same allocator with which it was allocated.  The return
  value must NOT be deallocated using free() etc. */
static char *trimwhitespace(char *str)
{
  char *end;

  /* Trim leading space */
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  /* Trim trailing space */
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  /* Write new null terminator */
  *(end + 1) = 0;

  return str;
}

/* print usage message */
void usage(void)
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "       fitsort - sort FITS header information from a list of files\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "       dfits <FITS files...> | fitsort [-d] [-s] <FITS keywords...>\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "OPTIONS\n");
  fprintf(stdout, "       -d     Do not print out the first output line. This option is useful to\n");
  fprintf(stdout, "              get  only  the  query  results, without the top line (giving all\n");
  fprintf(stdout, "              column names).  This  makes  it  easy  to  script  fitsort  from\n");
  fprintf(stdout, "              programs like awk or perl.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       -s     All spaces in header keyword values are output as\n");
  fprintf(stdout, "              underscores. This ensures that out tables from fitsort\n");
  fprintf(stdout, "              are well formatted (all lines have the same number of\n");
  fprintf(stdout, "              columns) and hence allow easy postprocessing.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION\n");
  fprintf(stdout, "       fitsort  extract  keyword values from a set of FITS headers and outputs\n");
  fprintf(stdout, "       it in an ASCII  table  format,  which  is  compatible  with  most  data\n");
  fprintf(stdout, "       processing software packages. It shall only be used in combination with\n");
  fprintf(stdout, "       the dfits utility.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       The ASCII output is shown in columns. Columns are  aligned  with  blank\n");
  fprintf(stdout, "       characters  and  also  separated by tabulations. Blank alignment allows\n");
  fprintf(stdout, "       human readers to visualize the output in a pretty  format,  tabulations\n");
  fprintf(stdout, "       are  there  for  spreadsheet  compatibility.  If  you  want to load out\n");
  fprintf(stdout, "       fitsort output into any  spreadsheet,  specify  that  fields  shall  be\n");
  fprintf(stdout, "       separated by tabulations and entries separated by linefeeds.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "EXAMPLES\n");
  fprintf(stdout, "       dfits *.fits | fitsort BITPIX NAXIS NAXIS1 NAXIS2\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       The output would look like:\n");
  fprintf(stdout, "       FILE           BITPIX   NAXIS    NAXIS1   NAXIS2\n");
  fprintf(stdout, "       file0001.fits  16       2        128      128\n");
  fprintf(stdout, "       file0002.fits  32       2        512      512\n");
  fprintf(stdout, "       ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       ESO  specific  features  in  the FITS header are also supported. To get\n");
  fprintf(stdout, "       values for ’HIERARCH ESO’ keywords, just give the complete names within\n");
  fprintf(stdout, "       double quotes. e.g.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       dfits *.fits | fitsort 'HIERARCH ESO INS LENS'\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       Another  way  of  giving HIERARCH ESO keywords is to use the short FITS\n");
  fprintf(stdout, "       notation, the above example can be given as:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       dfits *.fits | fitsort INS.LENS\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       Example: to retrieve the DPR keywords from  an  ESO  FITS  header,  you\n");
  fprintf(stdout, "       would use:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       dfits *.fits | fitsort To be completed...  DPR.CATG DPR.TYPE DPR.TECH\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       This second way of requesting HIERARCH ESO keywords is not only shorter\n");
  fprintf(stdout, "       to type, it also avoids typing quotes or double-quotes on the  command-\n");
  fprintf(stdout, "       line, making it easier to script with fitsort.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       Notice  that  the  keywords  you  give  on  the  command-line are case-\n");
  fprintf(stdout, "       insensitive.  The above line is equivalent to:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       dfits *.fits | fitsort dpr.catg dpr.type dpr.tech\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR\n");
  fprintf(stdout, "       Original Author:\n");
  fprintf(stdout, "       Nicolas Devillard within the ESO qfits library\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       Maintainer of this version: \n");
  fprintf(stdout, "       Thomas Erben (terben@astro.uni-bonn.de) \n");
  fprintf(stdout, "       \n");
}

/*--------------------------------------------------------------------------*/
