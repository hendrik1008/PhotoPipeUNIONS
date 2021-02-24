#!/bin/tcsh

foreach im (`\ls -1 *.fits | sed -e 's/.fits//g'  `)
 echo ------------------------------------
 echo Processing ${im}.fits
 echo 

 if (-e $im/all.avg.$1.$2) goto hadit

 cd $im/

 cat test.g | awk -v f1=$1 -v f2=$2 '$15==0 && $3>10*$4 && $3>f1&&$3<f2 && $11>0. && $9**2+$10**2<1' > out
 set rslt = `awk '{w=1/(0.07+$11**2+$12**2);cf=-0.41+(0.085*$17+0.63*$18)/$16;n+=1;s0+=w;g1w+=$9*w;g2w+=$10*w;g11w+=($9**2-$11**2)*w;g22w+=($10**2-$12**2)*w;cfw+=2*cf*w*($9**2+$10**2-$11**2-$12**2)}END{print(n,g1w/s0,g2w/s0,g11w/s0+g22w/s0-(g1w/s0)**2-(g2w/s0)**2,cfw/s0)}' out`
 set nl = `wc -l out | awk '{print(int((1+$1)/2))}'`
 set g1med=`sort -g -k  9 out| tail +$nl | head -1 | awk '{print($9)}'`
 set g2med=`sort -g -k 10 out| tail +$nl | head -1 | awk '{print($10)}'`
 echo $im $rslt $g1med $g2med > all.avg.$1.$2
 cat all.avg.$1.$2
 \rm -f out
 cd ..

 hadit:


end

cat */all.avg.$1.$2 > all.avg.$1.$2.all

foreach psf (0 1 2 3 4 5)
  foreach lens (0 3 4)
   echo $psf $lens `grep psf${psf}.lens${lens} all.avg.$1.$2.all | awk '{n+=1;g1+=$3;g2+=$4;g1m+=$7;g2m+=$8;e2m+=$5}END{e2m=e2m/n;print(g1/n/(1-e2m),g2/n/(1-e2m),g1m/n,g2m/n,g1/n,g2/n,1-e2m)}'` >> averages.out.$1.$2
end
end

