#!/bin/bash -u

# the script corrects a set of Science frames for fringe pattern.  The
# script uses the 'preprocess' program to perform the reduction.  It
# assumes that all images from ONE chip reside in the same physical
# directory

# $1: main directory (filter)
# $2: science directory
# $3: image ending
# $4: chips to be processed

# 02.05.2005:
# The preprocess call now uses the MAXIMAGES parameter.
#
# 14.08.2005:
# The preprocess calls now explicitely declare the statistics
# region on the command line.
#
# 05.12.2005:
# Chips whose NOTPROCESS flag is set are not processed at all.
#
# 23.06.2014:
# - I introduced THELI logging into the script
# - I removed all 'NOTPROCESS' related code. Chips that should not
#   be considered are dealt with via a BADCCD header keyword now.
# - commmand line arguments are referenced with meaningful names and
#   no longer via '$1' etc.
# - A BADCCD keyword in input images is properly propagated to all
#   SCIENCE images now.
#
# 31.07.2016:
# - The image ending is now a command line argument. DECam images
#   have a different image ending than the 'usual' OFC when they
#   are processed.
# - Not needed files are cleaned at the end if THELI_CLEAN is set.


# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

THELI_CLEAN=${THELI_CLEAN:-""}

if [ $# -ne 4 ]; then
  theli_error "USAGE: $0 MD SD ENDING CHIPS"
  exit 1;
fi

# give meaningful names to command line arguments:
MD=$1
SD=$2
ENDING=$3

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
        ln -s ${DIR}/${BASE}F.fits ${MD}/${SD}/${BASE}F.fits
        RESULTDIR[${CHIP}]=`dirname ${LINK}`
      fi
    done

    ${P_IMRED_ECL:?} ${FILES} \
         -BADCCD_KEYWORD BADCCD \
         -MAXIMAGES ${NFRAMES}\
         -STATSSEC ${STATSXMIN},${STATSXMAX},${STATSYMIN},${STATSYMAX} \
         -OVERSCAN N \
         -BIAS N \
         -FLAT N \
         -COMBINE N \
         -FRINGE Y \
         -FRINGE_IMAGE ${MD}/${SD}/${SD}_${CHIP}_fringe.fits \
         -FRINGE_REFIMAGE ${MD}/${SD}/${SD}_${CHIP}_illum.fits \
         -OUTPUT Y \
         -OUTPUT_DIR ${MD}/${SD}/ \
         -OUTPUT_SUFFIX F.fits \
         -TRIM N

    if [ -z ${THELI_CLEAN} ]; then
      test -d ${MD}/${SD}/${ENDING}_IMAGES || \
        mkdir ${MD}/${SD}/${ENDING}_IMAGES

      mv ${MD}/${SD}/*_${CHIP}${ENDING}.fits ${MD}/${SD}/${ENDING}_IMAGES
    else
      rm ${MD}/${SD}/*_${CHIP}${ENDING}.fits

      if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${SD}" ]; then
        rm ${RESULTDIR[${CHIP}]}/*_${CHIP}${ENDING}.fits
      fi
    fi
  else
    theli_warn "No images to process for chip ${CHIP} in $0"
  fi
done

# and bye:
theli_end "${!#}"
exit 0;
