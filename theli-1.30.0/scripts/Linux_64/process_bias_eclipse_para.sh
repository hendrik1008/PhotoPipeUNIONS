#!/bin/bash -u

# the script processes a set of BIAS/DARK frames.  The images are
# overscan corrected and stacked without any rescaling. This script is
# the parallel version and uses the 'preprocess' program to perform
# the reduction. In contrary to the script version based on FLIPS, the
# individual preprocessed images are NOT saved to disk but only the
# final coadded calibration frames.  It assumes that all images from
# ONE chip reside in the same physical directory

# $1: master directory
# $2: subdirectory with the BIAS/DARK images
# $3: chips to work on

# The result images are in the $1/$2 directory and have
# the same basic name as the subdirectory, e.g.
# The images are in .../BIAS. Then the stacked
# images have the names .../BIAS/BIAS_i.fits, where i=1..NCHIPS.

#
# 27.04.2005:
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
# 17.01.2012:
# temporary files are deleted at the end of the script
#
# 21.03.2014:
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
# The imcombflat command now accepts an argument for the output value
# of stacks to which no images contribute. I modified this script
# accordingly.
#
# 12.08.2015:
# I added some more error checking and reporting to the script.
#
# 29.07.2016:
# - DECam needs special treatment of the overscan regions. Therefore, the
#   overscan correction and cutting are done in two separate steps for this
#   instrument now.
# - If the variable THELI_CLEAN is set to a non-empty string we clean up
#   not needed files at the end.
#
# 09.08.2016:
# BUG FIX: xrags -> xargs (wrong spelling of Unix-command)
#
# 30.08.2016:
# The behaviour of DECam_overscan.py changed and we now need a command
# line option to overwrite original images with overscan corrected
# versions. I adapted this script accordingly.
#
# 31.08.2016:
# Bug fix: Cleaning up also deleted the newly created BIAS images - fixed!

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

THELI_CLEAN=${THELI_CLEAN:-""}

# check command lines and give meaningful names to comand line arguments:
if [ $# -ne 3 ]; then
  theli_error "SYNOPSIS: $0 main_dir bias_dir chips_to_process"
  exit 1;
fi

MD=$1
BIASDIR=$2

test -d ${MD}/${BIASDIR} || \
  { theli_error "${MD}/${BIASDIR} does not exit! Exiting!"; exit 1; }

for CHIP in ${!#}
do
  FILES=`${P_FIND} ${MD}/${BIASDIR} -maxdepth 1 -name \*_${CHIP}.fits`

  if [ "${FILES}" != "" ]; then
    RESULTDIR[${CHIP}]=${MD}/${BIASDIR}
    for FILE in ${FILES}
    do
      if [ -L ${FILE} ]; then
        LINK=`${P_READLINK} ${FILE}`
        BASE=`basename ${LINK} .fits`
        DIR=`dirname ${LINK}`
        ln -s ${DIR}/${BASE}OC.fits ${MD}/${BIASDIR}/${BASE}OC.fits
        RESULTDIR[${CHIP}]=`dirname ${LINK}`
      fi
    done

    # check whether the result (a FITS image with the
    # proper name) is already there; if yes, do nothing !!
    if [ -f ${RESULTDIR[${CHIP}]}/${BIASDIR}_${CHIP}.fits ]; then
        theli_error "${RESULTDIR[${CHIP}]}/${BIASDIR}_${CHIP}.fits already present !!! Exiting !!"
        exit 1
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

    # overscan correct and trim BIAS frames
    ${P_IMRED_ECL:?} ${FILES} \
        -BADCCD_KEYWORD BADCCD \
        -MAXIMAGES ${NFRAMES} \
        -STATSSEC ${STATSXMIN},${STATSXMAX},${STATSYMIN},${STATSYMAX} \
        ${OVERSCANARG} \
        -OVERSCAN_REGION ${OVSCANX1[${CHIP}]},${OVSCANX2[${CHIP}]} \
        -OUTPUT Y \
        -OUTPUT_DIR ${MD}/${BIASDIR} \
        -TRIM Y \
        -OUTPUT_SUFFIX OC.fits \
        -TRIM_REGION ${CUTX[${CHIP}]},${MAXX},${CUTY[${CHIP}]},${MAXY}

    # and combine them
    ${P_FIND} ${MD}/${BIASDIR} -maxdepth 1 -name \*_${CHIP}OC.fits > \
        ${TEMPDIR}/bias_images_$$

    ${P_IMCOMBFLAT_IMCAT} -i  ${TEMPDIR}/bias_images_$$ \
                    -o ${RESULTDIR[${CHIP}]}/${BIASDIR}_${CHIP}.fits \
                    -e 0 1 -f 0.0 -k BADCCD

    if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${BIASDIR}" ]; then
      ln -s ${RESULTDIR[${CHIP}]}/${BIASDIR}_${CHIP}.fits \
            ${MD}/${BIASDIR}/${BIASDIR}_${CHIP}.fits
    fi

    # clean up not needed files if THELI_CLEAN is enabled.
    if [ ! -z "${THELI_CLEAN}" ]; then
      find ${MD}/${BIASDIR} -maxdepth 1 -name \*_${CHIP}OC.fits |\
        xargs rm
      find ${MD}/${BIASDIR} -maxdepth 1 -name \*_${CHIP}.fits |\
        grep -v "${BIASDIR}_${CHIP}" | xargs rm
      if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${BIASDIR}" ]; then
         find  ${RESULTDIR[${CHIP}]} -maxdepth 1 -name \*_${CHIP}OC.fits |\
           xargs rm
         find  ${RESULTDIR[${CHIP}]} -maxdepth 1 -name \*_${CHIP}.fits |\
           grep -v "${BIASDIR}_${CHIP}" | xargs rm
      fi
    fi
  else
    theli_warn "Nothing to do for chip ${CHIP} in $0"
  fi
done

# clean up and bye:
test -f ${TEMPDIR}/bias_images_$$ && rm ${TEMPDIR}/bias_images_$$

theli_end "${!#}"
exit 0;
