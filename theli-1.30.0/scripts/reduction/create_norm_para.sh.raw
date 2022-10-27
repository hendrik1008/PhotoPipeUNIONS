#!/bin/bash -u

# the script creates normalised images.  It puts the result into new
# directories ..._norm.  The script assumes a threshhold. Pixels under
# this threshhold will be assigned a value of threshhold in the
# resulting images.
#
# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 12.04.2005:
# - I rewrote the script so that it works with the
#   'imstats' program instead with the FLIPS based
#   'immode'.
# - I substituted XMIN etc. by STATSXMIN etc. that are
#   now defined in the in the instrument initialisation
#   files.
#
# 13.09.2005:
# If a camera has more than 10 chips, results were not correct.
#
# 31.01.2012:
# cleaning up of temporary files at the end

# $1: main directory
# $2: directory from which normaliced images should be created
# $3: chips to be processed

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# first calculate the modes:
FILES=""
for CHIP in ${!#}
do
  if [ -s /$1/$2/$2*_${CHIP}.fits ]; then
    FILES="${FILES} `ls /$1/$2/$2*_${CHIP}.fits`"
  else
    theli_warn "File /$1/$2/$2*_${CHIP}.fits not present!" "${!#}"
  fi
done

if [ "${FILES}" != "" ]; then
  ${P_IMSTATS} ${FILES} -s \
               ${STATSXMIN} ${STATSXMAX} ${STATSYMIN} ${STATSYMAX} -o \
               ${TEMPDIR}/immode.dat_$$

  # create the new directory for the normalized images if it does not
  # exist already
  test -d /$1/$2_norm || mkdir  /$1/$2_norm

  MODES=`${P_GAWK} '($1!="#") {printf ("%f ", $2)}' ${TEMPDIR}/immode.dat_$$`

  # Set the threshold to 20% of the smallest mode value. Note
  # that only the chips in the current parallelisation node
  # contribute.
  THRESH=`${P_GAWK} '($1!="#") {print $2}' ${TEMPDIR}/immode.dat_$$ |\
          ${P_GAWK} 'BEGIN{min = 1000000} {
                       if ($1 < min) {
                         min = $1
                       }} END {print 0.2 * min}'`

  i=1
  for CHIP in ${!#}
  do
    ACTUMODE=`echo ${MODES} | ${P_GAWK} '{print $'${i}'}'`

    if [ -L "/$1/$2/$2_${CHIP}.fits" ]; then
      LINK=`${P_READLINK} /$1/$2/$2_${CHIP}.fits`
      RESULTDIR=`dirname ${LINK}`
      ln -s ${RESULTDIR}/$2_norm_${CHIP}.fits \
            /$1/$2_norm/$2_norm_${CHIP}.fits
    else
      RESULTDIR=/$1/$2_norm/
    fi

    ${P_IC} '%1 '${ACTUMODE}' / '${THRESH}' %1 '${THRESH}' > ?' \
            /$1/$2/$2_${CHIP}.fits > ${RESULTDIR}/$2_norm_${CHIP}.fits

    i=$(( $i + 1 ))
  done
else
  theli_error "No file to process!" "${!#}"
  exit 1;
fi

test -f ${TEMPDIR}/immode.dat_$$ && rm ${TEMPDIR}/immode.dat_$$

theli_end "${!#}"
exit 0;
