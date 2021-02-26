#!/bin/bash

# the script cleanes a directory from files with a given prefix/ending
# The script takes into account links
#

# 26.01.2004:
# The third argument now contains the "."  before the fits
# extension. This prevents that empty strings have to be given as
# third argument (what made problem if this script was called within
# another script)
#
# 15.03.2004:
# if a link points again to a link, it is simply deleted and NO files
# are removed. To keep a consistent structure, first a pass is made
# through all the data and double links (and those pointing to
# nowhere) are deleted before the real deletion process starts.
#
# 30.05.04:
# tempaorary files go to a TEMPDIR directory
#
# 25.09.2008:
# temporary files are deleted at the end of the script!
#
# 31.10.2009:
# file lists are created now with 'find', instead of 'ls'.
#
# 28.06.2012:
# Bug fix; the actual use of arguments did not match the intended
# usage of the script (thanks to Oliver Cordes).
#
# 27.12.2012:
# We now avoid error messages when the script is applied to empty
# directories.

# $1: master directory
# $2: prefix of files
# $3: ending "OFCSFF etc.". Files end with _i$3fits
#     where i is the chip number
#     The ending contains the "." before the fits
#     extension.

# preliminary work
. ./${INSTRUMENT:?}.ini

#
# first make a pass to identify double links and links pointing to
# nowhere
if [ -d $1 ]; then
  ${P_FIND} $1 -maxdepth 1 -name $2\*$3fits > ${TEMPDIR}/tmp_$$

  while read FILE
  do
    if [ -L ${FILE} ]; then
      REALFILE=`${P_READLINK} ${FILE}`
      #
      # if REALFILE is again a link, only the link is deleted but NO
      # other files
      if [ -L ${REALFILE} ] || [ ! -e ${REALFILE} ]; then
        rm ${FILE}
      fi
    fi
  done < ${TEMPDIR}/tmp_$$

  #
  # now do the real deleting of files
  ${P_FIND} $1 -maxdepth 1 -name $2\*$3fits > ${TEMPDIR}/tmp_$$

  while read FILE
  do
    if [ -L ${FILE} ]; then
      LINK=`${P_READLINK} ${FILE}`
      rm ${LINK}
      rm ${FILE}
    else
      rm ${FILE}
    fi
  done < ${TEMPDIR}/tmp_$$
else
  echo "Directory $1 does not exist!; Nothing to do for $0"
fi

# clean up and bye:
test -f ${TEMPDIR}/tmp_$$ && rm ${TEMPDIR}/tmp_$$

exit 0;
