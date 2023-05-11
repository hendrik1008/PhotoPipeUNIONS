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
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/:${PATH}
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

test ! -d ${mdfield} && mkdir ${mdfield}

### Read MP xxx yyy from field name
xxx=`echo $field_name|cut -d "." -f 2`
yyy=`echo $field_name|cut -d "." -f 3`

### THELI name
THELI_name=`python @RUNROOT@/@SCRIPTPATH@/translate_THELI2MP.py $xxx.$yyy`
RA=`echo  $THELI_name | cut -d "_" -f 2 | sed 's/p/\./g'`
Dec=`echo $THELI_name | cut -d "_" -f 3 | sed 's/p/\./g'`

### Paths to the photometric catalogues.
phot_cat=${cats_dir}/CFIS.${xxx}.${yyy}.r.cat

#### Count the number of objects in the photometric and star catalogues.
#if [ -f $phot_cat ]
#then 
#  no_obj_phot_cat=`ldacdesc -i $phot_cat | \
#    grep elements | awk '{if (NR==1) print $0}' | \
#    cut -d " " -f 3 | sed 's/\.//g' | cut -d ":" -f 2`
#else 
#  >&2 echo "ERROR: the catalogue does not exist?!"
#  exit 1
#fi 

##################################
### Here the real work starts. ###
##################################

### Convert MegaPipe ASCII catalogue into 
for mode in ${MODE}
do
    if [ "${mode}" = "CONVERT" ]; then
	cat_ASCII=$image_dir/catalogues_MP/CFIS.${xxx}.${yyy}.r.cat
	cat_LDAC=$image_dir/catalogues_MP/CFIS.${xxx}.${yyy}.r.ldac.cat
	if [ -f $cat_ASCII ] && [ ! -f $cat_LDAC ]
	then
	    echo asctoldac_theli \
		 -a $cat_ASCII \
		 -o $cat_LDAC \
		 -c @RUNROOT@/@CONFIGPATH@/asctoldac_MP.conf
	fi
    fi
done

### Prepare images and directories.
for mode in ${MODE}
do
    if [ "${mode}" = "PREPARE" ]; then
	
	### Loop over all VISTA bands.
	for band in u g r i
	do
	    ### Create band directory.
	    wdband=${mdfield}/${band}
	    test ! -d ${wdband} && mkdir ${wdband}
	done
	# MegaPipe u- and r-bands
	for filter in u r
	do
    	    prefix=CFIS
    	    base=$image_dir/${filter}/${prefix}.${xxx}.${yyy}.${filter}
	    if [ -f $base.fits ]
	    then
    		ln -sf $base.fits $md/$field_name/$filter/${field_name}_${filter}.fits
	    fi
	    if [ -f $base.weight.fits.fz ] && [ ! -f $base.weight.fits ]
	    then
		echo -n funpack $base.weight.fits.fz -O $base.weight.fits \;
	    fi
    	    test -f $md/$field_name/$filter/${field_name}_${filter}.weight.fits \
		&& rm $md/$field_name/$filter/${field_name}_${filter}.weight.fits
	    if [ -f $base.weight.fits ]
	    then
    		echo -n python @RUNROOT@/@SCRIPTPATH@/extract_MPweight.py \
    		     $base.weight.fits \
    		     $md/$field_name/$filter/${field_name}_${filter}.weight.fits \;
	    fi
	done
	
	# PanSTARRS i-band
	filter=i
	prefix=PS-DR3
	base=$image_dir/${filter}/${prefix}.${xxx}.${yyy}.${filter}
	if [ -f $base.fits ]
	then
	    ln -sf $base.fits        $md/$field_name/$filter/${field_name}_${filter}.fits
	    ln -sf $base.weight.fits $md/$field_name/$filter/${field_name}_${filter}.weight.fits
	fi
	
	# HSC g-band
	filter=g
	prefix=calexp-CFIS_
	base=$image_dir/${filter}/${prefix}${THELI_name}
	if [ -f $base.fits ]
	then
	    echo python3 @RUNROOT@/@SCRIPTPATH@/extract_HSC.py \
    		 $base.fits \
    		 $md/$field_name/$filter/${field_name}_${filter}
	fi
    fi
done

### Gaussianise the VIKING Chips
for mode in ${MODE}
do
  if [ "${mode}" = "GAUSSIANISE" ]; then

    ### Loop over all VISTA bands.
    for band in u g r i
    do
      ### band directory.
      wdband=${mdfield}/${band}

      image=$wdband/${field_name}_${band}.fits
      base=`basename $image .fits`
      gaussianised_image=${wdband}/${base}_smart_ggpsf.fits
      # Check for gaussianised images 
      if [ ! -f ${gaussianised_image} ]
      then 
	  echo -n "echo $base ; "
	  echo -n "cd $wdband ; "
          echo bash @RUNROOT@/@SCRIPTPATH@/gaussianise_chip.sh \
               $wdband/ \
               $image \
               @RUNROOT@/INSTALL/gapphot_TE/ \
               ${band} 
      fi
    done
  fi
done

### Extract GaAP photometry.
for mode in ${MODE}
do
  if [ "${mode}" = "GAAP" ]; then

    cat_LDAC=$image_dir/catalogues_MP/CFIS.${xxx}.${yyy}.r.ldac.cat
    ### Loop over all bands.
    for band in u g r i
    do
      ### band directory.
      wdband=${mdfield}/${band}

      image=${wdband}/${field_name}_${band}.fits
      allCounter=$((allCounter+1))
      base=`basename $image .fits`
      gaussianised_image=${wdband}/${base}_smart_ggpsf.fits
      # Check for gaussianised images 
      if [ ! -f ${gaussianised_image} ]
      then 
          >&2 echo WARNING: Gaussianised image does not exist ${gaussianised_image}
          exit 1
      else 
	  ####################################
	  ### This is the main work script ###
	  ####################################
	  echo -n bash -xv @RUNROOT@/@SCRIPTPATH@/dogauss_smart_VIKING_KiDSLegacy.sh \
               $wdband \
               $image \
               $gaussianised_image \
               $cat_LDAC \
               ALPHA_J2000 \
               DELTA_J2000 \
               @RUNROOT@/INSTALL/gapphot_TE/ \
               ${band} \;\ 
	  echo ls -l $wd/*.gaap \>\> $LOGFILE
	  ####################################
      fi 
    done
  fi
done

### Preparation of SDSS catalogue.
for mode in ${MODE}
do
  if [ "${mode}" = "SDSSPREP" ]; then
      echo -n mkdir ${mdfield}/SDSS \;\ 
      echo -n bash @RUNROOT@/@SCRIPTPATH@/retrieve_sloan.sh ${mdfield}/SDSS/ $field_name $RA $Dec \;\ 
  fi
done

### Comparisons to 2MASS (JHKs bands) and SDSS (Z band).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILE" ]; then
    SDSS_cat=${mdfield}/SDSS/${KiDS_field}_sdssdr8_stars.cat
    ### Loop over all VISTA bands.
    for ending in "" _minaper1p0 _stars _stars0p7
    do
      for band in Z Y J H Ks
      do
        wdband=${mdfield}/${band}
        ### Comparison to 2MASS
        echo bash @RUNROOT@/@SCRIPTPATH@/compare_2MASS_K1000.sh \
          ${wdband} \
          ${mdfield}/2MASS/${KiDS_field}_2MASS.cat \
          ${wdband}/${band}_smart$ending.cat \
          ${band} \
          15 17
        ### Comparison to SDSS if available.
        if [ ${band} = "Z" ]
        then
          echo bash @RUNROOT@/@SCRIPTPATH@/compare_SDSS_K1000.sh \
            ${wdband} \
            $SDSS_cat \
            ${wdband}/${band}_smart$ending.cat \
            ${band} \
            16.5 19
        fi
      done
    done
  fi
done

### Paste the measurements from individual bands
### into a full 9-band catalogue
for mode in ${MODE}
do
  if [ "${mode}" = "MERGE" ]; then

    echo -n set -e \;\ 
    echo -n cp $phot_cat ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp0_$$ \;\ 

    i=0

    ### Loop over all VISTA bands.
    for band in Z Y J H Ks
    do
      if [ -f ${mdfield}/${band}/${band}_smart.cat ]
      then
        echo -n ldacrenkey -i ${mdfield}/${band}/${band}_smart.cat \
          -o ${mdfield}/${band}/${band}_smart_rename.cat_$$ \
          -t OBJECTS -k \
          FLUX_GAAP_${band} FLUX_GAAP_0p7_${band} \
          FLUXERR_GAAP_${band} FLUXERR_GAAP_0p7_${band} \
          MAG_GAAP_${band} MAG_GAAP_0p7_${band} \
          MAGERR_GAAP_${band} MAGERR_GAAP_0p7_${band} \
          FLAG_GAAP FLAG_GAAP_0p7_${band} \
          GAAP_nexp GAAP_nexp_0p7_${band} \
          GAAP_chi_sq_dof GAAP_chi_sq_dof_0p7_${band} \;\ 
        echo -n ldacjoinkey -o ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp${i}_$$ \
          -p ${mdfield}/${band}/${band}_smart_rename.cat_$$ \
          -t OBJECTS \
          -k MAG_GAAP_0p7_${band} MAGERR_GAAP_0p7_${band} FLUX_GAAP_0p7_${band} \
          FLUXERR_GAAP_0p7_${band} FLAG_GAAP_0p7_${band} GAAP_nexp_0p7_${band} \
          GAAP_chi_sq_dof_0p7_${band} \
          \;\ 
      else
        echo -n ldacaddkey -o ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp${i}_$$ \
          -t OBJECTS \
          -k \
          MAG_GAAP_0p7_${band} -99.0 FLOAT \"${band} magnitude\" \
          MAGERR_GAAP_0p7_${band} -99.0 FLOAT \"${band} magnitude error\" \
          FLUX_GAAP_0p7_${band} -99.0 FLOAT \"${band} flux\" \
          FLUXERR_GAAP_0p7_${band} -99.0 FLOAT \"${band} flux error\" \
          FLAG_GAAP_0p7_${band} 1 SHORT \"GAAP photometry Flag\" \
          GAAP_nexp_0p7_${band} 0 SHORT \"GAAP number of exposures\" \
          GAAP_chi_sq_dof_0p7_${band} -99.0 FLOAT \"GAAP chi^2/dof\" \;\ 
      fi
      i=$[$i+1]
      if [ -f ${mdfield}/${band}/${band}_smart_minaper1p0.cat ]
      then
        echo -n ldacrenkey -i ${mdfield}/${band}/${band}_smart_minaper1p0.cat \
          -o ${mdfield}/${band}/${band}_smart_minaper1p0_rename.cat_$$ \
          -t OBJECTS -k \
          FLUX_GAAP_${band} FLUX_GAAP_1p0_${band} \
          FLUXERR_GAAP_${band} FLUXERR_GAAP_1p0_${band} \
          MAG_GAAP_${band} MAG_GAAP_1p0_${band} \
          MAGERR_GAAP_${band} MAGERR_GAAP_1p0_${band} \
          FLAG_GAAP FLAG_GAAP_1p0_${band} \
          GAAP_nexp GAAP_nexp_1p0_${band} \
          GAAP_chi_sq_dof GAAP_chi_sq_dof_1p0_${band} \;\ 
        echo -n ldacjoinkey -o ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp${i}_$$ \
          -p ${mdfield}/${band}/${band}_smart_minaper1p0_rename.cat_$$ \
          -t OBJECTS \
          -k MAG_GAAP_1p0_${band} MAGERR_GAAP_1p0_${band} FLUX_GAAP_1p0_${band} \
          FLUXERR_GAAP_1p0_${band} FLAG_GAAP_1p0_${band} GAAP_nexp_1p0_${band} \
          GAAP_chi_sq_dof_1p0_${band} \;\  
        echo -n rm ${mdfield}/${band}/*_$$ \;\ 
      else
        echo -n ldacaddkey -o ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp$[$i+1]_$$ \
          -i ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp${i}_$$ \
          -t OBJECTS \
          -k \
          MAG_GAAP_1p0_${band} -99.0 FLOAT \"${band} magnitude\" \
          MAGERR_GAAP_1p0_${band} -99.0 FLOAT \"${band} magnitude error\" \
          FLUX_GAAP_1p0_${band} -99.0 FLOAT \"${band} flux\" \
          FLUXERR_GAAP_1p0_${band} -99.0 FLOAT \"${band} flux error\" \
          FLAG_GAAP_1p0_${band} 1 SHORT \"GAAP photometry Flag\" \
          GAAP_nexp_1p0_${band} 0 SHORT \"GAAP number of exposures\" \
          GAAP_chi_sq_dof_1p0_${band} -99.0 FLOAT \"GAAP chi^2/dof\" \;\  
      fi
      i=$[$i+1]
    done

    echo -n ldacaddkey -i ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp${i}_$$ \
      -o ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp$[$i+1]_$$ \
      -t OBJECTS -k \
      THELI_NAME \"${field_name}\" STRING \"Name of the pointing in THELI convention\" \
      KIDS_TILE \"$AW_name\" STRING \"Name of the pointing in AW convention\" \;\  

    # values are the ratios A_X / A_SDSS_u for R_V=3.1 from Table 6 of
    # Schlafly & Finkbeiner 2011, where
    # X = SDSS_z, LSST_y, UKIRT_J, UKIRT_H, UKIRT_K
    echo -n ldaccalc -i ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp$[$i+1]_$$ \
      -o ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp \
      -t OBJECTS \
      -c \"EXTINCTION_u\*0.298\;\" -n EXTINCTION_Z  \"Galactic extinction in the Z band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION_u\*0.257\;\" -n EXTINCTION_Y  \"Galactic extinction in the Y band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION_u\*0.167\;\" -n EXTINCTION_J  \"Galactic extinction in the J band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION_u\*0.106\;\" -n EXTINCTION_H  \"Galactic extinction in the H band \(mag\)\" -k FLOAT \
      -c \"EXTINCTION_u\*0.071\;\" -n EXTINCTION_Ks \"Galactic extinction in the Ks band \(mag\)\" -k FLOAT \
      \;\ 
    echo -n rm ${mdfield}/*_$$ \;\ 

    u_offset=""
    g_offset=""
    r_offset=""
    i1_offset=""
    i2_offset=""
    if [ -f @G2KCORRECTIONSFILE@ ]
    then 
	    u_offset=`grep $AW_name @G2KCORRECTIONSFILE@ | awk 'BEGIN{FS=","}{print $27,$28}'`
    fi 
    if [ "$u_offset" == "" ]
    then 
      if [ -f @UBANDCORRECTIONSFILE@ ]
      then 
	      u_offset=`grep $AW_name @UBANDCORRECTIONSFILE@ | awk 'BEGIN{FS=","}{print $8,$9}'`
      fi 
      if [ "$u_offset" == "" ]
      then 
        u_offset='0.0 0.0'
      fi 
    fi 
    if [ -f @G2KCORRECTIONSFILE@ ]
    then 
	    g_offset=`grep $AW_name @G2KCORRECTIONSFILE@ | awk 'BEGIN{FS=","}{print $29,$30}'`
    fi 
    if [ "$g_offset" == "" ]
    then 
      g_offset='0.0 0.0'
    fi 
    if [ -f @G2KCORRECTIONSFILE@ ]
    then 
	    r_offset=`grep $AW_name @G2KCORRECTIONSFILE@ | awk 'BEGIN{FS=","}{print $31,$32}'`
    fi 
    if [ "$r_offset" == "" ]
    then 
      r_offset='0.0 0.0'
    fi 
    if [ -f @G2KCORRECTIONSFILE@ ]
    then 
	    i1_offset=`grep $AW_name @G2KCORRECTIONSFILE@ | awk 'BEGIN{FS=","}{print $33,$34}'`
    fi 
    if [ "$i1_offset" == "" ]
    then 
      i1_offset='0.0 0.0'
    fi 
    if [ -f @G2KCORRECTIONSFILE@ ]
    then 
	    i2_offset=`grep $AW_name @G2KCORRECTIONSFILE@ | awk 'BEGIN{FS=","}{print $35,$36}'`
    fi 
    if [ "$i2_offset" == "" ]
    then 
      i2_offset='0.0 0.0'
    fi 

    echo -n python @RUNROOT@/@SCRIPTPATH@/convert_fluxes_to_magnitudes.py \
      ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp \
      ${mdfield}/${field_name}_ugriZYJHKs.cat \
      $u_offset $g_offset $r_offset $i1_offset $i2_offset \;\ 

    echo rm ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp
  fi
done

### Run BPZ
for mode in ${MODE}
do
  if [ "${mode}" = "BPZ" ]; then
    echo -n "set -e ; "
    echo -n python @RUNROOT@/@SCRIPTPATH@/add_maglim.py ${mdfield}/${field_name}_ugriZYJHKs_mac.cat \
      ${mdfield}/${field_name}_ugriZYJHKs_maglim.cat \;\ 
    echo -n bash @RUNROOT@/@SCRIPTPATH@/create_bpz_photozs_NGVSprior_KiDS_2017_68CI.sh \
      ${mdfield}\
      ${field_name}_ugriZYJHKs_maglim.cat \
      \"u g r i1 i2 Z Y J H Ks\" \
      MAG_GAAP MAGERR_GAAP MAG_LIM FLAG_GAAP EXTINCTION AB \
      0.01 \;\ 
    echo -n python @RUNROOT@/@SCRIPTPATH@/apply_extinction_ugriZYJHKs.py \
      ${mdfield}/${field_name}_ugriZYJHKs_maglim_photoz.cat \
      ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat_tmp_$$ \;\ 
    echo -n ldacaddtab -i ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat_tmp_$$ \
      -o ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat_tmp2_$$ \
      -p ${mdfield}/${field_name}_ugriZYJHKs_maglim_photoz.cat \
      -t FIELDS \;\ 
    echo -n ldacdelkey -i ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat_tmp2_$$ \
      -o ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat \
      -t OBJECTS \
      -k  \
      MAG_LIM_0p7_u MAG_LIM_0p7_g MAG_LIM_0p7_r MAG_LIM_0p7_i1 MAG_LIM_0p7_i2 MAG_LIM_0p7_Z MAG_LIM_0p7_Y MAG_LIM_0p7_J MAG_LIM_0p7_H MAG_LIM_0p7_Ks \
      MAG_LIM_1p0_u MAG_LIM_1p0_g MAG_LIM_1p0_r MAG_LIM_1p0_i1 MAG_LIM_1p0_i2 MAG_LIM_1p0_Z MAG_LIM_1p0_Y MAG_LIM_1p0_J MAG_LIM_1p0_H MAG_LIM_1p0_Ks \;\ 
    echo rm ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat_tmp_$$ \
      ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat_tmp2_$$ \
      ${mdfield}/${field_name}_ugriZYJHKs_maglim.cat \
      ${mdfield}/BPZ_photoz/${field_name}_ugriZYJHKs_maglim_photoz.probs \
      ${mdfield}/BPZ_photoz/${field_name}_ugriZYJHKs_maglim_photoz.flux_comparison
  fi
done

### Comparison to SDSS redshifts.
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEZ" ]; then
    if [ "$RA" == "" ]
    then 
      >&2 echo "WARNING: Central RA/Dec is approximated from file name (no mask created yet)"
      RA=` echo $KiDS_field | cut -d '_' -f 2 | sed 's/p/\./g'`
      Dec=`echo $KiDS_field | cut -d '_' -f 3 | sed 's/p/\./g' | sed 's/m/-/g'`
    fi 
    continue=`echo $Dec | awk '{if ($1>-10) print 1; else print 0}'`
    if [ $continue -eq 1 ]
    then
      SDSS_cat=${mdfield}/SDSS/${KiDS_field}_sdssdr8_galz.cat      
      #### Comparison to SDSS.
      echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_SDSS_z_K1000.sh \
        ${mdfield} \
        $SDSS_cat \
        ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat \
        ${field_name} \;\ 
      echo
    else
      echo Southern field. No need to run COMPTILEZ.
    fi
  fi
done
