#!/bin/bash -u

# the script processes a set of FLAT frames the images are overscan
# corrected, debiased and stacked with mode rescaling.  The script
# uses the 'preprocess' program to perform the reduction. In contrary
# to the script version based on FLIPS, the individual preprocessed
# images are NOT saved to disk but only the final coadded calibration
# frames.

#$1: main directory
#$2: Bias directory
#$3: Flat directory
#$4: chips to process

#
# 02.05.2005:
# I rewrote the script to use the MAXIMAGE parameter in preprocess
# and the imcat-based imcombflat for the actual coaddition.
#
# 14.08.2005:
# The preprocess calls now explicitely declare the statistics
# region on the command line.
#
# 05.12.2005:
# - Chips whose NOTPROCESS flag is set are not processed at all.
# - The final co-addition for the master bias now uses the clipped
#   mean algorithm from imcombflat.
#
# 22.12.2005:
# I changed back the final co-addition to median stacking
# (with initial rejection of the highest value at each pixel
# position).
# The clipped mean turned out to be too affected by outliers.
#
# 31.01.2012:
# cleaning up of temporary files at the end
#
# 13.02.2013:
# I introduced the new THELI logging scheme into the script.
#
# 20.06.2014:
# - I removed all 'NOTPROCESS' related code. Chips that should not
#   be considered are dealt with via a BADCCD header keyword now.
# - commmand line arguments are referenced with meaningful names and
#   no longer via '$1' etc.
# - A BADCCD keyword in input images is properly propagated to the final
#   co-added BIAS images now.
#
# 26.06.2014:
# - Bug fix: The wrong images were co-added (bug was introduced with the
#   modifications from 20.06.2014).
# - The imcombflat command now accepts an argument for the output value
#   of stacks to which no images contribute. I modified this script
#   accordingly.
#
# 13.08.2015:
# - I changes a theli_error to a warning. There is no reason
#   to immediatley quite the whole script if a skyflat that
#   should be produced already exists - although this probably
#   means that the script was run unintentionally.
# - I check for the correct number of command line arguments
#   now.
#
# 29.07.2016:
# - DECam needs special treatment of the overscan regions, Therefore, the
#   overscan correction and cutting are done in two separate steps for this
#   instrument now.
# - If the variable THELI_CLEAN is set to a non-empty string we clean up
#   not needed files at the end.
#
# 30.08.2016:
# The behaviour of DECam_overscan.py changed and we now need a command
# line option to overwrite original images with overscan corrected
# versions. I adapted this script accordingly.
#
# 31.08.2016:
# Bug fix: Cleaning up also deleted the newly created flatfield images - fixed!


# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# Define THELI clean to an empty string if it is not
# yet defined (to make happy the '-u' bash setting)
THELI_CLEAN=${THELI_CLEAN:-""}

# check command line arguments and give meaningful names to them:
if [ $# -ne 4 ]; then
  echo "SYNOPSIS: $0 MD BIASDIR FLATDIR chips"
fi

# give meaningful names to command line arguments:
MD=$1
BIASDIR=$2
FLATDIR=$3

# Correction overscan; create config file on the fly we do not assume
# that the overscan is the same for all chips
for CHIP in ${!#}
do
  FILES=`${P_FIND} ${MD}/${FLATDIR} -maxdepth 1 -name \*_${CHIP}.fits`

  if [ "${FILES}" != "" ]; then
    RESULTDIR[${CHIP}]=${MD}/${FLATDIR}
    for FILE in ${FILES}
    do
      if [ -L ${FILE} ]; then
        LINK=`${P_READLINK} ${FILE}`
        BASE=`basename ${LINK} .fits`
        DIR=`dirname ${LINK}`
        ln -s ${DIR}/${BASE}OC.fits ${MD}/${FLATDIR}/${BASE}OC.fits
        RESULTDIR[${CHIP}]=`dirname ${LINK}`
      fi
    done

    # check whether the result (a FITS image with the
    # proper name) is already there; if yes, do nothing !!
    if [ -f ${RESULTDIR[${CHIP}]}/${FLATDIR}_${CHIP}.fits ]; then
      theli_warn "${RESULTDIR[${CHIP}]}/${FLATDIR}_${CHIP}.fits already present!"
      exit 1;
    fi

    # DECam needs special treatment for overscan correction:
    OVERSCANARG="-OVERSCAN Y"
    if [ "${INSTRUMENT}" = "DECam" ]; then
      OVERSCANARG="-OVERSCAN N"

      # an empty string for the '-o'-option means that original images
      # are overwritten with the overscan corrected versions.
      ${P_PYTHON3} ./DECam_overscan.py ${FILES} -o ""
    fi

    MAXX=$(( ${CUTX[${CHIP}]} + ${SIZEX[${CHIP}]} - 1 ))
    MAXY=$(( ${CUTY[${CHIP}]} + ${SIZEY[${CHIP}]} - 1 ))

    # overscan correct, trim and bias correct FLAT frames
    ${P_IMRED_ECL:?} ${FILES} \
        -BADCCD_KEYWORD BADCCD \
        -MAXIMAGES ${NFRAMES} \
        -STATSSEC ${STATSXMIN},${STATSXMAX},${STATSYMIN},${STATSYMAX} \
        ${OVERSCANARG} \
        -OVERSCAN_REGION ${OVSCANX1[${CHIP}]},${OVSCANX2[${CHIP}]} \
        -BIAS Y \
        -BIAS_IMAGE ${MD}/${BIASDIR}/${BIASDIR}_${CHIP}.fits \
        -OUTPUT Y \
        -OUTPUT_DIR ${MD}/${FLATDIR} \
        -TRIM Y \
        -OUTPUT_SUFFIX OC.fits \
        -TRIM_REGION ${CUTX[${CHIP}]},${MAXX},${CUTY[${CHIP}]},${MAXY}

    # and combine them
    FILES=`${P_FIND} ${MD}/${FLATDIR} -maxdepth 1 -name \*_${CHIP}OC.fits`
    ${P_IMSTATS} ${FILES}\
                 -s ${STATSXMIN} ${STATSXMAX} ${STATSYMIN} ${STATSYMAX}\
                 -o ${TEMPDIR}/flat_images_$$

    ${P_IMCOMBFLAT_IMCAT} -i  ${TEMPDIR}/flat_images_$$\
                    -o ${RESULTDIR[${CHIP}]}/${FLATDIR}_${CHIP}.fits \
                    -s 1 -e 0 1 -f 1.0 -k BADCCD

    if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${FLATDIR}" ]; then
      ln -s ${RESULTDIR[${CHIP}]}/${FLATDIR}_${CHIP}.fits \
            ${MD}/${FLATDIR}/${FLATDIR}_${CHIP}.fits
    fi

    # clean up not needed files if THELI_CLEAN is enabled.
    if [ ! -z "${THELI_CLEAN}" ]; then
      find ${MD}/${BIASDIR} -maxdepth 1 -name \*_${CHIP}OC.fits |\
        xargs rm
      find ${MD}/${BIASDIR} -maxdepth 1 -name \*_${CHIP}.fits |\
        grep -v "${FLATDIR}_${CHIP}" | xargs rm
      if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${BIASDIR}" ]; then
         find  ${RESULTDIR[${CHIP}]} -maxdepth 1 -name \*_${CHIP}OC.fits |\
           xargs rm
         find  ${RESULTDIR[${CHIP}]} -maxdepth 1 -name \*_${CHIP}.fits |\
           grep -v "${FLATDIR}_${CHIP}" | xargs rm
      fi
    fi
  else
    theli_warn "Nothing to do for chip ${CHIP} in $0"
  fi
done

# clean up and bye:
for FILE in ${TEMPDIR}/*_$$
do
  test -f ${FILE} && rm ${FILE}
done

theli_end "${!#}"
exit 0;
