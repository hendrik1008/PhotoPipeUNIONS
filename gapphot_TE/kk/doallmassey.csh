#!/bin/tcsh

setenv PATH ~/data/shapelets/kk:$PATH

\rm -rf default.conv default.nnw default.param default.sex
sexsetup
\rm -rf inimage.fits shearstats.* tmpcov

echo 10 > sn.dat
echo 0.5 1. 0.05 0.2 0.1 0.2 > shapecuts.dat

set ipsf=$1

 tcsh dopsf.csh STEPshapelets.psf${ipsf}.starfield.fits 8 2
 nice convert fitpsfmap.ps -rotate 90 fitpsfmap.jpg
 nice convert psfmap.ps -rotate 90 psfmap.jpg
 nice convert psfsel.ps -rotate 90 psfsel.jpg
 nice convert psfstarpos.ps -rotate 90 psfstarpos.jpg
 mkdir STEPshapelets.psf${ipsf}.starfield/
 \mv -f psf* STEPshapelets.psf${ipsf}.starfield/
 \mv -f *ps.gz *jpg *jpg.o STEPshapelets.psf${ipsf}.starfield/
 \cp -f STEPshapelets.psf${ipsf}.starfield/psf.map .
 foreach sheared (`ls -1 STEPshapelets.psf${ipsf}.shear*.fits | sed -e 's/.fits//g' `)
  tcsh donopsf.csh ${sheared}.fits 8 2
  mkdir $sheared/
  gzip *.ps
  \mv -f shearstats* *.par test.cat test.sh test.g *.jpg *.jpg.0 *.ps.gz ${sheared}/
  \cp -f psf.map *.dat ${sheared}/
 end


exit
