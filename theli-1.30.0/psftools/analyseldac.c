/* This file originates from an unknown, out-of-date, version of Nick Kaiser's
   imcat package. An early version, based on a simplistic binary catalogue
   format was provided to us by Henk Hoekstra. We rewrote the code substantially to
   integrate it with the LDAC catalogue format.
*/

#define	usage "\n\n\n\
SYNOPSIS\n\
   version with pixel interpolation.\n\
   last changes: 08.02.2010 (TE)\n\
\n\
		analyseldac	[option...]\n\
                        -i input catalog\n\
                        -o output catalog\n\
                      [ -f fitsfile       # fits file to analyse (the same as (h)findpeaks)\n\
		      [ -n nu	          # threshold nu (default -10.0) ]\n\
		      [ -a a1 a2          # annulus for sky: a1 < r <= a2, (16,32) ]\n\
		      [ -R r0	          # gaussian window scale (3.0) ]\n\
		      [ -x mul	          # gaussian window scale  = mul*obj->rg (1.0) ]\n\
		      [ -e n rc	          # use W(r) = (r^2 + rc^2)^n/2 window (-2, 2) ]\n\
		      [ -s	          # force recalculation of local sky params ]\n\
		      [ -z	          # switch off local sky determination ]\n\
		      [ -p	          # use object positions from catalog instead of reestimating\n\
                                          # the cetroid\n\
                      [ -c                # x, y to use as good coordinates if -p option is\n\
                                          # specified (default: Xpos, Ypos)\n\
		      [ -r alpha          # aperture = alpha * r_petrosian (3.0) ]\n\
		      [ -m x a	          # zap neighbouring images ]\n\
		      [ -S	          # force recalculation of image sigma, mode ]\n\
		      [ -Q	          # sky annulus matched to aperture ]\n\
		      [ -F deltam         # filter output to reject bad rh, mag ]\n\
\n\
DESCRIPTION\n\
		\"analyseldac\" analyses images around a catalogue of objects\n\
		created by (h)findpeaks. It determines a constant plus\n\
                gradient model for the local sky parameters using\n\
                NE, NW, SW, SE quadrants of an annulus, unless you tell\n\
                it not to. Only uses objects with nu, determined by\n\
                hfindpeaks, above threshold.  -f to do gauss ellipsoid fit.\n\
		By default, moments etc are determined with gaussian\n\
                window scale 3.0 pixels.\n\
		-e to specify a softened power law window.\n\
		-R option to change scale length.\n\
		-x option to override this and set window scale for moments\n\
		to be mul * obj.rg (rg as determined by hcat2cat)\n\
		Use -ve alpha to use aperture = (-alpha) * obj->rg for\n\
                photometry. Luminosities incorporate 1 / normfactor from\n\
                fits header. Use -m x a to zap disks radius r * r_x, where\n\
                x = \"g\" or \"n\" (for r_numax), around neighbours.\n\
		-Q with -ve alpha to make reference annlus run from r_ap to\n\
                2 * r_ap. With -F option, if total mag differs from hfindpeaks\n\
                mag estimate by > deltam we ignore rh, total mag in favor of\n\
                hfindpeaks values.\n\
\n\n\n"		

/* HISTORY:

modified do_object_stats, formerly in object_stuff.c
July 3, 1992:
Current version works as follows:
July 3
	* calculate the medians, non-MAGIC count and centroids
          for 4 surronding
	  sectors (NSEW) in an annulus a1 < r <= a2.
	* for each peak, accumulate 1 and f in unit width rings
		out to max radius GC_MAX (GC -> "growth curve").
	* find r_numax = integer radius of max sigma for interior light
	* set rmax = alpha * r_numax (or GC_MAX - 1, whichever is smaller)
	* calculate total luminosity, halflight radius, various moments
          and e1, e2
Mon Jul  6
	* added mode estimator for surrounding sectors
Wed Aug  5 23:38:03 EDT 1992
	* power law window function for ellipticity
Thu Aug  6 10:05:40 EDT 1992
	* new cat version - comments, hassky etc.
Fri Dec 10 09:37:13 EST 1993
	added estimators for Psm11, Psm22 and
                         Psh = (Psh11 + Psh22) / 2
Sat Dec 25 11:17:22 EST 1993
	changed to gaussian window for moments
Sun Sep  4 16:19:14 EDT 1994
	zapping neighbours added
		this works by creating new images fzap = f and nzap = 0:
			for each object
			do
				for each pixel in disk
				do
					set fzap = MAGIC
					increment nzap by 1
				done
			done
		we can then restoreobj() unmasked parts of a target object
                to fzap
		by copying disk pixels from f, but only if nzap = 1 and then
		eraseobj() to reset those pixels to MAGIC
		we then analyse fzap.
	converted to a pipe
Sat Nov 19 13:24:05 EST 1994
	converted to internal float image format

Tue Aug 20 1996 added correction for fractional pixel positions (HH)
Thu Nov 14 1996 added error calculation (HH)
Fri Nov 15 1996 added good estimate of half light radii (HH)

Fri Feb 20 1998 corrected calculations for the shear and smear
                polaricabilities
Mon May 17 1998 exchanged N1 and N2 in the loops for finding the object center.
                and added freeing memory of the dataregion matrix
Fri May 29 1998 fixed a bug in closing files: tempf should only
                be closed if zapneighbours was enabled (probably also the
                bug why the program did not run on st12.
Mon Dec 07 1998 the good, interpolated object centroid is now added to the
                object's structure.
Tue Dec 08 1998 added 0.5 to the good positions (different interpretation
                of the pixel position by interpol)
Tue Dec 15 1998 FIRST RELEASE of analyseldac. It uses LDAC catalogs as
                input and output. So far I have commented out the use of zapping
                and the Ffilter option. Also a analyse tabe that gives the
                parameters used should be created. I checked that this version
                gives the same results as the analstarok4 program.
Mon Dec 21 1998 set ffmode (the sky that is given if sky determination
                is turned off) to 0.0;
                corrected a bug in the creation of the dataregion array:
                there one row/column was read to much and so the array was
                asymmetric (2*rmax+1 -> 2*rmax)
Mon Jan 11 1999 corrected a bug when calculating dii and djj for the
                ellipticity components (they were calculated always from
                the middle of the pixel instead from the centroid position);
                deleted calculation of d[0] and d[1] in the moments calc.;
                corrected DD1 calculation in the moments (it was the wrong
                sign); exchanged q11 and q22 (so e1 is correct now !!!!);
                deleted the invr2 variable (do_objects_stats).
Wed Jan 13 1999 made nearly all floats in do_object_stats to doubles
                (to decrease roundoff errors);
                included calculation of a, b and theta (major, minor axis
                and position angle
Sat Jan 16 1999 The half light radius is no longer caculated if the flux
                is negative. In this case the object gets a cl of 102.;
                included the calculation of weighted a, b and theta
Wed Jan 20 1999 Deleted the part to fit a gaussian profile to the objects
Thu Jan 21 1999 changed back the distance calculations to x-rmax and y-rmax.
                This seems to be the right formula but I do not quite understand
                it.
Fri Jan 22 1999 added intermediate quantities (Xsh, Xsm, eh, em) to the
                catalog. But they do not coincide with the definitios
                in the Hoekstra/KSB papers.
                The position angle is now given in degrees
Sun Jan 24 1999 added ANALYSE table that gives information about the processing

Wed Feb 24 1999 changed A,B and Theta to float (problems with associate)
                set cl to short int (same reason)
Sun Feb 28 1999 I now copy the HFINDPEAKS, the FIELDS table and the
                desired keys from the OBJECTS table by hand. So it should
                be no problem to reprocess an already analysed catalog.
Tue Mar 03 1999 changed Xpos and Ypos to float numbers
Tue Mar 03 1999 changed back Xpos and Ypos to double numbers (problems
                with the anisotropy program)
Mon Mar 15 1999 initialised some quantities in do_objects_stats
                to zero that they are zero for objects with peoblems.
                If an object has problems in the position determination,
                deltai and deltaj are no longer calculated. It is now also
                checked whether deltai and deltaj are > 0 (sqrt calculation)
                New cl=104 if there was a problem calculating a or b.
Tue Mar 16 1999 included types.h to get the stuff on DEC running
Wed Mar 17 1999 changed the keyname 'l' to 'flux'. changed a '6' to
                '6.0' in pow command as this led to a crash under DEC.
Sat Mar 20 1999 introduced zapping possibility again.
Wed Apr 14 1999 included full tensor for Psm and Psh.
Tue Jun 06 1999 divided Xsm, Xsh by denom; deleted some superfluous
                "=0" assignments.
Tue Jul 06 1999 added 0.5 to the pixel positions to get them right
                with the FITS standard.
Tue Oct 27 1999 small changes (not influencing anything)

Thu Jan 20 2000 added calculation of S/N ratio for objects

Wed Jan 10 2001 removed all the ED_FREE's for catalog elements.
                (trouble on OSF); removed unused variables

Tue May 22 2001 replaced all doubles by floats. The accuracy is sufficient,
                the program needs less memory and the type is consistent
                with SExtractor.
Sat Jan 19 2002 function do_objects_stats:
                I ensure that rmax is ALWAYS at least one (not doing
                this led to crashes under Linux)
Thu Sep 30 2004 I introduced the possibility to use position estimates
                from the input catalog instead of reestimating the centroid
                in the do_object_stats routine.
Mon Jan 10 2005 I removed compiler warnings under gcc with optimisation
                enabled (-O3).
Sun Jan 29 2005 I now add HISTORY information to the output catalog.
Fri Apr 18 2008 I ensure that 'rc' (KSB Gaussian Window size) is at least '1'
                in do_objects_stats. Values of '0' can lead to a program crash!
Mon Feb 08 2010 removal of compiler warnings on possibly undefined variables
*/
	
/* meaning of the flag 'cl':

   0  : object is ok
   100: object near the border (not analysed)
   101: object has negative q11+q22 (no shape quan. calculated)
   102: object has negative total flux (no mag. calculated)
   103: object had problems in the position calculation (no rh, shape,
        magnitude calculated)
   104: the semi major and/or the semi minor axis a, b where 0 or less.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "fits.h"
#include "error.h"
#include "stats_stuff.h"
#include "object_stuff.h"
#include "arrays.h"
#include "analyseldac.h"
#include "magic.h"
#include "status.h"
#include "interpol.h"
#include "utils_macros.h"
#include "utils_globals.h"
#include "tabutil_globals.h"
#include "fitscat.h"
#include "tsize.h"
#include "nrutil.h"

#ifndef SIGN
#define SIGN(a) ((a)>0.0 ? (int)(1) : (int)(-1))
#endif

#define RMAX_MIN	6

#ifndef EPSILON
#define EPSILON 1.0e-06
#endif

short int    powerlawwindow;
short int    nosky = 0;
float  ff, ffi, ffj, ffmode=0.0;  /* parameters for fsky() model */
int Gusecatpos; /* use position estimate already in catalog ? (0=NO; 1=YES) */

/* quantities that will be added to the input LDAC
   catalog (OBJECTS table)
*/
float *Grnumax;
float *Gnumax;
LONG *Grmax;
float *Gl;
float *Gmag;
LONG *Gnbad;
float *Grh;
float *Ge1;
float *Ge2;
float *Ga;
float *Gb;
float *Gtheta;
float *Gdeltae1;
float *Gdeltae2;
float *Gxpos;
float *Gypos;
float *Gdeltaxpos;
float *Gdeltaypos;
float *GPsh11, *GPsh22, *GPsh12, *GPsh21;
float *GPsm11, *GPsm22, *GPsm12, *GPsm21;
short *Gcl;
float *Gq11, *Gq22, *Gq12;
float *GXsh11, *GXsh22, *GXsh12;
float *GXsm11, *GXsm22, *GXsm12;
float *Geh0, *Geh1;
float *Gem0, *Gem1;
float *Gsnratio;

#define G_RADIUS 0
#define N_RADIUS 1

#define	ZAP	0
#define RESTORE	1

#ifndef MAXSTRINGLENGTH
#define MAXSTRINGLENGTH 256
#endif

#ifndef RAD_GRAD
#define RAD_GRAD 57.295779513
#endif

int main(int argc, char *argv[])	
{
   char      *comv[MAX_COMMENTS];
   short     **nzap;
   int	     arg = 1, N1, N2, i, j;
   short int       a1, a2;
             /* The inner/outer radii for the skybackground determination */
   int	     comc;
   /* int       alreadyhassky; */
   int	     redosky;
   short int       scaledwindow, Ffilter;
   short int	     zapneighbours, zapradiustype=0, redostats;
   short int       margin, matchedannulus;
   float     **fzap, **f;
   float     nulimit;
   float     ne, alpha, rc, r0, azap;
   float     normfactor, mul, deltam;
   float goodxpos=0.0, goodypos=0.0; /* needed if Gusecatpos=1; otherwise these
                                        variables contain dummy values */
   char incatname[MAXSTRINGLENGTH]="";
   char outcatname[MAXSTRINGLENGTH]="";
   char fitsfile[MAXSTRINGLENGTH]="";
   char catxposname[MAXSTRINGLENGTH]="Xpos";
   char catyposname[MAXSTRINGLENGTH]="Ypos";
   fstatsrec srec;
   FILE	     *fitsf;
   skyquad   sky;

   /* quantities that are read from the input catalog's hfindpeaks table */
   float skymode;
   float skysigma;

   /* quantities that are read from the tables of the input catalog */
   LONG *ipos;
   LONG *jpos;
   float *rg;
   float *nu;
   float *catxpos=NULL; /* needed if Gusecatpos is set */
   float *catypos=NULL; /* needed if Gusecatpos is set */

   /* variables needed for dealing with the catalogs */
   catstruct *incat, *outcat;
   tabstruct *hfindpeakstab, *objectstab, *objectsoldtab;
   tabstruct *analysetab;
   keystruct *ikey, *jkey; /* i and j have the same meaning as the KSB's */
   keystruct *catxkey, *catykey;
   keystruct *rgkey, *nukey;
   keystruct *imagekey, *sigmakey, *modekey;
   int nobjects;

   /* defaults */
   a1 = 16;
   a2 = 32;
   nulimit = -10.0;
   ne = -2.0;
   redosky = 0;
   alpha = 3.0;
   rc = 2;
   powerlawwindow = 0;
   r0 = 3.0;
   mul = 1.0;
   scaledwindow = 0;
   zapneighbours = 0;
   redostats = 0;
   margin = 3;
   matchedannulus = 0;
   Ffilter = 0;
   Gusecatpos = 0; /* reestimate centroid position by default */

   while (arg < argc) {
      if (*argv[arg] == '-') {
   	 switch (*(argv[arg++]+1)) {
	    case 'i':
                    strncpy(incatname, argv[arg++], MAXSTRINGLENGTH-1);
                    break;
	    case 'o':
                    strncpy(outcatname, argv[arg++], MAXSTRINGLENGTH-1);
                    break;
	    case 'f':
                    strncpy(fitsfile, argv[arg++], MAXSTRINGLENGTH-1);
                    break;
   	    case 'e':
   		    if (EOF == sscanf(argv[arg++], "%f", &ne))
   			    error_exit(usage);
   		    if (EOF == sscanf(argv[arg++], "%f", &rc))
   			    error_exit(usage);
   		    powerlawwindow = 1;
   		    if (scaledwindow)
   		       error_exit("analyse: cannot use -x and -e options together\n");
   		    break;
   	    case 'n':
   		    if (EOF == sscanf(argv[arg++], "%f", &nulimit))
   			    error_exit(usage);
   		    break;
   	    case 'r':
   		    if (EOF == sscanf(argv[arg++], "%f", &alpha))
   			    error_exit(usage);
   		    break;
   	    case 'R':
   		    if (EOF == sscanf(argv[arg++], "%f", &r0))
   			    error_exit(usage);
   		    break;
   	    case 'a':
   		    if (EOF == sscanf(argv[arg++], "%hd", &a1))
   			    error_exit(usage);
   		    if (EOF == sscanf(argv[arg++], "%hd", &a2))
   			    error_exit(usage);
   		    redosky = 1;
   		    break;
   	    case 'z':
   		    nosky = 1;
   		    break;
   	    case 's':
   		    redosky = 1;
   		    break;
   	    case 'p':
   		    Gusecatpos = 1;
   		    break;
   	    case 'c':
   		    if (EOF == sscanf(argv[arg++], "%s", catxposname))
   			    error_exit(usage);
   		    if (EOF == sscanf(argv[arg++], "%s", catyposname))
   			    error_exit(usage);
   		    break;
   	    case 'S':
   		    redostats = 1;
   		    break;
   	    case 'Q':
   		    matchedannulus = 1;
   		    break;
   	    case 'F':
   		    Ffilter = 1;
   		    if (EOF == sscanf(argv[arg++], "%f", &deltam))
   			    error_exit(usage);
   		    break;
   	    case 'm':
   		    zapneighbours = 1;
   		    switch (argv[arg++][0]) {
   		       case 'g':
   			       zapradiustype = G_RADIUS;
   			       break;
   		       case 'n':
   			       zapradiustype = N_RADIUS;
   			       break;
   		       default:
   			       fprintf(stderr, "analyse: bad zap radius type\n");
   			       error_exit(usage);
   			       break;
   		    }
   		    if (EOF == sscanf(argv[arg++], "%f", &azap))
   		       error_exit(usage);
   		    break;
   	    case 'x':
   		    if (EOF == sscanf(argv[arg++], "%f", &mul))
   		       error_exit(usage);
   		    scaledwindow = 1;
   		    if (powerlawwindow)
   		       error_exit("analyse: cannot use -x and -e options together\n");
   		    break;
   	    default:
   		    error_exit(usage);
   		    break;
   	 }
      } else {
   	 error_exit(usage);
      }
   }
   if (alpha >= 0.0 && matchedannulus)
   	   error_exit("analyse: use negative alpha with -Q option\n");

   /* open the catalog and read parameters */
   if((incat=read_cat(incatname))==NULL)
   {
     syserr(FATAL, "main", "Error opening input catalog %s\n",
            incatname);
   }

   outcat=new_cat(1);
   inherit_cat(incat, outcat);

   /* copy only the FIELDS, HFINDPEAKS table and some selected
      keys from the OBJECTS table
   */
   if((copy_tab(incat, "FIELDS", 0, outcat, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying FIELDS table to the output catalog\n");
   }

   if((copy_tab(incat, "HFINDPEAKS" ,0, outcat, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying HFINDPEAKS table to output catalog\n");
   }

   objectstab = new_tab("OBJECTS");

   /*
    * Copy all required keys
    */
   if((objectsoldtab=name_to_tab(incat, "OBJECTS", 0))==NULL)
   {
     syserr(FATAL, "main", "Error reading OBJECTS table in input catalog\n");
   }

   if((copy_key(objectsoldtab,"FIELD_POS",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying FIELD_POS key to output catalog\n");
   }

   if((copy_key(objectsoldtab,"SeqNr",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying SeqNr key to output catalog\n");
   }

   if((copy_key(objectsoldtab,"x",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying x key to output catalog\n");
   }

   if((copy_key(objectsoldtab,"y",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying y key to output catalog\n");
   }

   if((copy_key(objectsoldtab,"xbad",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying xbad key to output catalog\n");
   }

   if((copy_key(objectsoldtab,"ybad",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying ybad key to output catalog\n");
   }

   if((copy_key(objectsoldtab,"rg",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying rg key to output catalog\n");
   }

   if((copy_key(objectsoldtab,"nu",objectstab, 0) != RETURN_OK))
   {
     syserr(FATAL, "main", "Error copying nu key to output catalog\n");
   }

   if(Gusecatpos == 1)
   {
     if((copy_key(objectsoldtab,catxposname,objectstab, 0) != RETURN_OK))
     {
       syserr(FATAL, "main", "Error copying %s key to output catalog\n", catxposname);
     }
     if((copy_key(objectsoldtab,catyposname,objectstab, 0) != RETURN_OK))
     {
       syserr(FATAL, "main", "Error copying %s key to output catalog\n", catyposname);
     }
   }

   /* read from the hfindpeaks table */
   if((hfindpeakstab=name_to_tab(outcat, "HFINDPEAKS", 0))==NULL)
   {
     syserr(FATAL, "main", "Error reading HFINDPEAKS table\n");
   }

   if(strcmp(fitsfile, "")==0)
   {
     if((imagekey=read_key(hfindpeakstab, "IMAGE"))==NULL)
     {
       syserr(FATAL, "main", "Error reading IMAGE key in HFINDPEAKS table\n");
     }

     strncpy(fitsfile, imagekey->ptr, MAXSTRINGLENGTH-1);
   }

   if((modekey=read_key(hfindpeakstab, "SKY_MODE"))==NULL)
   {
     syserr(FATAL, "main", "Error reading SKY_MODE key in HFINDPEAKS table\n");
   }
   skymode=*(float *)(modekey->ptr);

   if((sigmakey=read_key(hfindpeakstab, "SKY_SIGMA"))==NULL)
   {
     syserr(FATAL, "main", "Error reading SKY_SIGMA key in HFINDPEAKS table\n");
   }
   skysigma=*(float *)(sigmakey->ptr);

   /* read from the objects table */
   if((ikey=read_key(objectstab, "y"))==NULL)
   {
     syserr(FATAL, "main", "Error reading y key in OBJECTS table\n");
   }
   ipos=ikey->ptr;

   if((jkey=read_key(objectstab, "x"))==NULL)
   {
     syserr(FATAL, "main", "Error reading x key in OBJECTS table\n");
   }
   jpos=jkey->ptr;

   if((rgkey=read_key(objectstab, "rg"))==NULL)
   {
     syserr(FATAL, "main", "Error reading rg key in OBJECTS table\n");
   }
   rg=rgkey->ptr;

   if((nukey=read_key(objectstab, "nu"))==NULL)
   {
     syserr(FATAL, "main", "Error reading nu key in OBJECTS table\n");
   }
   nu=nukey->ptr;

   nobjects=nukey->nobj;

   if(Gusecatpos == 1)
   {
     if((catxkey=read_key(objectstab, catxposname))==NULL)
     {
       syserr(FATAL, "main", "Error reading %s key in OBJECTS table\n", catxposname);
     }
     switch (catxkey->ttype)
     {
       case T_FLOAT:
         ED_GETVEC(catxpos, "main", catxkey,
                  catxkey->name, float, float);
        break;
        case T_DOUBLE:
         ED_GETVEC(catxpos, "main", catxkey,
                  catxkey->name, float, double);
        break;
        default:
          syserr(FATAL, "main", "Unsupported key type for %s (only float and double)\n",
                 catxkey->name);
        break;
     }

     if((catykey=read_key(objectstab, catyposname))==NULL)
     {
       syserr(FATAL, "main", "Error reading %s key in OBJECTS table\n", catyposname);
     }
     switch (catykey->ttype)
     {
       case T_FLOAT:
         ED_GETVEC(catypos, "main", catykey,
                  catykey->name, float, float);
        break;
        case T_DOUBLE:
         ED_GETVEC(catypos, "main", catykey,
                  catykey->name, float, double);
        break;
        default:
          syserr(FATAL, "main", "Unsupported key type for %s (only float and double)\n",
                 catykey->name);
        break;
     }
   }

   if ((fitsf = fopen(fitsfile, "r"))==NULL)
   {
     syserr(FATAL, "main", "unable to open fits file %s for input\n", fitsfile);
   }

   set_fits_ipf(fitsf);
   fread_fits(&f, &N1, &N2, &comc, comv);

   /* get normfactor; this is superfluous in the moment but maybe
      I can need it for normalising fluxes (TE: 14.12.98)
    */
   normfactor = 1.0;

   if (redostats)
   {
      fdo_stats(f, N1, N2, margin, &(srec));
      skymode = srec.fmode;
      skysigma = srec.sigma;
   }
   fclose(fitsf);

   /* allocate memory for the objects */
   ED_MALLOC(Grnumax, "main", float, nobjects);
   ED_MALLOC(Gnumax, "main", float, nobjects);
   ED_MALLOC(Grmax, "main", LONG, nobjects);
   ED_MALLOC(Gl, "main", float, nobjects);
   ED_MALLOC(Gmag, "main", float, nobjects);
   ED_MALLOC(Gnbad, "main", LONG, nobjects);
   ED_MALLOC(Grh, "main", float, nobjects);
   ED_MALLOC(Ge1, "main", float, nobjects);
   ED_MALLOC(Ge2, "main", float, nobjects);
   ED_MALLOC(Ga, "main", float, nobjects);
   ED_MALLOC(Gb, "main", float, nobjects);
   ED_MALLOC(Gtheta, "main", float, nobjects);
   ED_MALLOC(Gdeltae1, "main", float, nobjects);
   ED_MALLOC(Gdeltae2, "main", float, nobjects);
   if(Gusecatpos == 0)
   {
     ED_MALLOC(Gxpos, "main", float, nobjects);
     ED_MALLOC(Gypos, "main", float, nobjects);
   }
   ED_MALLOC(Gdeltaxpos, "main", float, nobjects);
   ED_MALLOC(Gdeltaypos, "main", float, nobjects);
   ED_MALLOC(GPsh11, "main", float, nobjects);
   ED_MALLOC(GPsh22, "main", float, nobjects);
   ED_MALLOC(GPsh12, "main", float, nobjects);
   ED_MALLOC(GPsh21, "main", float, nobjects);
   ED_MALLOC(GPsm11, "main", float, nobjects);
   ED_MALLOC(GPsm22, "main", float, nobjects);
   ED_MALLOC(GPsm12, "main", float, nobjects);
   ED_MALLOC(GPsm21, "main", float, nobjects);
   ED_MALLOC(Gcl, "main", short, nobjects);
   ED_MALLOC(Gq11, "main", float, nobjects);
   ED_MALLOC(Gq22, "main", float, nobjects);
   ED_MALLOC(Gq12, "main", float, nobjects);
   ED_MALLOC(GXsh11, "main", float, nobjects);
   ED_MALLOC(GXsh22, "main", float, nobjects);
   ED_MALLOC(GXsh12, "main", float, nobjects);
   ED_MALLOC(GXsm11, "main", float, nobjects);
   ED_MALLOC(GXsm22, "main", float, nobjects);
   ED_MALLOC(GXsm12, "main", float, nobjects);
   ED_MALLOC(Geh0, "main", float, nobjects);
   ED_MALLOC(Geh1, "main", float, nobjects);
   ED_MALLOC(Gem0, "main", float, nobjects);
   ED_MALLOC(Gem1, "main", float, nobjects);
   ED_MALLOC(Gsnratio, "main", float, nobjects);

   /* to zap neighbours we need to do a first pass to make zapped images */
   if (zapneighbours)
   {
      allocFloatArray(&fzap, N1, N2);
      for (i = 0; i < N2; i++)
      {
         for (j = 0; j < N2; j++)
	 {
   	    fzap[i][j] = f[i][j];
	 }
      }

      allocShortArray(&nzap, N1, N2);

      for(i=0; i<nobjects; i++)
      {
   	 zap(ZAP, jpos[i], ipos[i], rg[i], zapradiustype, azap, f, fzap, nzap, N1, N2);
      }
   }
   else
   {
     fzap = f;
   }

   /* now loop through all the objects and analyse them */
   for(i=0; i<nobjects; i++)
   {
      if (nu[i] > nulimit)
      {
	 if (zapneighbours)
	 {
	   zap(RESTORE, jpos[i], ipos[i], rg[i], zapradiustype, azap, f,
   		fzap, nzap, N1, N2);
	 }
	   /* if (!alreadyhassky || redosky)
           { */
   	    if (matchedannulus) {
   	       a1 = ceil(-alpha * arg);
   	       a2 = 2 * a1;
   	    }
   	    dosky(fzap, N1, N2, ipos[i], jpos[i], a1, a2,
   		  &sky, skymode, skysigma);

	    /* } else {
   	    sky = obj.sky;
   	    } */
   	 setskyparameters(&sky);

	 if(Gusecatpos == 1)
	 {
	   goodxpos = catxpos[i];
	   goodypos = catypos[i];
	 }
   	 if (powerlawwindow)
	 {
   	    do_object_stats(i, ipos[i], jpos[i], rg[i],
                            fzap, N1, N2,
   			    fsky, skysigma, ne, rc, alpha,
                            goodxpos, goodypos);
	 }
   	 else
	 {
   	    if (scaledwindow) {
   	       do_object_stats(i, ipos[i], jpos[i], rg[i],
                               fzap, N1, N2, fsky,
   			       skysigma, 0.0, mul * rg[i], alpha,
                               goodxpos, goodypos);
	     }
   	    else
	
   	       do_object_stats(i, ipos[i], jpos[i], rg[i],
                               fzap, N1, N2, fsky,
   			       skysigma, 0.0, r0, alpha,
                               goodxpos, goodypos);
         }	
   	 if (zapneighbours)
	 {
   	   zap(ZAP, jpos[i], ipos[i], rg[i], zapradiustype, azap, f,
   		fzap, nzap, N1, N2);
	 }
   	 /* now correct luminosity */
   	 Gl[i] /= normfactor;
   	 if (Ffilter)
         {
	   /* if (obj.l > obj.lg * exp(0.92 * deltam) ||
                  obj.l < obj.lg * exp(-0.92 * deltam))
              {
   	       obj.l = obj.lg;
   	       obj.rh = RFACTOR * obj.rg;
   	      } */
   	 }
      }
   }

   add_key_to_tab(objectstab, "rnumax", nobjects, Grnumax,
                         T_FLOAT, 1, "pixel", "Petrosian radius (analyse)");
   /*ED_FREE(Grnumax);*/
   add_key_to_tab(objectstab, "numax", nobjects, Gnumax,
                         T_FLOAT, 1, "", "Significance at Petrosian radius (analyse)");
   /*ED_FREE(Gnumax);*/
   add_key_to_tab(objectstab, "rmax", nobjects, Grmax,
                         T_LONG, 1, "", "radius used for analysis (analyse)");
   /*ED_FREE(Grmax);*/
   add_key_to_tab(objectstab, "flux", nobjects, Gl,
                         T_FLOAT, 1, "pixel", "total flux within rmax (analyse)");
   /*ED_FREE(Gl);*/
   add_key_to_tab(objectstab, "mag", nobjects, Gmag,
                         T_FLOAT, 1, "", "magnitude calculated with l (analyse)");
   /*ED_FREE(Gmag);*/
   add_key_to_tab(objectstab, "nbad", nobjects, Gnbad,
                         T_LONG, 1, "", "number of bad pixels within rmax (analyse)");
   /*ED_FREE(Gnbad);*/
   add_key_to_tab(objectstab, "rh", nobjects, Grh,
                         T_FLOAT, 1, "pixel", "half light radius (analyse)");
   /*ED_FREE(Grh);*/
   add_key_to_tab(objectstab, "e1", nobjects, Ge1,
                         T_FLOAT, 1, "", "first ellipticity component (analyse)");
   /*ED_FREE(Ge1);*/
   add_key_to_tab(objectstab, "deltae1", nobjects, Gdeltae1,
                         T_FLOAT, 1, "", "error for first ellipticity component (analyse)");
   /*ED_FREE(Gdeltae1);*/
   add_key_to_tab(objectstab, "e2", nobjects, Ge2,
                         T_FLOAT, 1, "", "second ellipticity component (analyse)");
   /*ED_FREE(Ge2);*/
   add_key_to_tab(objectstab, "deltae2", nobjects, Gdeltae2,
                         T_FLOAT, 1, "", "error for second ellipticity component (analyse)");
   /*ED_FREE(Gdeltae2);*/
   add_key_to_tab(objectstab, "A", nobjects, Ga,
                         T_FLOAT, 1, "", "semi major axis (analyse)");
   /*ED_FREE(Ga);*/
   add_key_to_tab(objectstab, "B", nobjects, Gb,
                         T_FLOAT, 1, "", "semi minor axis (analyse)");
   /*ED_FREE(Gb);*/
   add_key_to_tab(objectstab, "Theta", nobjects, Gtheta,
                         T_FLOAT, 1, "degree", "position angle (analyse)");
   /*ED_FREE(Gtheta);*/
   if(Gusecatpos == 0)
   {
     add_key_to_tab(objectstab, "Xpos", nobjects, Gxpos,
                           T_FLOAT, 1, "pixel", "good centroid position x (analyse)");
     /*ED_FREE(Gxpos);*/
     add_key_to_tab(objectstab, "deltaXpos", nobjects, Gdeltaxpos,
                           T_FLOAT, 1, "pixel", "error for x centroid position (analyse)");
     /*ED_FREE(Gdeltaxpos);*/
     add_key_to_tab(objectstab, "Ypos", nobjects, Gypos,
                           T_FLOAT, 1, "pixel", "good centroid position y (analyse)");
     /*ED_FREE(Gypos);*/
     add_key_to_tab(objectstab, "deltaYpos", nobjects, Gdeltaypos,
                           T_FLOAT, 1, "pixel", "error for y centroid position (analyse)");
     /*ED_FREE(Gdeltaypos);*/
   }
   add_key_to_tab(objectstab, "Psh11", nobjects, GPsh11,
                         T_FLOAT, 1, "", "11 Shear polarication component (analyse)");
   /*ED_FREE(GPsh11);*/
   add_key_to_tab(objectstab, "Psh22", nobjects, GPsh22,
                         T_FLOAT, 1, "", "22 Shear polarication component (analyse)");
   /*ED_FREE(GPsh22);*/
   add_key_to_tab(objectstab, "Psh12", nobjects, GPsh12,
                         T_FLOAT, 1, "", "12 Shear polarication component (analyse)");
   /*ED_FREE(GPsh12);*/
   add_key_to_tab(objectstab, "Psh21", nobjects, GPsh21,
                         T_FLOAT, 1, "", "21 Shear polarication component (analyse)");
   /*ED_FREE(GPsh21);*/
   add_key_to_tab(objectstab, "Psm11", nobjects, GPsm11,
                         T_FLOAT, 1, "", "11 Smear polarication component (analyse)");
   /*ED_FREE(GPsm11);*/
   add_key_to_tab(objectstab, "Psm22", nobjects, GPsm22,
                         T_FLOAT, 1, "", "22 Smear polarication component (analyse)");
   /*ED_FREE(GPsm22);*/
   add_key_to_tab(objectstab, "Psm12", nobjects, GPsm12,
                         T_FLOAT, 1, "", "12 Smear polarication component (analyse)");
   /*ED_FREE(GPsm12);*/
   add_key_to_tab(objectstab, "Psm21", nobjects, GPsm21,
                         T_FLOAT, 1, "", "21 Smear polarication component (analyse)");
   /*ED_FREE(GPsm21);*/
   add_key_to_tab(objectstab, "cl", nobjects, Gcl,
                         T_SHORT, 1, "", "classification of object (analyse)");
   /*ED_FREE(Gcl);*/
   add_key_to_tab(objectstab, "q11", nobjects, Gq11,
                         T_FLOAT, 1, "", "11 second moment (analyse)");
   /*ED_FREE(Gq11);*/
   add_key_to_tab(objectstab, "q22", nobjects, Gq22,
                         T_FLOAT, 1, "", "22 second moment (analyse)");
   /*ED_FREE(Gq22);*/
   add_key_to_tab(objectstab, "q12", nobjects, Gq12,
                         T_FLOAT, 1, "", "12 second moment (analyse)");
   /*ED_FREE(Gq12);*/
   add_key_to_tab(objectstab, "Xsh11", nobjects, GXsh11,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(GXsh11);*/
   add_key_to_tab(objectstab, "Xsh22", nobjects, GXsh22,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(GXsh22);*/
   add_key_to_tab(objectstab, "Xsh12", nobjects, GXsh12,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(GXsh12);*/
   add_key_to_tab(objectstab, "Xsm11", nobjects, GXsm11,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(GXsm11);*/
   add_key_to_tab(objectstab, "Xsm22", nobjects, GXsm22,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(GXsm22);*/
   add_key_to_tab(objectstab, "Xsm12", nobjects, GXsm12,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(GXsm12);*/
   add_key_to_tab(objectstab, "eh0", nobjects, Geh0,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(Geh0);*/
   add_key_to_tab(objectstab, "eh1", nobjects, Geh1,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(Geh1);*/
   add_key_to_tab(objectstab, "em0", nobjects, Gem0,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(Gem0);*/
   add_key_to_tab(objectstab, "em1", nobjects, Gem1,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(Gem1);*/
   add_key_to_tab(objectstab, "snratio", nobjects, Gsnratio,
                         T_FLOAT, 1, "", "(analyse)");
   /*ED_FREE(Gsnratio);*/

   /* add the analyse table */
   analysetab=new_tab("ANALYSE");

   add_key_to_tab(analysetab, "IMAGE", 1, fitsfile, T_STRING, 128, "",
			 "the image that was processed");

   add_key_to_tab(analysetab, "NULIMIT", 1, &(nulimit), T_FLOAT, 1, "",
			 "limit in nu for analysing objects");

   add_key_to_tab(analysetab, "ALPHA", 1, &(alpha), T_FLOAT, 1, "",
			 "scaling radius");

   add_key_to_tab(analysetab, "NOSKY", 1, &(nosky), T_SHORT, 1, "",
			 "was a local sky for objects determined");

   add_key_to_tab(analysetab, "REDOSTATS", 1, &(redostats), T_SHORT, 1, "",
			 "where SKY_MODE and SKY_SIGMA recalculated");

   if(powerlawwindow != 0)
   {
     add_key_to_tab(analysetab, "POWERLAWWINDOW", 1, &(powerlawwindow), T_SHORT, 1, "",
	  		 "a power law window was used");

     add_key_to_tab(analysetab, "ne", 1, &(ne), T_FLOAT, 1, "",
	  		 "");

     add_key_to_tab(analysetab, "rc", 1, &(rc), T_FLOAT, 1, "",
	  		 "");
   }

   if(scaledwindow != 0)
   {
     add_key_to_tab(analysetab, "SCALEDWINDOW", 1, &(scaledwindow), T_FLOAT, 1, "",
	  		 "a scaled window was used");

     add_key_to_tab(analysetab, "mul", 1, &(mul), T_FLOAT, 1, "",
	  		 "scaling multiplication factor");

   }
   else
   {
     add_key_to_tab(analysetab, "r0", 1, &(r0), T_FLOAT, 1, "",
	  		 "scale used in analysis");
   }

   if(matchedannulus == 0)
   {
     add_key_to_tab(analysetab, "a1", 1, &(a1), T_SHORT, 1, "",
	  		 "small radius for sky determination");

     add_key_to_tab(analysetab, "a2", 1, &(a2), T_SHORT, 1, "",
	  		 "large radius for sky determination");

   }

   if (add_tab(objectstab,outcat,0) == RETURN_ERROR)
   {
      syserr(FATAL, "main", "Error adding OBJECTS table to %s\n",
   	outcat->filename);
   }

   if (add_tab(analysetab,outcat,0) == RETURN_ERROR)
   {
      syserr(FATAL, "main", "Error adding ANALYSE table to %s\n",
   	outcat->filename);
   }

   /* add some HISTORY information to the output catalg */
   historyto_cat(outcat, "analyseldac", argc, argv);

   save_cat(outcat, outcatname);
   free_cat(incat, 1);
   /*free_cat(outcat, 1);*/
   return 0;
}

/* this function calculates the skybackground for the object centered
   at ip, jp. It calculates the mode of the sky in four sections
   around each objects (These sections consist of four circular sections
   that lie north, south, east and west to the object)
*/
void	dosky(	float **f, int N1, int N2, LONG ip, LONG jp, int a1, int a2,
	skyquad *sky, float fmode, float sigma)
/**
 ** calculates occupation and mode for NSEW sectors;
 **/
{
   /* variables that are arguments to the function:
      f: The array containing the whole FITS image
      N1, N2: The dimensions of the FITS image
      ip, jp: the center of the object whose background has to be
              calculated.
      a1, a2: The radii that determine the size of the circular
              aperture in that the skybackground is determined
      fmode: The mode of all pixels of the FITS image (used when a region
             is empty (on the border of the image)
      sigma: The sigma of all pixels in the fits image (not used)
   */
   int          i, j, m, ii, s;
   int          sortarraysize;
   static int   lasta2 = 0;
   static float *fs[4] = {NULL, NULL, NULL, NULL};
   float        rdefault;
                     /* the radius of the center of mass if all sky
                        values would be equal (used for regions on
                        the border)
		     */
   float        mode;
   float        median, lquart, uquart, quadsigma;
                     /* the statistical quantities for the section
                        just under consideration
                     */

   sortarraysize = (a2 + 1) * (a2 + 1);
   if (a2 != lasta2) {	     /* need to allocate new sort arrays */
      lasta2 = a2;
      for (s = 0; s < 4; s++)	{
   	 if (fs[s] != NULL)     /* free the old ones if they exist */
   	    free(fs[s]);
   	 fs[s] = (float *) calloc(sortarraysize, sizeof(float));
   	 if (!fs[s])
   	    error_exit("dosky: memory allocation failed");
      }
   }


   rdefault = 8 * (a2 * a2 * a2 - a1 * a1 * a1) / (3 * sqrt(2.0) * PI *
   	   (a2 * a2 - a1 * a1));

   for (s = 0; s < 4; s++) {	
      sky->i[s] = sky->j[s] = sky->n[s] = 0;   /* zero cumulants */
      for (m = 0; m < sortarraysize; m++) 	    /* sero sort arrays */
   	 fs[s][m] = 0;
   }

   for (i = (int)(ip - a2); i <= (int)(ip + a2); i++) {
      if (i < 0 || i >= N2)
   	 continue;

      for (j = (int)(jp - a2); j <= (int)(jp + a2); j++) {
   	 if (j < 0 || j >= N1)
   	    continue;
   	 ii = (int)(i - ip) * (int)(i - ip) + (int)(j - jp) * (int)(j - jp);
   	 if (ii > a2 * a2 || ii <= a1 * a1)
   	    continue;
   	 if (f[i][j] != MAGIC) {
   	    if ((int)(j - jp) > 0 && (int)(i - ip) >= -(int)(j - jp)
   		&& (int)(i - ip) < (int)(j - jp))
   	       s = NORTH;
   	    if ((int)(j - jp) < 0 && (int)(i - ip) > (int)(j - jp)
   		&& (int)(i - ip) <= -(int)(j - jp)) 			
   	       s = SOUTH;
   	    if ((int)(i - ip) > 0 && (int)(j - jp) > -(int)(i - ip)
   		&& (int)(j - jp) <= (int)(i - ip))
   	       s = EAST;
   	    if ((int)(i - ip) < 0 && (int)(j - jp) >= (int)(i - ip)
   		&& (int)(j - jp) < -(int)(i - ip))
   	       s = WEST;
   	    if (sky->n[s] >= sortarraysize)
	    {
   	       error_exit("dosky: this cannot happen");
	    }
   	    fs[s][sky->n[s]] = f[i][j];
   	    sky->n[s]++;
   	    sky->i[s] += i;
   	    sky->j[s] += j;
   	 }	
      }
   }

   for (s = 0; s < 4; s++) {     /* normalise centroids */
      if (sky->n[s]) {					
   	 sky->i[s] /= sky->n[s];
   	 sky->j[s] /= sky->n[s];
   	 liststats(fs[s], sky->n[s], &mode,
   		   &median, &lquart, &uquart, &quadsigma);
   	 sky->f[s] = mode;
      } else {	             /* default behaviour for empty sectors */
   	 sky->f[s] = fmode;
   	 switch (s) {
   	    case NORTH:
   		      sky->i[s] = 0;
   		      sky->j[s] = rdefault;
   		      break;
   	    case SOUTH:
   		      sky->i[s] = 0;
   		      sky->j[s] = - rdefault;
   		      break;
   	    case EAST:
   		      sky->i[s] = rdefault;
   		      sky->j[s] = 0;
   		      break;
   	    case WEST:
   		      sky->i[s] = - rdefault;
   		      sky->j[s] = 0;
   		      break;
   	    default:
   		      error_exit("this cannot happen\n");
   		      break;
   	 }
      }
   }
}

/*
 * ne is the power law index for the ellipticity window function
 * aperture is alpha * r_numax
 */

/* all object analysis is done with the observed light
   distribution; possible corrections (see KSB paper from
   Peter are not taken into account)
*/
void	do_object_stats(int anumber, LONG aipos, LONG ajpos, float arg,
                        float **f, int N1, int N2,
                        float(*fsky)(int i, int j), float sigma,
			float ne, float rc, float alpha, float goodxpos,
                        float goodypos)
{
   /* variables that come as arguments:
      aipos, ajpos: The integer positions of the obejct to be analysed
      arg: The rg of the object to be analysed
       f: The array of the whole FITS image
      N1, N2: The dimensions of the FITS image
      fsky: The function that gives the calure for the sky background
            for a distance i,j from the center of the object
      sigma: The sigma of the sky background
      rc: The sigma value that is used in the gaussian weighting
          functions
      alpha: The multiple (if it is negative) of the size 'rg' to
             that quantities are integrated.
      goodxpos, goodypos: good central positions for the objects;
                          They are used if Gusecatpos==1, i.e. no
                          recontroiding is done in this routine.
   */
   int	        r, i, j, di, dj, teller;
   int          rmax;
                     /* the integration boundary for the shear
                        quantities */
   static float	sumf[GC_MAX];
                     /* the total light of the object (within
                        a radius of GC_MAX */
   static float sum1[GC_MAX];
                     /* the total number of good pixels in the object
                        (within a radius of GC_MAX) */
   static LONG summagic[GC_MAX];
                     /* the number of bad pixels of the object
                        (within a radius of GC_MAX)
                        (this is ONLY taken into account when
                        computing the light of an object, not
                        when doing the shear quantities) */
   float        q11, q22, q12;
                     /* The weighted second brightness moments */
   float        d[2], gew = 0.0, denom, snnormalication;
   float        A[2][2] = {{0, 0}, {0, 0}}, S[2][2] = {{0, 0}, {0, 0}};
                     /* needed for calculating the error in the
                        position */
   float        dq[2][2] = {{0, 0}, {0, 0}}, de[2] = {0, 0}, dqq[2] = {0, 0};
   float        fud;
                     /* needed for error calculation of position */
   float        mag=0.0;
                     /* the magnitude of the object */
   float        dx, dy;
   float        W, Wp, Wpp;
                     /* the weight function and the first two
                        derivatives */
   float        fc;
                     /* The pixel value for the objected corrected for the sky
                        background */
   float        DD, DD1, DD2;
                     /* x^2+y^2; x^2-y^2 and 2.*x*y respectively */
   float        Xsm11, Xsm22, Xsm12, Xsh11, Xsh22, Xsh12, em[2], eh[2];	
                     /* smear polarizability bits; according to KSB 95
                        (eh=e^{sh}; em=e^{sm} */
   float        imax, jmax;
                     /* the final centroid object position */
   float        imaxold, jmaxold, dii, djj;
   float        deltai = 0.0, deltaj = 0.0;
                     /* the errors in position */
   float        fr, frr, dfr = 0.0, rr = 0.0;
                     /* needed for calculating the half light radius */
   float        df;
                     /* the pixel value interpolated to a certain x, y */
   float        **dataregion;
                     /* the region of the FITS image that contains
                        the object (needed for interpolations) */
   float        x,y;
                     /* the currently considered x,y during half light radius
                        calculation */

   /* quantities to be calculated: */
   float numax = 0.0;
   float rnumax = 0.0;
   float lum = 0.0;
   float nbad = 0.0;
   float rh = 0.0;
   float e1 = 0.0, e2 = 0.0;
   float a = 0.0, b = 0.0, phi = 0.0;
   float xpos = 0.0, ypos = 0.0;
   float Psh11 = 0.0, Psh22 = 0.0, Psh12 = 0.0, Psh21 = 0.0;
   float Psm11 = 0.0, Psm22 = 0.0, Psm12 = 0.0, Psm21 = 0.0;
   float snratio = 0.0;
   short cl = 0;

   /* calculate total luminosity inside GC_MAX */
   for (r = 0; r < GC_MAX; r++)	/* zero the sums */
      sumf[r] = sum1[r] = summagic[r] = 0;


   for (i = (int)(aipos - GC_MAX); i <= (int)(aipos + GC_MAX); i++){ /* sum over shells */
      if (i < 0 || i >= N2) continue;
      di = (int)(i - aipos);
      for (j = (int)(ajpos - GC_MAX); j <= (int)(ajpos + GC_MAX); j++){
   	 if (j < 0 || j >= N1) continue;
   	 dj = (int)(j - ajpos);
   	 r = floor(0.5 + sqrt((double) (di * di + dj * dj)));
   	 if (r >= 0 && r < GC_MAX){
   	    if (fabs(f[i][j]-MAGIC) < EPSILON)
            {
   	       summagic[r]++;
   	    } else {
   	       sum1[r] 	+= 1;
   	       sumf[r] 	+= (f[i][j] - fsky(di, dj));
   	    }
   	 }
      }
   }

   /* calculate rnumax (Petrosian radius) */
   rnumax = GC_MAX;	
   for (r = 1; r < GC_MAX; r++) { /* cumulate sums */		
      sum1[r] += sum1[r - 1]; /* and find max-nu radius */
      sumf[r] += sumf[r - 1];
      summagic[r] += summagic[r - 1];
      if (sumf[r] * sumf[r] * sum1[r-1] < sumf[r-1] * sumf[r-1] * sum1[r]
          && rnumax == GC_MAX) {
         rnumax = r - 1;	/* peak-nu radius*/
   	 if (sum1[r-1] > 0)
            numax = sumf[r - 1] / (sqrt((double) sum1[r - 1]) * sigma);
   	 else
            numax = 0;
      }
   }

   if (alpha > 0)
   {
      rmax = floor(0.5 + (alpha * rnumax >
                   GC_MAX -1 ? GC_MAX - 1 : alpha * rnumax));
      rmax = (rmax < RMAX_MIN ? RMAX_MIN : rmax);
   }
   else
   {
         rmax = floor(0.5 - alpha * arg);
         rmax = (rmax > GC_MAX -1 ? GC_MAX - 1 : rmax);
   }

   /*
      ensure that rmax and rc are at least 1 (otherwise the program crashes
      lateron)!!
   */
   if(rmax < 1.0)
   {
     rmax = 1.0;
   }

   if(rc < 1.0)
   {
     rc = 1.0;
   }

   lum = sumf[rmax];
   /* pk->sigmal = sqrt((double) sum1[rmax]) * sigma; discontinued */
   nbad = summagic[rmax];


   /* now do 1st and second moments and Psm */
   q11 = q22 = q12 = 0.0;
   Xsm11 = Xsm22 = Xsm12 = Xsh11 = Xsh22 = Xsh12 = em[0] =
           em[1] = eh[0] = eh[1] = 0.0;
   dq[0][0] = dq[0][1] = dq[1][1] = dqq[0] = dqq[1] = 0.0;
   de[0] = de[1] = 0.0;

   /* find peak position by interpolation */

   if ((int)(aipos - rmax) > 1 &&
       (int)(aipos + rmax) < N2-1 &&
       (int)(ajpos - rmax) > 1 &&
       (int)(ajpos + rmax) < N1-1)
   {
     dataregion=(float**)KSBmatrix(0,2*rmax,0,2*rmax);
     for(i=0;i<=2*rmax;i++)
     {
       di= i - rmax;
       for(j=0;j<=2*rmax;j++)
       {
   	 dj= j - rmax;
   	 fc=f[(int)(aipos + di)][(int)(ajpos + dj)] - fsky(di,dj);
   	 dataregion[i][j]=fc;
       }
     }

     if(Gusecatpos == 0)
     {
       /* find peak position  */
       imax = rmax;
       jmax = rmax;
       imaxold = imax + 1000;
       jmaxold = jmax + 1000;
       teller = 0;
       dx=dy=0.25;

       while ((fabs(imax - imaxold) > 0.002 || fabs(jmax - jmaxold) > 0.002) &&
              teller < 200)
       {
         d[0] = d[1]  = gew = A[0][0] = A[0][1] = A[1][1] =
                              S[0][0] = S[0][1] = S[1][1] = 0.0;
         for(x=0.0;x<=2.00*rmax;x+=dx) {
     	   dii = x-rmax;
     	   for(y=0.0;y<=2.00*rmax;y+=dy) {
     	     djj = y-rmax;
     	     rr =  dii * dii + djj * djj;
     	     W = exp(-0.5 * rr / (rc * rc));
     	     df = interp_d2(2*rmax,2*rmax,dataregion,
                              x+imax-rmax,y+jmax-rmax) * dx * dy;
     	     d[0] += df * dii * W;
     	     d[1] += df * djj * W;
     	
     	     A[0][0] += dii * dii * W * df;
     	     A[1][1] += djj * djj * W * df;
     	     A[0][1] += dii * djj * W * df;
     	     S[0][0] += dii * dii * W * W * df;
     	     S[1][1] += djj * djj * W * W * df;
     	     S[0][1] += dii * djj * W * W * df;
     	     gew += W * df;
     	   }
         }
         imaxold = imax;
         jmaxold = jmax;
         imax += d[0] / (gew + 0.0000001);
         jmax += d[1] / (gew + 0.0000001);
         teller++;
       }

       A[0][0] = 2.0 / (rc * rc) * A[0][0] - gew;
       A[1][1] = 2.0 / (rc * rc) * A[1][1] - gew;
       A[0][1] *= 2.0 / (rc * rc);

       S[0][0] += sigma * sigma * PI * rc * rc * rc * rc / 8.0;
       S[1][1] += sigma * sigma * PI * rc * rc * rc * rc / 8.0;

       deltai = A[1][1] * A[1][1] * S[0][0] + A[0][1] * A[0][1] * S[1][1] -
                2.0 * A[0][1] * A[1][1] * S[0][1];
       deltaj = A[0][1] * A[0][1] * S[0][0] + A[0][0] * A[0][0] * S[1][1] -
                2.0 * A[0][1] * A[0][0] * S[0][1];
       fud = fabs(A[0][0] * A[1][1] - A[0][1] * A[0][1]);

       if (fabs(imax-rmax) > 2.0 || fabs(jmax-rmax) > 2.0 || teller >= 200)
       {
         imax = 1e6;
         jmax = 1e6;
         cl=103;
       }
       else
       {
         if(fud != 0.0 && deltai > 0.0 && deltaj > 0.0)
         {
    	   deltai = sqrt(deltai) / fud;
    	   deltaj = sqrt(deltaj) / fud;
         }
       }
     }
     else
     {
       imax = goodypos-1.0;
       jmax = goodxpos-1.0;
       imax = imax+rmax-aipos;
       jmax = jmax+rmax-ajpos;
     }

     /* found peak or used the given position */

     /* find half light radius by interpolation */
     if (imax > 9e5 || (aipos - rmax) < 1 || (aipos + rmax) > N2 ||
        (ajpos - rmax) < 1 || (ajpos + rmax) > N1 || lum <= 0.0)  {
       rh = 0.0;
     } else {
       fr = rr = 0.0;
       while (fr < 0.5 * lum && rr < rmax) {
   	 frr = 0.0;
   	 for(phi = 0.0; phi < 2.0 * PI; phi += 0.1 * PI) {
   	   x = imax + rr * cos(phi);
   	   y = jmax + rr * sin(phi);
   	
   	   df = interp_d2(2*rmax,2*rmax,dataregion,x,y) ;
   	   frr += df * 0.1 * PI;
   	 }
   	 dfr = rr * frr * 0.1;
   	 fr += dfr;
   	 rr += 0.1;
       }
       if (dfr > 0.0)
       {
          rh = rr + 0.1 * ((0.5 * lum - fr + dfr) / dfr) - 0.2;
       }
       else
       {
          rh = rr - 0.2;
       }
     }
     if (rr >= rmax) rh = 0.0;

     /* found half light radius */
     dx=dy=0.25;
     gew=snnormalication=0.0;

     if (imax < 1e6)
     {
       for(x=0.0;x<=2.00*rmax;x+=dx) {
  	 dii = x-rmax;
  	 for(y=0.0;y<=2.00*rmax;y+=dy) {
  	   djj = y-rmax;
  	   rr =  dii * dii + djj * djj;
  	   if (powerlawwindow) {
  	     W = pow((double) (rr + rc * rc), (double) (0.5 * ne));
  	     Wp = 0.5 * ne * pow((double) (rr + rc * rc),
  				 (double) (0.5 * ne - 1));
  	     Wpp = 0.5 * ne * (0.5 * ne - 1) *
  	       pow((double) (rr + rc * rc), (double) (0.5 * ne - 2));;
  	   } else {
  	     W = exp(-0.5 * rr / (rc * rc));
  	     Wp = -0.5 * W / (rc * rc);
  	     Wpp = 0.25 * W / (rc * rc * rc * rc);
  	   }
  	
  	   df = interp_d2(2*rmax,2*rmax,dataregion,
  			  x+imax-rmax,y+jmax-rmax) * dx * dy;

  	   /*df = dataregion[(int)(x+imax-rmax+0.5)][(int)(y+jmax-rmax+0.5)]*dx*dy;*/

  	   q22 += df * W * dii * dii;
  	   q11 += df * W * djj * djj;
  	   q12 += df * W * dii * djj;
  	   dq[0][0] += dii * dii * dii * dii * W * W * df;
  	   dq[1][1] += djj * djj * djj * djj * W * W * df;
  	   dq[0][1] += dii * dii * djj * djj * W * W * df;
  	   dqq[0] += dii * dii * dii * djj * W * W * df;
  	   dqq[1] += dii * djj * djj * djj * W * W * df;
  	
  	   DD = dii * dii + djj * djj;
	   DD1 = djj * djj - dii * dii;
  	   DD2 = 2. * dii * djj;
  	   /* corrected the Xsm values for a factor of two
  	      (TE: 20.02.1998) */
  	   Xsm11 += (W + 2. * Wp * DD + Wpp * DD1 * DD1) * df;
  	   Xsm22 += (W + 2. * Wp * DD + Wpp * DD2 * DD2) * df;
  	   Xsm12 += Wpp * DD1 * DD2 * df;
  	   Xsh11 += (2. * W * DD + 2. * Wp * DD1 * DD1) * df;
  	   Xsh22 += (2. * W * DD + 2. * Wp * DD2 * DD2) * df;
  	   Xsh12 += 2. * Wp * DD1 * DD2 * df;
  	   /* corrected values for em according to Henk's paper */
  	   em[0] += (2. * Wp + Wpp * DD) * DD1 * df;
  	   em[1] += (2. * Wp + Wpp * DD) * DD2 * df;
  	   eh[0] += 2. * Wp * DD * DD1 * df;
  	   eh[1] += 2. * Wp * DD * DD2 * df;
  	
  	   gew += W * df;
	   snnormalication += W * W;
  	 }
       }

       /* calculate ellipticities */
       denom = q11 + q22;	/* can change to sqrt(det) if desired */
       if (denom > 0.0) {
  	 e1 = (q11 - q22);
  	 a = 0.5*denom+sqrt(0.25*e1*e1+q12*q12);
  	 b = 0.5*denom-sqrt(0.25*e1*e1+q12*q12);
         if(a>0 && b>0)
	 {
  	   a = sqrt(a/gew);
  	   b = sqrt(b/gew);
	 }
	 else
	 {
           cl=104;
	 }
  	 e1 /= denom;
  	 e2 = 2. * q12 / denom;
  	 phi = (e1 == 0.0) ? 0.78539 : 0.5*atan2(e2, e1);
  	 em[0] /= denom;
  	 em[1] /= denom;
  	 eh[0] /= denom;
  	 eh[1] /= denom;
  	 /* changed a 4 to a 2. (TE: 20.02.1998) */
  	 eh[0] += 2. * e1;
  	 eh[1] += 2. * e2;

         Xsm11 /= denom;
         Xsm22 /= denom;
         Xsm12 /= denom;
         Xsh11 /= denom;
         Xsh22 /= denom;
         Xsh12 /= denom;

  	 Psm11 = Xsm11 - e1 * em[0];
  	 Psm22 = Xsm22 - e2 * em[1];
  	 Psm12 = Xsm12 - e1 * em[1];
  	 Psm21 = Xsm12 - e2 * em[0];
  	 Psh11 = Xsh11 - e1 * eh[0];
  	 Psh22 = Xsh22 - e2 * eh[1];
  	 Psh12 = Xsh12 - e1 * eh[1];
  	 Psh21 = Xsh12 - e2 * eh[0];
  	
  	 dq[0][0] += sigma * sigma * pow(rc, 6.0) * 0.75 * PI;
  	 dq[1][1] += sigma * sigma * pow(rc, 6.0) * 0.75 * PI;
  	 dq[0][1] += sigma * sigma * pow(rc, 6.0) * 0.25 * PI;
  	
  	 de[0] = (((1.0 - e1) * (1.0 - e1)) /
  		 (denom * denom)) * dq[0][0];
  	 de[0] += (((1.0 + e1) * (1.0 + e1)) /
  		 (denom * denom)) * dq[1][1];
  	 de[0] -= 2.0 * (1.0 - e1 * e1) /
  		 (denom * denom) * dq[0][1];
  	 if (de[0] > 0.0) { de[0] = sqrt(de[0]); } else { de[0] = 1000.0; }
  	
  	 de[1] = ((4.0 + 2.0 * e2 * e2) /
  		 (denom * denom)) * dq[0][1];
  	 de[1] += ((e2 * e2) /
  		 (denom * denom)) * (dq[0][0] + dq[1][1]);
  	 de[1] -= ((4.0 * e2) / (denom * denom)) * (dqq[0] + dqq[1]);
  	 if (de[1] > 0.0) { de[1] = sqrt(de[1]); } else { de[1] = 1000.0; }
  	
  	 /* calculate S/N ratio of the object */
         snratio = gew/(sqrt(snnormalication)*sigma);

  	 imax=imax-rmax+aipos;
  	 jmax=jmax-rmax+ajpos;

  	 /* it seems that interpol_2d treats the middle of a pixel
  	    as zero while we treat as one half; so I simply add this
            06.07.1999: added another 0.5 to make it correct according
            to the FITS standard.
  	 */
  	 xpos=jmax+1.0;
  	 ypos=imax+1.0;

  	 if (lum > 0.0) {
  	   mag = DEFAULT_ZEROPOINT -2.5 * log10(lum);
  	 } else {
  	   cl=102;
  	 }
       } else {
  	 /* object has negative q11+q22 */
  	 cl=101;
       }
     }
     free_KSBmatrix(dataregion, 0, 2*rmax, 0, 2*rmax);
   } else {
     /* object near the border */
     cl=100;
   }

   /* fill the key elements */
   Grnumax[anumber]=rnumax;
   Gnumax[anumber]=numax;
   Grmax[anumber]=rmax;
   Gl[anumber]=lum;
   Gmag[anumber]=mag;
   Gnbad[anumber]=nbad;
   Grh[anumber]=rh;
   Ge1[anumber]=e1;
   Gdeltae1[anumber]=de[0];
   Ge2[anumber]=e2;
   Gdeltae2[anumber]=de[1];
   Ga[anumber]=(float)a;
   Gb[anumber]=(float)b;
   Gtheta[anumber]=(float)phi*RAD_GRAD;
   if(Gusecatpos == 0)
   {
     Gxpos[anumber]=xpos;
     Gypos[anumber]=ypos;
   }
   Gdeltaxpos[anumber]=deltaj;
   Gdeltaypos[anumber]=deltai;
   GPsh11[anumber]=Psh11;
   GPsh22[anumber]=Psh22;
   GPsh12[anumber]=Psh12;
   GPsh21[anumber]=Psh21;
   GPsm11[anumber]=Psm11;
   GPsm22[anumber]=Psm22;
   GPsm12[anumber]=Psm12;
   GPsm21[anumber]=Psm21;
   Gcl[anumber]=cl;
   Gq11[anumber]=q11;
   Gq22[anumber]=q22;
   Gq12[anumber]=q12;
   GXsh11[anumber]=Xsh11;
   GXsh22[anumber]=Xsh22;
   GXsh12[anumber]=Xsh12;
   GXsm11[anumber]=Xsm11;
   GXsm22[anumber]=Xsm22;
   GXsm12[anumber]=Xsm12;
   Geh0[anumber]=eh[0];
   Geh1[anumber]=eh[1];
   Gem0[anumber]=em[0];
   Gem1[anumber]=em[1];
   Gsnratio[anumber]=snratio;
}

/* next two functions define the sky model - here a simple straight
   average plus linear trend */

void	setskyparameters(skyquad *sky)
{
   ff = 0.25 * (sky->f[NORTH] + sky->f[SOUTH] + sky->f[EAST] + sky->f[WEST]);
   ffi = (sky->f[EAST] - sky->f[WEST]) / (sky->i[EAST] - sky->i[WEST]);
   ffj = (sky->f[NORTH] - sky->f[SOUTH]) / (sky->j[NORTH] - sky->j[SOUTH]);
}

float	fsky(int di, int dj)
{
   if (nosky)
      return(ffmode);
   else
      return (ff + di * ffi + dj * ffj);
}


/* idea of zapping:
   all pixels that represent an object (everything within
   a certain radius around the object center) are set to
   MAGIC and a 'zapcounter' is increased for the corresponding
   pixel. Later only pixels that have a 'zapcounter' of 1
   (are covered only by one object) are restored and used
   in the analysis)
*/

void zap(int zapmode, LONG ax, LONG ay, double arg, int radiustype, float a,
         float **f, float **fzap, short **nzap, int N1, int N2)
{
   /* set fzap to MAGIC in disk and increment nzap */
   int	i, j, d;
   float	rmax=0.0, rr=0.0, rrmax=0.0;

   switch (radiustype) {
   	   case G_RADIUS:
   		   rmax = a * arg;
   		   break;
   	   default:
   		   error_exit("zap: bad radiustype\n");
   		   break;
   }
   d = ceil((double)rmax);
   fprintf(stderr, "%f %f %f\n", arg, a, rmax);
   rrmax = rmax * rmax;
   for (i = (int)(ay - d); i <= (int)(ay + d); i++) {
      if (i < 0 || i >= N2)
   	      continue;
      for (j = (int)(ax - d); j <= (int)(ax + d); j++) {
   	 if (j < 0 || j >= N1)
   		 continue;
   	 rr = (float)((i - ay) * (i - ay) + (j - ax) * (j - ax));
   	 if (rr <= rrmax) {
   	    switch (zapmode) {
   	       case ZAP:
   	           fzap[i][j] = MAGIC;
                   nzap[i][j]++;
   		   break;
   	       case RESTORE:
   	           if (nzap[i][j] == 1)
   	              fzap[i][j] = f[i][j];
   		   nzap[i][j]--;
   		   break;
   	       default:
   	           error_exit("zap: bad zapmode\n");
   		   break;
   	    }
   	 }
      }
   }
}

