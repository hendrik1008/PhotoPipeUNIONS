#!/bin/bash

orig_dir=`pwd`

bd=/vol/fohlen11/fohlen11_1/hendrik/data/KiDS/
md=/vol/fohlen12/data1/hendrik/KiDS/VIKING_2017-04-12/

wd=$md/mosaics

test ! -d $wd && mkdir $wd

for patch in G9 G12 G15 G23 GS
do
    file_list=""
    while read line
    do
    	field=`echo $line | awk '{print $2}'`
	#if [ ! -f $md/$field/${field}_footprint_w_wo_VIKING.png ]
	if [ $field = "KIDS_329p4_m32p1" ]
	then
	    echo $field
    	    ldacfilter -i $md/$field/${field}_rand.cat \
    		       -o $md/$field/${field}_rand_filt.cat \
    		       -t OBJECTS \
    		       -c \
    		       "(((((((((1!=(int(MASK/4)/2-int(int(MASK/4)/2))*2))\
                    AND(1!=(int(MASK/16)/2-int(int(MASK/16)/2))*2))\
                    AND(1!=(int(MASK/32)/2-int(int(MASK/32)/2))*2))\
                    AND(1!=(int(MASK/16384)/2-int(int(MASK/16384)/2))*2))\
                    AND(1!=(int(MASK/1024)/2-int(int(MASK/1024)/2))*2))\
                    AND(1!=(int(MASK/2048)/2-int(int(MASK/2048)/2))*2))\
                    AND(1!=(int(MASK/4096)/2-int(int(MASK/4096)/2))*2))\
                    AND(1!=(int(MASK/8192)/2-int(int(MASK/8192)/2))*2));"
    	    ldacfilter -i $bd/V0.5.9A/${field}_AW_THELI.random.cat \
    		       -o $md/$field/${field}_randK450_filt.cat \
    		       -t OBJECTS \
    		       -c \
    		       "((((((((1!=(int(MASK/4)/2-int(int(MASK/4)/2))*2))\
                    AND(1!=(int(MASK/16)/2-int(int(MASK/16)/2))*2))\
                    AND(1!=(int(MASK/16384)/2-int(int(MASK/16384)/2))*2))\
                    AND(1!=(int(MASK/1024)/2-int(int(MASK/1024)/2))*2))\
                    AND(1!=(int(MASK/2048)/2-int(int(MASK/2048)/2))*2))\
                    AND(1!=(int(MASK/4096)/2-int(int(MASK/4096)/2))*2))\
                    AND(1!=(int(MASK/8192)/2-int(int(MASK/8192)/2))*2));"
	    ldacrentab -i $md/$field/${field}_ugriZYJHKs_final.cat \
		       -o $md/$field/${field}_ugriZYJHKs_final.cat.tmp \
		       -t OBJECTS OBJECTS2
    	    ldacfilter -i $md/$field/${field}_ugriZYJHKs_final.cat.tmp \
    		       -o $md/$field/${field}_ugriZYJHKs_final_filt.cat \
    		       -t OBJECTS2 \
    		       -c \
    		       "(((((((((1!=(int(MASK/4)/2-int(int(MASK/4)/2))*2))\
                    AND(1!=(int(MASK/16)/2-int(int(MASK/16)/2))*2))\
                    AND(1!=(int(MASK/32)/2-int(int(MASK/32)/2))*2))\
                    AND(1!=(int(MASK/16384)/2-int(int(MASK/16384)/2))*2))\
                    AND(1!=(int(MASK/1024)/2-int(int(MASK/1024)/2))*2))\
                    AND(1!=(int(MASK/2048)/2-int(int(MASK/2048)/2))*2))\
                    AND(1!=(int(MASK/4096)/2-int(int(MASK/4096)/2))*2))\
                    AND(1!=(int(MASK/8192)/2-int(int(MASK/8192)/2))*2))\
                    AND(GAAP_Flag_ugriZYJHKs=0);"
	    rm $md/$field/${field}_ugriZYJHKs_final.cat.tmp
	    python plot_rand_vs_rand_vs_data.py \
	    	   $md/$field/${field}_randK450_filt.cat \
	    	   $md/$field/${field}_rand_filt.cat \
    	    	   $md/$field/${field}_ugriZYJHKs_final_filt.cat \
	    	   $md/$field/${field}_footprint_w_wo_VIKING.png \
	    	   $field
	fi
    	#KV450_field=`grep -c $field ../KV450_fields.txt`
    	#if [ -f $md/$field/${field}_rand_filt.cat ] && [ $KV450_field -eq 1 ]
    	#then
    	#    file_list=$file_list" "$md/$field/${field}_rand_filt.cat
    	#fi
    done < ../${patch}.txt
    #ldacpaste -i $file_list -o $wd/${patch}_rand_filt.cat
    #ldacfilter -i $md/../KV450_CATALOGUES_PATCH_V0.5.9/KV450_$patch.cat \
    #	       -o $md/../KV450_CATALOGUES_PATCH_V0.5.9/KV450_${patch}_filt.cat \
    #		   -t OBJECTS \
    #		   -c \
    #		   "(((((((((1!=(int(MASK/4)/2-int(int(MASK/4)/2))*2))\
    #                AND(1!=(int(MASK/16)/2-int(int(MASK/16)/2))*2))\
    #                AND(1!=(int(MASK/32)/2-int(int(MASK/32)/2))*2))\
    #                AND(1!=(int(MASK/16384)/2-int(int(MASK/16384)/2))*2))\
    #                AND(1!=(int(MASK/1024)/2-int(int(MASK/1024)/2))*2))\
    #                AND(1!=(int(MASK/2048)/2-int(int(MASK/2048)/2))*2))\
    #                AND(1!=(int(MASK/4096)/2-int(int(MASK/4096)/2))*2))\
    #                AND(1!=(int(MASK/8192)/2-int(int(MASK/8192)/2))*2))\
    #                AND(GAAP_Flag_ugriZYJHKs=0);"
    #python plot_rand_vs_data.py \
    #	   $wd/${patch}_rand_filt.cat \
    #	   $md/../KV450_CATALOGUES_PATCH_V0.5.9/KV450_${patch}_filt.cat \
    #	   $wd/${patch}_rand_vs_data.pdf \
    #	   $patch
done
