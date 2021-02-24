#!/bin/tcsh

# process fits file to shear estimators, assuming a psf.map already exists

setenv PATH ~/data/shapelets/kk/:$PATH

# make logical link to the image (argument 1)
ln -sf $1 inimage.fits
\rm -f err.fits

# if specified, set shapelet and spatial fit orders
if ($# == 1) then
 if -e orders.par then
   echo taking fit orders from orders.par:
   cat orders.par
 else
   echo Setting fit orders to default values:
   echo 8 > orders.par
   echo 2 >>orders.par
   cat orders.par
 endif
else if ($# == 2) then
 echo Specify the order of shapelet fit and spatial variations
 exit
else
 echo $2 \# shapelet order               >  orders.par
 echo $3 \# spatial variation fit order  >> orders.par
endif

if (-e default.sex) goto skip
sexsetup
skip:
sex inimage.fits

# Encode the source catalogue
sort -n -k 5 test.cat | nice cat2sherr > test.sh

# Make image showing all detected sources' shapelet expansion
#reconstructimage < test.sh
#ccube "inimage.fits outimage.fits -" diff.fits

# Measure shears for all sources, one by one
nice sh2gerrshift < test.sh > test.g

# compute average shear for the whole catalogue
nice shearstats  < test.g  > shearstats.avg
nice shearstats2 < test.g  > shearstats.cov
nice stackshear  < test.sh > shearstats.stk

# make plots of distribution of power distribution over shapelet order
nice fracpower < test.sh 
