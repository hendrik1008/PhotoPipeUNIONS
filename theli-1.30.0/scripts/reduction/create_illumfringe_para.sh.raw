#!/bin/bash -u

# script that creates an illumination correction and
# fringe image out of a blank sky field (superflat)

# HISTORY COMMENTS:
# =================
#
# 31.12.2012:
# If the file from that fringe and illumination image should be extracted
# did not exist the script created empty links - corrected.
#
# 23.06.2014:
# All variable names are now referenced with meaningful names and no longer
# via '$1' etc.
#
# 01.08.2016:
# - I introduced a new command line argument giving the location of the
#   initial superflat images. It is a default argument. If it is not
#   provided, the initial superflats are supposed to be in the sceince
#   directory. This is useful for instance for the processing of
#   standardstar fields or very short exposed science images for which
#   we want to user the superflats from the (long exposed) science
#   images.
# - Both SEXtractor output check-images of this script are now produced
#   with a single call to SExtractor.

# $1: main dir (filter)
# $2: Science directory
# $3: Supeflat directory (OPTIONAL; default is Science Dir.)
# $#: chips to be processed

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

if [ $# -ne 3 ] && [ $# -ne 4 ]; then
  echo "USAGE: $0 MD SD [SUPERFLDIR] CHIPS"
  exit 1;
fi

# give meaningful names to variables:
MD=$1
SD=$2
if [ $# -eq 4 ]; then
  SUPERD=$3
else
  SUPERD=${SD}
fi

for CHIP in ${!#}
do
  FILE=$(${P_FIND} ${MD}/${SUPERD} -maxdepth 1 \
        -name ${SUPERD}_${CHIP}.fits)

  if [ "${FILE}" != "" ]; then
    if [ -L ${FILE} ]; then
      LINK=$(${P_READLINK} ${FILE})
      BASE=$(basename ${LINK} .fits)
      DIR=$(dirname ${LINK})
      ln -s ${DIR}/${SD}_${CHIP}_illum.fits \
            ${MD}/${SD}/${SD}_${CHIP}_illum.fits
      ln -s ${DIR}/${SD}_${CHIP}_fringe.fits \
            ${MD}/${SD}/${SD}_${CHIP}_fringe.fits
      RESULTDIR[${CHIP}]=$(dirname ${LINK})
    else
      RESULTDIR[${CHIP}]=${MD}/${SD}
    fi

    DIR=${RESULTDIR[${CHIP}]}
    ${P_SEX} ${MD}/${SUPERD}/${SUPERD}_${CHIP}.fits \
      -c ${CONF}/illumfringe.sex \
      -CHECKIMAGE_TYPE BACKGROUND,-BACKGROUND \
      -CHECKIMAGE_NAME \
         ${DIR}/${SD}_${CHIP}_illum.fits,${DIR}/${SD}_${CHIP}_fringe.fits \
      -BACK_SIZE 256
  else
    theli_error "${MD}/${SUPERD}/${SUPERD}_${CHIP}.fits does not exist!" "${!#}"
    exit 1;
  fi
done

theli_end "${!#}"
exit 0;
