#!/bin/bash -u

# the script collects some header keywords of a directory with raw fits
# files into an LDAC catalog with a FILES table. These data are then
# used to distinguish 'sets' of images
#
# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 03.12.2004:
# I substituted the initial 'ls' listing all
# to be looked at by a find command. With 'ls'
# it could happen that we exceed its argument list.
#
# 14.08.2005:
# The call of the UNIX 'find' program is now done
# via a variable 'P_FIND'.
#
# 30.11.2005:
# A ${TEMPDIR} was mssing when creating the initial
# file list.
#
# 06.10.2008:
# temporary files are cleaned at the end of the script
#
# 13.06.2014:
# - I introduced THELI logging
# - command line arguments get meaningful names and are no longer
#   referenced via '$1' etc.

#$1: main directory
#$2: science directory
#$3: image extension (ext) on ..._iext.fits (i is the chip number)
#$4: minimum overlap for counting exposures to the same set
#    (provided in pixels)

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# give meaningful names to command line arguments:
MD=$1
SD=$2
ENDING=$3
MINOVERLAP=$4

# configuration parameters:
FILTERKEY="FILTER"
OBJECTKEY="OBJECT"
RAKEY="CRVAL1"
DECKEY="CRVAL2"

${P_FIND} ${MD}/${SD} -name \*${ENDING}.fits -print > ${TEMPDIR}/images_$$

test -f ${TEMPDIR}/images_tmp.dat_$$ && rm ${TEMPDIR}/images_tmp.dat_$$

while read IMAGE
do
  NAME=`basename ${IMAGE}`
  RA=`${P_DFITS}  ${IMAGE} | ${P_FITSORT} -d ${RAKEY} |\
      cut -f 2`
  DEC=`${P_DFITS}  ${IMAGE} | ${P_FITSORT} -d ${DECKEY} |\
      cut -f 2`
  OBJECT=`${P_DFITS}  ${IMAGE} | ${P_FITSORT} -d ${OBJECTKEY} |\
          cut -f 2`
  FILTER=`${P_DFITS}  ${IMAGE} | ${P_FITSORT} -d ${FILTERKEY} |\
          cut -f 2`

  echo ${NAME} ${OBJECT} ${FILTER} ${RA} ${DEC} >> ${TEMPDIR}/images_tmp.dat_$$
done < ${TEMPDIR}/images_$$

${P_GAWK} '{print NR, $0}' ${TEMPDIR}/images_tmp.dat_$$ \
                         > ${TEMPDIR}/images.dat_$$

${P_ASCTOLDAC} -i ${TEMPDIR}/images.dat_$$ -o ${TEMPDIR}/science.cat_$$ \
               -c ${CONF}/make_files_cat_asctoldac.conf \
               -t FILES

${P_SELECTOVERLAPS} -i ${TEMPDIR}/science.cat_$$ \
                    -o ${TEMPDIR}/science_set.cat_$$ \
                    -MIN_OVER ${MINOVERLAP}

${P_LDACTOASC} -i ${TEMPDIR}/science_set.cat_$$ \
               -b -t FILES \
               -k FITSFILE POSSET -s > ${TEMPDIR}/tmp.asc_$$

while read FILE SET
do
  test -d ${MD}/set_${SET} || mkdir -p ${MD}/set_${SET}

  mv ${MD}/${SD}/${FILE} ${MD}/set_${SET}
done < ${TEMPDIR}/tmp.asc_$$

# clean up:
test -f ${TEMPDIR}/tmp.asc_$$          && rm ${TEMPDIR}/tmp.asc_$$
test -f ${TEMPDIR}/science_set.cat_$$  && rm ${TEMPDIR}/science_set.cat_$$
test -f ${TEMPDIR}/images_tmp.dat_$$   && rm ${TEMPDIR}/images_tmp.dat_$$
test -f ${TEMPDIR}/images_$$           && rm ${TEMPDIR}/images_$$
test -f ${TEMPDIR}/images.dat_$$       && rm ${TEMPDIR}/images.dat_$$

theli_end
exit 0;
