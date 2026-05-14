#!/bin/bash

md=$1

tile=`basename $md`
xxx=`echo $tile|cut -d "." -f 2`
yyy=`echo $tile|cut -d "." -f 3`

# Check CONVERT failures.
size=`ls -l $md/CFIS.${xxx}.${yyy}.r.ldac.cat|awk '{print $5}'`
size_SP=`ls -l $md/CFIS_SP.${xxx}.${yyy}.r.ldac.cat|awk '{print $5}'`
if [ $size -lt 20000 ] || [ $size_SP -lt 60000 ]
then
    rm -rf $md/CFIS.${xxx}.${yyy}.r.ldac.cat
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_r_smart.gaap
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_r_smart_full*
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_r_smart_minaper*
    rm -rf $md/CFIS_SP.${xxx}.${yyy}.r.ldac.cat
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_SP_r_smart.gaap
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_SP_r_smart_full*
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_SP_r_smart_minaper*
    rm -rf $md/${tile}*
    rm -rf $md/BPZ*
    rm -rf $md/phot_comp*
fi

# Check PREPARE failures.
for band in u g r i z z2
do
    if [ -s $md/$band/${tile}_${band}.fits ]
    then
	size=`ls -l $md/$band/${tile}_${band}.fits|awk '{print $5}'`
	sizew=`ls -l $md/$band/${tile}_${band}.weight.fits|awk '{print $5}'`
	if [ $size -lt 400000000 ] || [ $sizew -lt 400000000 ]
	then
	    rm -rf $md/$band
	    rm -rf $md/${tile}*
	    rm -rf $md/BPZ*
	    rm -rf $md/phot_comp*
	fi
    fi
done

# Check GAUSSIANISE failures.
for band in u g r i z z2
do
    if [ -s $md/$band/${tile}_${band}.fits ]
    then
	size=`ls -l $md/$band/${tile}_${band}_smart_ggpsf.fits|awk '{print $5}'`
	if [ $size -lt 400000000 ] || [ ! -s $md/$band/${tile}_${band}_smart_ggpsf.fits ]
	then
	    rm -rf $md/$band/{*smart*,*.sky,*small*,*QC*,*cat*,*.png}
	    rm -rf $md/${tile}*
	    rm -rf $md/BPZ*
	    rm -rf $md/phot_comp*
	fi
    fi
done

# Check MASK failure.
if [ -s $md/${tile}_ugriz.mask.fits ]
then
    size=`ls -l $md/${tile}_ugriz.mask.fits|awk '{print $5}'`
    if [ $size -lt 100 ]
    then
	rm -rf $md/${tile}_ugriz.mask.fits*
    fi
fi
