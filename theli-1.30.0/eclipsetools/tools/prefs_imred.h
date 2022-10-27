#ifndef _PREFS_H_
#define _PREFS_H_

#ifndef MAXCHAR
#define MAXCHAR                 256
#endif

/*
   22.10.03:
   included flatthreshhold to the parameter list

   23.01.2005:
   I changed the location of the actual prefstruct definition to avoid
   double existence of it during the link process.

   30.01.2005:
   I added maximages to the prefstruct.

   08.04.2005:
   the inname variable of the prefstruct will now dynamically be
   allocated in the main program.

   07.03.2007:
   - I introduced OUTPUT_BITPIX and COMBINE_BITPIX configuration keys
     that allow configuration of the precision of preprocessed and
     stacked images.
   - function readprefs: the function has a new argument giving the
     maximum strinmglength of command line arguments to the program
     'preprocess'. This allows a dynamic allocation of string length
     and avoids core dumps due to too long command line arguments
     (hardcoded to a maximum length of MAXCHAR characters before).

   09.05.2010:
   New variable 'fringescaleflag' in the prefstruct structure. It is
   for the new option to turn off fringe scaling (request from Mischa)

   28.03.2012:
   New variable 'flatscalefactor' in the prefstruct structure. It is
   used if we give the flatscaling factor directly on the command
   line.

   20.11.2012:
   New variable 'gainkey' in the prefstruct structure. It is used to
   specify the name of the header keyword containing the gain of the
   image.

   23.11.2012:
   New variable 'satlevkey' in the prefstruct structure. It is used to
   specify the name of the header keyword containing the saturation
   level of the image.

   20.06.2014:
   New variable 'badccdkey' in the prefstruct structure. It is used to
   specify the name of a BADCCD header keyword in the image.
*/

#define	QCALLOC(ptr, typ, nel) \
		{if (!(ptr = (typ *)calloc((size_t)(nel),sizeof(typ)))) \
		  error(EXIT_FAILURE, "Not enough memory for ", \
			#ptr " (" #nel " elements) !");;}

#define QMALLOC(ptr, typ, nel) \
                {if (!(ptr = (typ *)malloc((size_t)(nel)*sizeof(typ)))) \
                  error(1, "Not enough memory for ", \
                        #ptr " (" #nel " elements) !");;}

static const char		notokstr[] = {" \t=,;\n\r\""};

/* prefs stuff */
typedef struct
{
    char	  prefs_name[MAXCHAR];      /* prefs filename */
    char          **inname;                 /* the input filenames */
    int           ninname;                  /* the number of input images */
    int           maximages;                /* the number of images to
                                               load in one go */
    char          gainkey[MAXCHAR];
    char          satlevkey[MAXCHAR];
    char          badccdkey[MAXCHAR];
    int           overscanflag;             /* do overscan ? */
    int           overscanreg[2];
    int           noverscanreg;
    int           overscanreject[2];
    int           noverscanreject;
    int           statssec[4];
    int           nstatssec;
    int           biasflag;
    char          biasimage[MAXCHAR];
    int           trimflag;
    int           trimreg[4];
    int           ntrimreg;
    int           flatflag;
    char          flatimage[MAXCHAR];
    int           flatscaleflag;
    double        flatscalefactor;
    double        flatthreshhold;
    char          *(flatscaleimage[MAXCHAR]);
    int           nflatscaleimage;
    int           fringeflag;
    char          fringeimage[MAXCHAR];
    int           fringescaleflag;
    char          fringerefimage[MAXCHAR];
    int           outputpreprocflag;
    int           outputbitpix;
    char          outputpreprocsuffix[MAXCHAR];
    char          outputdirectory[MAXCHAR];
    int           combineflag;
    int           combinebitpix;
    int           combinerescaleflag;
    char          combineoutput[MAXCHAR];
    double        combineminthresh;
    double        combinemaxthresh;
    int           combinereject[2];
    int           ncombinereject;	
}	prefstruct;

extern prefstruct	prefs;


/* function definitions */
extern int	cistrcmp(char *cs, char *ct, int mode);

extern void	dumpprefs(void);
extern void     readprefs(char *filename,
                          char **argkey,
                          char **argval,
                          int narg,
                          size_t maxargstringlength);

extern void    error(int, char *, char *);
extern void    warning(char *msg1, char *msg2);

#endif






