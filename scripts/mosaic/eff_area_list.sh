#!/bin/bash

md=/vol/fohlen12/data1/hendrik/KiDS/VIKING_2017-04-12/

orig_dir=`pwd`

cd $md

{
for field in KIDS_*
do
    KV450_field=`grep -c $field $orig_dir/../KV450_fields.txt`
    if [ $KV450_field -eq 1 ] && [ -f $field/${field}_AW_THELI_NIR.arcsec.flags.fits ]
    then
	area=`python $orig_dir/determine_eff_area.py $field/${field}_AW_THELI_NIR.arcsec.flags.fits bla 0.0 0.0 0.0 0.0 bla bla`
	echo $field $area
    fi
done
} > KV450_eff_area_list.txt

#for region in G12 #G15 G23 GS #G9
#do
#    {
#	while read line
#	do
#	    field=`echo $line | awk '{print $2}'`
#	    if [ -f $field/${field}_AW_THELI_NIR.arcsec.flags.fits ]
#	    then
#		area=`python $orig_dir/determine_eff_area.py $field/${field}_AW_THELI_NIR.arcsec.flags.fits bla 0.0 0.0 0.0 0.0 bla bla`
#		echo $field $area
#	    fi
#	done < $orig_dir/../${region}.txt
#    } > KV450_${region}_eff_area_list.txt
#done
