#!/bin/bash

orig_dir=`pwd`

wd=$1
region=$2
band=$3
pixscale=$4
projection=$5
center=$6
size=$7

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
shift

file_list=""

for file in $*
do
    #dir=`dirname $file`
    #base=`basename $file .gz`
    #echo -n gunzip $file \;
    #file_list=$file_list" "$dir/$base
    file_list=$file_list" "$file
done

echo -n mkdir $wd/${region}_${band}_${acc}_tmp$$/ \;
echo -n cd $wd/${region}_${band}_${acc}_tmp$$/ \;
echo -n $orig_dir/swarp $file_list -CENTER_TYPE MANUAL -CENTER $center -IMAGE_SIZE $size -PROJECTION_TYPE $projection -BACK_TYPE MANUAL -COMBINE_TYPE SUM -FSCALASTRO_TYPE NONE -HEADER_ONLY N -PIXELSCALE_TYPE MANUAL -PIXEL_SCALE ${pixscale} -IMAGEOUT_NAME $wd/${region}_${band}.32bit.$acc.$projection.sum.fits -RESAMPLE_DIR $wd/${region}_${band}_${acc}_tmp$$/ -WEIGHTOUT_NAME $wd/${region}_${band}.32bit.$acc.$projection.sum.weight.fits -RESAMPLING_TYPE NEAREST -OVERSAMPLING 1 \;
echo -n cd $orig_dir \;
echo -n python $orig_dir/convert_mask_32to16.py $wd/${region}_${band}.32bit.$acc.$projection.sum.fits $wd/${region}_${band}.16bit.$acc.$projection.sum.fits \;
echo -n rm -rf $wd/${region}_${band}_${acc}_tmp$$/ \;
echo -n gzip $wd/${region}_${band}.16bit.$acc.$projection.sum.fits \;
echo -n rm $wd/${region}_${band}.32bit.$acc.$projection.sum.fits \;
echo -n rm $wd/${region}_${band}.32bit.$acc.$projection.sum.weight.fits \;
echo

#for file in $*
#do
#    dir=`dirname $file`
#    base=`basename $file .gz`
#    echo -n gzip $dir/$base \;
#done
#
#echo
