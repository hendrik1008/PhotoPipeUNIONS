#!/bin/bash -u

# the script corrects a set of Science frames for illumination
# pattern. This is an effective second flat-fielding.  The script uses
# the 'preprocess' program to perform the reduction.  It assumes that
# all images from ONE chip reside in the same physical directory
#
# 28.07.2004:
# new RESCALE flag; it was introduced so that the user can decide
# whether the gain equalisation is done here with Superflats or in the
# process_science step with skyflats.
#
# 14.08.2005:
# The preprocess calls now explicitely declare the statistics region
# on the command line.
#
# 02.05.2005:
# The preprocess call now uses the MAXIMAGES parameter.
#
# 05.12.2005:
# - Chips whose NOTPROCESS flag is set are not processed at
#   all
# - Chips whose NOTUSE flag is set are not considered in the
#   Flatscale images.
# - New command line argument: The 4th command line argument now
#   determined whether images are superflatted with the original
#   (parameter value SUPER),
#   unsmoothed superflat or the illumination correction image
#   (parameter value ILLUM).
# - I corrected a bug in the indexing of NOTPROCESS arrays.
#
# 01.02.2011:
# I introduced the minimum threshhold for a good flatfield as a
# command line argument. We saw for medium band filters (WFI) that we
# need to accept values as low as 20 counts for the superflat!
#
# 31.12.2012:
# I made the script more robust against non-existing files.
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

# $1: main directory (filter)
# $2: Science directory
# $3: image ending
# $4: RESCALE/NORESCALE
# $5: ILLUM/SUPER (Is the illumination correction done with
#     the 'illumination correction image' or with the unsmoothed
#     superflat?)
# $6: threshhold to accept a flat field (OPTIONAL; default: 50)
# $#: chips to be processed

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

THELI_CLEAN=${THELI_CLEAN:-""}

if [ $# -ne 6 ] && [ $# -ne 7 ]; then
  theli_error "USAGE: $0 MD SD ENDING RESCALE ILLUM [THRESHOLD] CHIPS"
  exit 1;
fi

# give meaningful names to command line arguments:
MD=$1
SD=$2
ENDING=$3
RESCALEFLAG=$4
ILLUMFLAG=""
if [ "$5" = "ILLUM" ]; then
  ILLUMFLAG="_illum"
fi

# set threshhold to accept flat fields:
FLATTHRESH=50
if [ $# -gt 6 ]; then
  FLATTHRESH=$5
fi

for CHIP in ${!#}
do
  FILES=`${P_FIND} ${MD}/${SD} -maxdepth 1 -name \*_${CHIP}${ENDING}.fits`

  RESULTDIR[${CHIP}]=${MD}/${SD}
  if [ "${FILES}" != "" ]; then
    for FILE in ${FILES}
    do
      if [ -L ${FILE} ]; then
        LINK=`${P_READLINK} ${FILE}`
        BASE=`basename ${LINK} .fits`
        DIR=`dirname ${LINK}`
        ln -s ${DIR}/${BASE}S.fits ${MD}/${SD}/${BASE}S.fits
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
      BADCCD=`${P_DFITS} ${MD}/${SD}/${SD}_${i}${ILLUMFLAG}.fits |\
              ${P_FITSORT} -d BADCCD | cut -f 2`
      if [ "${j}" = "1" ] && [ "${BADCCD}" != "1" ]; then
        FLATSTR="${MD}/${SD}/${SD}_${i}${ILLUMFLAG}.fits"
        j=2
      else
        FLATSTR="${FLATSTR},${MD}/${SD}/${SD}_${i}${ILLUMFLAG}.fits"
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
       -FLAT_IMAGE ${MD}/${SD}/${SD}_${CHIP}${ILLUMFLAG}.fits \
       -COMBINE N \
       -OUTPUT Y \
       -OUTPUT_DIR ${MD}/${SD}/ \
       -OUTPUT_SUFFIX S.fits \
       -FLAT_THRESHHOLD ${FLATTHRESH} \
       -TRIM N ${RESCALEARG}

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
    theli_warn "No files for Chip ${CHIP} to process!" "${!#}"
  fi
done

theli_end "${!#}"
exit 0;

