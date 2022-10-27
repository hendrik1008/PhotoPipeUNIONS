#!/bin/bash -u

# the script creates flag maps for images; all pixels being non-zero
# will be set to 1 and all zeros remain zeros; the output image is an
# 8-bit FITS frame.

# HISTORY COMMENTS
# ================
#
# 12.02.2013:
# I included the new THELI logging scheme into the script.
#
# 13.06.2014
# command line arguments are referenced via meaningful names and
# no longer via '$1' etc.

# $1: main directory
# $2: science directory
# $3: image ending (including a . before the fits extension)
# $4: new image ending (including a . before the fits extension)
# $5: chips to be processed

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# give meaningful names to command line arguments:
MD=$1
SD=$2
OLDENDING=$3
NEWENDING=$4

# the chips that are to be processed
for CHIP in ${!#}
do
  ${P_FIND} ${MD}/${SD} -maxdepth 1 -name \*_${CHIP}${OLDENDING}fits > \
            ${TEMPDIR}/flag_images${CHIP}_$$

  if [ -s ${TEMPDIR}/flag_images${CHIP}_$$ ]; then
    while read file
    do
      if [ -L ${file} ]; then
        LINK=`${P_READLINK} ${file}`
        RESULTDIR[${CHIP}]=`dirname ${LINK}`
      else
        RESULTDIR[${CHIP}]="${MD}/${SD}"
      fi

      BASE=`basename ${file} ${3}fits`

      ${P_IC} -p 8 '0 1 %1 fabs 1.0e-09 < ?' \
        ${RESULTDIR[${CHIP}]}/${BASE}"${OLDENDING}fits" > \
        ${RESULTDIR[${CHIP}]}/${BASE}"${NEWENDING}fits"

    done < ${TEMPDIR}/flag_images${CHIP}_$$
  else
    theli_warn "No images to process for chip ${CHIP}!"
  fi

  test -f ${TEMPDIR}/flag_images${CHIP}_$$ && \
       rm ${TEMPDIR}/flag_images${CHIP}_$$
done

theli_end "${!#}"
exit 0;
