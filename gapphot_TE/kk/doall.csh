#!/bin/tcsh

if ($# != 2) then
 echo Specify the order of shapelet fit and spatial variations
 exit
endif

setenv PATH ~/data/shapelets/kk:$PATH

\rm -rf default.conv default.nnw default.param default.sex
sexsetup
\rm -rf inimage.fits shearstats.* tmpcov

echo 10 > sn.dat
echo 0.5 1. 0.05 0.2 0.1 0.2 > shapecuts.dat


echo \<h1\>SHEAR FITTING RESULTS\</h1\>                     > results.html
foreach im (`\ls -1 *.fits | sed -e 's/.fits//g'  `)
 echo ------------------------------------
 echo Processing ${im}.fits
 echo 
 mkdir $im/
 tcsh doiterrshift.csh ${cwd}/${im}.fits $1 $2
 echo \<h1\>$im\</h1\>                                      > ${im}/index.html

 nice convert shears.ps -rotate 90 shears.jpg
 echo \<h2\>Shear estimates\</h2\>                         >> ${im}/index.html
 echo \<img src=shears.jpg\>                               >> ${im}/index.html
 echo \<br\>                                               >> ${im}/index.html
 cat shearstats.avg                                        >> ${im}/index.html

 nice convert fracpower-sn.ps -rotate 90 -resize 150% fracpower-sn.jpg
 nice convert fracpower-mag.ps -rotate 90 -resize 150% fracpower-mag.jpg
 nice convert fracpower-fwhm.ps -rotate 90 -resize 150% fracpower-fwhm.jpg
 nice convert fracpower-sherr.ps -rotate 90 -resize 150% fracpower-sherr.jpg
 echo \<h2\>Cuts applied\</h2\>                            >> ${im}/index.html
 echo \<img src=fracpower-sn.jpg\>\</br\>\</br\>           >> ${im}/index.html
 echo \<img src=fracpower-mag.jpg\>\</br\>\</br\>          >> ${im}/index.html
 echo \<img src=fracpower-fwhm.jpg\>\</br\>\</br\>         >> ${im}/index.html
 echo \<img src=fracpower-sherr.jpg\>\</br\>\</br\>        >> ${im}/index.html

 nice convert psfsel.ps -rotate 90 psfsel.jpg
 nice convert psfstarpos.ps -rotate 90 psfstarpos.jpg
 echo \<h2\>PSF star selection\</h2\>                      >> ${im}/index.html
 echo \<img src=psfsel.jpg.0\>                             >> ${im}/index.html 
 echo \<img src=psfstarpos.jpg\>\<br\>                     >> ${im}/index.html 

 nice convert fitpsfmap.ps -rotate 90 fitpsfmap.jpg
 echo \<h2\>PSF map fitting residuals\</h2\>               >>${im}/index.html
 echo \<a href=fitpsfmap.ps.gz\>\<img src=fitpsfmap.jpg.0\>\</a\>\<br\>  \
                                                           >> ${im}/index.html
 echo \(Click on figure for plot for all coefficients\)    >> ${im}/index.html

 nice convert psfmap.ps -rotate 90 psfmap.jpg
 echo \<h2\>The PSF map\</h2\>                             >> ${im}/index.html
 echo \<img src=psfmap.jpg\>\<br\>\<br\>                   >> ${im}/index.html
 echo \(Click \<a href=psfstars.ps.gz\>here\</a\> for fits to >> ${im}/index.html
 echo the individual stars.\)                              >> ${im}/index.html
 echo \<br\>                                               >> ${im}/index.html
 echo \<hr\>                                               >> ${im}/index.html
 date                                                      >> ${im}/index.html
 gzip *.ps
 \mv -f *.ps.gz *.par *.map *.cat *.sh *.g *.stk *.avg *.cov *.jpg *.jpg.0  ${im}/
 \rm *.jpg.?

 echo \<a href=${im}/index.html\>$im\</a\>                 >> results.html
 cat ${im}/shearstats.avg                                  >> results.html
 echo \<br\>                                               >> results.html

 cat ${im}/shearstats.avg                                  >> shearstats.all
 cat ${im}/shearstats.cov                                  >> tmpcov

end
echo \<hr\>                                               >> results.html
date                                                      >> results.html

awk '{if ($1==1) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(1,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov       > shearstats.cov
awk '{if ($1==2) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(2,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov      >> shearstats.cov
awk '{if ($1==3) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(3,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov      >> shearstats.cov
awk '{if ($1==4) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(4,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov      >> shearstats.cov
awk '{if ($1==5) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(5,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov      >> shearstats.cov
awk '{if ($1==6) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(6,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov      >> shearstats.cov
awk '{if ($1==7) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(7,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov      >> shearstats.cov
awk '{if ($1==8) {s0+=$6;s1+=$2*$6;s2+=$3*$6;ss+=($4+$5)*$6/2}} \
     END{print(8,s1/s0,s2/s0,sqrt(ss/s0),s0)}' < tmpcov      >> shearstats.cov

\rm -rf tmpcov default.*
