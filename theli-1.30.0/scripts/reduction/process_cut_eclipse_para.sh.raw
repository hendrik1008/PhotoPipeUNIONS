#!/bin/bash -u

# the script trims a set of frames.  It is for instance useful to
# substitutes the SCIENCE processing in the case of Elixir
# preprocessed images (at CFHT) where only useless border regions need
# to be trimmed.

# HISTORY COMMENTS:
# =================
#
# 07.03.2007:
# - I included the possibility to give the BITPIX
#   of the trimmed output images on the command line
# - 'ls' commands are replaced by more robust 'find'
#   constructs.
#
# 20.02.2013:
# I introduced the new THELI logging scheme into the script.
#
# 05.03.2013:
# I corrected a bug when moving files to the SPLIT_IMAGES directory.
#
# 23.06.2014:
# - All command line arguments are now referenced via meaningful names and
#   no longer via '$1' etc.
# - preprocess is now called with a keyword to transfer properly a BADCCD
#   keyword to cutted images.

#$1: main directory
#$2: Science directory
#$3: BITPIX of output images (OPTIONAL; default is -32)
#${!#}: chips to be processed

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# give meaningful names to command line arguments:
MD=$1
SD=$2

# set BITPIX to use for output images:
BITPIX=-32

if [ $# -eq 4 ]; then
  BITPIX=$3
fi

# the resultdir is where the output coadded images will go. If ONE
# image of the corresponding chip is a link the image will go to THIS
# directory
for CHIP in ${!#}
do
  RESULTDIR[${CHIP}]=${MD}/${SD}
done

# perform cutting
for CHIP in ${!#}
do
  FILES=`${P_FIND} ${MD}/${SD} -maxdepth 1 -name \*_${CHIP}.fits`

  if [ "${FILES}" = "" ]; then
    theli_warn "No images to process for chip ${CHIP}!"
  else
    for FILE in ${FILES}
    do
      if [ -L ${FILE} ]; then
        LINK=`${P_READLINK} ${FILE}`
        BASE=`basename ${LINK} .fits`
        DIR=`dirname ${LINK}`
        ln -s ${DIR}/${BASE}C.fits ${MD}/${SD}/${BASE}C.fits
        RESULTDIR[${CHIP}]=`dirname ${LINK}`
      fi
    done

    MAXX=$(( ${CUTX[${CHIP}]} + ${SIZEX[${CHIP}]} - 1 ))
    MAXY=$(( ${CUTY[${CHIP}]} + ${SIZEY[${CHIP}]} - 1 ))

    # trim images:
    ${P_IMRED_ECL:?} ${FILES} \
      -BADCCD_KEYWORD BADCCD -MAXIMAGES ${NFRAMES} \
      -OVERSCAN N -BIAS N -FLAT N -COMBINE N \
      -OUTPUT Y -OUTPUT_BITPIX ${BITPIX} -OUTPUT_DIR ${MD}/${SD} \
      -OUTPUT_SUFFIX C.fits -TRIM Y \
      -TRIM_REGION ${CUTX[${CHIP}]},${MAXX},${CUTY[${CHIP}]},${MAXY}

    test -d ${MD}/${SD}/SPLIT_IMAGES || mkdir -p ${MD}/${SD}/SPLIT_IMAGES

    mv ${FILES} ${MD}/${SD}/SPLIT_IMAGES
  fi # if [ "${FILES}" = "" ]
done

# and bye:
theli_end "${!#}"
exit 0;
