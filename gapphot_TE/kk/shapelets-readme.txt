Instructions for the shapelet software:

you need:

sextractor
pgplot graphics library
cfitsio library
fortran 77 compiler

compile with the makefile (may take some fiddling to find the lbraries)
	make all

#the script doiterrshift012.csh gives the successive steps to follow:

#make a (soft) link from your fits file to inimage.fits (or rename it)
      
#copy the right sextractor parameter files
     cp default.* .

#write the shapelet order and spatial PSF orders to orders.par (8 2 usually OK)
     echo 8 > orders.par; echo 2 >> orders.par

#make a sextractor catalogue                            
     sex inimage.fits

#replace half-max radius by FWHM:
awk '{if($1!="#") print($1,$2,$3,$4,2*$5,$6,$7,$8,$9,$10)}' test.cat >test.cat2

#select the PSF stars from the magnitude-FWHM plot      
       selectpsfstars < test.cat2 > psf.cat
#or       selectpsfstars_auto < test.cat2 > psf.cat

#fit a shapelet expansion to all stars                  
     psfcat2sh < psf.cat > psf.sh
#fit these shapelet coefficients to a PSF map           
     fitpsfmap < psf.sh > psf.map
     showpsfmap < psf.map  # makes a plot of the PSF across the field
#fit shapelet expansions to all sources bigger than 1.1 * PSF
     set betamin = `head -1 psf.map | awk '{print(1.1*2.3*$2)}'`
     sort -n -k 5 test.cat2 | awk -v b=$betamin '$5>b' | cat2sherr > test.sh
#derive ellipticities                                   
	sh2gerrshift012 < test.sh > test.g

Each programme runs as a pipeline, reading from an input catalogue and
writing out a new one. See the script how this works.

Diagnostic plots along the way:

psfstars.ps: plots of PSF objects, residuals after shapelet fit, and
PSF model at each star's location
psfsel.ps: location of the PSF objects

The format of the catalogs is: (all text files)

.cat files have x,y,flx,flxerr,fwhm,a,b,PA,id,flags  (all from sextractor)

.sh files have, for each object: 
   first the line from the .cat file, 
   then a line with total integrated shapelet flux, centroid shift in x and
   y (applied to make the 0,1 and 1,0 coefficients zero), fitted
   background value, radius in which the shapelet fit was made, and
   error on each of the shapelet coefs
   then a line with shapelet scale (ie sigma of gaussian) and order
   then all coefficients, normalized to total flux, in order
    0,0 1,0 0,1 2,0 1,1 0,2 3,0 2,1 1,2 0,3 4,0 3,1 2,2 1,3 0,4 5,0
   ...

.g files have
x,y,flx,flxerr,fwhm,a,b,PA,g1,g2,g1err,g2err,cov,id,flags,c0,c2,c4,c6,...
   where g1,g1 are the estimated shears for that object, and
   c0,c2,c4... are the coefficients of a round source that
   approximates the source best

