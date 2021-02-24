#!/bin/tcsh

#Find PSF stars and make the PSF map

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

# choose interactive or automatic star finder
#interactive:
#selectpsfstars < test.cat > psf.cat
#automatic
nice selectpsfstars_auto < test.cat > psf.cat

nice psfcat2sh < psf.cat | nice fitpsfmap | grep -v PGPLOT > psf.map
# alternatively if you want the PSF shapelets for indiv stars:
#nice psfcat2sh < psf.cat > psf.sh
#nice fitpsfmap <psf.sh | grep -v PGPLOT > psf.map

nice showpsfmap < psf.map 

