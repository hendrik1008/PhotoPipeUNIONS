/*-------------------------------------------------------------------------*/
/**
  @file     decimaltodms.c
  @author   Nick Kaiser, Thomas Erben
  @date     2002
  @version  $Revision: 1.4 $
  @brief    converts an angle from decimal to sexagesimal (d:m:s) notation

  The programs main intention is to convert the declination from
  astronomical coordinates into sexagesimal notation.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: decimaltodms.c,v 1.4 2018/03/28 12:06:14 thomas Exp $
	$Author: thomas $
	$Date: 2018/03/28 12:06:14 $
	$Revision: 1.4 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
  14.10.2006:
  I extended the program to also accept an input file with
  coordinates.

  30.10.2006:
  We can give stdin as input file ('-' as filename)
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "radecio.h"
#include "error.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#ifndef MAXSTRINGLENGTH
#define MAXSTRINGLENGTH 256
#endif

/** usage information */
#define usage "\n\
NAME\n\
	decimaltodms --- convert an angle from decimal degrees to d:m:s format\n\
\n\
SYNOPSIS\n\
	decimaltodms angle or\n\
	decimaltodms -f inputfile\n\
\n\
DESCRIPTION\n\
	decimaltodms takes a decimal format angle (in degrees) as argument and\n\
	generates a string corresponding to that argument in d:m:s\n\
	format.\n\
        As an alternative an input file having one coordinate per line\n\
        can be given as input. '-' as filename means stdin.\n\
\n\
AUTHOR\n\
	Nick Kaiser	kaiser@hawaii.edu\n\
\n\n"

/*-----------------------------------------------------------------------------
                                Main
 -----------------------------------------------------------------------------*/

int main (int argc, char *argv[])
{
	char	hmsstring[64];
	FILE   *inputfile = stdin;
	char    tmpstring[MAXSTRINGLENGTH-1];

	if (argc < 2)
        {
	  error_exit(usage);
	}
	if (argv[1][0] == '-' && argv[1][1] == 'u')
        {
	  error_exit(usage);
	}

	/* only one coordinate given on the command line */
	if(argc == 2)
	{
	  if (!decimaltoxms(atof(argv[1]), hmsstring))
          {
	    error_exit(usage);
	  }
	  fprintf(stdout, "%s\n", hmsstring);
	}

	else /* a file with coordinates was given */
	{
	  if(strcmp(argv[1], "-f") != 0)
	  {
	    error_exit(usage);
	  }
	  else
	  {
	    if(strcmp(argv[2], "-") != 0)
	    {
	      if((inputfile = fopen(argv[2], "r")) == NULL)
	      {
	        error_exit("error opening input file!\n");
	      }
	    }
	    while(fgets((char *)tmpstring, MAXSTRINGLENGTH-1, inputfile))
	    {
	      if (!decimaltoxms(atof(tmpstring), hmsstring))
              {
	        error_exit("error while processing input file");
	      }
	      fprintf(stdout, "%s\n", hmsstring);
	    }
	    if(strcmp(argv[2], "-") != 0)
	    {
	      fclose(inputfile);
	    }
	  }
	}

	return 0;
}
