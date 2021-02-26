
/*----------------------------------------------------------------------------

   File name    :   imhead.c
   Author       :   Nick Kaiser & Thomas Erben
   Created on   :   12.05.2007
   Description  :   print FITS header information

 ---------------------------------------------------------------------------*/

/*
	$Id: imhead.c,v 1.3 2018/03/28 12:06:14 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:14 $
	$Revision: 1.3 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
  12.05.2007:
  code transfered from imcat to THELI

  03.04.2013:
  xmemory is no longer used by default but only if the variable
  __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
  etc. calls are used for memory management.
 */

/*----------------------------------------------------------------------------
                                Includes and Defines
 ---------------------------------------------------------------------------*/

#define usage "\n\n\n\
NAME\n\
	imhead --- extract fits header\n\
\n\
SYNOPSIS\n\
	imhead [options...] \n\
		-b		# output full binary header\n\
		-D		# discard header\n\
		-d		# discard header and tail\n\
		-v name		# print value of header item 'name'\n\
		-t name		# print value of header item 'name'\n\
		-g pixtype NAXIS NAXIS1 ...	# generate fits header.\n\
\n\
DESCRIPTION\n\
	\"imhead\" prints fits image header info\n\
	reads/writes from/to stdin/stdout.\n\
	By default it formats the header values, but with\n\
	-b option it will output full header with padding\n\
	exactly as it appears - this is useful if you have some\n\
	data in raw bytes format and want to construct a legal\n\
	fits header: use 'ic' to generate a suitable image and\n\
	then pipe this into 'imhead -b' to make the header.\n\
	Use -D, -d options to extract raw image data.\n\
	With -v we print the value for the last header\n\
	item whose name matches 'name' exactly. Only the\n\
	first word of the value will be printed.\n\
	With -t option we print everything following the '= '.\n\
\n\
AUTHOR\n\
	Nick Kaiser:  kaiser@cita.utoronto.ca\n\
\n\n\n"

/* uncomment the following line to obtain a status on allocated/freed
   memory at the end of the program. Only available if also
   __XMEM_DEBUG__ is defined at compile time.  */

/* #define MEMORY_DEBUG */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "fits.h"
#include "error.h"
#include "args.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

/*----------------------------------------------------------------------------
                            Main code
 ---------------------------------------------------------------------------*/

int		main(int argc, char *argv[])	
{
    int		getrawdata, printall, firstword, formatted, printtail, dim;
    int		pixtype, ndim, *n;
    int		i, nlines, nbytes;
    fitscomment	*com;
    fitsheader	*fits;
    char	name[9], word[80], *flag, *argstring, *value;
    void	*f;

    /* defaults */
    getrawdata = 0;
    printall = 1;
    formatted = 1;
    printtail = 0;
    fits = NULL;
    firstword = 0;

    /* parse args */
    argsinit(argc, argv, usage);
    while ((flag = getflag()))
    {
        switch (flag[0])
        {
    	    case 'b':
    	    	formatted = 0;
    	    	break;
    	    case 'D':
    	    	printtail = 1;
    	    case 'd':
    	    	getrawdata = 1;
    	    	break;
    	    case 'v':
    	    	firstword = 1;
    	    case 't':
    	    	argstring = getargs();
    	    	sprintf(name, "%-8s", argstring);
    	    	printall = 0;
    	    	break;
    	    case 'g':
    	        pixtype = getargi();
    	    	ndim = getargi();
    	    	n = (int *) calloc(ndim, sizeof(int));
    	    	for (i = 0; i < ndim; i++)
                {
    	    	    n[i] = getargi();
		    if(n[i] < 0)
		    {
    		        fprintf(stderr, "imhead: negative NAXIS value given !!\n");
    		        exit(1);
		    }
    	    	}
    	    	fits = newfitsheader(ndim, n, pixtype);

		/* add 50 dummy header keywords for later use */
		for(i=0; i< 50; i++)
		{
		    sprintf(word, "DUMMY%d", i+1);
		    com =  newnumericcomment(word, (double)(0), "");
		    appendcomment(com, fits);
		}
    	    	writefitsheader(fits);
		delfitsheader(fits);
    	    	exit(0);
    	    	break;
    	    default:
    	    	error_exit(usage);
    	    	break;
    	}
    		
    }

    fits = readfitsheader(stdin);
    if (!getrawdata)
    {
    	if (!formatted)
        {
    	    writefitsheader(fits);
    	}
        else
        {
            /* prepend all the special header items to the linked list */
    	    if (fits->bscaling)
            {
    	        prependcomment(newnumericcomment("BSCALE", (double) fits->bscale, ""), fits);
    		prependcomment(newnumericcomment("BZERO",  (double) fits->bzero, ""), fits);
    	    }
    	    if (fits->hasextensions)
            {
    		prependcomment(newnumericcomment("NEXTEND",  (double) fits->nextensions, "number of extensions"), fits);
    	        prependcomment(newtextcomment("EXTEND", "T", "has extensions"), fits);
    	    }
    	    for (dim = fits->ndim - 1; dim >= 0; dim--)
            {
    	        sprintf(word, "NAXIS%d", dim + 1);
    		prependcomment(newnumericcomment(word, (double) fits->n[dim], ""), fits);
    	    }
    	    prependcomment(newnumericcomment("NAXIS", (double) fits->ndim, ""), fits);
    	    prependcomment(newnumericcomment("BITPIX", (double) fits->extpixtype, ""), fits);
    	    prependcomment(newtextcomment("SIMPLE", "T", "written by IMCAT"), fits);
    	    if (printall)
            {
    	        com = fits->basecomment;
    		while (com)
                {
    		    fprintf(stdout, "%-8.8s= %-70.70s\n", com->name, com->value);
    		    com = com->next;
    		}
    	    }
            else
            {
    		if (!(com = getcommentbyname(name, fits)))
                {
    		    fprintf(stderr, "imhead: no header item %s\n", name);
    		    exit(1);
    		}
    		if (firstword)
                {
    		    fprintf(stdout, "%s\n", value=gettextvalue(com));
		    free(value);
    		}
                else
                {
    		    fprintf(stdout, "%s\n", com->value);
    		}
	    }
    	}
    }
    else
    {
    	nbytes = fits->n[0] * pixsize(fits->extpixtype);
    	f = calloc(nbytes, sizeof(char));
    	nlines =  1;
    	for (i = 1; i < fits->ndim; i++)
        {
    	    nlines *= fits->n[i];
    	}
    	for (i = 0; i < nlines; i++)
        {
    	    fread(f, nbytes, sizeof(char), stdin);
    	    fwrite(f, nbytes, sizeof(char), stdout);
    	}
    	if (printtail)
        {
    	    writefitstail(fits);
    	}
	free(f);
    }

    /* clean up allocated memory */
    delfitsheader(fits);

#ifdef MEMORY_DEBUG
    xmemory_status() ;
#endif

    exit(0);
}

