#!/bin/tcsh

# do $5 simulations with same parameters
# parameter order: 
# ranseed nx ny starflx siga sigb PA no. galflx rg g1 g2 ngal bg
# $1=a, $2=b, $3=pa, $4=rg

setenv PATH ~/data/shapelets/kk/:$PATH
#sexsetup

@ i = 0

mkdir a-b-pa-rg_$1_$2_$3_$4/

while ($i<$5)
 echo -`date +%s` 2000 2000 50000 $1 $2 $3 100 20000 $4 0 0 1000 10000 \
  | fakeimage > fake.cat
 tcsh doit.csh fake.fits 8 2
 @ i++
 mkdir a-b-pa-rg_$1_$2_$3_$4/fake$i
 gzip *.ps *.cat *.sh
 \mv -f *.cat.gz *.map *.sh.gz *.g *.ps.gz *.par *.out a-b-pa-rg_$1_$2_$3_$4/fake$i/
 # \mv -f fake.fits a-b-pa-rg_$1_$2_$3_$4/fake$i/inimage.fits
 echo Made fake simulation fake$i
 echo -----------------------------------------------
end

cat a-b-pa-rg_$1_$2_$3_$4/fake*/shearstats.out > a-b-pa-rg_$1_$2_$3_$4/shearstats.all
