
/*----------------------------------------------------------------------------
                                    E.S.O.
 -----------------------------------------------------------------------------
   File name    :   replacekey.c
   Author       :   N. Devillard
   Created on   :   July 14th, 1998
   Language     :   ANSI C
   Description  :   Search & Replace operations in FITS headers
 ---------------------------------------------------------------------------*/

/*

 $Id: replacekey_theli.c,v 1.2 2018/03/28 12:06:30 thomas Exp $
 $Author: thomas $
 $Date: 2018/03/28 12:06:30 $
 $Revision: 1.2 $
 $Log: replacekey_theli.c,v $
 Revision 1.2  2018/03/28 12:06:30  thomas
 I removed all white-space characters at the end of code lines.

 Revision 1.1  2016/12/08 19:52:22  thomas
 I renamed ESO programs to clearly identify them as THELI versions.
 There were too many name clashes with programs installed under the
 same name in system-wide diretories

 Revision 1.4  2012/01/26 11:11:15  thomas
 program cleanup phase:
 I exchanged the order of closing files and then memory unmapping them. It
 seems to me to make more sense to do munmap and the closing them, but
 I do not know whether this makes any difference at all.

 Revision 1.3  2010-09-28 11:54:29  thomas
 The program can now treat multiple FITS key exchanges simultaneously!

 Revision 1.2  2007/12/10 16:47:37  thomas
 The program ended in a permanent loop if keywords matched the placeholder
 in a substring, i.e. we want to replace EXPTIME and there was a key
 TEXPTIME present!

 Revision 1.1  2001-03-13 15:55:34  erben
 first checkin of atandalone external utility programs
 (they consist of single C-files that can be compiled standalone
 without the need for any library etc.)

 Revision 1.2  2000/03/21 15:59:45  ndevilla
 updated get_FITS_header_size()

 Revision 1.1  1999/10/14 16:19:22  ndevilla
 Initial revision


 */

/* ------------------------------------------------------------------------
                                History Comments
   ------------------------------------------------------------------------ */

/*
  10.12.2007:
  The program ended in a permanent loop if keywords matched the placeholder
  in a substring, i.e. we want to replace EXPTIME and there was a key
  TEXPTIME present!

  23.09.2010:
  The program can now treat multiple FITS key exchanges simultaneously!

  26.01.2012:
  program cleanup phase:
  I exchanged the order of closing files and then memory unmapping them. It
  seems to me to make more sense to do munmap and the closing them, but
  I do not know whether this makes any difference at all.
 */

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>


/*----------------------------------------------------------------------------
                                Defines
 ---------------------------------------------------------------------------*/

#define NM_SIZ                  512
#define DEFAULT_PLACEHOLDER     "COMMENT PLACEHOLDER"

/*----------------------------------------------------------------------------
                   Private functions and module variables
 ---------------------------------------------------------------------------*/

static char prog_desc[] = "replace keyholder in a FITS header" ;

static void             usage(char *) ;
static int              replace_placeholder(char *, int, char **, char **) ;
static size_t           get_FITS_header_size(char *) ;


/*----------------------------------------------------------------------------
                            main()
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char **placeholder = NULL;
    char **card = NULL;
    int nkeys; /* number of keys to change/replace */
    int i;

    if (argc < 4 || (argc - 1) % 2 != 1) {
      usage(argv[0]) ;
    }

    nkeys = (argc - 1) / 2;

    placeholder = (char **)malloc(nkeys * sizeof(char *));
    card = (char **)malloc(nkeys * sizeof(char *));

    if (placeholder == NULL || card == NULL) {
      fprintf(stderr, "Memory allocation error for %s", argv[0]);
      exit(1);
    }

    for (i = 0; i < nkeys; i++) {
      placeholder[i] = (char *)malloc(NM_SIZ * sizeof(char));
      card[i] = (char *)malloc(NM_SIZ * sizeof(char));

      if (placeholder[i] == NULL || card[i] == NULL) {
        fprintf(stderr, "Memory allocation error for %s", argv[0]);
        exit(1);
      }

      strncpy(card[i], argv[2 + (i * 2)], (NM_SIZ - 1)) ;
      strncpy(placeholder[i], argv[3 + (i * 2)], (NM_SIZ - 1)) ;

      /* Sanity test on card length */
      if (strlen(card[i]) > 80) {
        fprintf(stderr, "requested card is too long: %d chars\n",
                (int)strlen(card[i]));
        exit(1);
      }
    }

    if (replace_placeholder(argv[1], nkeys, card, placeholder) != 0) {
        fprintf(stderr, "error during placeholder replacement: aborting\n") ;
        exit(1);
    }
    return 0 ;
}


/*
 * This function only gives the usage for the program
 */

static void usage(char *pname)
{
    printf("%s : %s\n", pname, prog_desc) ;
    printf("use : %s <in> <card> <placeholder> <card> <placeholder> ..\n",
           pname) ;
    printf("\n\n") ;
    exit(0) ;
}


/*---------------------------------------------------------------------------
   Function :   replace_placeholder()
   In       :   name of the FITS file to modify, card to insert,
                name of the placeholder
   Out      :   int 0 if Ok, anything else otherwise
   Job      :   replaces a placeholder by a given FITS card
   Notice   :   modifies the input FITS file
                Does not check the validity of the given FITS card
 ---------------------------------------------------------------------------*/


static int replace_placeholder(char *namein, int nkeys,
                               char **card, char **placeholder)
{
    int    fd;
    int    i;
    char   *buf;
    char   *where;
    size_t hs;

    /*
     * mmap the FITS header of the input file
     */

    hs = get_FITS_header_size(namein) ;
    if (hs < 1) {
        fprintf(stderr, "error getting FITS header size for %s\n", namein);
        return 1;
    }
    fd = open(namein, O_RDWR) ;
    if (fd == -1) {
        fprintf(stderr, "cannot open %s: aborting\n", namein);
        return 1;
    }
    buf = (char*)mmap(0,
                      hs,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fd,
                      0) ;
    if (buf == (char*)-1) {
        perror("mmap") ;
        fprintf(stderr, "cannot mmap file: %s\n", namein);
        close(fd);
        return 1;
    }

    /*
     * Apply search and replace for the input keyword lists
     */

    for (i = 0; i < nkeys; i++) {
      where = buf;
      where--;

      /* The following loop ensures that only FITS keywords matching
         'placeholder' exactly are returned. The strstr function used
         for string comparisons would match each string that matches
         'placeholder' as a substring. Hence, we make sure that only
         'placeholder' matches at the start of a FITS line are treated.
         The 'where ++' command in the loop (and the where -- command
         above) are necessary because otherwise, subsequent calls to
         strstr always would match the SAME substring and produce and
         endless loop
      */
      do {
        where++;
        where = strstr(where, placeholder[i]);
        if (where == NULL) {
          fprintf(stderr, "not found in input: [%s]\n", placeholder[i]);
          close(fd);
          munmap(buf, hs);
          return 1;
        }
      } while ((where - buf) % 80) ;

      /* Replace current placeholder by blanks */
      memset(where, ' ', 80);
      /* Copy card into placeholder */
      memcpy(where, card[i], strlen(card[i]));
    }

    munmap(buf, hs);
    close(fd);

    return 0 ;
}


/*---------------------------------------------------------------------------
   Function :   get_FITS_header_size()
   In       :   FITS file name
   Out      :   unsigned long
   Job      :   compute the size (in bytes) of a FITS header
   Notice   :   should always be a multiple of 2880. This implementation
               assumes only that 80 characters are found per line.
 ---------------------------------------------------------------------------*/

#define FITS_BLSZ 2880
static size_t get_FITS_header_size(char *name)
{
    FILE    *in;
    char    line[81];
    int     found = 0;
    int     count;
    size_t  hs;

    if ((in = fopen(name, "r")) == NULL) {
        fprintf(stderr, "cannot open %s: aborting\n", name);
        return 0;
    }
    count = 0 ;
    while (!found) {
      if (fread(line, 1, 80, in) != 80) {
        break ;
      }
      count++;
      if (!strncmp(line, "END ", 4)) {
        found = 1 ;
      }
    }
    fclose(in);

    if (!found) {
      return 0;
    }
    /*
     * The size is the number of found cards times 80,
     * rounded to the closest higher multiple of 2880.
     */
    hs = (size_t)(count * 80);
    if ((hs % FITS_BLSZ) != 0) {
        hs = (1 + (hs / FITS_BLSZ)) * FITS_BLSZ;
    }
    return hs;
}
#undef FITS_BLSZ

/*------------------------------ end of file ------------------------------*/

