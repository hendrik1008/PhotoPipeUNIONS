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
# 1. PREPARE: Create directories.
# 2. GAAP: Extract GaAP photometry.
# 3. COMBINETILE: Combine flux measurements.
# 4. SDSSPREP: Preparation of SDSS catalogue.
# 5. COMPTILE: Comparisons to SDSS (ugri bands). Full tile.
# 6. MERGE: Paste the measurements from individual bands into a full 4-band catalogue.
# 7. BPZ: Run BPZ.
# 8. COMPTILEZ: Comparison to SDSS redshifts. Full tile.
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
      cats_dir=${2} # Catalogue directory where the lensfit catalogues live.
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

##################################
### Here the real work starts. ###
##################################

### Convert MegaPipe ASCII catalogue into 
for mode in ${MODE}
do
    if [ "${mode}" = "CONVERT" ]; then
	if [ -e $cat_ASCII ] && [ ! -e $cat_LDAC ]
	then
	    if [ ! -s $phot_cat_local ]
	    then
		echo -n vcp --vos-debug $phot_cat $phot_cat_local \;
	    fi
	    echo -n echo \;
	    echo -n echo Copy done.\;
	    echo -n echo \;
	    echo asctoldac_theli \
		 -a $phot_cat_local \
		 -o $cat_LDAC \
		 -c @RUNROOT@/@CONFIGPATH@/asctoldac_MP.conf
	fi
    fi
done

### Prepare images.
for mode in ${MODE}
do
    if [ "${mode}" = "PREPARE" ]; then
	
	### Loop over all UNIONS bands.
	for band in u g r i z
	do
	    ### Create band directory.
	    wdband=${mdfield}/${band}
	    test ! -d ${wdband} && mkdir ${wdband}
	done
	
	# MegaPipe u- and r-bands
	for filter in u r
	do
    	    prefix=CFIS
    	    base=$image_dir/tiles_DR5/${prefix}.${xxx}.${yyy}.${filter}
	    set +e
	    vls --vos-debug $base.fits >& /dev/null
	    if [ "$?" -eq "0" ]
	    then
		set -e
		if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.fits ]
		then
    		    echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.fits \;
		fi
		if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits.fz ]
		then
    		    echo -n vcp --vos-debug $base.weight.fits.fz $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits.fz \;
		fi
		echo -n funpack -O $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits.fz \;
    		test -e $md/$field_name/$filter/${field_name}_${filter}.weight.fits \
		    && rm $md/$field_name/$filter/${field_name}_${filter}.weight.fits
    		echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
		     @RUNROOT@/@SCRIPTPATH@/extract_MPweight.py \
    		     $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits \
    		     $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
		echo -n rm $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits \
		     $md/$field_name/$filter/${field_name}_${filter}.weightraw.fits.fz \;
	    else
		set -e
		echo -n ic -p -32 -c 10000 10000 \'0\' \
		     \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	    fi
	    echo
	done
	
	# PanSTARRS i-band
	filter=i
	prefix=PSS.DR4
	base=$image_dir/panstarrs/DR4/resamp/${prefix}.${xxx}.${yyy}.${filter}
	set +e
	vls --vos-debug $base.fits >& /dev/null
	if [ "$?" -eq "0" ]
	then
	    set -e
	    if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.fits ]
	    then
		echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.fits \;
	    fi
	    if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.weight.fits ]
	    then
		echo -n vcp --vos-debug $base.weight.fits $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	    fi
	    #echo -n replacekey_theli \
	    #	 $md/$field_name/$filter/${field_name}_${filter}.fits \
	    #	 \"CRPIX1\ \ \=\ \ \ 5.000672043000E\+03\ \/\ Reference\ pixel\ on\ this\ axis\" \
	    #	 CRPIX1 \
	    #	 \"CRPIX2\ \ \=\ \ \ 5.000672043000E\+03\ \/\ Reference\ pixel\ on\ this\ axis\" \
	    #	 CRPIX2 \;
	    #echo -n replacekey_theli \
	    #	 $md/$field_name/$filter/${field_name}_${filter}.weight.fits \
	    #	 \"CRPIX1\ \ \=\ \ \ 5.000672043000E\+03\ \/\ Reference\ pixel\ on\ this\ axis\" \
	    #	 CRPIX1 \
	    #	 \"CRPIX2\ \ \=\ \ \ 5.000672043000E\+03\ \/\ Reference\ pixel\ on\ this\ axis\" \
	    #	 CRPIX2 \;
	else
	    set -e
	    echo -n ic -p -32 -c 10000 10000 \'0\' \
		 \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	fi
	echo
	
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
	    if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.raw.fits ]
	    then
		echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
	    fi
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
	    echo -n rm $md/$field_name/$filter/${field_name}_${filter}.01.fits \
		 $md/$field_name/$filter/${field_name}_${filter}.weight.tmp.fits \;
		 #$md/$field_name/$filter/${field_name}_${filter}.flag.fits \;
	    echo -n rm $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
	else
	    set -e
	    echo -n ic -p -32 -c 10000 10000 \'0\' \
		 \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	fi
	echo
	
	# HSC z-band
	filter=z
	prefix=WISHES
	base=$image_dir/wishes_1/coadd/${prefix}.${xxx}.${yyy}.${filter}
	set +e
	vls --vos-debug $base.fits >& /dev/null
	if [ "$?" -eq "0" ]
	then
	    set -e
	    if [ ! -s $md/$field_name/$filter/${field_name}_${filter}.raw.fits ]
	    then
		echo -n vcp --vos-debug $base.fits $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
	    fi
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
	    echo -n rm $md/$field_name/$filter/${field_name}_${filter}.01.fits \
		 $md/$field_name/$filter/${field_name}_${filter}.weight.tmp.fits \;
		 #$md/$field_name/$filter/${field_name}_${filter}.flag.fits \;
	    echo -n rm $md/$field_name/$filter/${field_name}_${filter}.raw.fits \;
	else
	    set -e
	    echo -n ic -p -32 -c 10000 10000 \'0\' \
		 \>$md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	fi
	echo
    fi
done

### Preparation of Gaia catalogue.
for mode in ${MODE}
do
  if [ "${mode}" = "GAIAPREP" ]; then
      echo -n mkdir ${mdfield}/Gaia \;
      echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/prepare_gaia.sh ${mdfield}/Gaia/ $field_name $RA $Dec \;
      echo
  fi
done

### Gaussianise the images
for mode in ${MODE}
do
  if [ "${mode}" = "GAUSSIANISE" ]; then

    ### Loop over all bands.
    for band in u g r i z
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
          echo bash @RUNROOT@/@SCRIPTPATH@/gaussianise_chip.sh \
               $wdband/ \
               $image \
               @RUNROOT@/INSTALL/gapphot_TE/ \
               ${band} \
	       ${field_name}
      fi
    done
  fi
done

### Extract GaAP photometry.
for mode in ${MODE}
do
  if [ "${mode}" = "GAAP" ]; then
    ### Count the number of objects in the photometric and star catalogues.
    if [ -e $cat_LDAC ]
    then 
	no_obj_phot_cat=`ldacdesc -i $cat_LDAC | \
	    grep elements | awk '{if (NR==1) print $0}' | \
	        cut -d " " -f 3 | sed 's/\.//g' | cut -d ":" -f 2`
    else 
	>&2 echo "ERROR: the catalogue does not exist?!"
	exit 1
    fi 

    ### Loop over all bands.
    for band in u g r i z
    do
      ### band directory.
      wdband=${mdfield}/${band}

      image=${wdband}/${field_name}_${band}.fits
      base=`basename $image .fits`
      gaussianised_image=${wdband}/${base}_smart_ggpsf.fits
      # Check for gaussianised images 
      if [ ! -e ${gaussianised_image} ]
      then 
          >&2 echo WARNING: Gaussianised image does not exist ${gaussianised_image}
          #exit 1
      else
	  if [ ! -s ${wdband}/${base}_smart_stars0p7.gaap ]
	  then
	      ####################################
	      ### This is the main work script ###
	      ####################################
	      echo -n bash @RUNROOT@/@SCRIPTPATH@/dogauss_smart_VIKING_KiDSLegacy.sh \
		   $wdband \
		   $image \
		   $gaussianised_image \
		   $cat_LDAC \
		   ALPHA_J2000 \
		   DELTA_J2000 \
		   @RUNROOT@/INSTALL/gapphot_TE/ \
		   ${band} \;
	      echo -n ls -l $wdband/*.gaap \>\> $LOGFILE \;
	      ####################################
	      MAGZP=30
	      if [ $band = "g" ] || [ $band = "z" ]
	      then
		  MAGZP=27
	      fi
	      for ending in "" "_minaper1p0" "_stars" "_stars0p7"
	      do
		  echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
		       @RUNROOT@/@SCRIPTPATH@/average_fluxes_list.py \
		       $no_obj_phot_cat \
		       $wdband/${base}_smart${ending}_full.gaap \
		       $wdband/${base}_smart${ending}.gaap \;
		  echo -n bash @RUNROOT@/@SCRIPTPATH@/convert_gaap_fluxes.sh \
		       $wdband/${base}_smart${ending}_full.gaap \
		       $cat_LDAC \
		       $wdband/${base}_smart${ending}.cat \
		       $band \
		       $MAGZP \
		       ALPHA_J2000 \
		       DELTA_J2000 \;
	      done	      
	      echo
	  fi
      fi 
    done
  fi
done

### Preparation of SDSS catalogue.
for mode in ${MODE}
do
  if [ "${mode}" = "SDSSPREP" ]; then
      echo -n mkdir ${mdfield}/SDSS \;
      echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/retrieve_sloan.sh ${mdfield}/SDSS/ $field_name $RA $Dec \;
      echo
  fi
done

### Preparation of Seb's redshift catalogue.
for mode in ${MODE}
do
  if [ "${mode}" = "ZPREP" ]; then
      echo -n mkdir ${mdfield}/specz \;
      echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/prepare_specz.sh ${mdfield}/specz/ $field_name $RA $Dec \;
      echo
  fi
done

### Comparisons SDSS (ugri bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILE" ]; then
    SDSS_cat=${mdfield}/SDSS/${field_name}_sdssdr10_stars.cat
    ### Loop over all UNIONS bands.
    for ending in "" _minaper1p0 _stars _stars0p7
    do
      for band in u g r i z
      do
        wdband=${mdfield}/${band}
        echo bash @RUNROOT@/@SCRIPTPATH@/compare_SDSS_K1000.sh \
             ${wdband} \
             $SDSS_cat \
             ${wdband}/${field_name}_${band}_smart${ending}_full.cat \
             ${band} ${field_name}
      done
    done
  fi
done

### Paste the measurements from individual bands
### into a full 9-band catalogue
for mode in ${MODE}
do
  if [ "${mode}" = "MERGE" ]; then

    echo -n set -e \;
    echo -n cp $cat_LDAC ${mdfield}/${field_name}_ugriz.cat_tmp0_$$ \;

    i=0

    ### Loop over all UNIONS bands.
    for band in u g r i z
    do
      if [ -e ${mdfield}/${band}/${field_name}_${band}_smart_full.cat ]
      then
        echo -n ldacrenkey -i ${mdfield}/${band}/${field_name}_${band}_smart_full.cat \
          -o ${mdfield}/${band}/${field_name}_${band}_smart_full_rename.cat_$$ \
          -t OBJECTS -k \
          FLUX_GAAP_${band} FLUX_GAAP_0p7_${band} \
          FLUXERR_GAAP_${band} FLUXERR_GAAP_0p7_${band} \
          MAG_GAAP_${band} MAG_GAAP_0p7_${band} \
          MAGERR_GAAP_${band} MAGERR_GAAP_0p7_${band} \
          FLAG_GAAP FLAG_GAAP_0p7_${band} \
          GAAP_nexp GAAP_nexp_0p7_${band} \
          GAAP_chi_sq_dof GAAP_chi_sq_dof_0p7_${band} \;
        echo -n ldacjoinkey -o ${mdfield}/${field_name}_ugriz.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriz.cat_tmp${i}_$$ \
          -p ${mdfield}/${band}/${field_name}_${band}_smart_full_rename.cat_$$ \
          -t OBJECTS \
          -k MAG_GAAP_0p7_${band} MAGERR_GAAP_0p7_${band} FLUX_GAAP_0p7_${band} \
          FLUXERR_GAAP_0p7_${band} FLAG_GAAP_0p7_${band} GAAP_nexp_0p7_${band} \
          GAAP_chi_sq_dof_0p7_${band} \
          \;
      else
        echo -n ldacaddkey -o ${mdfield}/${field_name}_ugriz.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriz.cat_tmp${i}_$$ \
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
      if [ -e ${mdfield}/${band}/${field_name}_${band}_smart_minaper1p0_full.cat ]
      then
        echo -n ldacrenkey -i ${mdfield}/${band}/${field_name}_${band}_smart_minaper1p0_full.cat \
          -o ${mdfield}/${band}/${field_name}_${band}_smart_minaper1p0_full_rename.cat_$$ \
          -t OBJECTS -k \
          FLUX_GAAP_${band} FLUX_GAAP_1p0_${band} \
          FLUXERR_GAAP_${band} FLUXERR_GAAP_1p0_${band} \
          MAG_GAAP_${band} MAG_GAAP_1p0_${band} \
          MAGERR_GAAP_${band} MAGERR_GAAP_1p0_${band} \
          FLAG_GAAP FLAG_GAAP_1p0_${band} \
          GAAP_nexp GAAP_nexp_1p0_${band} \
          GAAP_chi_sq_dof GAAP_chi_sq_dof_1p0_${band} \;
        echo -n ldacjoinkey -o ${mdfield}/${field_name}_ugriz.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriz.cat_tmp${i}_$$ \
          -p ${mdfield}/${band}/${field_name}_${band}_smart_minaper1p0_full_rename.cat_$$ \
          -t OBJECTS \
          -k MAG_GAAP_1p0_${band} MAGERR_GAAP_1p0_${band} FLUX_GAAP_1p0_${band} \
          FLUXERR_GAAP_1p0_${band} FLAG_GAAP_1p0_${band} GAAP_nexp_1p0_${band} \
          GAAP_chi_sq_dof_1p0_${band} \; 
        echo -n rm ${mdfield}/${band}/*_$$ \;
      else
        echo -n ldacaddkey -o ${mdfield}/${field_name}_ugriz.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriz.cat_tmp${i}_$$ \
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
      i=$[$i+1]
    done

    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/add_extinction_python2.py \
	 ${mdfield}/${field_name}_ugriz.cat_tmp${i}_$$ \
	 ${mdfield}/${field_name}_ugriz.cat_tmp$[$i+1]_$$ \;

    i=$[$i+1]

    echo -n ldaccalc -i ${mdfield}/${field_name}_ugriz.cat_tmp${i}_$$ \
      -o ${mdfield}/${field_name}_ugriz.cat_tmp$[$i+1]_$$ \
      -t OBJECTS \
      -c \"EXTINCTION\*4.239\;\" -n EXTINCTION_u  \"Galactic extinction in the u band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION\*3.303\;\" -n EXTINCTION_g  \"Galactic extinction in the g band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION\*2.285\;\" -n EXTINCTION_r  \"Galactic extinction in the r band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION\*1.698\;\" -n EXTINCTION_i  \"Galactic extinction in the i band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION\*1.263\;\" -n EXTINCTION_z  \"Galactic extinction in the z band \(mag\)\" -k FLOAT \;\

    i=$[$i+1]

    echo -n ldacaddkey -i ${mdfield}/${field_name}_ugriz.cat_tmp${i}_$$ \
      -o ${mdfield}/${field_name}_ugriz.cat_tmp \
      -t OBJECTS -k \
      MP_NAME \"${field_name}\" STRING \"Name of the pointing in MegaPipe convention\" \
      THELI_NAME \"$THELI_name\" STRING \"Name of the pointing in THELI convention\" \; 

    echo -n rm ${mdfield}/*_$$ \;

    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
	 @RUNROOT@/@SCRIPTPATH@/convert_fluxes_to_magnitudes5.py \
	 ${mdfield}/${field_name}_ugriz.cat_tmp \
	 ${mdfield}/${field_name}_ugriz.cat \;

    echo rm ${mdfield}/${field_name}_ugriz.cat_tmp
  fi
done

### Run BPZ on ugri-bands ONLY
for mode in ${MODE}
do
  if [ "${mode}" = "BPZ" ]; then
    echo -n "set -e ; "
    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
	 @RUNROOT@/@SCRIPTPATH@/add_maglim5.py ${mdfield}/${field_name}_ugriz.cat \
	 ${mdfield}/${field_name}_ugri_maglim.cat \;
    echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
      ${mdfield}\
      ${field_name}_ugri_maglim.cat \
      \"u g r i\" \
      MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
      0.01 \;
    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
	 @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugriz.py \
	 ${mdfield}/${field_name}_ugri_maglim_photoz.cat \
	 ${mdfield}/${field_name}_ugri_photoz_ext.cat_tmp_$$ \;
    echo -n ldacaddtab -i ${mdfield}/${field_name}_ugri_photoz_ext.cat_tmp_$$ \
      -o ${mdfield}/${field_name}_ugri_photoz_ext.cat_tmp2_$$ \
      -p ${mdfield}/${field_name}_ugri_maglim_photoz.cat \
      -t FIELDS \;
    echo -n ldacdelkey -i ${mdfield}/${field_name}_ugri_photoz_ext.cat_tmp2_$$ \
      -o ${mdfield}/${field_name}_ugri_photoz_ext.cat \
      -t OBJECTS \
      -k  \
      MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i MAG_LIM_0p7_z \
      MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i MAG_LIM_1p0_z \;
    echo rm ${mdfield}/${field_name}_ugri_photoz_ext.cat_tmp_$$ \
      ${mdfield}/${field_name}_ugri_photoz_ext.cat_tmp2_$$ \
      ${mdfield}/${field_name}_ugri_maglim.cat \
      ${mdfield}/${field_name}_ugri_maglim_photoz.cat \
      ${mdfield}/BPZ_photoz/${field_name}_ugri_maglim_photoz.probs \
      ${mdfield}/BPZ_photoz/${field_name}_ugri_maglim_photoz.flux_comparison
  fi
done

### Run BPZ
for mode in ${MODE}
do
  if [ "${mode}" = "BPZ5" ]; then
    echo -n "set -e ; "
    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
	 @RUNROOT@/@SCRIPTPATH@/add_maglim5.py ${mdfield}/${field_name}_ugriz.cat \
	 ${mdfield}/${field_name}_ugriz_maglim.cat \;
    echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
      ${mdfield}\
      ${field_name}_ugriz_maglim.cat \
      \"u g r i z\" \
      MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
      0.01 \;
    echo -n @RUNROOT@/INSTALL/anaconda2/bin/python \
	 @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugriz.py \
	 ${mdfield}/${field_name}_ugriz_maglim_photoz.cat \
	 ${mdfield}/${field_name}_ugriz_photoz_ext.cat_tmp_$$ \;
    echo -n ldacaddtab -i ${mdfield}/${field_name}_ugriz_photoz_ext.cat_tmp_$$ \
      -o ${mdfield}/${field_name}_ugriz_photoz_ext.cat_tmp2_$$ \
      -p ${mdfield}/${field_name}_ugriz_maglim_photoz.cat \
      -t FIELDS \;
    echo -n ldacdelkey -i ${mdfield}/${field_name}_ugriz_photoz_ext.cat_tmp2_$$ \
      -o ${mdfield}/${field_name}_ugriz_photoz_ext.cat \
      -t OBJECTS \
      -k  \
      MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i \
      MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i \;
    echo rm ${mdfield}/${field_name}_ugriz_photoz_ext.cat_tmp_$$ \
      ${mdfield}/${field_name}_ugriz_photoz_ext.cat_tmp2_$$ \
      ${mdfield}/${field_name}_ugriz_maglim.cat \
      ${mdfield}/${field_name}_ugriz_maglim_photoz.cat \
      ${mdfield}/BPZ_photoz/${field_name}_ugriz_maglim_photoz.probs \
      ${mdfield}/BPZ_photoz/${field_name}_ugriz_maglim_photoz.flux_comparison
  fi
done

### Comparison to SDSS redshifts.
### Full tile.
for mode in ${MODE}
do
    if [ "${mode}" = "COMPTILEZ" ]; then
	for cat in ugri_photoz ugriz_photoz
	do
	    filters=`echo $cat|cut -d "_" -f 1`
	    cat_file=${mdfield}/${field_name}_${cat}_ext.cat
	    if [ -e $cat_file ]
	    then
		for label in SDSS specz
		do
		    case $label in
			SDSS) specz_cat=${mdfield}/SDSS/${field_name}_sdssdr10_galz.cat;;
			specz) specz_cat=${mdfield}/specz/${field_name}_redshifts-2024-01-04.cat;;
		    esac
		    if [ -e $specz_cat ]
		    then
			echo bash @RUNROOT@/@SCRIPTPATH@/compare_z.sh \
			     ${mdfield} \
			     $specz_cat \
			     ${cat_file} \
			     ${field_name} \
			     $label \
			     $filters
		    fi
		done
	    fi
	done
    fi
done

### Mask.
for mode in ${MODE}
do
    if [ "${mode}" = "MASK" ]; then
	echo -n @RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic \
	     \'0 1 \%1 0 \> \? 0 2 \%2 0 \> \? \+ 0 4 \%3 0 \> \? \+ 0 8 \%4 0 \> \? \+ 0 16 \%5 0 \> \? \+\' \
	     ${mdfield}/[ugriz]/${field_name}_?.weight.fits \
	     \> ${mdfield}/${field_name}_ugriz.mask.fits \;
	echo -n gzip -c ${mdfield}/${field_name}_ugriz.mask.fits \
	     \> ${mdfield}/${field_name}_ugriz.mask.fits.gz \;
	echo -n cd /arc/home/hendrik/src/automask/scripts/Linux_64 \;
	echo -n export INSTRUMENT\=MEGAPRIME \;
	echo -n bash ./maskstars.sh \
	          $mdfield/r/ \
		  ${field_name}_r.fits \
		  ${field_name}_r.weight.fits \
		  MEGAPRIME_mask.ini \;
	echo
    fi
done

### QC.
for mode in ${MODE}
do
  if [ "${mode}" = "QC" ]; then
      #### Width of stellar locus.
      for band in u g r i z
      do
	  echo @RUNROOT@/INSTALL/anaconda2/bin/python \
	       @RUNROOT@/@SCRIPTPATH@/width_stellar_locus.py \
               ${mdfield}/${band}/${field_name}_${band}_star_cat_GAaP
      done
  fi
done

### CLEAN.
for mode in ${MODE}
do
  if [ "${mode}" = "CLEAN" ]; then
      #### Clean up temporary and duplicated data products.
      for band in u g r i z
      do
	  echo rm -f \
	     ${mdfield}/${band}/${field_name}_${band}.fits \
	     ${mdfield}/${band}/${field_name}_${band}.weight.fits \
	     ${mdfield}/${band}/${field_name}_${band}.flag.fits \
	     ${mdfield}/${band}/${field_name}_${band}.weight.small.fits \
	     ${mdfield}/${band}/${field_name}_${band}.small.fits \
	     ${mdfield}/${band}/${field_name}_${band}_smart_ggpsf.fits
      done
      echo rm -f \
	   ${mdfield}/${field_name}_ugriz.mask.fits \
	   ${mdfield}/*tmp* \
	   ${mdfield}/make* \
	   ${mdfield}/merg* \
	   ${mdfield}/CFIS*
  fi
done

### COPY.
for mode in ${MODE}
do
  if [ "${mode}" = "COPY" ]; then
      #### Copy data from /scratch to permanent storage.
      echo rsync -atvu ${mdfield} /arc/home/hendrik/UNIONS/UNIONS5000/
  fi
done

### COPY.
for mode in ${MODE}
do
  if [ "${mode}" = "COPYBACK" ]; then
      #### Copy data from /scratch to permanent storage.
      echo rsync -atvu /arc/home/hendrik/UNIONS/UNIONS5000/${field_name} ${md}/
  fi
done

