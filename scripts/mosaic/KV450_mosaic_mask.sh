#!/bin/bash

orig_dir=`pwd`

md=/vol/fohlen12/data1/hendrik/KiDS/VIKING_2017-04-12/

wd=$md/mosaics

test ! -d $wd && mkdir $wd

### First create mosaic sum images

for region in G9 G12 G15 G23 GS
do
    case $region in
    	G9)  center="1.352002201283E+02,5.000296878505E-01"; size="1000,500";
	     RAmin=127.; RAmax=143.; Decmin=-2.5; Decmax=3.;;
    	G12) center="1.735496508914E+02,2.971869376966E-05"; size="3500,500";
	     RAmin=149.; RAmax=200.; Decmin=-3.; Decmax=3.;;
    	G15) center="2.220001225439E+02,2.971869376966E-05"; size="2500,500";
	     RAmin=203.; RAmax=239.5; Decmin=-3.; Decmax=3.;;
    	G23) center="3.441042286673E+02,-3.164904329627E+01"; size="2100,700";
	     RAmin=325.; RAmax=6.; Decmin=-35.; Decmax=-27.;;
    	GS)  center="3.599996384587E+01,-3.214900445835E+01"; size="2000,500";
	     RAmin=15.; RAmax=55.; Decmin=-35.; Decmax=-27.;;
    esac
    for band in Z Y J H Ks
    do
    	
    	file_list=""
    	while read line
    	do
    	    field=`echo $line | awk '{print $2}'`
    	    KV450_field=`grep -c $field ../KV450_fields.txt`
    	    if [ -f $md/$field/$band/${field}_${band}_swarp.sum.fits ] && [ $KV450_field -eq 1 ]
    	    then
    		RAmin=`grep $field  ../KIDS_ra_dec_cuts.txt | awk '{print $2}'`
    		RAmax=`grep $field  ../KIDS_ra_dec_cuts.txt | awk '{print $3}'`
    		Decmin=`grep $field ../KIDS_ra_dec_cuts.txt | awk '{print $4}'`
    		Decmax=`grep $field ../KIDS_ra_dec_cuts.txt | awk '{print $5}'`
    		file_list=$file_list" "$md/$field/$band/${field}_${band}_swarp_cut.sum.fits
    		#echo python add_WCS_cuts_to_sum_image.py \
    		#       $md/$field/$band/${field}_${band}_swarp.sum.fits \
    		#       $md/$field/$band/${field}_${band}_swarp_cut.sum.fits \
    		#       $RAmin $RAmax $Decmin $Decmax
    	    fi
    	done < ../${region}.txt
    	###echo $file_list
    	#resolution=60.0
    	#acc=arcmin
    	resolution=60.0
    	acc=arcmin
    	size2=`echo $size | awk 'BEGIN{FS=",";OFS=","}{print $1*60/'$resolution',$2*60/'$resolution'}'`
    	projection=AIT
    	./create_mosaic_sum.sh $wd/ $region $band $resolution $projection $center $size2 $file_list
    	#ic '1 0 %1 1.0e-06 > ?' $wd/${region}_${band}.16bit.$acc.$projection.sum.fits \
    	#> $wd/${region}_${band}.16bit.$acc.$projection.01.fits
    done
    
    #ic '%1 %2 + %3 + %4 + %5 +' \
    #   $wd/${region}_{Z,Y,J,H,Ks}.16bit.arcmin.AIT.01.fits \
    #   > $wd/${region}_ZYJHKs.16bit.arcmin.AIT.05.fits
    #ic '1 0 %1 5 == ?' \
    #   $wd/${region}_ZYJHKs.16bit.arcmin.AIT.05.fits \
    #   > $wd/${region}_ZYJHKs.16bit.arcmin.AIT.01.fits
    #gunzip -c /vol/fohlen11/fohlen11_1/hendrik/data/KiDS/masks/V0.5.9A/${region}.16bit.arcmin.AIT.reg2.fits.gz \
    #	   > $wd/${region}.16bit.arcmin.AIT.reg2.fits
    #ic '%1 ! 32 *' \
    #   $wd/${region}_ZYJHKs.16bit.arcmin.AIT.01.fits \
    #   > $wd/${region}_ZYJHKs.16bit.arcmin.AIT.320.fits
    #ic '%1 %2 +' \
    #   $wd/${region}.16bit.arcmin.AIT.reg2.fits \
    #   $wd/${region}_ZYJHKs.16bit.arcmin.AIT.320.fits \
    #   > $wd/${region}_ugriZYJHKs.16bit.arcmin.AIT.reg2.fits
    #rm $wd/${region}.16bit.arcmin.AIT.reg2.fits $wd/${region}_ZYJHKs.16bit.arcmin.AIT.320.fits
    
    python ./determine_eff_area.py \
    	$wd/${region}_ugriZYJHKs.16bit.arcmin.AIT.reg2.fits \
    	$wd/${region}_KV450_footprint.png \
    	$RAmin $RAmax $Decmin $Decmax \
    	$region \
    	$wd/${region}_KV450_footprint.fits
done

#cd $wd
#
#$orig_dir/../swarp \
#    $wd/G9_KV450_footprint.fits \
#    $wd/G12_KV450_footprint.fits \
#    $wd/G15_KV450_footprint.fits \
#    -CENTER_TYPE MANUAL \
#    -CENTER 185.0,2.971869376966E-05 \
#    -IMAGE_SIZE 7000,500 \
#    -PROJECTION_TYPE AIT \
#    -BACK_TYPE MANUAL \
#    -COMBINE_TYPE MIN \
#    -FSCALASTRO_TYPE NONE \
#    -HEADER_ONLY N \
#    -PIXELSCALE_TYPE MANUAL \
#    -PIXEL_SCALE 60.0 \
#    -IMAGEOUT_NAME $wd/KiDS-N_KV450_footprint.fits \
#    -WEIGHTOUT_NAME $wd/KiDS-N_KV450_footprint.weight.fits \
#    -RESAMPLING_TYPE NEAREST \
#    -OVERSAMPLING 1
#
#$orig_dir/../swarp \
#    $wd/G23_KV450_footprint.fits \
#    $wd/GS_KV450_footprint.fits \
#    -CENTER_TYPE MANUAL \
#    -CENTER 10.24,-35.0 \
#    -IMAGE_SIZE 4500,1000 \
#    -PROJECTION_TYPE AIT \
#    -BACK_TYPE MANUAL \
#    -COMBINE_TYPE MIN \
#    -FSCALASTRO_TYPE NONE \
#    -HEADER_ONLY N \
#    -PIXELSCALE_TYPE MANUAL \
#    -PIXEL_SCALE 60.0 \
#    -IMAGEOUT_NAME $wd/KiDS-S_KV450_footprint.fits \
#    -WEIGHTOUT_NAME $wd/KiDS-S_KV450_footprint.weight.fits \
#    -RESAMPLING_TYPE NEAREST \
#    -OVERSAMPLING 1
#
#$orig_dir/../swarp \
#    $wd/G9_KV450_footprint.fits \
#    $wd/G12_KV450_footprint.fits \
#    $wd/G15_KV450_footprint.fits \
#    -CENTER_TYPE MANUAL \
#    -CENTER 185.0,2.971869376966E-05 \
#    -IMAGE_SIZE 42000,3000 \
#    -PROJECTION_TYPE AIT \
#    -BACK_TYPE MANUAL \
#    -COMBINE_TYPE MIN \
#    -FSCALASTRO_TYPE NONE \
#    -HEADER_ONLY N \
#    -PIXELSCALE_TYPE MANUAL \
#    -PIXEL_SCALE 10.0 \
#    -IMAGEOUT_NAME $wd/KiDS-N_KV450_footprint.10arcs.fits \
#    -WEIGHTOUT_NAME $wd/KiDS-N_KV450_footprint.10arcs.weight.fits \
#    -RESAMPLING_TYPE NEAREST \
#    -OVERSAMPLING 1
#
#$orig_dir/../swarp \
#    $wd/G23_KV450_footprint.fits \
#    $wd/GS_KV450_footprint.fits \
#    -CENTER_TYPE MANUAL \
#    -CENTER 10.24,-35.0 \
#    -IMAGE_SIZE 27000,6000 \
#    -PROJECTION_TYPE AIT \
#    -BACK_TYPE MANUAL \
#    -COMBINE_TYPE MIN \
#    -FSCALASTRO_TYPE NONE \
#    -HEADER_ONLY N \
#    -PIXELSCALE_TYPE MANUAL \
#    -PIXEL_SCALE 10.0 \
#    -IMAGEOUT_NAME $wd/KiDS-S_KV450_footprint.10arcs.fits \
#    -WEIGHTOUT_NAME $wd/KiDS-S_KV450_footprint.10arcs.weight.fits \
#    -RESAMPLING_TYPE NEAREST \
#    -OVERSAMPLING 1
