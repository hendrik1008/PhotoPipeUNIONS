#!/bin/bash -u

# ----------------------------------------------------------------
# File Name:           process_science_eclipse_para.sh
# Author:              Thomas Erben (terben@astro.uni-bonn.de)
# Last modified on:    31.07.2016
# Description:         Perform Bias-subtraction, cutting and initial
#                      flatfielding of science data.
# ----------------------------------------------------------------

# HISTORY COMMENTS:
#
# 31.07.2016:
# The former 'process_science_eclipse_para.sh' script was split in two
# parts: - A new 'process_science_eclipse_para.sh' script which does now
# all the steps up to the initial flat-fielding with dome- or sky-flats.
# - A 'process_superflat_eclipse_para.sh' which creates the superflat
# image. The splitting was primarily done to allow special initial
# science processinf ig necessary, i.e. to write specialised
# 'process_science' scripts. This is for instance necessary for DECam.

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# define THELI_CLEAN because of the '-u' script flag (the use of
# undefined variables would be treated as errors!)
#
THELI_CLEAN=${THELI_CLEAN:-""}

##
## function definitions:
##
function printUsage
{
  echo "SCRIPT NAME:"
  echo "    process_science_eclipse_para.sh"
  echo ""
  echo "    Basic instrumental signature removal of science data"
  echo ""
  echo "ARGUMENTS:"
  echo "     1. main_dir."
  echo "     2. bias_dir."
  echo "     3. flat_dir."
  echo "     4. science_dir."
  echo "     5. rescale_to_equalise_gain (RESCALE OR NORESCALE)"
  echo "    (6. chips to be processed)"
  echo ""
  echo "DESCRIPTION: "
  echo "    The script performs initial instrumental signature removal"
  echo "    from science data. This includes overscan correction, cutting"
  echo "    images, bias substraction and initial flat-fielding with dome-"
  echo "    or sky-flats.       "
  echo ""
  echo "AUTHOR:"
  echo "    Thomas Erben (terben@astro.uni-bonn.de)"
}

# check validity of command line arguments:
if [ $# -ne 6 ]; then
  theli_error "Wrong number of command line arguments!" "${!#}"
  printUsage
  exit 1
fi

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      exit 1" INT

# give meaningful names to command line arguments:
MD=$1
BIASDIR=$2
FLATDIR=$3
SD=$4
RESCALEFLAG=$5

# Existence of necessary directories:
for DIR in ${MD}/${BIASDIR} ${MD}/${FLATDIR} ${MD}/${SD}
do
  test -d ${DIR} || { theli_error \
                    "Can't find directory ${DIR}" "${!#}"; exit 1; }
done

# perform preprocessing (overscan correction, BIAS subtraction, first
# flatfield pass)
for CHIP in ${!#}
do
  # exclude from the file list any combined SCIENCE (superflat) data:
  FILES=$(${P_FIND} ${MD}/${SD} -maxdepth 1 -name \*_${CHIP}.fits |\
         grep -v ${MD}/${SD}/${SD})

  RESULTDIR[${CHIP}]=${MD}/${SD}

  if [ "${FILES}" != "" ]; then
    for FILE in ${FILES}
    do
      if [ -L ${FILE} ]; then
        LINK=$(${P_READLINK} ${FILE})
        BASE=$(basename ${LINK} .fits)
        DIR=$(dirname ${LINK})
        ln -s ${DIR}/${BASE}OFC.fits ${MD}/${SD}/${BASE}OFC.fits
        RESULTDIR[${CHIP}]=$(dirname ${LINK})
      fi
    done

    MAXX=$(( ${CUTX[${CHIP}]} + ${SIZEX[${CHIP}]} - 1 ))
    MAXY=$(( ${CUTY[${CHIP}]} + ${SIZEY[${CHIP}]} - 1 ))

    FLATFLAG=""
    if [ "${RESCALEFLAG}" = "RESCALE" ]; then
      # build up list of all flatfields necessary for rescaling when
      # gains between chips need to be equalised here.
      FLATSTR=""
      for i in $(seq 1 ${NCHIPS})
      do
        if [ "${FLATSTR}" = "" ]; then
          FLATSTR="/${MD}/${FLATDIR}/${FLATDIR}_${i}.fits"
        else
          FLATSTR="${FLATSTR},/${MD}/${FLATDIR}/${FLATDIR}_${i}.fits"
        fi
      done

      FLATFLAG="-FLAT_SCALE Y -FLAT_SCALEIMAGE ${FLATSTR}"
    fi

    # overscan correct, bias subtract and flatfield science images:
    ${P_IMRED_ECL:?} ${FILES} \
      -MAXIMAGES ${NFRAMES}\
      -BADCCD_KEYWORD BADCCD \
      -STATSSEC ${STATSXMIN},${STATSXMAX},${STATSYMIN},${STATSYMAX} \
      -OVERSCAN Y \
      -OVERSCAN_REGION ${OVSCANX1[${CHIP}]},${OVSCANX2[${CHIP}]} \
      -BIAS Y -BIAS_IMAGE ${MD}/${BIASDIR}/${BIASDIR}_${CHIP}.fits \
      -FLAT Y -FLAT_IMAGE ${MD}/${FLATDIR}/${FLATDIR}_${CHIP}.fits \
      -COMBINE N \
      -OUTPUT Y -OUTPUT_DIR ${MD}/${SD} -OUTPUT_SUFFIX OFC.fits \
      -TRIM Y \
      -TRIM_REGION ${CUTX[${CHIP}]},${MAXX},${CUTY[${CHIP}]},${MAXY} \
      ${FLATFLAG}

    # delete original, uncorrected data if cleaning is activated:
    if [ ! -z "${THELI_CLEAN}" ]; then
      rm ${FILES}
    else
      test -d ${MD}/${SD}/SPLIT_IMAGES || mkdir ${MD}/${SD}/SPLIT_IMAGES
      mv ${FILES} ${MD}/${SD}/SPLIT_IMAGES
    fi
  else
    theli_warn "No files for Chip ${CHIP} present!" "${!#}"
  fi
done

theli_end "${!#}"
exit 0;
