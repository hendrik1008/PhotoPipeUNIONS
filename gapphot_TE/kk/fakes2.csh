#!/bin/tcsh

# do $6 simulations with same parameters
# parameter order: 
# ranseed nx ny starflx siga sigb PA no. galflx ga gb g1 g2 ngal bg
# $1=a, $2=b, $3=pa, $4=ga %5=gb 

setenv PATH ~/data/shapelets/kk/:$PATH
#sexsetup

@ i = 0

set folder = a-b-pa-rg_$1_$2_$3_$4_$5
mkdir $folder/

while ($i<$6)
 echo -`date +%s` 2000 2000 50000 $1 $2 $3 100 20000 $4 $5 0 0 1000 10000 \
  | fakeimage2 > fake.cat
 tcsh doit.csh fake.fits 8 2
 @ i++
 mkdir $folder/fake$i
 gzip *.ps *.sh *.cat
 \mv -f *.cat.gz *.map *.sh.gz *.g *.ps.gz *.par *.out $folder/fake$i/
 # \mv -f fake.fits $folder/fake$i/inimage.fits
 echo Made fake simulation fake$i
 echo -----------------------------------------------
end

cat $folder/fake*/shearstats.out > $folder/shearstats.all
