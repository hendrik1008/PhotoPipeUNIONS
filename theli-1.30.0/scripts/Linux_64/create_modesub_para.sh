#!/bin/bash -u

# the script subtracts the mode from the input images (with Nick
# Kaisers stats program). It is used to subtract a remaining, non
# zero, mode from swarp resampled images before they are coadded.

# 22.12.2005:
# - I introduced a prefix as selection criterium of the
#   images whose mode has to be subtracted.
# - For the extension the "." before in final .fits extension
#   has to be included in the command line argument now.
#   This avoids empty command line arguments in some
#   circumstances.
#
# 21.03.2007:
# - I replaces an 'ls' command to search for files
#   by 'find' (more robust to long lists; no error message
#   in case that no files are present).
# - not needed, temporary, files are deleted at the end of
#   processing.
#
# 04.09.2008:
# When subtracting the mode we leave areas which contain exactly
# zero untouched. This allows us to perform operations on such
# areas lateron.
#
# 03.10.2008:
# if present the SATLEVEL keyword is adapted
#
# 12.10.2008:
# I make sure that the SATLEVEL keyword is always a float with two
# digits
#
# 14.08.2009:
# I forgot to include 'bash_functions.include' containing functions
# for header updates (SATURATE level keyword)
#
# 16.09.2009:
# Bug fix when writing SATLEVEL keywords. The imagename provided
# was not correct.
#
# 07.02.2013:
# I introduced the new THELI logging scheme into the script.

# $1: main directory
# $2: science directory
# $3: prefix
# $4: extension (including a . before the fits extension)
# $5: chips to be processed

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# the chips that are to be processed
for CHIP in $5
do
  ${P_FIND} /$1/$2/ -maxdepth 1 -name ${3}\*_${CHIP}${4}fits > \
            ${TEMPDIR}/modesub_images${CHIP}_$$

  if [ -s ${TEMPDIR}/modesub_images${CHIP}_$$ ]; then
  while read file
  do
    if [ -L ${file} ]; then
      LINK=`${P_READLINK} ${file}`
      RESULTDIR[${CHIP}]=`dirname ${LINK}`
    else
      RESULTDIR[${CHIP}]="/$1/$2"
    fi

    BASE=`basename ${file} ${4}fits`
    MODE=`${P_STATS} < \
            ${RESULTDIR[${CHIP}]}/${BASE}"${4}fits" | \
            ${P_GAWK} '($1=="mode") {print $3}'`

    # subtract the mode but leave areas which have exactly zero
    # untouched.
    ${P_IC} '0 %1 '${MODE}' - %1 fabs 1.0e-09 < ?' \
       ${RESULTDIR[${CHIP}]}/${BASE}"${4}fits" > \
       ${RESULTDIR[${CHIP}]}/${BASE}"tmp.fits"

    mv ${RESULTDIR[${CHIP}]}/${BASE}"tmp.fits" \
       ${RESULTDIR[${CHIP}]}/${BASE}"${4}fits"

    # deal with saturation level if necessary:
    SATLEVEL=`${P_DFITS} ${RESULTDIR[${CHIP}]}/${BASE}"${4}fits" |\
              ${P_FITSORT} -d SATLEVEL | ${P_GAWK} '{print $2}'`

    if [ "${SATLEVEL}" != "KEY_N/A" ]; then
      NEWSATLEVEL=`${P_GAWK} 'BEGIN {
                                printf("%.2f", '${SATLEVEL}'-('${MODE}'))
                              }'`
      value ${NEWSATLEVEL}
      writekey ${RESULTDIR[${CHIP}]}/${BASE}"${4}fits" SATLEVEL \
               "${VALUE} / Saturation Level" REPLACE
    fi
  done < ${TEMPDIR}/modesub_images${CHIP}_$$
  else
    theli_warn "No images to process for chip ${CHIP}."
  fi

  test -f ${TEMPDIR}/modesub_images${CHIP}_$$ && \
       rm ${TEMPDIR}/modesub_images${CHIP}_$$
done

theli_end "${!#}"
exit 0;
