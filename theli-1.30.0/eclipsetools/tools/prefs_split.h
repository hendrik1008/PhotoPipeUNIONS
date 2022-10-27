
#ifndef _PREFS_H_
#define _PREFS_H_

/* currently the code only can handle one image at a time.
   To be flexible for the future we introduce the following variable
*/
#define MAXIMAGE                1
#define MAXCHIPS                128
#ifndef MAXCHAR
#define MAXCHAR                 50*MAXCHIPS
#endif

/*
   05.09.03:
   changes to include the OBJECT keyword in the new headers

   13.10.03:
   included possibility to flip frames in x and/or y

   30.11.2004:
   I increased the number of chips from 40 to 80 (to be
   able to split MMT Megacam data having 72 chips).
   MAXCHAR is set to 40*MAXCHIPS (assuming that about
   40 char are necessary for describing one chip on the
   command line.

   23.01.2005:
   I changed the location of the actual prefstruct definition
   to avoid double existence of it during the link process.

   19.04.2005:
   I replaced in the prefstruct structure the flip? elements
   by Mij matrix coefficients.

   11.10.2005:
   new members imageid,nimageid in the prefstruct.

   23.11.2005:
   new member outputdir in the prefstruct.

   27.05.2010:
   I increased MAXCHIPS to 128. 80 is too low for future instruments
   (e.g. HyperSuprimeCam with 108)

   10.07.2014:
   The prefstruct gets the new variable imageidkey which can specify
   a header keyword specifying the chip number of an extension.

   17.07.2014:
   Bug fix: The variables outputdir and imageidkey within the prefs structure
   needed to be of type (char **). The error splipped through up to now
   because output_dir only consisted out of 1 string which was always
   set on the command line, i.e. the variable was nevber empty!
*/

#define QMALLOC(ptr, typ, nel) \
                {if (!(ptr = (typ *)malloc((size_t)(nel)*sizeof(typ)))) \
                  error(1, "Not enough memory for ", \
                        #ptr " (" #nel " elements) !");;}

static const char		notokstr[] = {" \t=,;\n\r\""};

/* prefs stuff */
typedef struct
{
  char		prefs_name[MAXCHAR];      /* prefs config gile filename */
  char		*(image_name[MAXIMAGE]);  /* image filenames */
  int           nimage_name;	          /* number of input images */
  double        crpix1[MAXCHIPS];
  int           ncrpix1;
  double        crpix2[MAXCHIPS];
  int           ncrpix2;
  double        cd11[MAXCHIPS];
  int           ncd11;
  double        cd22[MAXCHIPS];
  int           ncd22;
  double        cd21[MAXCHIPS];
  int           ncd21;
  double        cd12[MAXCHIPS];
  int           ncd12;
  int           m11[MAXCHIPS];
  int           nm11;
  int           m22[MAXCHIPS];
  int           nm22;
  int           m21[MAXCHIPS];
  int           nm21;
  int           m12[MAXCHIPS];
  int           nm12;
  int           imageid[MAXCHIPS];
  char          *(imageidkey[MAXCHAR]);
  int           nimageidkey;
  int           nimageid;
  double        crval1;
  double        crval2;
  double        exptime;
  double        equinox;
  double        airmass;
  int           gabodsid;
  char          filter[MAXCHAR];
  char          object[MAXCHAR];
  char          *(outputdir[MAXCHAR]);
  int           noutputdir;
}	prefstruct;

extern prefstruct	prefs;


/* function definitions */
extern int	cistrcmp(char *cs, char *ct, int mode);

extern void	dumpprefs(void);
extern void     readprefs(char *filename,
                          char **argkey,
                          char **argval,
                          int narg);

extern void    error(int, char *, char *);
extern void    warning(char *msg1, char *msg2);

#endif






