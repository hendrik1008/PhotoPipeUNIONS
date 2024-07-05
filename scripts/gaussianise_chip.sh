#!/bin/bash -xv

# Script to gaussianise one VISTA chip
#
# Input:
# - Background-subtracted VISTA chip processed by
#   Angus Wright.
#
# Output:
# - Gaussianised image
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2018-01-22 V1.0

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/:@RUNROOT@/INSTALL/anaconda2/lib/
set -e
#}}}

### command line parameters

wd=$1                # work directory
measurement_image=$2 # VISTA chip
gaap_dir=${3}       # path to the GAaP code
band=${4}           # VISTA band of the chip, i.e. Z, Z, J, H, or Ks
tile=${5}

test ! -d $wd && mkdir $wd

### need to go to work directory as GaAP assumes
### hard-coded links (e.g. inimage.fits)
orig_dir=`pwd`

cd $wd

measurement_image_dir=`dirname $measurement_image`
measurement_image_base=`basename $measurement_image .fits`
measurement_image_weight=${measurement_image_dir}/${measurement_image_base}.weight.fits
measurement_image_flag=${measurement_image_dir}/${measurement_image_base}.flag.fits

### Dimensions of the image
NAXIS1=`dfits $measurement_image | \
fitsort -d NAXIS1 | gawk '{print $2}'` 
NAXIS2=`dfits $measurement_image | \
fitsort -d NAXIS2 | gawk '{print $2}'` 

### Sky coordinates of the corners
minmin_hms=`xy2sky $measurement_image 0 0`
maxmax_hms=`xy2sky $measurement_image $NAXIS1 $NAXIS2`
RAmin_hms=`echo $maxmax_hms | gawk '{print $1}'`
RAmax_hms=`echo $minmin_hms | gawk '{print $1}'`
Decmax_dms=`echo $maxmax_hms | gawk '{print $2}'`
Decmin_dms=`echo $minmin_hms | gawk '{print $2}'`
RAmin=`hmstodecimal $RAmin_hms | gawk '{print $1}'`
RAmax=`hmstodecimal $RAmax_hms | gawk '{print $1}'`
Decmin=`dmstodecimal $Decmin_dms | gawk '{print $1}'`
Decmax=`dmstodecimal $Decmax_dms | gawk '{print $1}'`

### New star catalogue based on the image itself ###
# saturation levels
case $band in
    "u")  sat_level=`dfits_theli $measurement_image | fitsort_theli -d SATURATE|awk '{print $2*0.5}'`; ZP=30;;
    "g")  sat_level=1000; ZP=27;;
    "r")  sat_level=`dfits_theli $measurement_image | fitsort_theli -d SATURATE|awk '{print $2*0.5}'`; ZP=30;;
    "i")  sat_level=`dfits_theli $measurement_image | fitsort_theli -d SATURATE|awk '{print $2*100}'`; ZP=30;; # HACK: not sure about the saturation level
    "z")  sat_level=1000; ZP=27;; # HACK: not sure about any of this
esac

if [ ! -f $wd/${measurement_image_base}_smart_ggpsf.fits ]
then
    cp @RUNROOT@/@CONFIGPATH@/default* $wd/
    if [ -s $measurement_image_flag ]
    then
	sex $measurement_image \
    	    -WEIGHT_IMAGE $measurement_image_weight \
    	    -WEIGHT_TYPE MAP_WEIGHT\
    	    -CATALOG_NAME $wd/${measurement_image_base}_cat.asc \
    	    -DETECT_THRESH 10 -SATUR_LEVEL $sat_level \
    	    -MAG_ZEROPOINT $ZP \
    	    -FLAG_IMAGE $measurement_image_flag \
	    -FLAG_TYPE MAX \
	    -PARAMETERS_NAME default_flags.param
    else
	sex $measurement_image \
    	    -WEIGHT_IMAGE $measurement_image_weight \
    	    -WEIGHT_TYPE MAP_WEIGHT\
    	    -CATALOG_NAME $wd/${measurement_image_base}_cat.asc \
    	    -DETECT_THRESH 10 -SATUR_LEVEL $sat_level \
    	    -MAG_ZEROPOINT $ZP
    fi    
    rm $wd/default.*

    fmax=`awk '{if ($10==0 && $3<1e6) print $3}' $wd/${measurement_image_base}_cat.asc | sort -gr |head -1`
    rad1=`awk '{if ($3>'$fmax'/30. && $10==0) print $5}' $wd/${measurement_image_base}_cat.asc|$gaap_dir/kk/mode | awk '{printf "%f\n",$1}'`
    fmax=`awk '{if ($10==0 && $5<'$rad1'+0.5) print $3}' $wd/${measurement_image_base}_cat.asc | sort -gr |head -1`
    rad2=`awk '{if ($3>'$fmax'/30. && $5<'$rad1'+0.5 && $10==0) print $5}' $wd/${measurement_image_base}_cat.asc|$gaap_dir/kk/mode | awk '{printf "%f\n",$1}'`
    #awk '{if ($3>'$fmax'/30. && $5<'$rad2'+0.3 && $5>'$rad2'-0.3 && $10==0) print $0}' $wd/${measurement_image_base}_cat.asc > $wd/${measurement_image_base}_star_cat_GAaP.asc
    awk '{if ($3>'$fmax'/30. && $5<'$rad2'+0.3 && $5>'$rad2'/2.0 && $10==0) print $0}' $wd/${measurement_image_base}_cat.asc > $wd/${measurement_image_base}_star_cat_GAaP.asc
    echo $rad1 $rad2 $fmax `wc -l $wd/${measurement_image_base}_star_cat_GAaP.asc`
   
   ### Start the GaAP processing
   rm -f inimage.fits
   ln -sf $measurement_image inimage.fits
   
   if [ ! -e orders.par ]; then
   echo 8 > orders.par
   echo 3 >> orders.par
   fi
   
   ### Gaussianise the image
   tcsh $gaap_dir/doittwk.csh \
       $measurement_image \
       $wd/${measurement_image_base}_star_cat_GAaP.asc \
       -1 \
       $gaap_dir \
       $wd/${measurement_image_base}_smart_ker.map \
       $wd/${measurement_image_base}_smart_ggpsf.fits
   
   ### make maps of the PSF of the Gaussianized images
   rm -f inimage.fits
   ln -sf $wd/${measurement_image_base}_smart_ggpsf.fits inimage.fits
   
   echo MAKING SHAPELET EXPANSION FOR GAUSSIANIZED IMAGE
   $gaap_dir/kk/bigim/psfcat2sherr_2 \
       < $wd/${measurement_image_base}_star_cat_GAaP.asc \
       > $wd/${measurement_image_base}_smart_ggpsf.sh
   
   echo FITTING MAP TO GAUSSIANIZED PSF
   $gaap_dir/kk/bigim/fitpsfmap2 \
       < $wd/${measurement_image_base}_smart_ggpsf.sh \
       > $wd/${measurement_image_base}_smart_ggpsf.map
   
   $gaap_dir/kk/showpsfmapcol \
       < $wd/${measurement_image_base}_smart_ggpsf.map
   mv -f psfmap.ps $wd/${measurement_image_base}_smart_ggpsf_map.ps
   
   echo PLOTTING ASTROMETRIC RESIDUALS FOR GAUSSIANIZED IMAGE
   $gaap_dir/kk/bigim/showdxdy \
       < $wd/${measurement_image_base}_smart_ggpsf.sh
   
   echo PLOTTING STICK PLOTS FOR GAUSSIANIZED IMAGE
   $gaap_dir/kk/bigim/fitpsfmap2 \
       < $wd/${measurement_image_base}_smart_ggpsf.sh \
       > /dev/null
   
   rm noise-est.ps  #$wd/${measurement_image_base}_smart_ggnoise-est.ps
   mv fitpsfmap.ps  $wd/${measurement_image_base}_smart_ggfitpsfmap.ps
   mv psfstarpos.ps $wd/${measurement_image_base}_smart_ggpsfstarpos.ps
   rm psfstars.ps   #$wd/${measurement_image_base}_smart_ggpsfstars.ps
   mv starsused.txt $wd/${measurement_image_base}_smart_ggstarsused.txt
   mv psfsticks.ps  $wd/${measurement_image_base}_smart_ggpsfsticks.ps
   mv dxdy.ps       $wd/${measurement_image_base}_smart_ggdxdy.ps
   mv checkgpsf2d_pages.ps $wd/${measurement_image_base}_smart_checkgpsf2d_pages.ps
   
   rm -f inimage.fits
fi 

###############

awk 'BEGIN{minx=10000;maxx=0;miny=10000;maxy=0}
    !/#/{
    if ($1>maxx) maxx=$1
    if ($1<minx) minx=$1
    if ($2>maxy) maxy=$2
    if ($2<miny) miny=$2
    }
    END{
    print "#minx, maxx, miny, maxy"
    print minx, maxx, miny, maxy
    }' $wd/${measurement_image_base}_cat.asc \
	> $wd/${measurement_image_base}_cat_stats.txt

awk 'BEGIN{minx=10000;maxx=0;miny=10000;maxy=0}
    !/#/{
    if ($1>maxx) maxx=$1
    if ($1<minx) minx=$1
    if ($2>maxy) maxy=$2
    if ($2<miny) miny=$2
    }
    END{
    print "#minx, maxx, miny, maxy"
    print minx, maxx, miny, maxy
    }' $wd/${measurement_image_base}_star_cat_GAaP.asc \
	> $wd/${measurement_image_base}_star_cat_GAaP_stats.txt

###############
###############

ln -sf $wd/${measurement_image_base}_ker.in gpsfsig.dat

rm -f inimage.fits
ln -sf $wd/${measurement_image_base}_smart_ggpsf.fits inimage.fits
$gaap_dir/kk/shummary < $wd/${measurement_image_base}_smart_ggpsf.sh | \
    sort -g -k 8 | gawk '$7==0' | tail -200 | $gaap_dir/kk/bigim/checkgpsf2d
test -f $wd/checkgpsf2d_pages.ps && mv $wd/checkgpsf2d_pages.ps $wd/${measurement_image_base}_smart_checkgpsf2d_pages.ps
test -f $wd/checkgpsf2d.ps && mv $wd/checkgpsf2d.ps $wd/${measurement_image_base}_smart_checkgpsf2d.ps

rm -f orders.par
rm -f bgnoise.dat
rm -f gpsfsig.dat
rm -f inimage.fits

bash @RUNROOT@/@SCRIPTPATH@/gaussianise_QC.sh $wd $tile $band

cd $orig_dir
