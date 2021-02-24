#!/bin/tcsh

setenv PATH ~/data/shapelets/kk:$PATH

\rm -rf inimage.fits

foreach im (`\ls -1 *.fits | sed -e 's/.fits//g'  `)
 cd $im/
 echo 10 > sn.dat
 echo 0.5 1. 0.05 0.2 0.1 0.2 > shapecuts.dat

 echo ------------------------------------
 echo Processing ${im}.fits
 echo 
 stackshear < test.sh > test.stsh
 cd ..
end
