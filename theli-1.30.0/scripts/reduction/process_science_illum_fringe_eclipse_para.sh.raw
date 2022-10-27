#!/bin/bash -u

# the script corrects a set of Science frames for illumination pattern
# and subtracts a fringe pattern at the same time by estimating
# appropriate scaling factors for the fringe frame to subtract. The
# script uses the 'preprocess' program to perform the reduction. It
# assumes that all images from ONE chip reside in the same physical
# directory

# 25.11.2003:
# in the links that are created, the supplementary ending
# of the images was S.fits instead of SF.fits
#
# 28.07.2004:
# new RESCALE flag; it was introduced so that the user can decide
# whether the gain equalisation is done here with Superflats
# or in the process_science step with skyflats.
#
# 02.05.2005:
# The preprocess call now uses the MAXIMAGES parameter.
#
# 14.08.2005:
# The preprocess calls now explicitely declare the statistics
# region on the command line.
#
#
# 05.12.2005:
# - Chips whose NOTPROCESS flag is set are not processed at all.
# - Chips whose NOTUSE flag is set are not considered for flat scaling.
#
# 31.12.2012:
# I made the script more robust against non-existing files
#
# 23.06.2014:
# - I removed all 'NOTPROCESS' related code. Chips that should not
#   be considered are dealt with via a BADCCD header keyword now.
# - commmand line arguments are referenced with meaningful names and
#   no longer via '$1' etc.
# - A BADCCD keyword in input images is properly propagated to all
#   SCIENCE images now.
#
# 26.06.2014:
# BADCCD Flat images are no longer considered for the illumincation
# scaling factor.
#
# 31.07.2016:
# - The image ending is now a command line argument. DECam images
#   have a different image ending than the 'usual' OFC when they
#   are processed.
# - Not needed files are cleaned at the end if THELI_CLEAN is set.

# $1: main directory
# $2: Science directory
# $3: image ending
# $4: RESCALE/NORESCALE
# $5: chips to be processed

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

THELI_CLEAN=${THELI_CLEAN:-""}

if [ $# -ne 5 ]; then
  theli_error "USAGE: $0 MD SD ENDING RESCALEFLAG CHIPS"
  exit 1;
fi

# give meaningful names to command line arguments:
MD=$1
SD=$2
ENDING=$3
RESCALEFLAG=$4

for CHIP in ${!#}
do
  FILES=`${P_FIND} ${MD}/${SD} -maxdepth 1 -name \*_${CHIP}${ENDING}.fits`

  if [ "${FILES}" != "" ]; then
    RESULTDIR[${CHIP}]=${MD}/${SD}
    for FILE in ${FILES}
    do
      if [ -L ${FILE} ]; then
        LINK=`${P_READLINK} ${FILE}`
        BASE=`basename ${LINK} .fits`
        DIR=`dirname ${LINK}`
        ln -s ${DIR}/${BASE}SF.fits $1/$2/${BASE}SF.fits
        RESULTDIR[${CHIP}]=`dirname ${LINK}`
      fi
    done

    i=1
    j=1
    FLATSTR=""
    while [ "${i}" -le "${NCHIPS}" ]
    do
      # do not consider bad flats for the illumination scaling
      # factor:
      BADCCD=`${P_DFITS} ${MD}/${SD}/${SD}_${i}_illum.fits |\
              ${P_FITSORT} -d BADCCD | cut -f 2`
      if [ "${j}" = "1" ] && [ "${BADCCD}" != "1" ]; then
        FLATSTR="${MD}/${SD}/${SD}_${i}_illum.fits"
	j=2
      else
        FLATSTR="${FLATSTR},${MD}/${SD}/${SD}_${i}_illum.fits"
      fi
      i=$(( $i + 1 ))
    done

    RESCALEARG=""
    if [ "${RESCALEFLAG}" = "RESCALE" ]; then
      RESCALEARG="-FLAT_SCALE Y -FLAT_SCALEIMAGE ${FLATSTR}"
    fi

    ${P_IMRED_ECL:?} ${FILES} \
         -BADCCD_KEYWORD BADCCD \
         -MAXIMAGES ${NFRAMES}\
         -STATSSEC ${STATSXMIN},${STATSXMAX},${STATSYMIN},${STATSYMAX} \
         -OVERSCAN N \
         -BIAS N \
         -FLAT Y \
         -FLAT_IMAGE ${MD}/${SD}/${SD}_${CHIP}_illum.fits \
         -FRINGE Y \
         -FRINGE_IMAGE ${MD}/${SD}/${SD}_${CHIP}_fringe.fits \
         -FRINGE_REFIMAGE ${MD}/${SD}/${SD}_${CHIP}_illum.fits \
         -COMBINE N \
         -OUTPUT Y \
         -OUTPUT_DIR ${MD}/${SD}/ \
         -OUTPUT_SUFFIX SF.fits \
         -TRIM N ${RESCALEARG}

    if [ -z ${THELI_CLEAN} ]; then
      test -d ${MD}/${SD}/${ENDING}_IMAGES || mkdir ${MD}/${SD}/${ENDING}_IMAGES
      mv ${MD}/${SD}/*_${CHIP}${ENDING}.fits ${MD}/${SD}/${ENDING}_IMAGES
    else
      rm ${MD}/${SD}/*_${CHIP}${ENDING}.fits

      if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${SD}" ]; then
        rm ${RESULTDIR[${CHIP}]}/*_${CHIP}${ENDING}.fits
      fi
    fi
  else
    theli_warn "No files for Chip ${CHIP} to process!" "${!#}"
  fi
done

theli_end "${!#}"
exit 0;
