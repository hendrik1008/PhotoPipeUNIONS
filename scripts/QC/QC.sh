#!/bin/bash

export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/

bd=/arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/
md=$bd/../UNIONS5000_QC/

test ! -d $md/ && mkdir $md/

###############################
### Survey-wide QC per band ###
###############################

#for band in z #g r i # u z
#do
#    ###########################
#    ### Raw PSF star counts ###
#    ###########################
#    wc $bd/UNIONS.*/$band/*cat_GAaP.asc \
#       | sed '$d' \
#       > $md/UNIONS5000_${band}_star_cat_GAaP_wc.asc
#    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/star_count.py \
#    	   $md/UNIONS5000_${band}_star_cat_GAaP_wc $band
#    
#    #############################
#    #### Used PSF star counts ###
#    #############################
#    wc $bd/UNIONS.*/$band/*_smart_ggstarsused.txt \
#       | sed '$d' \
#       > $md/UNIONS5000_${band}_starsused.asc
#    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/star_count.py \
#    	   $md/UNIONS5000_${band}_starsused $band
#    
#    ###############################
#    ### Fraction of used stars  ###
#    ###############################
#    echo "#tile stars used-stars fraction" > \
#    	 $md/UNIONS5000_${band}_starfrac.asc
#    while read tile
#    do
#    	stars=$bd/$tile/$band/${tile}_${band}_star_cat_GAaP.asc
#    	starsused=$bd/$tile/$band/${tile}_${band}_smart_ggstarsused.txt
#    	if [ -s $stars ] && [ -s $starsused ]
#    	then
#    	    n=`wc $stars | awk '{print $1}'`
#    	    nused=`wc $starsused | awk '{print $1}'`
#    	    echo $n $nused | awk '{if ($1!=0) print "'$tile'", $1, $2, $2/$1}'
#    	fi
#    done < @RUNROOT@/ugri_tiles.txt >> $md/UNIONS5000_${band}_starfrac.asc
#    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/starfrac.py \
#    	   $md/UNIONS5000_${band}_starfrac $band
#    
#    ####################################################
#    ### Extent of PSF star vs. full catalogue in x/y ###
#    ####################################################
#    echo "#tile x-range y-range x-range_stars y-range_stars" > \
#    	 $md/UNIONS5000_${band}_xy_range.asc
#    while read tile
#    do
#    	stats=$bd/$tile/$band/${tile}_${band}_cat_stats.txt
#    	stats_stars=$bd/$tile/$band/${tile}_${band}_star_cat_GAaP_stats.txt
#    	if [ -s $stats ] && [ -s $stats_stars ]
#    	then
#    	    
#    	    paste $stats $stats_stars | awk '{if (NR>1) print "'$tile'", $2-$1,$4-$3,$6-$5,$8-$7}'
#    	fi
#    done < @RUNROOT@/ugri_tiles.txt >> $md/UNIONS5000_${band}_xy_range.asc
#    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/xy_range.py \
#    	   $md/UNIONS5000_${band}_xy_range $band
#    
#    ##############################
#    ### Width of stellar locus ###
#    ##############################
#    echo "#tile SL-mean SL-std" > \
#    	 $md/UNIONS5000_${band}_SL.asc
#    while read tile
#    do
#    	stats=$bd/$tile/$band/${tile}_${band}_star_cat_GAaP_SL.txt
#    	if [ -s $stats ]
#    	then
#    	    cat $stats | awk '{print "'$tile'", $1, $2, $2/$1}'
#    	fi
#    done < @RUNROOT@/ugri_tiles.txt >> $md/UNIONS5000_${band}_SL.asc
#    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/SL_dist.py \
#    	   $md/UNIONS5000_${band}_SL $band
#done


#############################
### Select "clean" fields ###
#############################
#
#bash @RUNROOT@/@SCRIPTPATH@/QC/exclude_fields.sh
#bash @RUNROOT@/@SCRIPTPATH@/QC/exclude_fields_photZP.sh


###########################
### r-band numbercounts ###
###########################

#bash @RUNROOT@/@SCRIPTPATH@/QC/r_numbercounts.sh

#################################################
### Summarise photo-z stats (bright vs. SDSS) ###
#################################################

ulimit -n 20000

#for suffix in "_SP" #""
#do
#    for filters in  ugriz #ugri #ugriz2 #ugri #uriz 
#    do
#	filters2=$filters
#	if [ $filters = "ugriz2" ]
#	then
#	    filters2=ugriz
#	fi
#	for calib in "" # "_recalibSDSSplus" # "_recalibSDSS" # "" #"_recalib" "_recalibplus"
#	do
#	    for ending in "" # "_cleanZP" # "_cleanZPi" # ""
#	    do
#		for survey in specz #SDSS
#		do
#		    pointings=${filters}_tiles$ending.txt
#		    FILES=""
#		    while read field
#		    do
#			FILES=$FILES" "$bd/$field/${field}${suffix}_${filters2}_photoz${calib}_ext_${survey}.cat
#		    done<@RUNROOT@/$pointings
#		    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#							   $md/UNIONS5000${suffix}_${filters}${calib}_${survey}${ending}.cat \
#							   OBJECTS \
#							   $FILES
#		    if [ -f  $md/UNIONS5000${suffix}_${filters}${calib}_${survey}${ending}.cat ]
#		    then
#			#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#			    #	   $md/UNIONS5000_${filters}${calib}_${survey}${ending}.cat \
#			    #	   z_spec_spec \
#			    #	   Z_B \
#			    #	   MAG_AUTO \
#			    #	   2.0 \
#			    #	   24.0 \
#			    #	   $filters"-"$calib"-"$ending"-"$survey \
#			    #	   $md/UNIONS5000_${filters}${calib}_${survey}${ending}_zz
#			@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot_weighted.py \
#							       $md/UNIONS5000${suffix}_${filters}${calib}_${survey}${ending}.cat \
#							       z_spec_spec \
#							       Z_B \
#							       MAG_AUTO \
#							       2.0 \
#							       24.0 \
#							       ${filters}${suffix} \
#							       $md/UNIONS5000${suffix}_${filters}${calib}_${survey}${ending}_zzw \
#							       $md/UNIONS100_ugriz_photoz_ext_nc.txt
#		    fi
#		done
#	    done
#	done
#    done
#done

#############################
#### Photo-z matrix plots ###
#############################
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugri_specz.cat \
#       $md/UNIONS5000_ugriz_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri vs. ugriz photo-z" \
#       $md/UNIONS5000_ugri_vs_ugriz

@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot_weighted.py \
       $md/UNIONS5000_ugri_specz.cat \
       $md/UNIONS5000_ugriz_specz.cat \
       z_spec_spec \
       Z_B \
       MAG_AUTO \
       10. \
       25. \
       "ugri vs. ugriz photo-z" \
       $md/UNIONS5000_ugri_vs_ugriz_weighted \
       $md/UNIONS100_ugriz_photoz_ext_nc.txt

@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot_weighted.py \
       $md/UNIONS5000_ugri_specz.cat \
       $md/UNIONS5000_ugriz_specz.cat \
       z_spec_spec \
       Z_B \
       MAG_AUTO \
       10. \
       23. \
       "ugri vs. ugriz photo-z r<23" \
       $md/UNIONS5000_ugri_vs_ugriz_weighted_rlt23 \
       $md/UNIONS100_ugriz_photoz_ext_nc.txt

#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugri_specz_cleanZP.cat \
#       $md/UNIONS5000_ugriz_specz_cleanZP.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri vs. ugriz photo-z" \
#       $md/UNIONS5000_ugri_vs_ugriz_cleanZP

#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugri_specz.cat \
#       $md/UNIONS5000_ugri_recalib_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri photo-z raw vs. recalib" \
#       $md/UNIONS5000_ugri_raw_vs_recalib
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugri_specz.cat \
#       $md/UNIONS5000_ugri_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri photo-z raw vs. recalibplus" \
#       $md/UNIONS5000_ugri_raw_vs_recalibplus
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugri_recalib_specz.cat \
#       $md/UNIONS5000_ugri_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri photo-z recalib vs. recalibplus" \
#       $md/UNIONS5000_ugri_recalib_vs_recalibplus
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugriz_specz.cat \
#       $md/UNIONS5000_ugriz_recalib_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugriz photo-z raw vs. recalib" \
#       $md/UNIONS5000_ugriz_raw_vs_recalib
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugri_recalib_specz.cat \
#       $md/UNIONS5000_ugriz_recalib_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "recalibrated photo-z, ugri vs. ugriz" \
#       $md/UNIONS5000_ugri_recalib_vs_ugriz_recalib
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugriz_specz.cat \
#       $md/UNIONS5000_ugriz_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugriz photo-z raw vs. recalibplus" \
#       $md/UNIONS5000_ugriz_raw_vs_recalibplus
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugriz_recalib_specz.cat \
#       $md/UNIONS5000_ugriz_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugriz photo-z recalib vs. recalibplus" \
#       $md/UNIONS5000_ugriz_recalib_vs_recalibplus
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_ugri_recalibplus_specz.cat \
#       $md/UNIONS5000_ugriz_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "recalibrated plus photo-z, ugri vs. ugriz" \
#       $md/UNIONS5000_ugri_recalibplus_vs_ugriz_recalib


##########################################################################
### Summarise photo-z stats (deep vs. DEEP2/3, VVDS, zCOSMOS, GOODS-N) ###
##########################################################################

#ulimit -n 20000
#
#for filters in uri ugri
#do
#    FILES=""
#    while read field
#    do
#	FILES=$FILES" "$bd/$field/${field}_${filters}_photoz_ext_specz.cat
#    done<@RUNROOT@/EGS_COSMOS_VVDS_GOODS-N_ugri.txt
#
#
#    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#	   $md/UNIONS5000_DEEP_$filters.cat \
#	   OBJECTS \
#	   $FILES
#
#    @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#	   $md/UNIONS5000_DEEP_$filters.cat \
#	   z_spec_spec \
#	   Z_B \
#	   MAG_AUTO \
#	   9.9 \
#	   $filters"---EGS/COSMOS/VVDS/GOODS-N" \
#	   $md/UNIONS5000_DEEP_${filters}_zz
#done

#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_DEEP_uri.cat \
#       $md/UNIONS5000_DEEP_ugri.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "uri vs. ugri" \
#       $md/UNIONS5000_ugr_vs_ugri

#ulimit -n 20000
#
#FILES=""
#while read field
#do
#    n=`grep -c $field @RUNROOT@/EGS_COSMOS_VVDS_GOODS-N_ugri.txt`
#    if [ $n -eq 0 ]
#    then
#	FILES=$FILES" "$bd/$field/${field}_ugri_photoz_ext_specz.cat
#    fi
#done<@RUNROOT@/ugri_tiles.txt
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       $md/UNIONS5000_noDEEP.cat \
#       OBJECTS \
#       $FILES

#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       $md/UNIONS5000_noDEEP.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       9.9 \
#       "ugri---survey w/o deep fields" \
#       $md/UNIONS5000_noDEEP_zz
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       $md/UNIONS5000_noDEEP_plus_DEEP.cat \
#       OBJECTS \
#       $md/UNIONS5000_noDEEP.cat \
#       $md/UNIONS5000_DEEP.cat
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       $md/UNIONS5000_noDEEP_plus_DEEP.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       9.9 \
#       "ugri---survey w/ deep fields" \
#       $md/UNIONS5000_noDEEP_plus_DEEP_zz

#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       $md/UNIONS5000_noDEEP.cat \
#       $md/UNIONS5000_DEEP.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri, survey vs. deep fields" \
#       $md/UNIONS5000_ugri_survey_vs_deep_fields

#######################################################
#### Summarise photo-z stats  KiDS(bright vs. SDSS) ###
#######################################################
#
#ulimit -n 10000
#FILES=""
#while read field
#do
#    file=~/KiDS/KiDS-DR5/work_KiDS-DR5/$field/${field}_ugriZYJHKs_photoz_ext_SDSS.cat
#    if [ -f $file ]
#    then
#	FILES=$FILES" "$file
#    fi
#done<~/KiDS/KiDS-DR5/KiDS-Legacy_pointings.txt
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       $md/KiDS-DR5_SDSS.cat \
#       OBJECTS \
#       $FILES
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       $md/KiDS-DR5_SDSS.cat \
#       z_spec_SDSS \
#       Z_B \
#       "KiDS-DR5 photo-z vs. SDSS spec-z" \
#       $md/KiDS-DR5_SDSS_zz.png \
#       $md/KiDS-DR5_SDSS_zz.pdf

## Summarise photo-z stats  KiDS-DR4 (bright vs. SDSS)
#
#ulimit -n 10000
#FILES=""
#while read field
#do
#    file=/net/home/fohlen11/hendrik/data/KiDS/VIKING/$field/${field}_ugriZYJHKs_photoz_ext_SDSS.cat
#    if [ -f $file ]
#    then
#	FILES=$FILES" "$file
#    fi
#done<~/KiDS/KiDS-DR5/KiDS-Legacy_pointings.txt
#
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       $md/KiDS-DR4_SDSS.cat \
#       OBJECTS \
#       $FILES

#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       $md/KiDS-DR4_SDSS.cat \
#       z_spec_SDSS \
#       Z_B \
#       "KiDS-DR4 photo-z vs. SDSS spec-z" \
#       $md/KiDS-DR4_SDSS_zz.png \
#       $md/KiDS-DR4_SDSS_zz.pdf

## Summarise photo-z stats (KiDS-DR5 vs. deep spec-z)
#
#ulimit -n 10000
#FILES=""
#while read field
#do
#    file=~/KiDS/KiDS-DR5/work_KiDS-DR5/$field/${field}_ugriZYJHKs_photoz_ext_mask_DEEP.cat
#    if [ -f $file ]
#    then
#	FILES=$FILES" "$file
#    fi
#done<~/KiDS/KiDS-DR5/KiDS-Legacy_pointings.txt
#
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       $md/KiDS-DR5_DEEP.cat \
#       OBJECTS \
#       $FILES
#
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       $md/KiDS-DR5_DEEP.cat \
#       z_spec_DEEP \
#       Z_B \
#       "KiDS-DR5 vs. all deep spec-z" \
#       $md/KiDS-DR5_DEEP_zz.png \
#       $md/KiDS-DR5_DEEP_zz.pdf
