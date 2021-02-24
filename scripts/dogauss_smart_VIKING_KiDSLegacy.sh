#!/bin/bash -xv

# Script to gaussianise one VISTA chip and
# extract GaAP photometry.
#
# Input:
# - Background-subtracted VISTA chip processed by
#   Angus Wright.
# - KiDS-like 4-band catalogue from AstroWise.
# - Star catalogue covering the chip.
#
# Output:
# - GaAP photometric catalogue for input chip.
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2017-02-18 V1.0

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-1.6.1/bin/Linux_64/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/lib/
set -e 
#}}}

### command line parameters

wd=$1                 # work directory
original_image=$2     # VISTA chip
gaussianised_image=$3 # VISTA chip
phot_cat=$4           # photometric input catalogue
phot_cat_RA=$5        # RA key in photometric catalogue
phot_cat_Dec=$6       # Dec key in photometric catalogue
gaap_dir=${7}         # path to the GAaP code
band=${8}             # VISTA band of the chip, i.e. Z, Y, J, H, or Ks

### need to go to work directory as GaAP assumes
### hard-coded links (e.g. inimage.fits)
orig_dir=`pwd`

cp @RUNROOT@/@CONFIGPATH@/default.* $wd/
cd $wd

gaussianised_image_dir=`dirname $gaussianised_image`
gaussianised_image_base=`basename $gaussianised_image .fits`

original_image_dir=`dirname $original_image`
original_image_base=`basename $original_image .fits`
original_image_weight=${original_image_dir}/${original_image_base}.weight.fits

### Dimensions of the image
NAXIS1=`dfits $original_image | \
  fitsort -d NAXIS1 | gawk '{print $2}'` 
NAXIS2=`dfits $original_image | \
  fitsort -d NAXIS2 | gawk '{print $2}'` 

### Sky coordinates of the corners
minmin_hms=`xy2sky $original_image 0 0`
maxmax_hms=`xy2sky $original_image $NAXIS1 $NAXIS2`
RAmin_hms=`echo $maxmax_hms | gawk '{print $1}'`
RAmax_hms=`echo $minmin_hms | gawk '{print $1}'`
Decmax_dms=`echo $maxmax_hms | gawk '{print $2}'`
Decmin_dms=`echo $minmin_hms | gawk '{print $2}'`
RAmin=`hmstodecimal $RAmin_hms | gawk '{print $1}'`
RAmax=`hmstodecimal $RAmax_hms | gawk '{print $1}'`
Decmin=`dmstodecimal $Decmin_dms | gawk '{print $1}'`
Decmax=`dmstodecimal $Decmax_dms | gawk '{print $1}'`

### Names for the catalogues that are cut to the image dimensions
phot_cat_image=$wd/${original_image_base}.cat

#################
#################

echo EXTRACTING GAaP photometry

### Check for RA=0 fields

RA0=`echo $RAmin $RAmax | awk '{if ($1>$2) print 1; else print 0}'`

### Filter object catalogue to image dimensions.
if [ $RA0 -eq 0 ]
then
    ldacfilter -i $phot_cat -o $phot_cat_image \
		    -t OBJECTS \
		    -c "((($phot_cat_RA>$RAmin)AND($phot_cat_RA<=$RAmax))\
AND($phot_cat_Dec>$Decmin))AND($phot_cat_Dec<=$Decmax);"
else
    echo ldacfilter -i $phot_cat -o $phot_cat_image \
		    -t OBJECTS \
		    -c "((($phot_cat_RA>$RAmin)OR($phot_cat_RA<=$RAmax))\
AND($phot_cat_Dec>$Decmin))AND($phot_cat_Dec<=$Decmax);"
    ldacfilter -i $phot_cat -o $phot_cat_image \
		    -t OBJECTS \
		    -c "((($phot_cat_RA>$RAmin)OR($phot_cat_RA<=$RAmax))\
AND($phot_cat_Dec>$Decmin))AND($phot_cat_Dec<=$Decmax);"
fi    

### Main GAaP script - photometric catalogue
tcsh $gaap_dir/gaap-corr.csh \
    $phot_cat_image \
    $original_image \
    $gaussianised_image \
    $original_image_weight \
    0.7 \
    2.0 \
    $gaussianised_image_dir/${original_image_base}_smart_ker.map \
    $wd/${original_image_base}_smart.gaap \
    $gaap_dir \
    $phot_cat_RA \
    $phot_cat_Dec

### Main GAaP script - photometric catalogue - minaper=1.0

phot_cat_image_minaper1p0=$wd/${original_image_base}_minaper1p0.cat

ln -sf $phot_cat_image $phot_cat_image_minaper1p0

tcsh $gaap_dir/gaap-corr.csh \
    $phot_cat_image_minaper1p0 \
    $original_image \
    $gaussianised_image \
    $original_image_weight \
    1.0 \
    2.0 \
    $gaussianised_image_dir/${original_image_base}_smart_ker.map \
    $wd/${original_image_base}_smart_minaper1p0.gaap \
    $gaap_dir \
    $phot_cat_RA \
    $phot_cat_Dec

rm orders.par starsused.txt kerpos.ps fitkermap.ps \
   kersticks.ps $phot_cat_image $phot_cat_image_minaper1p0 default.*

mv cov.fits   $wd/${original_image_base}_smart_cov.fits
mv covsh.fits $wd/${original_image_base}_smart_covsh.fits

cd $orig_dir
