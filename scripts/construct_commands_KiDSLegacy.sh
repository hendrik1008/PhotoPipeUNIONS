#!/bin/bash

# Script to construct commands needed to process one KiDS field.
# Script is called by run_PhotoPipe.sh and should not be called directly.
#
# Required Inputs:
# - Background-subtracted VISTA chips, returned from the VIKING
#   reduction pipeline: 
#   https://github.com/AngusWright/VIKINGProcessingPipeline.git
#   (Written by Angus H. Wright)
# - KiDS 4-band catalogues from AstroWISE.
#
# Output:
# - 9-band photometric catalogues with photoz 
# - 9-band photometric masks 
#
# Author: Angus H Wright
# Adapted from "meta_wrapper_K1000.sh" written by H. Hildebrandt
#
# Version history:
# 2020-08-05 V1.0

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/lib/
set -e
#}}}

# MODES: {{{
#
# 1. CONVERT: Convert AW catalogues.
# 2. COLLECT: Collect VISTA chips.
# 3. LINK: Link VISTA chips.
# 4. PREPARE: Create directories.
# 5. GAAP: Extract GaAP photometry.
# 6. COMBINEPAW: Combine flux measurements of all chips per pawprint. (OPTIONAL)
# 7. COMBINETILE: Combine flux measurements of all chips per tile. 
# 8. 2MASSPREP: Preparation of 2MASS catalogue.
# 9. SDSSPREP: Preparation of SDSS catalogue.
# 10. COMPPAW: Comparisons to 2MASS (JHKs bands) and SDSS (Z band). Individual pawprints. (OPTIONAL)
# 11. COMPTILE: Comparisons to 2MASS (JHKs bands) and SDSS (Z band). Full tile.
# 12. MERGE: Paste the measurements from individual bands into a full 9-band catalogue.
# 13. COMPTILEVST: Comparisons to SDSS (ugri-bands). Full tile.
# 14. STACK: Create a stack and sum image of all chips that went into the photometry.
# 15. BPZ: Run BPZ.
# 16. COMPTILEZ: Comparison to SDSS redshifts. Full tile.
# 17. COMPTILE2DF: Comparison to 2dFLenS redshifts. Full tile.
# 18. MASK4: Create the 4-band MASK.
# 19. MASK: Create the 9-band MASK.
# 20. RAND: Create a new random catalogue.
# 21. COPY: Copy data products into THELI tree.
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
      cats_dir=${2} # Catalogue directory where the KiDS catalogues live.
      shift 2
      ;;
    -bd)
      bd=${2} # Directory where ALL the background-subtracted
      # VISTA chips live.
      shift 2
      ;;
    -id)
      image_dir=${2} # Directory where the background-subtracted
      # VISTA chips for this tile live.
      shift 2
      ;;
    -fi)
      KiDS_field=${2} # KiDS field name (THELI convenction).
      shift 2
      ;;
    -fn)
      field_name="${2}" # Descriptive field name (e.g. COSMOS, G15Deep).
      shift 2
      ;;
    -ma)
      mask="${2}" # Location of mask.
      shift 2
      ;;
    -th)
      THELIDATAPATH="${2}" # Base directory of THELI tree.
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

#Define the pointing name in AstroWISE convention {{{
AW_name=`echo ${field_name} | sed 's/p/\./g' | sed 's/m/\-/g'`
AW_name_new=`echo $AW_name | sed 's/KIDS/@SURVEY@/g'`
#}}}

### Create a working directory
mdfield=$md/${field_name}

test ! -d ${mdfield} && mkdir ${mdfield}

### Read RA and Dec from KiDS field name.

if [ -f ${mask} ]
then
  RA=`dfits ${mask}|fitsort -d CRVAL1|awk '{print $2}'`
  Dec=`dfits ${mask}|fitsort -d CRVAL2|awk '{print $2}'`
fi 

### Paths to the photometric, lensfit, and star catalogues.

phot_cat=${mdfield}/${KiDS_field}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_GAaP_@REFERENCE@.ldac.cat
AW_cat=${cats_dir}/${KiDS_field}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_GAaP_@REFERENCE@.fits

### Count the number of objects in the photometric and star catalogues.

if [ -f $phot_cat ]
then 
  no_obj_phot_cat=`ldacdesc -i $phot_cat | \
    grep elements | awk '{if (NR==1) print $0}' | \
    cut -d " " -f 3 | sed 's/\.//g' | cut -d ":" -f 2`
elif [ -f $AW_cat ]
then 
  no_obj_phot_cat=`dfits $AW_cat | \
    grep NAXIS2 | awk '{print $3}'`
else 
  >&2 echo "ERROR: the catalogue does not exist?!"
  exit 1
fi 

##################################
### Here the real work starts. ###
##################################

### Convert AW catalogues.
for mode in ${MODE}
do
  if [ "${mode}" = "CONVERT" ]; then
    echo bash @RUNROOT@/@SCRIPTPATH@/convert_AW.sh ${cats_dir} ${field_name} ${mdfield}/
  fi
done

### Collect VISTA chips.
for mode in ${MODE}
do
  if [ "${mode}" = "COLLECT" ]; then
    test ! -d ${image_dir} && mkdir ${image_dir}
    test ! -d ${image_dir}/${KiDS_field} && mkdir ${image_dir}/${KiDS_field}
    WCS_cuts=`grep $KiDS_field @POINTINGLIMITSFILE@ | awk '{printf "%f %f %f %f", $2,$3,$4,$5}'`

    for filter in Z Y J H Ks
    do
      #Check that the VIKING chip list is present
      if [ ! -f @RUNROOT@/@CONFIGPATH@/VIKING_${filter}_wcs.txt ]
      then 
        #Construct the chip list
        chiplist=`find  @VIKINGROOT@/@VIKINGTYPE@/${filter}/ | grep "_r.fits"`
        for chip in ${chiplist}
        do 
          if [ -f ${chip} ]
          then 
            chipRA=`dfits ${chip}|fitsort -d CRVAL1|awk '{print $2}'`
            chipDec=`dfits ${chip}|fitsort -d CRVAL2|awk '{print $2}'`
            echo `basename ${chip} .fits` ${chipRA} ${chipDec} >> @RUNROOT@/@CONFIGPATH@/VIKING_${filter}_wcs.txt
          fi 
        done
        #>&2 echo "ERROR: the VIKING Chip List is not available for Filter ${filter}"
        #exit 1
      fi
      #Construct the list of chips for this pointing
      test ! -d ${image_dir}/${KiDS_field}/${filter} && mkdir ${image_dir}/${KiDS_field}/${filter}
      echo bash @RUNROOT@/@SCRIPTPATH@/collect_chips.sh \
        @RUNROOT@/@CONFIGPATH@/VIKING_${filter}_wcs.txt \
        ${WCS_cuts} \
        \> ${image_dir}/${KiDS_field}/${filter}/chips_list.txt
    done
  fi
done

### Link VISTA chips.
for mode in ${MODE}
do
  if [ "${mode}" = "LINK" ]; then
    for filter in Z Y J H Ks
    do
      awk -v chipdir=$bd/$filter/ -v workdir=${image_dir}/${KiDS_field}/$filter/ \
        '{print "ln -sf " chipdir $1 ".fits "        workdir " ; " \
                "ln -sf " chipdir $1 ".weight.fits " workdir " ; "}' \
                  ${image_dir}/${KiDS_field}/$filter/chips_list.txt
    done
  fi
done

### Create directories.
for mode in ${MODE}
do
  if [ "${mode}" = "PREPARE" ]; then
    ### Loop over all VISTA bands.
    for band in Z Y J H Ks
    do
      ### Create band directory.
      wdband=${mdfield}/${band}
      test ! -d ${wdband} && mkdir ${wdband}

      ### Create a list of all chips.
      ls $image_dir/${KiDS_field}/${band}/ | grep "_r.fits$" > ${wdband}/file_list.txt || \
        >&2 echo "There are no VISTA chips in $image_dir/${KiDS_field}/${band}/"
      nimage=`cat ${wdband}/file_list.txt | wc -l `

      if [ "${nimage}" != "0" ]
      then
        ### Create a list of all pawprints.
        {
          while read file
          do
            basename $file
          done < ${wdband}/file_list.txt
        } | cut -d "_" -f 1-2 |sort | uniq > ${wdband}/pawprint_list.txt

        ### Loop over all pawprints.
        for pawname in `cat ${wdband}/pawprint_list.txt`
        do
          ### Create a pawprint directory.
          wdpaw=${wdband}/${pawname}
          test ! -d ${wdpaw} && mkdir ${wdpaw}

          ### Loop over all chips in this pawprint.
          for image in $image_dir/${KiDS_field}/${band}/${pawname}_*_r.fits
          do
            base=`basename $image _r.fits`
            wd=${wdpaw}/$base
            test ! -d $wd && mkdir $wd
          done
        done
      else 
        echo > ${wdband}/pawprint_list.txt
      fi 
    done
    echo "echo PREPARE has no parallel section. Folders were set up correctly"
  fi
done

### Gaussianise the VIKING Chips
for mode in ${MODE}
do
  if [ "${mode}" = "GAUSSIANISE" ]; then

    ### Loop over all VISTA bands.
    for band in Z Y J H Ks
    do
      ### band directory.
      wdband=${mdfield}/${band}

      ### Loop over all pawprints.
      for pawname in `cat ${wdband}/pawprint_list.txt`
      do
        ### pawprint directory.
        wdpaw=${wdband}/${pawname}
        ### Loop over all chips in this pawprint.
        for image in $image_dir/${KiDS_field}/${band}/${pawname}_*_r.fits
        do
          base=`basename $image _r.fits`
          wd=${image_dir}/all/
          echo -n "echo $base ; "
          echo -n "cd $wd ; "
          if [ ! -d $wd ] 
          then 
            mkdir $wd
          fi 
          gaussianised_image=${image_dir}/all/${base}_r/${base}_r_smart_ggpsf.fits
          # Check for gaussianised images 
          if [ ! -f ${gaussianised_image} ]
          then 
            echo bash @RUNROOT@/@SCRIPTPATH@/gaussianise_chip.sh \
              $wd/$base/ \
              $image \
              @RUNROOT@/INSTALL/gapphot_TE/ \
              ${band} 
          fi
        done
      done
    done
  fi
done

### Extract GaAP photometry.
for mode in ${MODE}
do
  if [ "${mode}" = "GAAP" ]; then

    ### Loop over all VISTA bands.
    missingCounter=0
    allCounter=0
    for band in Z Y J H Ks
    do
      ### band directory.
      wdband=${mdfield}/${band}

      ### Loop over all pawprints.
      for pawname in `cat ${wdband}/pawprint_list.txt`
      do
        ### pawprint directory.
        wdpaw=${wdband}/${pawname}
        ### Loop over all chips in this pawprint.
        for image in $image_dir/${KiDS_field}/${band}/${pawname}_*_r.fits
        do
          allCounter=$((allCounter+1))
          base=`basename $image _r.fits`
          wd=${wdpaw}/$base
          gaussianised_image=${image_dir}/all/${base}/${base}_r_smart_ggpsf.fits
          # Check for gaussianised images 
          if [ ! -f ${gaussianised_image} ]
          then 
            >&2 echo WARNING: Gaussianised image does not exist ${gaussianised_image}
            #exit 1
            missingCounter=$((missingCounter+1))
          else 
            ####################################
            ### This is the main work script ###
            ####################################
            echo -n bash @RUNROOT@/@SCRIPTPATH@/dogauss_smart_VIKING_KiDSLegacy.sh \
              $wd \
              $image \
              $gaussianised_image \
              $phot_cat \
              RAJ2000 \
              DECJ2000 \
              @RUNROOT@/INSTALL/gapphot_TE/ \
              ${band} \;\ 
            echo ls -l $wd/*.gaap \>\> $LOGFILE
            ####################################
          fi 
        done
      done
    done
    missingFraction=`echo $missingCounter $allCounter | awk '{printf "%.2f", $1/$2*100}'`
    errorMissing=`echo $missingCounter $allCounter | awk '{ if ($1/$2 > 0.1) { print "BREAK" } }'`
    if [ "$errorMissing" == "BREAK" ]
    then 
       >&2 echo "ERROR: Too many Gaussianised images do not exist: ${missingFraction}% > 10%"
       exit 1
    else 
       >&2 echo "NB: ${missingFraction}% of the Gaussianised images do not exist!"
    fi 
  fi
done

### Combine flux measurements of different chips per pawprint.
for mode in ${MODE}
do
  if [ "${mode}" = "COMBINEPAW" ]; then
    ### Loop over all VISTA bands.
    for band in Z Y J H Ks
    do
      wdband=${mdfield}/${band}
      ### Loop over all pawprints.
      for pawname in `cat ${wdband}/pawprint_list.txt`
      do
        wdpaw=${wdband}/${pawname}
        ### If there is no individual VISTA chip QC (K1000) ###
        gaap_input_files=${wdpaw}/v*_bsub/v*_bsub_r_smart$ending.gaap
        ### Combine flux measurements for photometric catalogue.
        echo -n python @RUNROOT@/@SCRIPTPATH@/average_fluxes_list.py \
          $no_obj_phot_cat \
          ${wdpaw}/${pawname}_smart.gaap \
          $gaap_input_files \;\ 
        echo bash @RUNROOT@/@SCRIPTPATH@/convert_gaap_fluxes.sh \
          ${wdpaw}/${pawname}_smart.gaap \
          $phot_cat \
          ${wdpaw}/${pawname}_smart.cat \
          ${band} 30 \
          RA DEC
      done
    done
  fi
done

### Combine flux measurements of all chips per tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMBINETILE" ]; then
    ### Loop over all VISTA bands.
    for ending in _minaper1p0 ""
    do
      home=`pwd`
      for band in Z Y J H Ks
      do
        wdband=${mdfield}/${band}
        cd ${wdband}
        ### If there is no individual VISTA chip QC ###
        gaap_input_files=v*/v*_bsub/v*_bsub_r_smart$ending.gaap
        num_input_files=( $gaap_input_files )
        num_input_files=${#num_input_files[@]}
        ### Combine flux measurements for photometric catalogue.
        if [ ${num_input_files} -le 1 ]
        then
          >&2 echo "ERROR: Directory ${wdband} contains no VISTA chips?!"
        else
          echo -n "cd ${wdband} ; "
          echo -n python @RUNROOT@/@SCRIPTPATH@/average_fluxes_list.py \
            $no_obj_phot_cat \
            ${wdband}/${band}_smart$ending.gaap \
            $gaap_input_files \;\  
          echo bash @RUNROOT@/@SCRIPTPATH@/convert_gaap_fluxes.sh \
            ${wdband}/${band}_smart$ending.gaap \
            $phot_cat \
            ${wdband}/${band}_smart$ending.cat \
            ${band} 30 \
            RAJ2000 DECJ2000
        fi 
        cd ${home}
      done
    done
  fi
done

### Preparation of 2MASS catalogue.
for mode in ${MODE}
do
  if [ "${mode}" = "2MASSPREP" ]; then
    if [ ! -f ${mdfield}/2MASS/${field_name}_2MASS.cat ]
    then
      if [ "$RA" == "" ]
      then 
        >&2 echo "WARNING: Central RA/Dec is approximated from file name (no mask created yet)"
        RA=` echo $KiDS_field | cut -d '_' -f 2 | sed 's/p/\./g'`
        Dec=`echo $KiDS_field | cut -d '_' -f 3 | sed 's/p/\./g' | sed 's/m/-/g'`
      fi 
      echo -n mkdir ${mdfield}/2MASS/ \;\  
      echo bash @RUNROOT@/@SCRIPTPATH@/prepare_2MASS_2018-04-10.sh \
        ${mdfield}/2MASS/ \
        ${KiDS_field} $RA $Dec
    fi
  fi
done

### Preparation of SDSS catalogue.
for mode in ${MODE}
do
  if [ "${mode}" = "SDSSPREP" ]; then
    if [ "$RA" == "" ]
    then 
      >&2 echo "WARNING: Central RA/Dec is approximated from file name (no mask created yet)"
      RA=` echo $KiDS_field | cut -d '_' -f 2 | sed 's/p/\./g'`
      Dec=`echo $KiDS_field | cut -d '_' -f 3 | sed 's/p/\./g' | sed 's/m/-/g'`
    fi 
    continue=`echo $Dec | awk '{if ($1>-10) print 1; else print 0}'`
    if [ $continue -eq 1 ]
    then
      echo -n mkdir ${mdfield}/SDSS \;\ 
      echo -n bash @RUNROOT@/@SCRIPTPATH@/retrieve_sloan.sh ${mdfield}/SDSS/ $KiDS_field $RA $Dec \;\ 
    else 
      echo "echo 'Field is in the South'"
    fi
  fi
done

### Comparisons to 2MASS (JHKs bands) and SDSS (Z band).
### Individual pawprints.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPPAW" ]; then
    SDSS_cat=${mdfield}/SDSS/${KiDS_field}_sdssdr8_stars.cat

    ### Loop over all VISTA bands.
    for band in Z Y J H Ks
    do
      wdband=${mdfield}/${band}
      ### Loop over all pawprints.
      for pawname in `cat ${wdband}/pawprint_list.txt`
      do
        wdpaw=${wdband}/${pawname}
        ### Comparison to 2MASS
        echo bash @RUNROOT@/@SCRIPTPATH@/compare_2MASS_K1000.sh \
          ${wdpaw} \
          ${mdfield}/2MASS/${KiDS_field}_2MASS.cat \
          ${wdpaw}/${pawname}_smart.cat \
          ${band} \
          15 17

        ### Comparison to SDSS z-band if available.
        if [ ${band} = "Z" ]
        then
          echo bash @RUNROOT@/@SCRIPTPATH@/compare_SDSS_K1000.sh \
            ${wdpaw} \
            $SDSS_cat \
            ${wdpaw}/${pawname}_smart.cat \
            ${band} \
            16.5 19
        fi
      done
    done
  fi
done

### Comparisons to 2MASS (JHKs bands) and SDSS (Z band).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILE" ]; then
    SDSS_cat=${mdfield}/SDSS/${KiDS_field}_sdssdr8_stars.cat
    ### Loop over all VISTA bands.
    for ending in _minaper1p0 ""
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

    if [ -f @UBANDCORRECTIONSFILE@ ]
    then 
      u_offset=`grep $AW_name_new @UBANDCORRECTIONSFILE@ | awk '{print $2}'`
    else 
      u_offset='0.0'
    fi 

    echo -n python @RUNROOT@/@SCRIPTPATH@/convert_fluxes_to_magnitudes.py \
      ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp \
      ${mdfield}/${field_name}_ugriZYJHKs.cat \
      $u_offset \;\ 

    echo rm ${mdfield}/${field_name}_ugriZYJHKs.cat_tmp
  fi
done

### Comparisons to SDSS (ugri-bands).
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEVST" ]; then
    if [ "$RA" == "" ]
    then 
      >&2 echo "WARNING: Central RA/Dec is approximated from file name (no mask created yet)"
      RA=` echo $KiDS_field | cut -d '_' -f 2 | sed 's/p/\./g'`
      Dec=`echo $KiDS_field | cut -d '_' -f 3 | sed 's/p/\./g' | sed 's/m/-/g'`
    fi 
    continue=`echo $Dec | awk '{if ($1>-10) print 1; else print 0}'`
    if [ $continue -eq 1 ]
    then
      SDSS_cat=${mdfield}/SDSS/${KiDS_field}_sdssdr8_stars.cat
      ### Loop over all VST bands.
      for band in u g r i1 i2
      do
        wdband=${mdfield}/${band}
        test ! -d ${wdband} && mkdir ${wdband}
        #### Comparison to SDSS if available.
        echo bash @RUNROOT@/@SCRIPTPATH@/compare_SDSS_VST_K1000.sh \
          ${wdband} \
          $SDSS_cat \
          ${mdfield}/${field_name}_ugriZYJHKs.cat \
          ${band} \
          16.5 19
      done
    else
      echo Southern field. No need to run COMPTILEVST.
    fi
  fi
done

### Create a stack of all chips that went into the photometry
for mode in ${MODE}
do
  if [ "${mode}" = "STACK" ]; then
    ### Loop over all VISTA bands.
    for filter in Z Y J H Ks
    do
      nr=`wc ${image_dir}/${KiDS_field}/$filter/chips_list.txt | awk '{print $1}'`

      ### Science image stack (NOT USED) ###
      ###if [ ! -f $md/${KiDS_field}/$filter/${KiDS_field}_${filter}_swarp.fits ] && [ $nr -gt 0 ]
      ###then
      ###    input_files=${image_dir}/${KiDS_field}/$filter/*_r.fits
      ###    echo -n cd $md/${KiDS_field}/$filter/ \;
      ###    echo ~/src/KiDS-VIKING/swarp $input_files \
        ###	     -NTHREADS 1 \
        ###	     -BACK_TYPE MANUAL \
        ###	     -IMAGEOUT_NAME  ${KiDS_field}_${filter}_swarp.fits \
        ###	     -WEIGHTOUT_NAME ${KiDS_field}_${filter}_swarp.weight.fits
      ###fi

      if [ "$nr" != "" ] 
      then 
        if [ $nr -gt 0 ] && [ ! -f $md/${KiDS_field}/$filter/${KiDS_field}_${filter}_swarp.sum.fits ]
        then
          weights=${image_dir}/${KiDS_field}/$filter/*_r.weight.fits
          input_files01=""
          for file in $weights
          do
            chip=`basename $file | cut -d "_" -f 1-4`
            paw=`echo $chip | cut -d "_" -f 1-2`
            chip_dir=$md/$KiDS_field/$filter/$paw/${chip}_bsub
            gaap_qc_flag=0 #`grep $chip_dir $md/ALL_KiDZ.QC.update | ${P_GAWK} '{print $1}'`
            Angus_qc_flag=1 #`grep -c ${chip}_bsub $md/QC_passed_filelist.dat`
            if [ $gaap_qc_flag -lt 16 ] && [ $Angus_qc_flag -ge 1 ] && [ -f $chip_dir/${chip}_bsub_r_smart.gaap ]
            then
              echo -n "ic '1 0 %1 1.0e-06 > ?'" $file ">" $file.01.fits \;\ 
              input_files01=$input_files01" "$file.01.fits
            fi
          done
          echo -n cd $md/${KiDS_field}/$filter/ \;\ 
          if [ "$RA" == "" ]
          then 
            >&2 echo "WARNING: Central RA/Dec is approximated from file name (no mask created yet)"
            RA=` echo $KiDS_field | cut -d '_' -f 2 | sed 's/p/\./g'`
            Dec=`echo $KiDS_field | cut -d '_' -f 3 | sed 's/p/\./g' | sed 's/m/-/g'`
          fi 
          ### THIS NEEDS TO BE TESTED WITH swarp_theli!!! ###
          echo -n swarp $input_files01 \
            -NTHREADS 1 \
            -BACK_TYPE MANUAL -COMBINE_TYPE SUM \
            -PIXELSCALE_TYPE MANUAL -PIXEL_SCALE 0.214 \
            -CENTER_TYPE MANUAL -CENTER $RA,$Dec \
            -IMAGE_SIZE 21000,21000 \
            -RESAMPLING_TYPE NEAREST -FSCALASTRO_TYPE NONE \
            -IMAGEOUT_NAME  ${KiDS_field}_${filter}_swarp.sum.fits \
            -WEIGHTOUT_NAME ${KiDS_field}_${filter}_swarp.sum.weight.fits \;\ 
          echo rm $input_files01 ${KiDS_field}_${filter}_swarp.sum.weight.fits swarp.xml
        fi

        ### If there is no data we create an empty sum image ###
        if [ $nr -eq 0 ] && [ ! -f $md/${KiDS_field}/$filter/${KiDS_field}_${filter}_swarp.sum.fits ]
        then
          echo -n ic -c 21000 21000 \'0\' \> $md/${KiDS_field}/$filter/${KiDS_field}_${filter}_swarp.sum.tmp.fits \;\ 
          echo -n python @RUNROOT@/@SCRIPTPATH@/add_header.py \
            $md/${KiDS_field}/$filter/${KiDS_field}_${filter}_swarp.sum.tmp.fits \
            $md/${KiDS_field}/$filter/${KiDS_field}_${filter}_swarp.sum.fits \
            ${THELIDATAPATH}/${field_name}/@THELIFILTER@/coadd_@THELIVERSION@/${KiDS_field}_@THELIFILTER@.@THELIVERSION@.swarp.cut.fits \;\ 
          echo rm $md/${KiDS_field}/$filter/${KiDS_field}_${filter}_swarp.sum.tmp.fits
        fi
      fi
    done
  fi
done

### Run BPZ
for mode in ${MODE}
do
  if [ "${mode}" = "BPZ" ]; then
    echo -n "set -e ; "
    echo -n python @RUNROOT@/@SCRIPTPATH@/add_maglim.py ${mdfield}/${field_name}_ugriZYJHKs.cat \
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

### Comparison to 2dFLenS redshifts.
### Full tile.
for mode in ${MODE}
do
  if [ "${mode}" = "COMPTILEZ2DF" ]; then
    if [ "$RA" == "" ]
    then 
      >&2 echo "WARNING: Central RA/Dec is approximated from file name (no mask created yet)"
      RA=` echo $KiDS_field | cut -d '_' -f 2 | sed 's/p/\./g'`
      Dec=`echo $KiDS_field | cut -d '_' -f 3 | sed 's/p/\./g' | sed 's/m/-/g'`
    fi 
    continue=`echo $Dec | awk '{if ($1<=-10) print 1; else print 0}'`
    if [ $continue -eq 1 ] && [ -f @2DFLENSCATALOGUE@ ]
    then
      #### Comparison to SDSS.
      echo -n bash @RUNROOT@/@SCRIPTPATH@/compare_2dFLenS_z_K1000.sh \
        ${mdfield} \
        @TWODFLENSCATALOGUE@ \
        ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat \
        ${field_name} \;\ 
      echo
    else
      echo Northern field. No need to run COMPTILEZ2DF.
    fi
  fi
done

### Create the 4-band MASK
for mode in ${MODE}
do
  if [ "${mode}" = "MASK4" ]; then
    for filter in u g r i
    do
      #if [ -f ${cats_dir}/${field_name}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_Pulecenella_${filter}.fits ]
      #then
      #  echo -n gzip -c ${cats_dir}/${field_name}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_Pulecenella_${filter}.fits \
      #    \> ${cats_dir}/${field_name}_${filter}_mask_AW.fits.gz \;\ 
      #elif [ -f ${cats_dir}/${field_name}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_Pulecenella_${filter}.fits.gz ]
      #then
        echo -n ln -sf ${cats_dir}/${field_name}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_Pulecenella_${filter}.fits.gz \
          ${mdfield}/${field_name}_${filter}_mask_AW.fits.gz \;\ 
      #fi
    done

    if [ -f ${THELIDATAPATH}/@THELIFILTER@/coadd_@THELIVERSION@/${field_name}_@THELIFILTER@.@THELIVERSION@.swarp.cut.flag.fits.gz ] && \
       [ ! -f ${THELIDATAPATH}/@THELIFILTER@/coadd_@THELIVERSION@/${field_name}_@THELIFILTER@.@THELIVERSION@.swarp.cut.flag.fits ]
    then
	echo -n gunzip -c ${THELIDATAPATH}/@THELIFILTER@/coadd_@THELIVERSION@/${field_name}_@THELIFILTER@.@THELIVERSION@.swarp.cut.flag.fits.gz \
	     \> ${THELIDATAPATH}/@THELIFILTER@/coadd_@THELIVERSION@/${field_name}_@THELIFILTER@.@THELIVERSION@.swarp.cut.flag.fits \;\ 
    fi

  ### create the combined flag file
  echo python @RUNROOT@/@SCRIPTPATH@/make_KIDS_bitmask.py \
    ${field_name} @THELIVERSION@ \"r_SDSS u_SDSS g_SDSS i_SDSS\" \
    ${mdfield} ${THELIDATAPATH} ${mdfield} @POINTINGLIMITSFILE@ ${mask}
fi
done

### Create the 9-band MASK
for mode in ${MODE}
do
  if [ "${mode}" = "MASK" ]; then
    RAmin=`grep  ${field_name} @POINTINGLIMITSFILE@ | awk '{print $2}'`
    RAmax=`grep  ${field_name} @POINTINGLIMITSFILE@ | awk '{print $3}'`
    Decmin=`grep ${field_name} @POINTINGLIMITSFILE@ | awk '{print $4}'`
    Decmax=`grep ${field_name} @POINTINGLIMITSFILE@ | awk '{print $5}'`
    if [ ! -f ${mdfield}/${field_name}_AW_THELI_NIR.mask.fits ]
    then
      for band in Z Y J H Ks
      do
        if [ ! -f $md/${field_name}/${band}/${field_name}_${band}_swarp_cut.sum.fits ]
        then
          echo -n python @RUNROOT@/@SCRIPTPATH@/mosaic/add_WCS_cuts_to_sum_image.py \
            $md/${field_name}/${band}/${field_name}_${band}_swarp.sum.fits \
            $md/${field_name}/${band}/${field_name}_${band}_swarp_cut.sum.fits \
            $RAmin $RAmax $Decmin $Decmax \;\ 
        fi
      done
      echo -n bash @RUNROOT@/@SCRIPTPATH@/create_9band_mask.sh \
        ${mdfield}\
        ${field_name} \
        ${mask} \
        $RA $Dec \;\ 
      for band in Z Y J H Ks
      do
        if [ -f $md/${field_name}/${band}/${field_name}_${band}_swarp_cut.sum.fits ]
        then
          echo -n gzip $md/${field_name}/${band}/${field_name}_${band}_swarp_cut.sum.fits \;\ 
        fi
      done
      echo -n bash @RUNROOT@/@SCRIPTPATH@/addmask_fits_WCS.sh \
        ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat \
        ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext_mask.cat \
        ${mdfield}/${field_name}_AW_THELI_NIR.mask.fits \
        MASK \
        \"9-band mask information\" \
        LONG \
        ${mdfield}/ \;\ 
      echo gzip -c ${mdfield}/${field_name}_AW_THELI_NIR.mask.fits \
        \> ${mdfield}/${field_name}_AW_THELI_NIR.mask.fits.gz
      echo
    fi
  fi
done

### Create a new random catalogue
for mode in ${MODE}
do
  if [ "${mode}" = "RAND" ]; then
    mask_base=`basename ${mask} .flags.fits`
    if [ -f ${mdfield}/${mask_base}_NIR.mask.fits.gz ] && [ ! -f ${mdfield}/${mask_base}_NIR.mask.fits ]
    then
      echo -n gunzip -c ${mdfield}/${mask_base}_NIR.mask.fits.gz \> ${mdfield}/${mask_base}_NIR.mask.fits \;\ 
    fi
    echo -n bash @RUNROOT@/@SCRIPTPATH@/create_random.sh \
      ${mdfield}\
      ${field_name} \
      ${mdfield}/${mask_base}_NIR.mask.fits \;\ 
    if [ -f ${mdfield}/${mask_base}_NIR.mask.fits.gz ]
    then
      echo -n rm ${mdfield}/${mask_base}_NIR.mask.fits \;\ 
    fi
    echo
  fi
done

### Copy data products into THELI tree
for mode in ${MODE}
do
  if [ "${mode}" = "COPY" ]; then
    test ! -d ${THELIDATAPATH}/${field_name}/@THELIFILTER@/colourcat_@THELIVERSION@/ && \
      mkdir ${THELIDATAPATH}/${field_name}/@THELIFILTER@/colourcat_@THELIVERSION@/
    echo -n chmod -R g+wX ${THELIDATAPATH}/${field_name}/@THELIFILTER@/colourcat_@THELIVERSION@/ \;\ 
    echo -n cp ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext.cat \
      ${THELIDATAPATH}/${field_name}/@THELIFILTER@/colourcat_@THELIVERSION@/${field_name}_@THELIFILTER@.@THELIVERSION@_ugriZYJHKs_photoz.cat \;\ 
    echo -n cp ${mdfield}/${field_name}_AW_THELI_NIR.mask.fits.gz \
      ${THELIDATAPATH}/${field_name}/@THELIFILTER@/masks_@THELIVERSION@/ \;\ 
    echo -n cp ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext_*_zz.txt \
      ${THELIDATAPATH}/${field_name}/@THELIFILTER@/postcoadd_@THELIVERSION@/plots/ \;\ 
    echo -n cp ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext_*_zz.png \
      ${THELIDATAPATH}/${field_name}/@THELIFILTER@/postcoadd_@THELIVERSION@/plots/ \;\ 
    echo -n cp ${mdfield}/${field_name}_ugriZYJHKs_photoz_ext_*_zz.pdf \
      ${THELIDATAPATH}/${field_name}/@THELIFILTER@/postcoadd_@THELIVERSION@/plots/ \;\ 
    echo
  fi
done
