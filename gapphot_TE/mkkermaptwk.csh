#!/usr/bin/tcsh

# $1 = image
# $2 = psf catalogue
# $3 = target PSF size (beta)
# $4 = output kernel map
# $5 = GAaP path

if $# != 5 then
   echo Usage: mkkermap image psfcat gpsfsig outputkermap
   exit
   endif
   
set code = $5/kk
set bigim = $code/bigim

ln -sf $1 inimage.fits

#(echo $3; cat $2) | $bigim/psfcat2gauskerwithtweak > tmp_ker.sh2
(echo $3; cat $2) | $bigim/psfcat2gauskerwithtweak_no_recentre > tmp_ker.sh2

time $bigim/fitkermaptwk < tmp_ker.sh2 > $4
$bigim/showpsfmaptwk < $4
\mv -f psfmap.ps $4.ps
#to show just the outer shapelet (not terribly informative..)    
#grep -A 9999 OUTER $4 |$code/showpsfmapcol      # grep writes out 9999 lines after matching OUTER
#\mv -f psfmap.ps $4.out.ps

ls -l $4

mv fitkermaptwk.ps $4.fitkermaptwk.ps
rm tmp_ker.sh2
