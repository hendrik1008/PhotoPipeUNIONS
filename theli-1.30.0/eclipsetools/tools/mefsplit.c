/*---------------------------------------------------------------------------

   File name  :	mefsplit.c
   Author     :	T. Erben
   Created on	:	September 2003
   Description	:	MEF frame splitter

   ---------------------------------------------------------------------------*/

/*
        $Id: mefsplit.c,v 1.23 2018/03/28 12:06:23 thomas Exp $
        $Author: thomas $
        $Date: 2018/03/28 12:06:23 $
        $Revision: 1.23 $
 */

/*---------------------------------------------------------------------------
                                                                History
   ---------------------------------------------------------------------------*/

/*
   05.09.03:
   changes to include the OBJECT keyword in the new headers

   14.09.03:
   included set_verbose(1) to get comment messages printed by
   e_comment etc. The messages previously appeared on Linux SuSE
   but not on Sun and Linux Debian

   13.10.03:
   included possibility to flip frames in x and/or y

   20.07.2004:
   I added the '-d' option (dump configuration default values)
   to the command line arguments

   30.11.2004:
   corrected two bugs in the sanity checking of command
   line arguments.

   23.01.2005:
   I changed the location of the actual prefstruct definition
   to avoid double existence of it during the link process.

   27.01.2005:
   added global version number information to the splitted images

   18.02.2005:
   I increased the minor version number due to HISTORY output
   changes in wfip_lib.c

   24.02.2005:
   I increased the minor version number due to bug fixes
   in wfip_lib.c

   19.04.2005:
   - I introduced a matrix transformation for images allowing
     arbitrary flips and rotations by a multiple of 90 degrees.
     (This replaces the old flipping possibility in x and y)
   - I increased the version number to 2.0
   - I added USAGE information text.
   - New mode 'no header update': We can split images with
     no header updateing/remapping. In this case the old headers
     (main and extension) are preserved. In this case we do not
     check the integrity of given header keywords!

   26.04.2005:
   I increased the minor version number due to a bug fix
   in wfip_lib.c.

   05.07.2005:
   I increased the minor version number due to a major bug fix
   in wfip_lib.c.

   11.10.2005:
   I included the possibility to give the actual IMAGEID under that
   a certain extension should be saved; increase of middle version
   number.

   24.10.2005:
   Bug fix concerning the treatment of IMAGEID parameter ->
   increase of minor version number.

   23.11.2005:
   I introduced the OUTPUT_DIR command
   line argument. It allows to explicitely specify
   the output directory for splitted data.

   16.05.2006:
   I updated the error message on inconsistent parameters.

   05.08.2006:
   I included the possibility to provide header information for
   only ONE image if a single extension has to extracted
   (command line option '-s').

   28.09.2010:
   There is now the possibility to set header keywords together
   with the splitting process. This happens with successive
   '-HEADER keyword keyvalue keycomment' blocks as command line
   options.

   26.11.2010:
   I added the possibility to transfer header keywords from the
   original images to the splitted data. Keyword values are read
   from each extension separately.

   19.11.2012:
   When transfering header keywords to the splitted data, the header
   key in the new data can have a different name - useful to transfer
   ESO HIERARCH keyword to something shorter.

   10.07.2014:
   New command line option 'IMAGEID_KEY': It gives the string of a
   header keyword in extensions of raw data. It specifies the chip
   number of that extension (DECAM has such a header keyword
   'CCDNUM'). It can now be used as an alternative to specify the
   chip numbering of the output images.
 */

/*---------------------------------------------------------------------------
                                                                Includes
   ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global_vers.h"
#include "eclipse.h"
#include "wfip_lib.h"
#include "prefs_split.h"

static void printUsage(void);

prefstruct prefs;

/*---------------------------------------------------------------------------
                                                                        Main
   ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  int    a, narg, nim;
  int    i;
  char   name_i[FILENAMESZ];
  char   name_o[FILENAMESZ];
  int    xtnum;
  char **argkey, **argval, *str;
  char **headerkeys, **headervalues, **headercomments;
  char **transferkeys, **transferkeynames;
  int    nheaderkeys, ntransferkeys;
  int    headerupdate = 1;            /* should headers of original images
                                         be replaced ? */
  int singleimagemode = 0;               /* If we extract only one extension
                                            from an MEF file, do we have to
                                            provide header info. for all the
                                            extensions nevertheless (0=YES) */
  int imageidcheck[MAXCHIPS];

  xtnum = narg = nim = 0;

  QMALLOC(argkey, char *, argc);
  QMALLOC(argval, char *, argc);
  QMALLOC(headerkeys, char *, argc);
  QMALLOC(headervalues, char *, argc);
  QMALLOC(headercomments, char *, argc);
  QMALLOC(transferkeys, char *, argc);
  QMALLOC(transferkeynames, char *, argc);

  /* initialisations */
  strcpy(prefs.prefs_name, "default.split");
  prefs.nimageid = 0;
  nheaderkeys    = 0;
  ntransferkeys  = 0;

  /* read command line arguments */

  if (argc < 2) {
    printUsage();
  }

  for (a = 1; a < argc; a++) {
    if (argv[a][0] == '-') {
      if (strlen(argv[a]) < 3) {
        switch ((int)tolower((int)argv[a][1])) {
        /*-------- Config filename */
        case 'x':   if (a < (argc - 1)) {
            xtnum = atoi(argv[++a]);
        } else {
            printUsage();
        }
          break;
        case 'd':   dumpprefs();
          exit(0);
          break;
        case 'h':   printUsage();
          break;
        case 'u':   headerupdate = 0;
          break;
        case 's':   singleimagemode = 1;
          break;
        case 'c':   if (a < (argc - 1)) {
            strncpy(prefs.prefs_name, argv[++a], MAXCHAR - 1);
        } else {
            printUsage();
        }
          break;
        default:    printUsage();
          break;
        }
      } else {
        /*------ Config parameters or header keywords to set */
        if (strcmp(&(argv[a][1]), "HEADER") == 0) {
          if (a < (argc - 3)) {
            headerkeys[nheaderkeys]     = argv[++a];
            headervalues[nheaderkeys]   = argv[++a];
            headercomments[nheaderkeys] = argv[++a];
            nheaderkeys++;
          } else {
            printUsage();
          }
        } else if (strcmp(&(argv[a][1]), "HEADTRANSFER") == 0) {
          if (a < (argc - 2)) {
            transferkeys[ntransferkeys]         = argv[++a];
            transferkeynames[ntransferkeys]     = argv[++a];
            ntransferkeys++;
          } else {
            printUsage();
          }
        } else {
          argkey[narg]   = &argv[a][1];
          argval[narg++] = argv[++a];
        }
      }
    } else {
      /*---- The input image filename(s) */
      for (; (a < argc) && (*argv[a] != '-'); a++) {
        for (str = NULL; (str = strtok(str ? NULL : argv[a], notokstr));
             nim++) {
          if ((nim < MAXIMAGE)) {
            prefs.image_name[nim] = str;
          } else {
            error(EXIT_FAILURE, "*Error*: Too many input images: ", str);
          }
        }
      }
      prefs.nimage_name = nim;
      a--;
    }
  }

  readprefs(prefs.prefs_name, argkey, argval, narg);

  free(argkey);
  free(argval);

  /* Initialize eclipse environment */
  eclipse_init();
  set_verbose(1);   /* print comment messages */

  /* sanity check; we do not care if NO header update
     is performed */
  if (headerupdate != 0) {
    /* set default ordering of chips, i.e. the nth chip in the MEF
       file gets IMAGEID n if the user did not supply the ordering
       manually. */
    if (prefs.nimageid == 0) {
      prefs.nimageid = prefs.ncrpix1;

      /* if we extract only a single extension, the default IMAGEID is
         equivalent to the extension number */
      if (xtnum != 0) {
        prefs.imageid[0] = xtnum;
      } else {
        for (i = 1; i <= prefs.nimageid; i++) {
          prefs.imageid[i - 1] = i;
        }
      }
    } else {
      for (i = 0; i < MAXCHIPS; i++) {
        imageidcheck[i] = 0;
      }

      for (i = 0; i < prefs.nimageid; i++) {
        if (imageidcheck[prefs.imageid[i] - 1] != 0) {
          error(EXIT_FAILURE, "Double IMAGEID !!\n", "");
        }
        imageidcheck[prefs.imageid[i] - 1] = 1;
      }
    }

    if ((prefs.ncrpix1 != prefs.ncrpix2) ||
        (prefs.ncrpix1 != prefs.ncd11) ||
        (prefs.ncrpix1 != prefs.ncd22) ||
        (prefs.ncrpix1 != prefs.ncd12) ||
        (prefs.ncrpix1 != prefs.ncd21) ||
        (prefs.ncrpix1 != prefs.nm11) ||
        (prefs.ncrpix1 != prefs.nm12) ||
        (prefs.ncrpix1 != prefs.nm21) ||
        (prefs.ncrpix1 != prefs.nm22) ||
        (prefs.ncrpix1 != prefs.nimageid)) {
      error(EXIT_FAILURE, "Inconsistent number of elements in crpix1, crpix2, cd11, cd22, cd12, cd21, M11, M12, M21, M22\n", "");
    }
  }

  /* do the real stuff */
  for (a = 0; a < prefs.nimage_name; a++) {
    /* Get input and output names */
    strcpy(name_i, prefs.image_name[a]);

    if (prefs.noutputdir == 0) {
      strcpy(name_o, get_rootname(name_i));
    } else {
      sprintf(name_o, "%s/%s", prefs.outputdir[0],
              get_rootname(get_basename(name_i)));
    }

    /* In the following call we use implicitely that the variable
       prefs.imageidkey[0] ist set to "" (empty string) if it was NOT
       set within the options */
    wfi_mysplit(name_i, name_o, xtnum,
                headerupdate, singleimagemode,
                headerkeys, headervalues, headercomments, nheaderkeys,
                transferkeys, transferkeynames, ntransferkeys,
                prefs.crpix1,
                prefs.crpix2,
                prefs.cd11,
                prefs.cd22,
                prefs.cd12,
                prefs.cd21,
                prefs.m11,
                prefs.m22,
                prefs.m12,
                prefs.m21,
                prefs.imageid,
                prefs.imageidkey[0],
                prefs.ncrpix1,
                prefs.crval1,
                prefs.crval2,
                prefs.exptime,
                prefs.equinox,
                prefs.airmass,
                prefs.gabodsid,
                prefs.filter,
                prefs.object,
                __PIPEVERS__);
  }

  if (debug_active()) {
    xmemory_status();
  }

  return(0);
}

/*
 * This function only gives the usage for the program
 */

static void printUsage(void)
{
  fprintf(stdout, "PROGRAMMNAME:\n");
  fprintf(stdout, "        mefsplit - Mosaic camera image splitter (MEF files) \n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS:\n");
  fprintf(stdout, "        mefsplit image [options]\n");
  fprintf(stdout, "        -h       # print this and leave\n");
  fprintf(stdout, "        -d       # dump standard configuration file and leave\n");
  fprintf(stdout, "        -c       # name of the configuration file\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        -x       # extension to extract from MEF file\n");
  fprintf(stdout, "                 # (default is 0, i.e. extract all extensions)\n");
  fprintf(stdout, "        -s       # Provide header information ONLY for the extension\n");
  fprintf(stdout, "                 # to be splitted (in case '-x' is set)\n");
  fprintf(stdout, "        -u       # NO update of original image headers\n");
  fprintf(stdout, "        -HEADER  # block of 'key', 'keyvalue', 'keycomment'\n");
  fprintf(stdout, "                 # A header keyword 'key' with value 'keyvalue'\n");
  fprintf(stdout, "                 # and comment 'keycomment' is transfered to all\n");
  fprintf(stdout, "                 # output images\n");
  fprintf(stdout, "                 # This option can be given multiple times!\n");
  fprintf(stdout, "        -HEADTRANSFER\n");
  fprintf(stdout, "                 # a block of 'key' and 'newkey'.  The value of 'key'\n");
  fprintf(stdout, "                 # that is present in the extensions of the original\n");
  fprintf(stdout, "                 # images is transfered to the splitted data. The\n");
  fprintf(stdout, "                 # header name in the new images is 'newkey'.  This\n");
  fprintf(stdout, "                 # option can be given multiple times!\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        All options in the configuration file can be\n");
  fprintf(stdout, "        overriden on the command line via:\n");
  fprintf(stdout, "        -OPTIONNAME OPTIONVALUE\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION: \n");
  fprintf(stdout, "        The program splits a Multi Extension FITS File (MEF) into\n");
  fprintf(stdout, "        individual chips. By default, the output consists of all\n");
  fprintf(stdout, "        extensions from the MEF file. Output filenames are\n");
  fprintf(stdout, "        BASE_i.fits, where 'i' is the number of the extension\n");
  fprintf(stdout, "        (starting from 1) and the original filename is BASE.fits.  The\n");
  fprintf(stdout, "        chip number 'i' of the output images is, by default, theorder\n");
  fprintf(stdout, "        of extensions in the input MEF file. It can be be modified\n");
  fprintf(stdout, "        with the IMAGEID or the IMAGEID_KEY options.  IMAGE_ID lists\n");
  fprintf(stdout, "        the extension mnumbers directly. IMAGEID_KEY is a header\n");
  fprintf(stdout, "        keyword containing the chip numbers.\n");
  fprintf(stdout, "        With command line option '-x' only a specified extension is\n");
  fprintf(stdout, "        selected.\n");
  fprintf(stdout, "        Headers of the original images are replaced by new headers\n");
  fprintf(stdout, "        containing only keywords to uniquely determine astrometric and\n");
  fprintf(stdout, "        photometric properties: CRPIXi, CDij, CRVALi EXPTIME, EQUINOX,\n");
  fprintf(stdout, "        AIRMASS, GABODSID, FILTER, OBJECT. To this end the values for\n");
  fprintf(stdout, "        these quantities have to be provided on the command line.\n");
  fprintf(stdout, "        Call 'mefsplit -d' for an example of dummy values for WFI@ESO\n");
  fprintf(stdout, "        2.2m.  Additionally, for each extension a matrix Mij has to be\n");
  fprintf(stdout, "        given.  Chips are transformed according to the matrix before\n");
  fprintf(stdout, "        being written to disk. The matrix's elements have to be\n");
  fprintf(stdout, "        0,+1,-1 and det(M)=+1 or -1. This allows arbitraty flips and\n");
  fprintf(stdout, "        rotations by angles of 90 degrees. Where it is important,\n");
  fprintf(stdout, "        images are first rotated and then flipped.  During this\n");
  fprintf(stdout, "        process, new astrometric quantities (CRPIXi, CDij) are\n");
  fprintf(stdout, "        calculated out of the values given on the command line.  We\n");
  fprintf(stdout, "        strongly recommend to bring all chips to the SAME sky\n");
  fprintf(stdout, "        orientation during this process. Additional header keywords\n");
  fprintf(stdout, "        with fixed values can be set in the output images with blocks\n");
  fprintf(stdout, "        of '-HEADER' options. Header keywords present in the original\n");
  fprintf(stdout, "        image extensions can be transfered to splitted data with the\n");
  fprintf(stdout, "        '-HEADTRANSFER' option. With the option '-u' files can be\n");
  fprintf(stdout, "        splitted without header update. In this case, the old headers\n");
  fprintf(stdout, "        are completely preserved for each extension.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR:\n");
  fprintf(stdout, "        Thomas Erben (terben@astro.uni-bonn.de)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        The code is largely based on the eclipse library (see\n");
  fprintf(stdout, "        http://www.eso.org/eclipse) and on sources from Emmanuel\n");
  fprintf(stdout, "        Bertin for the handling of configuration options.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");
}
