#!/bin/bash

md=$1

tile=`basename $md`
xxx=`echo $tile|cut -d "." -f 2`
yyy=`echo $tile|cut -d "." -f 3`
xy=$xxx.$yyy

# Check CONVERT failures.
size=0
if [ -s $md/CFIS.${xxx}.${yyy}.r.ldac.cat ]
then
    size=`ls -l $md/CFIS.${xxx}.${yyy}.r.ldac.cat|awk '{print $5}'`
fi
if [ $size -lt 20000 ]
then
    rm -rf $md/CFIS.${xxx}.${yyy}.r.ldac.cat
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_?_smart.gaap
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_?_smart_full*
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_?_smart_minaper*
    rm -rf $md/${tile}_{u,g}*.cat
    rm -rf $md/BPZ_photoz/${tile}_{u,g}*
    rm -rf $md/phot_comp*/${tile}_{u,g}*
fi

size_SP=0
if [ -s $md/CFIS_SP.${xxx}.${yyy}.r.ldac.cat ]
then
    size_SP=`ls -l $md/CFIS_SP.${xxx}.${yyy}.r.ldac.cat|awk '{print $5}'`
fi
if [ $size_SP -lt 60000 ]
then
    rm -rf $md/CFIS_SP.${xxx}.${yyy}.r.ldac.cat
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_SP_r_smart.gaap
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_SP_r_smart_full*
    rm -rf $md/{u,g,r,i,z,z2}/${tile}_SP_r_smart_minaper*
    rm -rf $md/${tile}_SP_{u,g}*.cat
    rm -rf $md/BPZ_photoz/${tile}_SP_{u,g}*
    rm -rf $md/phot_comp*/${tile}_SP_{u,g}*
fi

# Check PREPARE failures.
for band in u g r i z z2
do
    band2=$band
    if [ $band = "z" ]
    then
	band2=z1
    fi
    size=400000000
    if [ -s $md/$band/${tile}_${band}.fits ]
    then
	size=`ls -l $md/$band/${tile}_${band}.fits|awk '{print $5}'`
    else
	missing=`grep -c $xy /arc/home/hendrik/UNIONS/status_2026-06-03/r${band2}_tiles.wcs.txt`
	if [ $missing -eq 1 ]
	then
	    size=0
	fi
    fi
    sizew=0
    if [ -s $md/$band/${tile}_${band}.weight.fits ]
    then
	sizew=`ls -l $md/$band/${tile}_${band}.weight.fits|awk '{print $5}'`
    fi
    if [ $size -lt 400000000 ] || [ $sizew -lt 400000000 ]
    then
	rm -rf $md/$band
	rm -rf $md/BPZ*
	rm -rf $md/phot_comp*
	accounted=`grep -c $band2 $md/${tile}_missing_bands.txt`
	if [ $accounted -eq 0 ]
	then
	    rm -rf $md/${tile}*
	fi
    fi
done

## Check GAUSSIANISE failures.
#for band in u g r i z z2
#do
#    if [ -s $md/$band/${tile}_${band}.fits ]
#    then
#	size=0
#	if [ -s $md/$band/${tile}_${band}_smart_ggpsf.fits ]
#	then
#	    size=`ls -l $md/$band/${tile}_${band}_smart_ggpsf.fits|awk '{print $5}'`
#	fi
#	if [ $size -lt 400000000 ]
#	then
#	    rm -rf $md/$band/{*smart*,*.sky,*small*,*QC*,*cat*,*.png}
#	    rm -rf $md/${tile}*
#	    rm -rf $md/BPZ*
#	    rm -rf $md/phot_comp*
#	fi
#    fi
#done

# Check MASK failure.
if [ -s $md/${tile}_ugriz.mask.fits ]
then
    size=0
    if [ -s $md/${tile}_ugriz.mask.fits ]
    then
	size=`ls -l $md/${tile}_ugriz.mask.fits|awk '{print $5}'`
    fi
    if [ $size -lt 100 ]
    then
	rm -rf $md/${tile}_ugriz.mask.fits*
    fi
fi
