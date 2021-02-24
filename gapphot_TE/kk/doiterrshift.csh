#!/bin/tcsh

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
awk '{if($1!="#") print($1,$2,$3,$4,2*$5,$6,$7,$8,$9,$10)}' test.cat >test.cat2

# choose interactive or automatic star finder
#interactive:
#selectpsfstars < test.cat2 > psf.cat
#automatic
nice selectpsfstars_auto < test.cat2 > psf.cat

nice psfcat2sh < psf.cat | nice fitpsfmap | grep -v PGPLOT > psf.map
# alternatively if you want the PSF shapelets for indiv stars:
#nice psfcat2sh < psf.cat > psf.sh
#nice fitpsfmap <psf.sh | grep -v PGPLOT > psf.map

nice showpsfmap < psf.map 

#gv psfmap.ps&
#gv psfstars.ps&
#gv fitpsfmap.ps&
#gv psfstarpos.ps&

# Encode the source catalogue
sort -n -k 5 test.cat2 | nice cat2sherr > test.sh

# Make image showing all detected sources' shapelet expansion
#reconstructimage < test.sh
#ccube "inimage.fits outimage.fits -" diff.fits

# Measure shears for all sources, one by one
nice sh2gerrshift < test.sh > test.g

# compute average shear for the whole catalogue
nice shearstats  < test.g  > shearstats.avg
nice shearstats2 < test.g  > shearstats.cov
#nice stackshear  < test.sh > shearstats.stk

# make plots of distribution of power distribution over shapelet order
nice fracpower < test.sh 

\rm -f test.cat2
