#!/bin/bash

orig_dir=`pwd`

wd=$1
region=$2
pixscale=$3
projection=$4
center=$5
size=$6

case $pixscale in
    1.0)  acc=arcsec;;
    5.0)  acc=5arcs;;
    15.0) acc=15arcs;;
    30.0) acc=30arcs;;
    60.0) acc=arcmin;;
esac

shift 
shift
shift
shift
shift
shift

file_list=""

for file in $*
do
    file_list=$file_list" "$file
done

echo -n mkdir $wd/${region}_${acc}_tmp$$/ \;
echo -n cd $wd/${region}_${acc}_tmp$$/ \;
echo -n $orig_dir/swarp $file_list -CENTER_TYPE MANUAL -CENTER $center -IMAGE_SIZE $size -PROJECTION_TYPE $projection -BACK_TYPE MANUAL -COMBINE_TYPE MIN -FSCALASTRO_TYPE NONE -HEADER_ONLY N -PIXELSCALE_TYPE MANUAL -PIXEL_SCALE ${pixscale} -IMAGEOUT_NAME $wd/$region.32bit.$acc.$projection.reg.fits -RESAMPLE_DIR $wd/${region}_${acc}_tmp$$/ -WEIGHTOUT_NAME $wd/$region.32bit.$acc.$projection.reg.weight.fits -RESAMPLING_TYPE NEAREST -OVERSAMPLING 1 \;
echo -n cd $orig_dir \;
echo -n rm $wd/$region.16bit.$acc.$projection.reg.fits $wd/$region.16bit.$acc.$projection.reg.weight.fits \;
echo -n python $orig_dir/convert_mask_32to16.py $wd/$region.32bit.$acc.$projection.reg.fits $wd/$region.16bit.$acc.$projection.reg.fits \;
echo -n python $orig_dir/convert_mask_32to16.py $wd/$region.32bit.$acc.$projection.reg.weight.fits $wd/$region.16bit.$acc.$projection.reg.weight.fits \;
echo -n ic \'%2 ! 16384 \* %1 +\' $wd/$region.16bit.$acc.$projection.reg.fits $wd/$region.16bit.$acc.$projection.reg.weight.fits \> $wd/$region.16bit.$acc.$projection.reg2.fits \;
echo -n rm -rf $wd/${region}_${acc}_tmp$$/ \;
echo -n rm $wd/$region.16bit.$acc.$projection.reg2.fits.gz \;
#echo -n gzip $wd/$region.16bit.$acc.$projection.reg2.fits \;
