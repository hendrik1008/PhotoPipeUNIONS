
/*---------------------------------------------------------------------------

   File name 	:	mefcreate.c
   Author 		:	T. Erben
   Created on	:	31 May 2005
   Description	:	merges FITS files to a MEF file.

 ---------------------------------------------------------------------------*/

/*
	$Id: mefcreate.c,v 1.4 2018/03/28 12:06:23 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:23 $
	$Revision: 1.4 $
*/

/*---------------------------------------------------------------------------
   						       History Information
 ---------------------------------------------------------------------------*/

/*
   31.05.2005:
   project started

   07.03.2006:
   I corrected an error in the USAGE message.

   20.07.2015:
   The variable 'data_beg' indicating the start of data sections from
   MEF extensions was modified from type int to long int. DECam
   MEF-files can be larger than 2 GB and hence the range of the
   int-type is not large enough. Note that this foc only works on
   64-bit machines!
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global_vers.h"
#include "eclipse.h"
#include "prefs_create.h"

#define __PROGRAM__ "mefcreate: Multi-Extension-Fits (MEF) file creator"
#define __VERS__ "1.0"

#define __USAGE__ "\
USAGE:\n\
   mefcreate images [options]\n\n\
OPTIONS:\n\
-h      print this and leave\n\
-d      dump standard configuration file and leave\n\
-c      name of the configuration file\n\n\
all options in the configuration file can be\n\
overriden on the command line via:\n\
-OPTIONNAME OPTIONVALUE\n\n\
DESCRIPTION:\n\
The program creates a Multi-Extension_Fits (MEF) file\n\
from 2D FITS images which are not MEF files themselfes.\n\n\
EXAMPLES:\n\
  mefcreate file57.fits file58.fits -HEADER_TRANSFER OBJECT\n\
            -OUTPUT_IMAGE mef.fits\n\n\
Creates a MEF file called 'mef.fits' from the files file57.fits\n\
and file58.fits. From file57.fits the header keyword OBJECT\n\
is transfered to the new file.\n\n\
AUTHOR:\n\
  Thomas Erben (terben@astro.uni-bonn.de)\n\n\
  The code is largely based on the eclipse library\n\
  (see http://www.eso.org/eclipse) and on sources\n\
  from Emmanuel Bertin for the handling of configuration\n\
  options.\n"

static void usage(char *pname) ;
prefstruct	prefs;

/*---------------------------------------------------------------------------
									Main	
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int             a, narg = 0, nim =0 ;
    int             i ;
    char	    **argkey, **argval, *str;
    qfits_header*   h_main ; /* main header of the MEF file */
    qfits_header*   h_ext ;
    history         *hs ;    /* history on the merging process in the main header */
    FILE            *outputimage = NULL ;
    char            *fdata = NULL ;
    char            *headerline;
    long int        data_beg;
    int             data_size, data_pad ;
    size_t          fsize ;
    int             naxis, naxis1, naxis2, bitpix ;
    int             n_ext ; /* number of extensions in the files to be pasted
                               (must be 0) */
    char            dummystring[MAXCHAR];

    QMALLOC(argkey, char *, argc);
    QMALLOC(argval, char *, argc);

    strcpy(prefs.prefs_name, "default.create");

    /* read command line arguments */

    if(argc < 2)
    {
	usage(argv[0]);
    }

    for (a=1; a<argc; a++)
    {
      if (argv[a][0] == '-')
      {
        if (strlen(argv[a])<3)
          switch((int)tolower((int)argv[a][1]))
          {
  /*-------- Config filename */
            case 'd':	dumpprefs();
         		exit(0);
  			break;
            case 'h':	usage(argv[0]);
  			break;
            case 'c':	if(a<(argc-1))
	                {
                            strncpy(prefs.prefs_name, argv[++a], MAXCHAR-1);
			}
	                else
		        {
			    usage(argv[0]);
			}
  			break;
            default:	usage(argv[0]);
		        break;
          }
        else
        {
  /*------ Config parameters */
          argkey[narg] = &argv[a][1];
          argval[narg++] = argv[++a];
        }
      }
      else
      {
  /*---- The input image filename(s) */
        for(; (a<argc) && (*argv[a]!='-'); a++)
          for (str=NULL;(str=strtok(str?NULL:argv[a], notokstr)); nim++)
            if ((nim<MAXIMAGE))
              prefs.image_name[nim] = str;
            else
              error(EXIT_FAILURE, "*Error*: Too many input images: ", str);
        prefs.nimage_name = nim;
        a--;
      }
    }

    /* is there something to do ? */
    if(nim < 1)
    {
      e_error("No input images given !!");
      exit(1);
    }
    readprefs(prefs.prefs_name, argkey, argval, narg);

    free(argkey);
    free(argval);

    /* Initialize eclipse environment */
    eclipse_init() ;
    set_verbose(1); /* print comment messages */

    /* create main header for MEF file */
    h_main = qfits_header_default();
    /* is it important which BITPIX value the main header
       of an MEF has ?? */
    qfits_header_add(h_main, "BITPIX", "-32", "", NULL);
    qfits_header_add(h_main, "NAXIS", "0", "", NULL);
    qfits_header_add(h_main, "EXTEND", "T", "Extensions are present", NULL);

    /* go image by image */
    for(a=0; a < prefs.nimage_name; a++)
    {
      /* Sanity checks */
      if (is_fits_file(prefs.image_name[a])!=1)
      {
        e_error("cannot find file [%s]", prefs.image_name[a]);
	if(a>0)
	{
	  fclose(outputimage);
	  remove(prefs.output);
	}
	exit(1) ;
      }

      e_comment(0, "opening %s", prefs.image_name[a]);

      h_ext = qfits_header_read(prefs.image_name[a]);
      n_ext = qfits_query_n_ext(prefs.image_name[a]);
      naxis = qfits_header_getint(h_ext, "NAXIS", 0);

      if((n_ext >= 1) || (naxis != 2))
      {
	e_error("only 2D images without extensions can be pasted !");
	if(a>0)
	{
	  fclose(outputimage);
	  remove(prefs.output);
	}
	exit(1);
      }

      /* the first image needs special treatment as we may
         transfer header information from it to the main
         header */
      if(a==0)
      {
	for(i=0; i < prefs.nheader_keys; i++)
	{
          e_comment(0, "transfering header keyword %s from %s to MEF file",
                    prefs.header_keys[i], prefs.image_name[a]);

	  headerline = qfits_header_getline(h_ext, prefs.header_keys[i]);

	  if(headerline != NULL)
	  {
	    qfits_header_add(h_main, prefs.header_keys[i], NULL, NULL,
                             headerline);
	  }
	  else
	  {
	    e_warning("Header keyword %s not found in %s\n",
                      prefs.header_keys[i], prefs.image_name[a]);
	  }
	}
        /* add DUMMY keywords to the main header */
	e_comment(0, "adding %d DUMMY keywords to main header", prefs.header_dummy);
        for(i=0; i < prefs.header_dummy; i++)
        {
          sprintf(dummystring, "DUMMY%d", i);
          qfits_header_add(h_main, dummystring, "0", "", NULL);
        }

	/* add history information */
	hs = history_new();
	history_add(hs, "");
	history_add(hs, "mefcreate: %s", __PIPEVERS__);
	history_add(hs, "mefcreate: called at %s", get_datetime_iso8601());
	history_add(hs, "mefcreate: Created MEF file from images:");
	
        for(i=0; i < prefs.nimage_name; i++)
        {
	  history_add(hs, "Ext. %d: %s", i+1, get_basename(prefs.image_name[i]));
	}

	history_addfits(hs, h_main);

	/* write main header */
        e_comment(0, "writing main header of %s", prefs.output);
	if ((outputimage = fopen(prefs.output, "w"))==NULL)
        {
          e_error("cannot output to file [%s]", "extension.fits");
          exit(1);
        }
	qfits_header_dump(h_main, outputimage);
	
	/* free memory for main header that is not needed any more */
	qfits_header_destroy(h_main);
	history_del(hs);
      }

      /* modify header from single FITS file to an extension header */
      /* first some sanity deletions */
      /* The first two should not be present but anyway. The third one
         is no longer valid in the MEF file
      */
      qfits_header_del(h_ext, "PCOUNT");
      qfits_header_del(h_ext, "GCOUNT");
      qfits_header_del(h_ext, "CHECKSUM");
      qfits_header_add_after(h_ext, "SIMPLE", "XTENSION", "IMAGE", NULL, NULL);
      qfits_header_add_after(h_ext, "NAXIS2", "PCOUNT", "0", NULL, NULL);
      qfits_header_add_after(h_ext, "PCOUNT", "GCOUNT", "1", NULL, NULL);
      qfits_header_add_after(h_ext, "GCOUNT", "EXTNAME",
                             get_basename(prefs.image_name[a]), NULL, NULL);
      qfits_header_del(h_ext, "SIMPLE");

      /* and dump it to the file */
      e_comment(1, "writing extension header for %s", prefs.image_name[a]);
      qfits_header_dump(h_ext, outputimage);

      /* map input pixel data and dump it again */
      fdata = falloc(prefs.image_name[a], 0, &fsize);
      if (fdata==NULL)
      {
        e_error("mapping input file [%s]", prefs.image_name[a]);
	exit(1) ;
      }
      if (qfits_get_datinfo(prefs.image_name[a], 0, &data_beg, NULL)!=0)
      {
	e_error("getting offset in file %s", prefs.image_name[a]);
	exit(1) ;
      }
      naxis1 = qfits_header_getint(h_ext, "NAXIS1", 0);
      naxis2 = qfits_header_getint(h_ext, "NAXIS2", 0);
      bitpix = qfits_header_getint(h_ext, "BITPIX", 0);

      data_size = naxis1 * naxis2 * BYTESPERPIXEL(bitpix);
      if (data_size<1)
      {
        e_error("cannot determine data size from file %s",
                prefs.image_name[a]);
	exit(1);
      }

      data_pad = FITS_BLOCK_SIZE - (data_size%FITS_BLOCK_SIZE);
      data_size += data_pad;

      /* Dump pixels from one file to the other */
      e_comment(1, "copying pixel data from %s", prefs.image_name[a]);
      fwrite(fdata+data_beg, 1, data_size, outputimage);

      /* free memory */
      qfits_header_destroy(h_ext);
      free(fdata);
    }

    /* Close file pointer */
    fclose(outputimage);

    if (debug_active())
		xmemory_status() ;
    return(0) ;
}



/*
 * This function only gives the usage for the program
 */

static void usage(char *pname)
{
    printf("%s \n(Program Version: %s; %s)\n\n%s", __PROGRAM__,
           __VERS__, __PIPEVERS__, __USAGE__) ;
    exit(1) ;
}
