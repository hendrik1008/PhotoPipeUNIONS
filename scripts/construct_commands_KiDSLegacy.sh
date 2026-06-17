#!/bin/bash

# Script to construct commands needed to process one UNIONS tile.
# Script is called by run_PhotoPipe.sh and should not be called directly.
#
# Required Inputs:
# - ugri UNIONS stacks
# - lensfit input catalogue
#
# Output:
# - 4-band photometric catalogue with photoz 
#
# Author: H. Hildebrandt
# Adapted from the original PhotoPipe repository written by A. Wright
#
# Version history:
# 2023-02-17 V1.0

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

# MODES: {{{
#
# 1. CONVERT: Convert input catalogue to FITS-LDAC.
# 2. PREPARE: Create directories and copy images.
# 3. GAUSSIANISE: Gaussianise the PSF of the images.
# 4. GAAP: Extract GaAP photometry.
# 5. SDSSPREP: Preparation of SDSS catalogue.
# 6. ZPREP: Preparation of redshift catalogue.
# 7. PSPREP: Preparation of PanSTARRS catalogue.
# 8. COMPTILE: Photometric comparisons to SDSS.
# 9. COMPTILEPS: Photometric comparisons to PanSTARRS.
# 10. MERGE: Paste measurements from all bands into 5-band catalogue.
# 11. COMPTILEPOSTMERGE: Photometric comparisons to SDSS.
# 12. COMPTILEPSPOSTMERGE: Photometric comparisons to PanSTARRS.
# 13. BPZ: Run BPZ.
# 14. COMPTILEZ: Compare BPZ photo-z to spectroscopic redshifts.
# 15. MASK: Create 5-band mask.
# 16. QC: Run some quality control scripts.
# 17. CLEAN: Erase temporary data.
# 18. COPY: Copy results back to mass storage.
# 19. COPYBACK: Copy directory from mass storage to scratch.
# 20. ERASE: Erase everything from scratch.
#}}}

#Read command line modes {{{
MODE=""
while [ $# -gt 1 ]
do
  case $1 in
    -md)
      md=${2} # Main directory where the products go.
      shift 2
      ;;
    -cd)
      cats_dir=${2} # Catalogue directory where the MegaPipe catalogues live.
      shift 2
      ;;
    -cd2)
      cats_dir2=${2} # Catalogue directory where the ShapePipe catalogues live.
      shift 2
      ;;
    -id)
      image_dir=${2} # Directory where the images live.
      shift 2
      ;;
    -fi)
      field_name=${2} # Field name (MP convenction).
      shift 2
      ;;
    -ma)
      mask="${2}" # Location of mask.
      shift 2
      ;;
    -lg)
      LOGFILE="${2}" # Logfile.
      shift 2
      ;;
    -m)
      MODE=${2}
      shift 2
      ;;
    *)
      echo "Unknown command line option: ${1}"
      exit 1	   
      ;;
  esac
done
#}}}

### Create a working directory
mdfield=$md/${field_name}

test ! -d ${mdfield} && mkdir -p ${mdfield}

### Read MP xxx yyy from field name
xxx=`echo $field_name|cut -d "." -f 2`
yyy=`echo $field_name|cut -d "." -f 3`

### THELI name
THELI_name=`@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/translate_THELI2MP.py $xxx.$yyy`
RA=`echo  $THELI_name | cut -d "_" -f 1 | sed 's/p/\./g'`
Dec=`echo $THELI_name | cut -d "_" -f 2 | sed 's/p/\./g'`

### Paths to the photometric catalogues.
phot_cat=${cats_dir}/CFIS.${xxx}.${yyy}.r.cat
phot_cat_local=${mdfield}/CFIS.${xxx}.${yyy}.r.cat
cat_LDAC=${mdfield}/CFIS.${xxx}.${yyy}.r.ldac.cat

### Paths to the ShapePipe catalogues.
phot_cat2=${cats_dir2}/sexcat-${xxx}-${yyy}.fits
cat_LDAC2=${mdfield}/CFIS_SP.${xxx}.${yyy}.r.ldac.cat

##################################
### Here the real work starts. ###
##################################

### Convert MegaPipe ASCII catalogue into 
for mode in ${MODE}
do
    if [ "${mode}" = "CONVERT" ]; then
	if [ ! -e $cat_LDAC ]
	then
	    if [ ! -s $phot_cat_local ]
	    then
		#echo -n vcp --cert=$HOME/cadcproxy.pem --vos-debug $phot_cat $phot_cat_local \;
		echo -n vcp --vos-debug $phot_cat $phot_cat_local \;
	    fi
	    echo -n echo \;
	    echo -n echo Copy done.\;
	    echo -n echo \;
	    echo -n asctoldac_theli \
		 -a $phot_cat_local \
		 -o $cat_LDAC \
		 -c @RUNROOT@/@CONFIGPATH@/asctoldac_MP.conf \;
	    #echo -n rm -f $phot_cat_local \;
	fi
	if [ ! -e $cat_LDAC2 ]
	then
	    echo -n ldacconv_theli \
		 -i $phot_cat2 \
		 -o $cat_LDAC2.tmp \
		 -b 1 \
		 -f r \
		 -c MegaCam \;
	    echo -n ldacrenkey_theli \
		 -i $cat_LDAC2.tmp \
		 -o $cat_LDAC2 \
		 -t OBJECTS \
		 -k X_WORLD ALPHA_J2000 Y_WORLD DELTA_J2000 \;
	    echo -n rm -f $cat_LDAC2.tmp \;
	fi
	echo sleep 1
    fi
done

### Prepare images.
for mode in ${MODE}
do
    if [ "${mode}" = "PREPARE" ]; then
	
	### Loop over all UNIONS bands.
	for band in u g r i z z2
	do
	    ### Create band directory.
	    wdband=${mdfield}/${band}
	    test ! -d ${wdband} && mkdir ${wdband}
	done
	
	# MegaPipe u- and r-bands
	for filter in u r
	do
    	    prefix=CFIS
    	    base=$image_dir/tiles_DR6/${prefix}.${xxx}.${yyy}.${filter}
	    set +e
	    vls --vos-debug $base.fits >& /dev/null
	    if [ "$?" -eq "0" ]
	    then
		set -e
		if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.fits ] || \
		       [ ! -s $md/$field_name/$filter/${field_name}_${filter}.weight.fits ]
		then
    		    echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.fits \;
    		    echo -n vcp --vos-debug $base.weight.fits.fz $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits.fz \;
		    echo -n funpack -O $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits.fz \;
    		    test -e $md/$field_name/$filter/${field_name}_${filter}.weight.fits \
			&& rm -f $md/$field_name/$filter/${field_name}_${filter}.weight.fits
    		    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
			 @RUNROOT@/@SCRIPTPATH@/extract_MPweight.py \
    			 $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits \
    			 $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
		    echo -n rm -f $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits \
			 $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits.fz \;
		fi
	    else
		set -e
		echo -n ic -p -32 -c 10000 10000 \'0\' \
		     \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	    fi
	    echo sleep 1
	done
	
	# PanSTARRS i-band DR4
	filter=i
	prefix=PSS.DR4gold
	base=$image_dir/panstarrs/DR4gold/resamp/${prefix}.${xxx}.${yyy}.${filter}
	set +e
	vls --vos-debug $base.fits >& /dev/null
	if [ "$?" -eq "0" ]
	then
	    set -e
	    if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.fits ] || \
		   [ ! -s $md/$field_name/$filter/${field_name}_${filter}.weight.fits ]
	    then
		echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.fits \;
		echo -n vcp --vos-debug $base.weight.fits $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	    fi
	else
	    set -e
	    echo -n ic -p -32 -c 10000 10000 \'0\' \
		 \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	fi
	echo sleep 1
	
	# PanSTARRS z-band DR4
	filter=z2
	prefix=PSS.DR4gold
	base=$image_dir/panstarrs/DR4gold/resamp/${prefix}.${xxx}.${yyy}.z
	set +e
	vls --vos-debug $base.fits >& /dev/null
	if [ "$?" -eq "0" ]
	then
	    set -e
	    if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.fits ] || \
		   [ ! -s $md/$field_name/$filter/${field_name}_${filter}.weight.fits ]
	    then
		echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.fits \;
		echo -n vcp --vos-debug $base.weight.fits $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	    fi
	else
	    set -e
	    echo -n ic -p -32 -c 10000 10000 \'0\' \
		 \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	fi
	echo sleep 1
	
	# HSC g-band
	filter=g
	prefix=calexp-CFIS_
	HSC_field_name=`echo ${xxx} ${yyy}|awk '{printf "%i_%i\n",$1,$2}'`
	base=$image_dir/whigs/stack_images_CFIS_scheme/${prefix}${HSC_field_name}
	set +e
	vls --vos-debug $base.fits >& /dev/null
	if [ "$?" -eq "0" ]
	then
	    set -e
	    size=`vls -l $base.fits | awk '{print $5}'`
	    if [ ! $size -eq 0 ]
	    then
		if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.fits ] || \
		       [ ! -s $md/$field_name/$filter/${field_name}_${filter}.weight.fits ]
		then
		    echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
		    echo -n python3 @RUNROOT@/@SCRIPTPATH@/extract_HSC.py \
    			 $md/$field_name/$filter/${field_name}_${filter}.raw.fits \
    			 $md/$field_name/$filter/${field_name}_${filter} \
			 EXTNAME \;
		    echo -n @RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic \
			 \'0 1 %1 259 \> \?\' \
			 $md/$field_name/$filter/${field_name}_${filter}.flag.fits \
			 \> $md/$field_name/$filter/${field_name}_${filter}.01.fits \;
		    echo -n @RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic \
			 \'%1 %2 \*\' \
			 $md/$field_name/$filter/${field_name}_${filter}.weight.tmp.fits \
			 $md/$field_name/$filter/${field_name}_${filter}.01.fits \
			 \> $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
		    echo -n rm -f $md/$field_name/$filter/${field_name}_${filter}.01.fits \
			 $md/$field_name/$filter/${field_name}_${filter}.weight.tmp.fits \;
		    echo -n rm -f $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
		fi
	    else
		echo -n ic -p -32 -c 10000 10000 \'0\' \
		     \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	    fi
	else
	    set -e
	    echo -n ic -p -32 -c 10000 10000 \'0\' \
		 \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	fi
	echo sleep 1
	
	# HSC z-band
	filter=z
	prefix=WISHES
	base=$image_dir/wishes_1/coadd_gold/${prefix}.${xxx}.${yyy}.${filter}
	set +e
	vls --vos-debug $base.fits >& /dev/null
	if [ "$?" -eq "0" ]
	then
	    set -e
	    if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.fits ] || \
		   [ ! -s $md/$field_name/$filter/${field_name}_${filter}.weight.fits ]
	    then
		echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
		echo -n python3 @RUNROOT@/@SCRIPTPATH@/extract_HSC.py \
    		     $md/$field_name/$filter/${field_name}_${filter}.raw.fits \
    		     $md/$field_name/$filter/${field_name}_${filter} \
		     EXTTYPE \;
		echo -n @RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic \
		     \'0 1 %1 259 \> \?\' \
		     $md/$field_name/$filter/${field_name}_${filter}.flag.fits \
		     \> $md/$field_name/$filter/${field_name}_${filter}.01.fits \;
		echo -n @RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic \
		     \'%1 %2 \*\' \
		     $md/$field_name/$filter/${field_name}_${filter}.weight.tmp.fits \
		     $md/$field_name/$filter/${field_name}_${filter}.01.fits \
		     \> $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
		echo -n rm -f $md/$field_name/$filter/${field_name}_${filter}.01.fits \
		     $md/$field_name/$filter/${field_name}_${filter}.weight.tmp.fits \;
		echo -n rm -f $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
	    fi
	else
	    set -e
	    echo -n ic -p -32 -c 10000 10000 \'0\' \
		 \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	fi
	echo sleep 1
    fi
done

### Gaussianise the images
for mode in ${MODE}
do
  if [ "${mode}" = "GAUSSIANISE" ]; then
      
      ### Loop over all bands.
      for band in u g r i z z2
      do
	  ### band directory.
	  wdband=${mdfield}/${band}
	  
	  image=$wdband/${field_name}_${band}.fits
	  base=`basename $image .fits`
	  gaussianised_image=${wdband}/${base}_smart_ggpsf.fits
	  # Check for gaussianised images 
	  if [ -e $image ] && [ ! -e ${gaussianised_image} ]
	  then 
	      echo -n "echo $base ; "
	      echo -n "cd $wdband ; "
              echo -n bash @RUNROOT@/@SCRIPTPATH@/gaussianise_chip.sh \
		   $wdband/ \
		   $image \
		   @RUNROOT@/INSTALL/gapphot_TE/ \
		   ${band} \
		   ${field_name} \;
	      echo -n bash @RUNROOT@/@SCRIPTPATH@/gaussianise_QC.sh \
		   $wdband \
		   $field_name \
		   $band \;
	  fi
      done
      echo sleep 1
  fi
done

### Extract GaAP photometry.
for mode in ${MODE}
do
    if [ "${mode}" = "GAAP" ]; then
	for incat in $cat_LDAC $cat_LDAC2
	do
	    ### Count the number of objects in the photometric catalogue.
	    if [ -e $incat ]
	    then 
		no_obj_phot_cat=`ldacdesc -i $incat | \
			    grep elements | awk '{if (NR==1) print $0}' | \
	        	    cut -d " " -f 3 | sed 's/\.//g' | cut -d ":" -f 2`
	    else 
		>&2 echo "ERROR: the catalogue does not exist?!"
		#exit 1
	    fi 
	    
	    ### Loop over all bands.
	    for band in u g r i z z2
	    do
		### band directory.
		wdband=${mdfield}/${band}
		
		image=${wdband}/${field_name}_${band}.fits
		base=`basename $image .fits`
		gaussianised_image=${wdband}/${base}_smart_ggpsf.fits
		if [ $incat = $cat_LDAC2 ]
		then
		    base=${field_name}_SP_${band}
		fi
		# Check for gaussianised images 
		if [ ! -e ${gaussianised_image} ]
		then 
		    >&2 echo WARNING: Gaussianised image does not exist ${gaussianised_image}
		    echo
		    #exit 1
		else
		    if [ ! -s ${wdband}/${base}_smart.gaap ]
		    then
			####################################
			### This is the main work script ###
			####################################
			echo -n bash @RUNROOT@/@SCRIPTPATH@/dogauss_smart_VIKING_KiDSLegacy.sh \
			     $wdband \
			     $image \
			     $gaussianised_image \
			     $incat \
			     ALPHA_J2000 \
			     DELTA_J2000 \
			     @RUNROOT@/INSTALL/gapphot_TE/ \
			     ${band} \
			     ${base} \;
			echo -n ls -l $wdband/*.gaap \>\> $LOGFILE \;
			####################################
			MAGZP=30
			if [ $band = "g" ] || [ $band = "z" ]
			then
			    MAGZP=27
			fi
			for ending in "" "_minaper1p0" #"_stars" "_stars0p7"
			do
			    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
				 @RUNROOT@/@SCRIPTPATH@/average_fluxes_list.py \
				 $no_obj_phot_cat \
				 $wdband/${base}_smart${ending}_full.gaap \
				 $wdband/${base}_smart${ending}.gaap \;
			    echo -n bash @RUNROOT@/@SCRIPTPATH@/convert_gaap_fluxes.sh \
				 $wdband/${base}_smart${ending}_full.gaap \
				 $incat \
				 $wdband/${base}_smart${ending}.cat \
				 $band \
				 $MAGZP \
				 ALPHA_J2000 \
				 DELTA_J2000 \;
			done	      
		    fi
		fi 
	    done
	done
	echo sleep 1
    fi
done

### Preparation of SDSS catalogue.
for mode in ${MODE}
do
    if [ "${mode}" = "SDSSPREP" ]; then
	if [ ! -s ${mdfield}/SDSS/${field_name}_sdssdr10_stars.cat ]
	then
	    echo -n mkdir ${mdfield}/SDSS \;
	    echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/retrieve_sloan.sh ${mdfield}/SDSS/ $field_name $RA $Dec \;
	fi
	echo sleep 1
    fi
done

### Preparation of Seb's redshift catalogue.
for mode in ${MODE}
do
    if [ "${mode}" = "ZPREP" ]; then
	if [ ! -s ${mdfield}/specz/${field_name}_redshifts-2024-01-04.cat ]
	then
	    echo -n mkdir ${mdfield}/specz \;
	    echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/prepare_specz.sh ${mdfield}/specz/ $field_name $RA $Dec \;
	fi
	echo sleep 1
    fi
done

### Preparation of Pan-STARRS catalogue.
for mode in ${MODE}
do
    if [ "${mode}" = "PSPREP" ]; then
	if [ ! -s ${mdfield}/PS/${field_name}_PS1-DR2.cat ]
	then
	    echo -n mkdir ${mdfield}/PS \;
	    echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/retrieve_PS.sh ${mdfield}/ $field_name $RA $Dec \;
	fi
	echo sleep 1
    fi
done


### Preparation of PGM catalogue.
for mode in ${MODE}
do
    if [ "${mode}" = "PGMPREP" ]; then
	if [ ! -s ${mdfield}/PGM/${field_name}_PGM.cat ]
	then
	    echo -n mkdir ${mdfield}/PGM \;
	    echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/retrieve_PGM.sh ${mdfield}/ $field_name $RA $Dec \;
	fi
	echo sleep 1
    fi
done

### Comparisons SDSS (ugriz bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILE" ]; then
      SDSS_cat=${mdfield}/SDSS/${field_name}_sdssdr10_stars.cat
      if [ -s $SDSS_cat ]
      then
	  for suffix in "_SP" ""
	  do
	      ### Loop over all UNIONS bands.
	      for ending in "" _minaper1p0 #_stars _stars0p7
	      do
		  for band in u g r i z z2
		  do
		      wdband=${mdfield}/${band}
		      if [ -s ${wdband}/${field_name}${suffix}_${band}_smart${ending}_full.cat ] && \
			     [ ! -s $wdband/${field_name}${suffix}_${band}_smart${ending}_full_SDSS_${band}_offset.asc ]
		      then
			  echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_SDSS_K1000.sh \
			       ${wdband} \
			       $SDSS_cat \
			       ${wdband}/${field_name}${suffix}_${band}_smart${ending}_full.cat \
			       ${band} ${field_name} \;
		      fi
		  done
	      done
	  done
      fi
      echo sleep 1
  fi
done

### Comparisons PS (ugriz bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEPS" ]; then
      PS_cat=${mdfield}/PS/${field_name}_PS1-DR2.cat
      if [ -s $PS_cat ]
      then
	  for suffix in "_SP" ""
	  do
	      ### Loop over all UNIONS bands.
	      for ending in "" _minaper1p0 #_stars _stars0p7
	      do
		  for band in u g r i z z2
		  do
		      wdband=${mdfield}/${band}
		      if [ -s ${wdband}/${field_name}${suffix}_${band}_smart${ending}_full.cat ] && \
			     [ ! -s $wdband/${field_name}${suffix}_${band}_smart${ending}_full_PS_${band}_offset.asc ]
		      then
			  echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_PS.sh \
			       ${wdband} \
			       $PS_cat \
			       ${wdband}/${field_name}${suffix}_${band}_smart${ending}_full.cat \
			       ${band} ${field_name} \;
		      fi
		  done
	      done
	  done
      fi
      echo sleep 1
  fi
done

### Comparisons PGM (griz bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEPGM" ]; then
      PGM_cat=${mdfield}/PGM/${field_name}_PGM.cat
      if [ -s $PGM_cat ]
      then
	  for suffix in "" "_SP"
	  do
	      ### Loop over all UNIONS bands.
	      for ending in "" _minaper1p0 #_stars _stars0p7
	      do
		  for band in g r i z z2
		  do
		      wdband=${mdfield}/${band}
		      if [ -s ${wdband}/${field_name}${suffix}_${band}_smart${ending}_full.cat ] && \
			     [ ! -s $wdband/${field_name}${suffix}_${band}_smart${ending}_full_PGM_${band}_offset.asc ]
		      then
			  echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_PGM.sh \
			       ${wdband} \
			       $PGM_cat \
			       ${wdband}/${field_name}${suffix}_${band}_smart${ending}_full.cat \
			       ${band} ${field_name} \;
		      fi
		  done
	      done
	  done
      fi
      echo sleep 1
  fi
done

### Paste the measurements from individual bands
### into a full 9-band catalogue
for mode in ${MODE}
do
  if [ "${mode}" = "MERGE" ]; then
      for incat in $cat_LDAC $cat_LDAC2
      do
	  suffix=""
	  if [ $incat = $cat_LDAC2 ]
	  then
	      suffix="_SP"
	  fi
	  if [ ! -s ${mdfield}/${field_name}${suffix}_ugriz.cat ]
	  then
	      #echo -n set -e \;
	      echo -n cp $incat ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp0_$$ \;
	      
	      i=0
	      
	      ### Loop over all UNIONS bands.
	      for band in u g r i z z2
	      do
		  echo -n echo $band minaper0p7 \;
		  echo -n echo \;
		  if [ -e ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_full.cat ]
		  then
		      echo -n ldacrenkey -i ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_full.cat \
			   -o ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_full_rename.cat_$$ \
			   -t OBJECTS -k \
			   FLUX_GAAP_${band} FLUX_GAAP_0p7_${band} \
			   FLUXERR_GAAP_${band} FLUXERR_GAAP_0p7_${band} \
			   MAG_GAAP_${band} MAG_GAAP_0p7_${band} \
			   MAGERR_GAAP_${band} MAGERR_GAAP_0p7_${band} \
			   FLAG_GAAP FLAG_GAAP_0p7_${band} \
			   GAAP_nexp GAAP_nexp_0p7_${band} \
			   GAAP_chi_sq_dof GAAP_chi_sq_dof_0p7_${band} \;
		      echo -n ldacjoinkey -o ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp$[$i+1]_$$ \
			   -i ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
			   -p ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_full_rename.cat_$$ \
			   -t OBJECTS \
			   -k MAG_GAAP_0p7_${band} MAGERR_GAAP_0p7_${band} FLUX_GAAP_0p7_${band} \
			   FLUXERR_GAAP_0p7_${band} FLAG_GAAP_0p7_${band} GAAP_nexp_0p7_${band} \
			   GAAP_chi_sq_dof_0p7_${band} \;
		  else
		      echo -n ldacaddkey -o ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp$[$i+1]_$$ \
			   -i ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
			   -t OBJECTS \
			   -k \
			   MAG_GAAP_0p7_${band} -99.0 FLOAT \"${band} magnitude\" \
			   MAGERR_GAAP_0p7_${band} -99.0 FLOAT \"${band} magnitude error\" \
			   FLUX_GAAP_0p7_${band} -99.0 FLOAT \"${band} flux\" \
			   FLUXERR_GAAP_0p7_${band} -99.0 FLOAT \"${band} flux error\" \
			   FLAG_GAAP_0p7_${band} 1 SHORT \"GAAP photometry Flag\" \
			   GAAP_nexp_0p7_${band} 0 SHORT \"GAAP number of exposures\" \
			   GAAP_chi_sq_dof_0p7_${band} -99.0 FLOAT \"GAAP chi^2/dof\" \;
		  fi
		  i=$[$i+1]
		  echo -n echo $band minaper1p0 \;
		  echo -n echo \;
		  if [ -e ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_minaper1p0_full.cat ]
		  then
		      echo -n ldacrenkey -i ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_minaper1p0_full.cat \
			   -o ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_minaper1p0_full_rename.cat_$$ \
			   -t OBJECTS -k \
			   FLUX_GAAP_${band} FLUX_GAAP_1p0_${band} \
			   FLUXERR_GAAP_${band} FLUXERR_GAAP_1p0_${band} \
			   MAG_GAAP_${band} MAG_GAAP_1p0_${band} \
			   MAGERR_GAAP_${band} MAGERR_GAAP_1p0_${band} \
			   FLAG_GAAP FLAG_GAAP_1p0_${band} \
			   GAAP_nexp GAAP_nexp_1p0_${band} \
			   GAAP_chi_sq_dof GAAP_chi_sq_dof_1p0_${band} \;
		      echo -n ldacjoinkey -o ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp$[$i+1]_$$ \
			   -i ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
			   -p ${mdfield}/${band}/${field_name}${suffix}_${band}_smart_minaper1p0_full_rename.cat_$$ \
			   -t OBJECTS \
			   -k MAG_GAAP_1p0_${band} MAGERR_GAAP_1p0_${band} FLUX_GAAP_1p0_${band} \
			   FLUXERR_GAAP_1p0_${band} FLAG_GAAP_1p0_${band} GAAP_nexp_1p0_${band} \
			   GAAP_chi_sq_dof_1p0_${band} \;
		  else
		      echo -n ldacaddkey -o ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp$[$i+1]_$$ \
			   -i ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
			   -t OBJECTS \
			   -k \
			   MAG_GAAP_1p0_${band} -99.0 FLOAT \"${band} magnitude\" \
			   MAGERR_GAAP_1p0_${band} -99.0 FLOAT \"${band} magnitude error\" \
			   FLUX_GAAP_1p0_${band} -99.0 FLOAT \"${band} flux\" \
			   FLUXERR_GAAP_1p0_${band} -99.0 FLOAT \"${band} flux error\" \
			   FLAG_GAAP_1p0_${band} 1 SHORT \"GAAP photometry Flag\" \
			   GAAP_nexp_1p0_${band} 0 SHORT \"GAAP number of exposures\" \
			   GAAP_chi_sq_dof_1p0_${band} -99.0 FLOAT \"GAAP chi^2/dof\" \;
		  fi
		  echo -n rm -f ${mdfield}/${band}/${field_name}${suffix}_${band}*_$$ \;
		  i=$[$i+1]
	      done
	      
	      echo -n echo Adding E_B-V \;
	      echo -n echo \;
	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/add_extinction_python2.py \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp$[$i+1]_$$ \;
	      
	      i=$[$i+1]
	      
	      echo -n echo Calculating absorption \;
	      echo -n echo \;
	      echo -n ldaccalc -i ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
		   -o ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp$[$i+1]_$$ \
		   -t OBJECTS \
		   -c \"EXTINCTION\*4.239\;\" -n EXTINCTION_u  \"Galactic extinction in the u band \(mag\)\" -k FLOAT \
		   -c \"EXTINCTION\*3.303\;\" -n EXTINCTION_g  \"Galactic extinction in the g band \(mag\)\" -k FLOAT \
		   -c \"EXTINCTION\*2.285\;\" -n EXTINCTION_r  \"Galactic extinction in the r band \(mag\)\" -k FLOAT \
		   -c \"EXTINCTION\*1.698\;\" -n EXTINCTION_i  \"Galactic extinction in the i band \(mag\)\" -k FLOAT \
		   -c \"EXTINCTION\*1.263\;\" -n EXTINCTION_z  \"Galactic extinction in the z band \(mag\)\" -k FLOAT \
		   -c \"EXTINCTION\*1.263\;\" -n EXTINCTION_z2 \"Galactic extinction in the z2 band \(mag\)\" -k FLOAT \;
	      
	      i=$[$i+1]
	      
	      echo -n echo Adding tile names \;
	      echo -n echo \;
	      #echo -n ldacaddkey -i ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
	      #	   -o ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp \
	      #	   -t OBJECTS -k \
	      #	   MP_NAME \"${field_name}\" STRING \"Name of the pointing in MegaPipe convention\" \
	      #	   THELI_NAME \"$THELI_name\" STRING \"Name of the pointing in THELI convention\" \;
	      
	      #echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
	      echo -n python3 \
		   @RUNROOT@/@SCRIPTPATH@/ldacaddkey.py \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp${i}_$$ \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp \
		   OBJECTS \
		   MP_NAME \"${field_name}\" \"Name of the pointing in MegaPipe convention\" \
		   THELI_NAME \"$THELI_name\" \"Name of the pointing in THELI convention\" \;
	      
	      echo -n rm -f ${mdfield}/${field_name}${suffix}_ugriz*_$$ \;
	      
	      echo -n echo Converting to magnitudes \;
	      echo -n echo \;
	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
		   @RUNROOT@/@SCRIPTPATH@/convert_fluxes_to_magnitudes6.py \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat \;
	      
	      echo -n rm -f ${mdfield}/${field_name}${suffix}_ugriz.cat_tmp \;
	  fi
      done
      echo sleep 1
  fi
done

### Comparisons SDSS (ugriz bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEPOSTMERGE" ]; then
      SDSS_cat=${mdfield}/SDSS/${field_name}_sdssdr10_stars.cat
      for suffix in "" "_SP"
      do
	  if [ -s ${mdfield}/${field_name}${suffix}_ugriz.cat ] && \
		 [ ! -s ${mdfield}/phot_comp_SDSS/${field_name}${suffix}_ugriz_SDSS_r_offset.asc ]
	  then
	      echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_SDSS_ugriz.sh \
		   ${mdfield} \
		   $SDSS_cat \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat \
		   ${field_name} \;
	  fi
      done
      echo sleep 1
  fi
done

### Comparisons PS (ugriz bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEPSPOSTMERGE" ]; then
      PS_cat=${mdfield}/PS/${field_name}_PS1-DR2.cat
      for suffix in "" "_SP"
      do
	  if [ -s ${mdfield}/${field_name}${suffix}_ugriz.cat ] && \
		 [ ! -s ${mdfield}/phot_comp_PS/${field_name}${suffix}_ugriz_PS_r_offset.asc ]
	  then
	      echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_PS_ugriz.sh \
		   ${mdfield} \
		   $PS_cat \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat \
		   ${field_name} \;
	  fi
      done
      echo sleep 1
  fi
done

### Comparisons PGM (griz bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEPGMPOSTMERGE" ]; then
      PGM_cat=${mdfield}/PGM/${field_name}_PGM.cat
      for suffix in "" "_SP"
      do
	  if [ -s ${mdfield}/${field_name}${suffix}_ugriz.cat ] && \
		 [ ! -s ${mdfield}/phot_comp_PGM/${field_name}${suffix}_ugriz_PGM_r_offset.asc ]
	  then
	      echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_PGM_griz.sh \
		   ${mdfield} \
		   $PGM_cat \
		   ${mdfield}/${field_name}${suffix}_ugriz.cat \
		   ${field_name} \;
	  fi
      done
      echo sleep 1
  fi
done

### Run BPZ
for mode in ${MODE}
do
  if [ "${mode}" = "BPZ" ]; then
      #echo -n "set -e ; "
      for suffix in "" "_SP"
      do
	  for filters in ugriz griz # ugri # uriz
	  do
	      if [ ! -s ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat ]
	      then
		  filters2="u g r i z z2"
		  if [ $filters = "ugri" ]
		  then
		      filters2="u g r i"
		  elif [ $filters = "uriz" ]
		  then
		      filters2="u r i z z2"
		  elif [ $filters = "griz" ]
		  then
		      filters2="g r i z z2"
		  fi
		  echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
		       @RUNROOT@/@SCRIPTPATH@/add_maglim6.py ${mdfield}/${field_name}${suffix}_ugriz.cat \
		       ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \;
		  echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
		       ${mdfield}\
		       ${field_name}${suffix}_${filters}_maglim.cat \
		       \"$filters2\" \
		       MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
		       0.01 \;
		  echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
		       @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugrizz2.py \
		       ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz.cat \
		       ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat_tmp_$$ \;
		  echo -n ldacaddtab -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat_tmp_$$ \
		       -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat_tmp2_$$ \
		       -p ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz.cat \
		       -t FIELDS \;
		  echo -n ldacdelkey -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat_tmp2_$$ \
		       -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat \
		       -t OBJECTS \
		       -k  \
		       MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i MAG_LIM_0p7_z MAG_LIM_0p7_z2 \
		       MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i MAG_LIM_1p0_z MAG_LIM_1p0_z2 \;
		  echo -n rm -f ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat_tmp_$$ \
		       ${mdfield}/${field_name}${suffix}_${filters}_photoz_ext.cat_tmp2_$$ \
		       ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \
		       ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz.cat \
		       ${mdfield}/BPZ_photoz/${field_name}${suffix}_${filters}_maglim_photoz.probs \
		       ${mdfield}/BPZ_photoz/${field_name}${suffix}_${filters}_maglim_photoz.flux_comparison \;
	      fi
	  done
      done
      echo sleep 1
  fi
done

#### Run BPZ
#for mode in ${MODE}
#do
#  if [ "${mode}" = "BPZRECALIB" ]; then
#      echo -n "set -e ; "
#      for suffix in "" #"_SP"
#      do
#	  for filters in ugriz #ugri uriz
#	  do
#	      filters2="u g r i z"
#	      if [ $filters = "ugri" ]
#	      then
#		  filters2="u g r i"
#	      elif [ $filters = "uriz" ]
#	      then
#		  filters2="u r i z"
#	      fi
#	      for filter in $filters2
#	      do
#		  case $filter in
#		      "u") offset=-0.061;;
#		      "g") offset=-0.036;;
#		      "r") offset=-0.031;;
#		      "i") offset=0.036;;
#		      "z") offset=-0.006;;
#		  esac
#		  echo $filter $offset
#	      done > $mdfield/${field_name}_${filters}_offsets.asc
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/add_maglim5.py ${mdfield}/${field_name}${suffix}_ugriz.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \;
#	      echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
#		   ${mdfield}\
#		   ${field_name}${suffix}_${filters}_maglim.cat \
#		   \"$filters2\" \
#		   MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
#		   0.01 \
#		   $mdfield/${field_name}_${filters}_offsets.asc \
#		   _recalib \;
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugriz.py \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalib.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalib_ext.cat_tmp_$$ \;
#	      echo -n ldacaddtab -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalib_ext.cat_tmp_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalib_ext.cat_tmp2_$$ \
#		   -p ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalib.cat \
#		   -t FIELDS \;
#	      echo -n ldacdelkey -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalib_ext.cat_tmp2_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalib_ext.cat \
#		   -t OBJECTS \
#		   -k  \
#		   MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i MAG_LIM_0p7_z \
#		   MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i MAG_LIM_1p0_z \;
#	      echo rm -f ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalib_ext.cat_tmp_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalib_ext.cat_tmp2_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz.cat \
#		   ${mdfield}/BPZ_photoz_recalib/${field_name}${suffix}_${filters}_maglim_photoz_recalib.probs \
#		   ${mdfield}/BPZ_photoz_recalib/${field_name}${suffix}_${filters}_maglim_photoz_recalib.flux_comparison
#	  done
#      done
#  fi
#done
#
#### Run BPZ
#for mode in ${MODE}
#do
#  if [ "${mode}" = "BPZRECALIBPLUS" ]; then
#      echo -n "set -e ; "
#      for suffix in "" #"_SP"
#      do
#	  for filters in ugriz #ugri uriz
#	  do
#	      filters2="u g r i z"
#	      if [ $filters = "ugri" ]
#	      then
#		  filters2="u g r i"
#	      elif [ $filters = "uriz" ]
#	      then
#		  filters2="u r i z"
#	      fi
#	      for filter in $filters2
#	      do
#		  case $filter in
#		      "u") offset=0.061;;
#		      "g") offset=0.036;;
#		      "r") offset=0.031;;
#		      "i") offset=-0.036;;
#		      "z") offset=0.006;;
#		  esac
#		  echo $filter $offset
#	      done > $mdfield/${field_name}_${filters}_offsetsplus.asc
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/add_maglim5.py ${mdfield}/${field_name}${suffix}_ugriz.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \;
#	      echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
#		   ${mdfield}\
#		   ${field_name}${suffix}_${filters}_maglim.cat \
#		   \"$filters2\" \
#		   MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
#		   0.01 \
#		   $mdfield/${field_name}_${filters}_offsetsplus.asc \
#		   _recalibplus \;
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugriz.py \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalibplus.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibplus_ext.cat_tmp_$$ \;
#	      echo -n ldacaddtab -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibplus_ext.cat_tmp_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibplus_ext.cat_tmp2_$$ \
#		   -p ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalibplus.cat \
#		   -t FIELDS \;
#	      echo -n ldacdelkey -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibplus_ext.cat_tmp2_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibplus_ext.cat \
#		   -t OBJECTS \
#		   -k  \
#		   MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i MAG_LIM_0p7_z \
#		   MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i MAG_LIM_1p0_z \;
#	      echo rm -f ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibplus_ext.cat_tmp_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibplus_ext.cat_tmp2_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz.cat \
#		   ${mdfield}/BPZ_photoz_recalibplus/${field_name}${suffix}_${filters}_maglim_photoz_recalibplus.probs \
#		   ${mdfield}/BPZ_photoz_recalibplus/${field_name}${suffix}_${filters}_maglim_photoz_recalibplus.flux_comparison
#	  done
#      done
#  fi
#done
#
#### Run BPZ
#for mode in ${MODE}
#do
#  if [ "${mode}" = "BPZRECALIBSDSS" ]; then
#      echo -n "set -e ; "
#      for suffix in "" #"_SP"
#      do
#	  for filters in uriz # ugriz #ugri
#	  do
#	      filters2="u g r i z"
#	      if [ $filters = "ugri" ]
#	      then
#		  filters2="u g r i"
#	      elif [ $filters = "uriz" ]
#	      then
#		  filters2="u r i z"
#	      fi
#	      for filter in $filters2
#	      do
#		  awk '{printf "'$filter' %f\n",-$1}' \
#		      ${mdfield}/${filter}/${field_name}_${filter}_smart_full_SDSS_${filter}_offset.asc
#	      done > $mdfield/${field_name}_${filters}_offsetsSDSS.asc
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/add_maglim5.py ${mdfield}/${field_name}${suffix}_ugriz.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \;
#	      echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
#		   ${mdfield}\
#		   ${field_name}${suffix}_${filters}_maglim.cat \
#		   \"$filters2\" \
#		   MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
#		   0.01 \
#		   $mdfield/${field_name}_${filters}_offsetsSDSS.asc \
#		   _recalibSDSS \;
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugriz.py \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSS.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSS_ext.cat_tmp_$$ \;
#	      echo -n ldacaddtab -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSS_ext.cat_tmp_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSS_ext.cat_tmp2_$$ \
#		   -p ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSS.cat \
#		   -t FIELDS \;
#	      echo -n ldacdelkey -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSS_ext.cat_tmp2_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSS_ext.cat \
#		   -t OBJECTS \
#		   -k  \
#		   MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i MAG_LIM_0p7_z \
#		   MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i MAG_LIM_1p0_z \;
#	      echo rm -f ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSS_ext.cat_tmp_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSS_ext.cat_tmp2_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz.cat \
#		   ${mdfield}/BPZ_photoz_recalibSDSS/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSS.probs \
#		   ${mdfield}/BPZ_photoz_recalibSDSS/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSS.flux_comparison
#	  done
#      done
#  fi
#done
#
#### Run BPZ
#for mode in ${MODE}
#do
#  if [ "${mode}" = "BPZRECALIBSDSSPLUS" ]; then
#      echo -n "set -e ; "
#      for suffix in "" #"_SP"
#      do
#	  for filters in ugriz #ugri uriz
#	  do
#	      filters2="u g r i z"
#	      if [ $filters = "ugri" ]
#	      then
#		  filters2="u g r i"
#	      elif [ $filters = "uriz" ]
#	      then
#		  filters2="u r i z"
#	      fi
#	      for filter in $filters2
#	      do
#		  awk '{printf "'$filter' %f\n",$1}' \
#		      ${mdfield}/${filter}/${field_name}_${filter}_smart_full_SDSS_${filter}_offset.asc
#	      done > $mdfield/${field_name}_${filters}_offsetsSDSSplus.asc
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/add_maglim5.py ${mdfield}/${field_name}${suffix}_ugriz.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \;
#	      echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
#		   ${mdfield}\
#		   ${field_name}${suffix}_${filters}_maglim.cat \
#		   \"$filters2\" \
#		   MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
#		   0.01 \
#		   $mdfield/${field_name}_${filters}_offsetsSDSSplus.asc \
#		   _recalibSDSSplus \;
#	      echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
#		   @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugriz.py \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSSplus.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSSplus_ext.cat_tmp_$$ \;
#	      echo -n ldacaddtab -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSSplus_ext.cat_tmp_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSSplus_ext.cat_tmp2_$$ \
#		   -p ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSSplus.cat \
#		   -t FIELDS \;
#	      echo -n ldacdelkey -i ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSSplus_ext.cat_tmp2_$$ \
#		   -o ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSSplus_ext.cat \
#		   -t OBJECTS \
#		   -k  \
#		   MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i MAG_LIM_0p7_z \
#		   MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i MAG_LIM_1p0_z \;
#	      echo rm -f ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSSplus_ext.cat_tmp_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_photoz_recalibSDSSplus_ext.cat_tmp2_$$ \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim.cat \
#		   ${mdfield}/${field_name}${suffix}_${filters}_maglim_photoz.cat \
#		   ${mdfield}/BPZ_photoz_recalibSDSSplus/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSSplus.probs \
#		   ${mdfield}/BPZ_photoz_recalibSDSSplus/${field_name}${suffix}_${filters}_maglim_photoz_recalibSDSSplus.flux_comparison
#	  done
#      done
#  fi
#done

### Comparison to SDSS redshifts.
### Full tile.
for mode in ${MODE}
do
    if [ "${mode}" = "COMPTILEZ" ]; then
	for filters in  ugriz griz # ugri #uriz # ugri
	do
	    for suffix in "" "_SP"
	    do
		for recalib in "" #"_recalibSDSSplus" #"_recalibSDSS" #"_recalibplus" # "_recalib" #""
		do
		    cat_file=${mdfield}/${field_name}${suffix}_${filters}_photoz${recalib}_ext.cat
		    if [ -e $cat_file ]
		    then
			for label in SDSS specz
			do
			    case $label in
				SDSS) specz_cat=${mdfield}/SDSS/${field_name}_sdssdr10_galz.cat;;
				specz) specz_cat=${mdfield}/specz/${field_name}_redshifts-2024-01-04.cat;;
			    esac
			    if [ -e $specz_cat ] && \
				   [ ! -s ${mdfield}/${field_name}${suffix}_${filters}_photoz${recalib}_ext_${label}_zz.txt ]
			    then
				echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_z.sh \
				     ${mdfield} \
				     $specz_cat \
				     ${cat_file} \
				     ${field_name} \
				     $label \
				     $filters \
				     A$suffix \;
			    fi
			done
		    fi
		done
	    done
	done
	echo sleep 1
    fi
done

### Mask.
for mode in ${MODE}
do
    if [ "${mode}" = "MASK" ]; then
	if [ ! -s ${mdfield}/${field_name}_ugriz.mask.fits ]
	then
	    rm -f $mdfield/${field_name}_missing_bands.txt
	    touch $mdfield/${field_name}_missing_bands.txt
	    for filter in u g r i z z2
	    do
		filter2=$filter
		if [ $filter = "z" ]
		then
		    filter2=z1
		fi
		if [ -s ${mdfield}/$filter/${field_name}_$filter.weight.fits ]
		then
		    if [ -s ${mdfield}/$filter/${field_name}_${filter}_smart_full.cat ]
		    then
			echo -n ln -sf ${mdfield}/$filter/${field_name}_$filter.weight.fits ${mdfield}/$filter/${field_name}_$filter.weight2.fits \;
		    else
			echo -n ic -p -32 -c 10000 10000 \'0\' \
			     \>$md/$field_name/$filter/${field_name}_${filter}.weight2.fits \;
			echo $filter2 >> $mdfield/${field_name}_missing_bands.txt
		    fi
		else
		    echo -n ic -p -32 -c 10000 10000 \'0\' \
			 \>$md/$field_name/$filter/${field_name}_${filter}.weight2.fits \;
		    echo $filter2 >> $mdfield/${field_name}_missing_bands.txt
		fi
	    done
	    echo -n @RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic -p 16 \
		 \'0 64 \%1 0 \> \? 0 16 \%2 0 \> \? \+ 0 32 \%3 0 \> \? \+ 0 128 \%4 0 \> \? \+ 0 256 \%5 0 \> \? \+ 0 2048 \%6 0 \> \? \+\' \
		 ${mdfield}/r/${field_name}_r.weight2.fits \
		 ${mdfield}/u/${field_name}_u.weight2.fits \
		 ${mdfield}/g/${field_name}_g.weight2.fits \
		 ${mdfield}/i/${field_name}_i.weight2.fits \
		 ${mdfield}/z/${field_name}_z.weight2.fits \
		 ${mdfield}/z2/${field_name}_z2.weight2.fits \
		 \> ${mdfield}/${field_name}_ugriz.mask.fits \;
	    for filter in u g r i z z2
	    do
		echo -n rm -f ${mdfield}/$filter/${field_name}_$filter.weight2.fits \;
	    done
	    echo -n gzip -c ${mdfield}/${field_name}_ugriz.mask.fits \
		 \> ${mdfield}/${field_name}_ugriz.mask.fits.gz \;
	fi
	if [ ! -s ${mdfield}/r/${field_name}_r_maskstars.reg ]
	then
	    echo -n cd /arc/home/hendrik/src/automask/scripts/Linux_64 \;
	    echo -n export INSTRUMENT\=MEGAPRIME \;
	    echo -n bash ./maskstars.sh \
	         $mdfield/r/ \
		 ${field_name}_r.fits \
		 ${field_name}_r.weight.fits \
		 MEGAPRIME_mask.ini \;
	fi
	echo sleep 1
    fi
done

### QC.
for mode in ${MODE}
do
  if [ "${mode}" = "QC" ]; then
      #### Width of stellar locus.
      for band in u g r i z z2
      do
	  if [ -s ${mdfield}/${band}/${field_name}_${band}_star_cat_GAaP.asc ] && \
		 [ ! -s ${mdfield}/${band}/${field_name}_${band}_star_cat_GAaP_SL.txt ]
	  then
	      echo @RUNROOT@/INSTALL/anaconda2/bin/python \
		   @RUNROOT@/@SCRIPTPATH@/width_stellar_locus.py \
		   ${mdfield}/${band}/${field_name}_${band}_star_cat_GAaP
	  fi
      done
      echo sleep 1
  fi
done

### CLEAN.
for mode in ${MODE}
do
  if [ "${mode}" = "CLEAN" ]; then
      #### Clean up temporary and duplicated data products.
      #for band in u g r i z z2
      #do
      #	  echo rm -f \
      #	     ${mdfield}/${band}/${field_name}_${band}.fits \
      #	     ${mdfield}/${band}/${field_name}_${band}.weight.fits \
      #	     ${mdfield}/${band}/${field_name}_${band}.flag.fits \
      #	     ${mdfield}/${band}/${field_name}_${band}.weight.small.fits \
      #	     ${mdfield}/${band}/${field_name}_${band}.small.fits \
      #	     ${mdfield}/${band}/${field_name}_${band}_smart_ggpsf.fits
      #done
      echo rm -f \
	   ${mdfield}/*tmp* \
	   ${mdfield}/make* \
	   ${mdfield}/merg*
  fi
done

### COPY.
for mode in ${MODE}
do
  if [ "${mode}" = "COPY" ]; then
      #### Copy data from /scratch to permanent storage.
      echo rsync -atvu ${mdfield} /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS_DR6/
  fi
done

### COPYBACK.
for mode in ${MODE}
do
  if [ "${mode}" = "COPYBACK" ]; then
      #### Copy data from /scratch to permanent storage.
      echo rsync --exclude old_PS-DR4 -atvu /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS_DR6/${field_name} ${md}/
  fi
done

### CHECKFAIL.
for mode in ${MODE}
do
  if [ "${mode}" = "CHECKFAIL" ]; then
      #### Check for failures in previous processing and delete faulty products.
      echo bash @RUNROOT@/@SCRIPTPATH@/check_fail.sh /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS_DR6/${field_name}
  fi
done

### ERASE.
for mode in ${MODE}
do
  if [ "${mode}" = "ERASE" ]; then
      #### Erase all data from /scratch.
      echo rm -r ${mdfield}
  fi
done

