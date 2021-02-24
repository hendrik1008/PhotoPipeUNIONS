#!/usr/bin/tcsh

#$1 is image
#$2 is kernel map
#$3 is output name
#$4 GAaP path

if $# != 4 then
   echo Usage: im2gau image kernalmap outputimname GAaP_path
   exit
   endif
   
set code = $4/kk
set bigim = $code/bigim

ln -sf $1 inimage.fits

$bigim/imxshmapwithtweak < $2
\mv -f convolved.fits $3

ls -l $3
