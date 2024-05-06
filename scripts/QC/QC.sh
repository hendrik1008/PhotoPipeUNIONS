#!/bin/bash

test ! -d @RUNROOT@/@WORKINGDIR@/QC/ && mkdir @RUNROOT@/@WORKINGDIR@/QC/

###############################
### Survey-wide QC per band ###
###############################

#for band in z #g r i # u z
#do
#    ###########################
#    ### Raw PSF star counts ###
#    ###########################
#    wc @RUNROOT@/@WORKINGDIR@/UNIONS.*/$band/*cat_GAaP.asc \
#       | sed '$d' \
#       > @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_star_cat_GAaP_wc.asc
#    python @RUNROOT@/@SCRIPTPATH@/QC/star_count.py \
#    	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_star_cat_GAaP_wc $band
#    
#    #############################
#    #### Used PSF star counts ###
#    #############################
#    wc @RUNROOT@/@WORKINGDIR@/UNIONS.*/$band/*_smart_ggstarsused.txt \
#       | sed '$d' \
#       > @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_starsused.asc
#    python @RUNROOT@/@SCRIPTPATH@/QC/star_count.py \
#    	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_starsused $band
#    
#    ###############################
#    ### Fraction of used stars  ###
#    ###############################
#    echo "#tile stars used-stars fraction" > \
#    	 @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_starfrac.asc
#    while read tile
#    do
#    	stars=@RUNROOT@/@WORKINGDIR@/$tile/$band/${tile}_${band}_star_cat_GAaP.asc
#    	starsused=@RUNROOT@/@WORKINGDIR@/$tile/$band/${tile}_${band}_smart_ggstarsused.txt
#    	if [ -s $stars ] && [ -s $starsused ]
#    	then
#    	    n=`wc $stars | awk '{print $1}'`
#    	    nused=`wc $starsused | awk '{print $1}'`
#    	    echo $n $nused | awk '{if ($1!=0) print "'$tile'", $1, $2, $2/$1}'
#    	fi
#    done < @RUNROOT@/ugri_tiles.txt >> @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_starfrac.asc
#    python @RUNROOT@/@SCRIPTPATH@/QC/starfrac.py \
#    	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_starfrac $band
#    
#    ####################################################
#    ### Extent of PSF star vs. full catalogue in x/y ###
#    ####################################################
#    echo "#tile x-range y-range x-range_stars y-range_stars" > \
#    	 @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_xy_range.asc
#    while read tile
#    do
#    	stats=@RUNROOT@/@WORKINGDIR@/$tile/$band/${tile}_${band}_cat_stats.txt
#    	stats_stars=@RUNROOT@/@WORKINGDIR@/$tile/$band/${tile}_${band}_star_cat_GAaP_stats.txt
#    	if [ -s $stats ] && [ -s $stats_stars ]
#    	then
#    	    
#    	    paste $stats $stats_stars | awk '{if (NR>1) print "'$tile'", $2-$1,$4-$3,$6-$5,$8-$7}'
#    	fi
#    done < @RUNROOT@/ugri_tiles.txt >> @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_xy_range.asc
#    python @RUNROOT@/@SCRIPTPATH@/QC/xy_range.py \
#    	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_xy_range $band
#    
#    ##############################
#    ### Width of stellar locus ###
#    ##############################
#    echo "#tile SL-mean SL-std" > \
#    	 @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_SL.asc
#    while read tile
#    do
#    	stats=@RUNROOT@/@WORKINGDIR@/$tile/$band/${tile}_${band}_star_cat_GAaP_SL.txt
#    	if [ -s $stats ]
#    	then
#    	    cat $stats | awk '{print "'$tile'", $1, $2, $2/$1}'
#    	fi
#    done < @RUNROOT@/ugri_tiles.txt >> @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_SL.asc
#    python @RUNROOT@/@SCRIPTPATH@/QC/SL_dist.py \
#    	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${band}_SL $band
#done


#############################
### Select "clean" fields ###
#############################
#
#bash @RUNROOT@/@SCRIPTPATH@/QC/exclude_fields.sh


#################################################
### Summarise photo-z stats (bright vs. SDSS) ###
#################################################

#ulimit -n 20000
#
#for filters in ugri ugriz
#do
#    for calib in "" "_recalib" "_recalibplus"
#    do
#	for ending in "" "_clean"
#	do
#	    for survey in specz SDSS #
#	    do
#		#case $ending in
#		#    "") pointings=${filters}_tiles.txt;;
#		#    "_clean") pointings=${filters}_tiles.txt_clean.txt;;
#		#esac
#		#FILES=""
#		#while read field
#		#do
#		#    FILES=$FILES" "@RUNROOT@/@WORKINGDIR@/$field/${field}_${filters}_photoz${calib}_ext_${survey}.cat
#		#done<@RUNROOT@/$pointings
#		#python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#		#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${filters}${calib}_${survey}${ending}.cat \
#		#       OBJECTS \
#		#       $FILES
#		if [ -f  @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${filters}${calib}_${survey}${ending}.cat ]
#		then
#		    python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#			   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${filters}${calib}_${survey}${ending}.cat \
#			   z_spec_spec \
#			   Z_B \
#			   MAG_AUTO \
#			   9.9 \
#			   $filters"-"$calib"-"$ending"-"$survey \
#			   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_${filters}${calib}_${survey}${ending}_zz
#		fi
#	    done
#	done
#    done
#done


############################
### Photo-z matrix plots ###
############################

#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri vs. ugriz photo-z" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_vs_ugriz

#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalib_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri photo-z raw vs. recalib" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_raw_vs_recalib
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri photo-z raw vs. recalibplus" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_raw_vs_recalibplus
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalib_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri photo-z recalib vs. recalibplus" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalib_vs_recalibplus
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_recalib_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugriz photo-z raw vs. recalib" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_raw_vs_recalib
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalib_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_recalib_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "recalibrated photo-z, ugri vs. ugriz" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalib_vs_ugriz_recalib
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugriz photo-z raw vs. recalibplus" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_raw_vs_recalibplus
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_recalib_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugriz photo-z recalib vs. recalibplus" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_recalib_vs_recalibplus
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalibplus_specz.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugriz_recalibplus_specz.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "recalibrated plus photo-z, ugri vs. ugriz" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_recalibplus_vs_ugriz_recalib


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
#	FILES=$FILES" "@RUNROOT@/@WORKINGDIR@/$field/${field}_${filters}_photoz_ext_specz.cat
#    done<@RUNROOT@/EGS_COSMOS_VVDS_GOODS-N_ugri.txt
#
#
#    python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_DEEP_$filters.cat \
#	   OBJECTS \
#	   $FILES
#
#    python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_DEEP_$filters.cat \
#	   z_spec_spec \
#	   Z_B \
#	   MAG_AUTO \
#	   9.9 \
#	   $filters"---EGS/COSMOS/VVDS/GOODS-N" \
#	   @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_DEEP_${filters}_zz
#done

#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_DEEP_uri.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_DEEP_ugri.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "uri vs. ugri" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugr_vs_ugri

#ulimit -n 20000
#
#FILES=""
#while read field
#do
#    n=`grep -c $field @RUNROOT@/EGS_COSMOS_VVDS_GOODS-N_ugri.txt`
#    if [ $n -eq 0 ]
#    then
#	FILES=$FILES" "@RUNROOT@/@WORKINGDIR@/$field/${field}_ugri_photoz_ext_specz.cat
#    fi
#done<@RUNROOT@/ugri_tiles.txt
#python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP.cat \
#       OBJECTS \
#       $FILES

#python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       9.9 \
#       "ugri---survey w/o deep fields" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP_zz
#
#python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP_plus_DEEP.cat \
#       OBJECTS \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_DEEP.cat
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP_plus_DEEP.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       9.9 \
#       "ugri---survey w/ deep fields" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP_plus_DEEP_zz

#python @RUNROOT@/@SCRIPTPATH@/QC/zz_matrix_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_noDEEP.cat \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_DEEP.cat \
#       z_spec_spec \
#       Z_B \
#       MAG_AUTO \
#       10. \
#       "ugri, survey vs. deep fields" \
#       @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_ugri_survey_vs_deep_fields

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
#python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_SDSS.cat \
#       OBJECTS \
#       $FILES
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_SDSS.cat \
#       z_spec_SDSS \
#       Z_B \
#       "KiDS-DR5 photo-z vs. SDSS spec-z" \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_SDSS_zz.png \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_SDSS_zz.pdf

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
#python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR4_SDSS.cat \
#       OBJECTS \
#       $FILES

#python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR4_SDSS.cat \
#       z_spec_SDSS \
#       Z_B \
#       "KiDS-DR4 photo-z vs. SDSS spec-z" \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR4_SDSS_zz.png \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR4_SDSS_zz.pdf

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
#python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_DEEP.cat \
#       OBJECTS \
#       $FILES
#
#python @RUNROOT@/@SCRIPTPATH@/QC/zz_plot.py \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_DEEP.cat \
#       z_spec_DEEP \
#       Z_B \
#       "KiDS-DR5 vs. all deep spec-z" \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_DEEP_zz.png \
#       @RUNROOT@/@WORKINGDIR@/QC/KiDS-DR5_DEEP_zz.pdf
