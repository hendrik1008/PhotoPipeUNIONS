#!/usr/bin/tcsh

# $1 is gaussianized image
# $2 is star catalogue
# $3 GAaP path

if $# != 3 then
   echo Usage: chkgau image cataloguename GAaP_path
   exit
   endif
   
set code = $3/kk
set bigim = $code/bigim

ln -sf $1 inimage.fits
$bigim/checkgpsf2d < $2

\mv -f checkgpsf2d.ps $1.gpsf.ps
ls -l $1.gpsf.ps
