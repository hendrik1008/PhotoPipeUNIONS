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
echo 0.1 0.2 0 200 > starsel.par 

foreach im (`\ls -1 *.fits | sed -e 's/.fits//g'  `)
 echo ------------------------------------
 echo Processing ${im}.fits
 echo 

 if (-e $im/test.g) goto hadit

 mkdir $im/
 tcsh doit2.csh ${cwd}/${im}.fits $1 $2

 gzip -f *.ps *.sh

 cat test.g | awk '$15==0 && $3>10*$4 && $11>0. && $9**2+$10**2<1' > out
 set rslt = `awk '{w=1/(0.1**2+$11**2+$12**2);n+=1;s0+=w;g1w+=$9*w;g2w+=$10*w;g11w+=($9**2-$11**2)*w;g22w+=($10**2-$12**2)*w}END{print(n,g1w/s0,g2w/s0,g11w/s0+g22w/s0-(g1w/s0)**2-(g2w/s0)**2)}' out`
 set nl = `wc -l out | awk '{print(int((1+$1)/2))}'`
 set g1med=`sort -g -k  9 out| head -$nl | tail -1 | awk '{print($9)}'`
 set g2med=`sort -g -k 10 out| head -$nl | tail -1 | awk '{print($10)}'`
 echo $im $rslt $g1med $g2med > all.avg
 cat all.avg
 \rm -f out
 \mv -f *.ps.gz *.map *.cat *.sh.gz *.g *.g.cal *.stk *.avg *.cov ${im}/
 \cp -r *.par ${im}/

 hadit:
end

\rm -rf tmpcov default.* inimage.fits 
