#!/bin/bash

# Essential columns:
# 2, 8, 21, 33, 34, 35, 36, 47, 60, 73, 85, 87, 99, 100
# awk '{sum=$2+$8+$21+$33+$34+$35+$36+$47+$60+$73+$85+$87+$99+$100; if (sum<14) print $1}'

# Essential columns including SP:
# 2, 3, 8, 21, 33, 34, 35, 36, 37, 47, 60, 73, 85, 86, 87, 88, 99, 100
# awk '{sum=$2+$3+$8+$21+$33+$34+$35+$36+$37+$47+$60+$73+$85+$86+$87+$88+$99+$100; if (sum<18) print $1}'

wd=$1

tile=`basename $wd`
xxx=`echo $tile|cut -d "." -f 2`
yyy=`echo $tile|cut -d "." -f 3`

tileid=$xxx.$yyy

# 1; tile ID

echo -n $tile" "

# 2, 3; Check CONVERT

if [ -s $wd/CFIS.$tileid.r.ldac.cat ]
then
    echo -n 1" "
else
    echo -n 0" "
fi


if [ -s $wd/CFIS_SP.$tileid.r.ldac.cat ]
then
    echo -n 1" "
else
    echo -n 0" "
fi


# 4, 5, 6; Check SDSSPREP, PSPREP, and ZPREP

if [ -s $wd/SDSS/${tile}_sdssdr10_stars.cat ]
then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/PS/${tile}_PS1-DR2.cat ]
then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/specz/${tile}_redshifts-2024-01-04.cat ]
then
    echo -n 1" "
else
    echo -n 0" "
fi


# 7-19 (u), 20-32 (g), 33-45 (r), 46-58 (i), 59-71 (z), 72-84 (z2); Check PREPARE, GAUSSIANISE, GAAP, and COMPTILE/COMPTILEPS/COMPTILEPOSTMERGE/COMPTILEPSPOSTMERGE

for band in u g r i z z2
do
    if [ -s $wd/$band/${tile}_${band}.fits ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_${band}.weight.fits ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_${band}_smart_ggpsf.fits ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_${band}_smart_full.cat ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_SP_${band}_smart_full.cat ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_${band}_smart_full_SDSS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_${band}_smart_full_PS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_SP_${band}_smart_full_SDSS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/$band/${tile}_SP_${band}_smart_full_PS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/phot_comp_SDSS/${tile}_ugriz_SDSS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/phot_comp_SDSS/${tile}_SP_ugriz_SDSS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/phot_comp_PS/${tile}_ugriz_PS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi

    if [ -s $wd/phot_comp_PS/${tile}_SP_ugriz_PS_${band}_offset.asc ]
    then
	echo -n 1" "
    else
	echo -n 0" "
    fi
done


# 85, 86; Check MERGE

if [ -s $wd/${tile}_ugriz.cat ]
then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_SP_ugriz.cat ]
then
    echo -n 1" "
else
    echo -n 0" "
fi


# 87-98; Check BPZ, COMPTILEZ

if [ -s $wd/${tile}_ugriz_photoz_ext.cat ] #87
then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_SP_ugriz_photoz_ext.cat ] #88
then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_griz_photoz_ext.cat ] #89
then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_SP_griz_photoz_ext.cat ] #90
then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_ugriz_photoz_ext_specz_zz.txt ] #91
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_SP_ugriz_photoz_ext_specz_zz.txt ] #92
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_griz_photoz_ext_specz_zz.txt ] #93
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_SP_griz_photoz_ext_specz_zz.txt ] #94
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_ugriz_photoz_ext_SDSS_zz.txt ] #95
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_SP_ugriz_photoz_ext_SDSS_zz.txt ] #96
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_griz_photoz_ext_SDSS_zz.txt ] #97
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/${tile}_SP_griz_photoz_ext_SDSS_zz.txt ] #98
   then
    echo -n 1" "
else
    echo -n 0" "
fi


# 99, 100; Check MASK

if [ -s $wd/${tile}_ugriz.mask.fits ]
   then
    echo -n 1" "
else
    echo -n 0" "
fi

if [ -s $wd/r/${tile}_r_maskstars.reg ]
   then
    echo -n 1" "
else
    echo -n 0" "
fi

echo
