#!/bin/bash -u

# checks images for integrity. For the moment this consists of
# calculating the mode and shifting images that are under/over a given
# mode to a BADMODE directory the script cannot be parallelised as
# chips in one parallel channel may be filtered and not in
# another. But we want to filter all chips of an image that has ONE or
# more bad chips
#

# 30.05.2004:
# temporary files go to a TEMPDIR directory
#
# 11.04.2005:
# I rewrote the script to use the 'imstats' program
# instead of the FLIPS based 'immode'.
#
# 12.04.2005:
# I substituted XMIN etc. by STATSXMIN etc. that are
# now defined in the in the instrument initialisation
# files.
#
# 14.08.2005:
# The call of the UNIX 'find' program is now done
# via a variable 'P_FIND'.
#
# 09.09.2005:
# I corrected a bug in the FIND command. We need to
# test regular files and links, not only regular files.
#
# 12.09.2005:
# The imstats call is now done chip by chip. This prevents
# an "Argument list too long error" for cameras with many
# chips (e.g. MMT with 72).
#
# 05.12.2005:
# - Chips whose NOTUSE or NOTPROCESS flag is set are not considered in
#   the statistics that determine whether to reject exposures.
# - Temporary files are now cleaned up at the end of the
#   script
# - I added a missing TEMPDIR
#
# 28.11.2008:
# temporary files are cleaned at the end of the script!
#
# 01.10.2010:
# I made the call to 'imstats' more efficient by doing only ONE
# call for data of all chips instead of a call for each chip.
#
# 04.01.2013:
# I made the script more robust to non-existing files.
#
# 03.05.2013:
# I added an optional argument giving the name of a file to which
# BADMODE files are written.

# $1: main directory (filter)
# $2: science directory
# $3: ending (OFCSFF etc.)
# $4: low limit of files labelled as GOOD
# $5: high limit of files labelled as GOOD
# $6: outputfile listing BADMODE images (optional)

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

test -f ${TEMPDIR}/alldata_$$.dat    && rm ${TEMPDIR}/alldata_$$.dat
test -f ${TEMPDIR}/statsfiles.txt_$$ && rm ${TEMPDIR}/statsfiles.txt_$$

i=1
while [ "${i}" -le "${NCHIPS}" ]
do
  if [ ${NOTUSE[${i}]:=0} -eq 0 ] && [ ${NOTPROCESS[${i}]:=0} -eq 0 ]
  then
    ${P_FIND} /$1/$2/ -maxdepth 1 \( -type f -o -type l \) \
      -name \*_${i}$3.fits >> ${TEMPDIR}/statsfiles.txt_$$
  else
    theli_warn "Chip ${i} will not be used!"
  fi
  i=$(( $i + 1 ))
done

if [ -s ${TEMPDIR}/statsfiles.txt_$$ ]; then
  ${P_IMSTATS} `cat ${TEMPDIR}/statsfiles.txt_$$` -s \
               ${STATSXMIN} ${STATSXMAX} ${STATSYMIN} ${STATSYMAX} -o \
               ${TEMPDIR}/alldata_$$.dat

  # now get all sets that have to be moved:
  ${P_GAWK} '($1 != "#") {
             if ($2 < '$4' || $2 > '$5') {
               a = match($1, "_[0-9]*'$3'.fits");
               print substr($1, 1, a - 1);
             }}' \
            ${TEMPDIR}/alldata_$$.dat > ${TEMPDIR}/move_$$

  # now do the moving
  while read FILE
  do
    DIR=`dirname ${FILE}`
    if [ ! -d "${DIR}/BADMODE" ]; then
      mkdir "${DIR}/BADMODE"
    fi

    i=1
    while [ "${i}" -le "${NCHIPS}" ]
    do
      # the following 'if' is necessary as the
      # file(s) may already have been moved
      test -f "${FILE}_${i}$3.fits" && mv ${FILE}_${i}$3.fits ${DIR}/BADMODE

      i=$(( $i + 1 ))
    done
  done < ${TEMPDIR}/move_$$

  if [ $# -eq 6 ]; then
    awk -F/ '{print $NF}' ${TEMPDIR}/move_$$ > $6
  fi
else
  theli_error "No files to process"
  test -f ${TEMPDIR}/statsfiles.txt_$$ && rm ${TEMPDIR}/statsfiles.txt_$$
  exit 1;
fi

# clean up:
test -f ${TEMPDIR}/alldata_$$.dat    && rm ${TEMPDIR}/alldata_$$.dat
test -f ${TEMPDIR}/move_$$           && rm ${TEMPDIR}/move_$$
test -f ${TEMPDIR}/immode.dat_$$     && rm ${TEMPDIR}/immode.dat_$$
test -f ${TEMPDIR}/statsfiles.txt_$$ && rm ${TEMPDIR}/statsfiles.txt_$$

theli_end
exit 0;
