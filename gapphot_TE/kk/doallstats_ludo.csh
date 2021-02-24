#!/bin/tcsh
\rm -rf tmpcov
foreach im ( `find . -name "*cat" | grep vlt` )
~/data/shapelets/kk/shearstats2_ludo < $im >> tmpcov
end

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

awk '{print(int(2000/$1), $2+$3, $5)}' shearstats.cov
