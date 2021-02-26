#!/bin/bash -u

# the script changes the BITPIX from input images It is used to
# convert FLOAT data that effectively only contain INTEGER or BYTE
# values to images with lower disk space needs.

# HISTORY COMMENTS:
#
# 10.06.2015:
# - The script is renamed to create_bitpixchange_para.sh as
#   it is a parallel script!
# - command line arguments get meaningful names
# - inclusion of THELI logging scheme

# $1: main directory
# $2: science directory
# $3: prefix
# $4: extension (including a . before the fits extension)
# $5: new BITPIX value
# $6: chips to be processed

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# give meaningful name sto command line arguments:
MD=$1
SD=$2
PREFIX="$3"
EXTENSION=$4
NEWBITPIX=$5

# the chips that are to be processed
for CHIP in ${!#}
do
  ${P_FIND} ${MD}/${SD} -name "${PREFIX}"\*_${CHIP}${4}fits > \
    ${TEMPDIR}/bitpix_change_${CHIP}_$$

  while read file
  do
    if [ -L ${file} ]; then
      LINK=`${P_READLINK} ${file}`
      RESULTDIR[${CHIP}]=`dirname ${LINK}`
    else
      RESULTDIR[${CHIP}]="${MD}/${SD}"
    fi

    BASE=`basename ${file} ${EXTENSION}fits`

    ${P_IOFITS} ${RESULTDIR[${CHIP}]}/${BASE}"${EXTENSION}fits" \
                ${RESULTDIR[${CHIP}]}/${BASE}"tmp.fits_$$" ${NEWBITPIX}
    mv ${RESULTDIR[${CHIP}]}/${BASE}"tmp.fits_$$" \
       ${RESULTDIR[${CHIP}]}/${BASE}"${EXTENSION}fits"
  done < ${TEMPDIR}/bitpix_change_${CHIP}_$$

  rm ${TEMPDIR}/bitpix_change_${CHIP}_$$
done

theli_end "${!#}"
exit 0;
