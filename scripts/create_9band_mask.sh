#!/bin/bash

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/lib/
set -e 
#}}}

md=$1
field=$2
mask=$3
RA=$4
Dec=$5

orig_dir=`pwd`

if [ ! -f $mask ] && [ -f $mask.gz ]
then
    gunzip $mask.gz
fi

for filter in Z Y J H Ks
do
    if [ ! -f $md/$filter/${field}_${filter}_swarp_cut.sum.fits ] && [ -f $md/$filter/${field}_${filter}_swarp_cut.sum.fits.gz ]
    then
	gunzip $md/$filter/${field}_${filter}_swarp_cut.sum.fits.gz
    fi
done

ic '1 0 %1 1.0e-06 > ? ! 512 *' \
   $md/Z/${field}_Z_swarp_cut.sum.fits \
   > $md/Z/${field}_Z_swarp_cut.mask.fits
#gzip $md/Z/${field}_Z_swarp_cut.sum.fits

ic '1 0 %1 1.0e-06 > ? ! 1024 *' \
   $md/Y/${field}_Y_swarp_cut.sum.fits \
   > $md/Y/${field}_Y_swarp_cut.mask.fits
#gzip $md/Y/${field}_Y_swarp_cut.sum.fits

ic '1 0 %1 1.0e-06 > ? ! 2048 *' \
   $md/J/${field}_J_swarp_cut.sum.fits \
   > $md/J/${field}_J_swarp_cut.mask.fits
#gzip $md/J/${field}_J_swarp_cut.sum.fits

ic '1 0 %1 1.0e-06 > ? ! 4096 *' \
   $md/H/${field}_H_swarp_cut.sum.fits \
   > $md/H/${field}_H_swarp_cut.mask.fits
#gzip $md/H/${field}_H_swarp_cut.sum.fits

ic '1 0 %1 1.0e-06 > ? ! 8192 *' \
   $md/Ks/${field}_Ks_swarp_cut.sum.fits \
   > $md/Ks/${field}_Ks_swarp_cut.mask.fits
#gzip $md/Ks/${field}_Ks_swarp_cut.sum.fits

ic '%1 %2 + %3 + %4 + %5 + %6 +' \
   $md/Z/${field}_Z_swarp_cut.mask.fits \
   $md/Y/${field}_Y_swarp_cut.mask.fits \
   $md/J/${field}_J_swarp_cut.mask.fits \
   $md/H/${field}_H_swarp_cut.mask.fits \
   $md/Ks/${field}_Ks_swarp_cut.mask.fits \
   $mask \
   > $md/${field}_AW_THELI_NIR.mask.fits

rm $md/Z/${field}_Z_swarp_cut.mask.fits
rm $md/Y/${field}_Y_swarp_cut.mask.fits
rm $md/J/${field}_J_swarp_cut.mask.fits
rm $md/H/${field}_H_swarp_cut.mask.fits
rm $md/Ks/${field}_Ks_swarp_cut.mask.fits

#if [ -f $mask ] && [ ! -f $mask.gz ]
#then
#    gzip $mask
#fi
