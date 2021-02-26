#!/bin/bash -u

# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 24.11.05:
# Instead of giving the keyword to be substituted
# on the command line we take the next free DUMMY
# keyword and substitute it with RUN.
#
# 02.05.2006:
# I replaced the 'replacekey' command by a call to
# 'writekey'. This ensures proper treatment if the
# script is run multiple times.
#
# 05.10.2008:
# - temporary files are deleted at the end of the script
# - replacement of 'ls' by 'find'
#
# 25.01.2012:
# A comment is added to the RUN key.
#
# 04.01.2013:
# I made the script more robust to non-existing files

# the script gives every image a RUNID
# (it replaces a superfluous header keyword
# by RUN) (parallel version)

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# $1: main dir
# $2: science dir (the cat dir is a subdirectory of this)
# $3: extension
#     This argument contains the "." before the fits
#     extension.
# $4: The runID the image should get. This is a string
#     without the surrounding '...' characteristic for
#     a string in FITS format
# $5: chips to be processed

for CHIP in ${!#}
do
  ${P_FIND} /$1/$2/ -maxdepth 1 \
            -name \*_${CHIP}$3fits > ${TEMPDIR}/uniqueidimages_$$

  if [ -s ${TEMPDIR}/uniqueidimages_$$ ]; then
    while read FILE
    do
      value "'$4'"
      writekey ${FILE} RUN "${VALUE} / THELI Observing Run" REPLACE
      # the following touch ensures that the file
      # gets a new timestamp (necessary to overcome
      # kernel bugs not updating the change time after
      # memory mapping)
      touch ${FILE}
    done < ${TEMPDIR}/uniqueidimages_$$
  else
    theli_error "No files to process for Chip ${CHIP}" "${!#}"
  fi
done

test -f ${TEMPDIR}/uniqueidimages_$$ && rm ${TEMPDIR}/uniqueidimages_$$

theli_end "${!#}"
exit 0;
