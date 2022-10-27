#!/bin/bash -u

# the script creates global weight and flag images

# 18.02.04:
# the creation of the WEIGHTS directory has to be shifted
# to the very start of the script. Otherwise it may be that
# the script tries to create links into a not yet existing
# directory.
#
# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 03.02.05:
# I resolved the hardcoding as the 11th argument
# being the last argument. The last argument can
# directly be accessed via ${!#}.
#
# 06.01.2013:
# The second argument now indicates whether links of the input
# files are followed or not. In some cases, e.g. KIDS processing,
# we have different calibration files linked to different places
# and we would like to just leave the global weight and flag files
# in the WEIGHTS directory.
#
# 18.02.2013:
# I introduced the new THELI logging scheme into the script.
#
# 14.12.2015:
# Temporary files were cleaned in the middle of the script. We want
# to keep them in debug mode - fixed.

# $1: main directory
# $2: LINK / NOLINK (do we follow links of input data)
# $3....: directories from which the weights have to be
#         created. The first dir is also the input weight.
#         Every sequence has three arguments: The dir.,
#         the low and high thressholds
# last argument: chips to be processed

# creates global weight AND Flag images

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# define THELI_DEBUG variable because of the '-u' script flag
THELI_DEBUG=${THELI_DEBUG:-""}

function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"

    for CHIP in ${!#}
    do
      test -f ${TEMPDIR}/flag_${CHIP}_$$.fits && \
           rm ${TEMPDIR}/flag_${CHIP}_$$.fits

      test -f  ${TEMPDIR}/${CHIP}.ww_$$ && \
            rm ${TEMPDIR}/${CHIP}.ww_$$
    done
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles \"${!#}\"; exit 1" INT

# Here the main script task starts:
if [ ! -d /$1/WEIGHTS ]; then
  mkdir -p /$1/WEIGHTS
fi

for CHIP in ${!#}
do
  j=3
  l=4
  m=5
  i=1
  WNAMES=""
  WMIN=""
  WMAX=""
  WFLAG=""
  ACTUFLAG=2

  MAXARG=$(( $# - 1 ))

  # create dummy flag
  ${P_IC} -c ${SIZEX[${CHIP}]} ${SIZEY[${CHIP}]} \
          -p 8 '0' > ${TEMPDIR}/flag_${CHIP}_$$.fits

  while [ "$j" -le "${MAXARG}" ]
  do
    DIR=`echo $* | awk '{print $'${j}'}'`
    MIN=`echo $* | awk '{print $'${l}'}'`
    MAX=`echo $* | awk '{print $'${m}'}'`

    # check existence of requested file:
    if [ ! -s /$1/${DIR}/${DIR}_${CHIP}.fits ]; then
      theli_error "File /$1/${DIR}/${DIR}_${CHIP}.fits does not exist! Exiting!"
      cleanTmpFiles "${!#}"
      exit 1;
    fi

    if [ "${i}" -eq "1" ]; then
      WNAMES="/$1/${DIR}/${DIR}_${CHIP}.fits"
      WMIN="${MIN}"
      WMAX="${MAX}"
      WFLAG="${ACTUFLAG}"
    else
      WNAMES="${WNAMES},/$1/${DIR}/${DIR}_${CHIP}.fits"
      WMIN="${WMIN},${MIN}"
      WMAX="${WMAX},${MAX}"
      WFLAG="${WFLAG},${ACTUFLAG}"
    fi

    # create link if necessary:
    if [ "$2" = "LINK" ] && [ -L "/$1/${DIR}/${DIR}_${CHIP}.fits" ]; then
      LINK=`${P_READLINK} /$1/${DIR}/${DIR}_${CHIP}.fits`
      RESULTDIR=`dirname ${LINK}`
      ln -s ${RESULTDIR}/globalweight_${CHIP}.fits \
            /$1/WEIGHTS/globalweight_${CHIP}.fits
      ln -s ${RESULTDIR}/globalflag_${CHIP}.fits \
            /$1/WEIGHTS/globalflag_${CHIP}.fits
    else
      RESULTDIR="/$1/WEIGHTS/"
    fi

    ACTUFLAG=$(( ${ACTUFLAG} * 2 ))
    j=$(( $j + 3 ))
    l=$(( $l + 3 ))
    m=$(( $m + 3 ))
    i=$(( $i + 3 ))

    #write config file for ww
    {
      echo "WEIGHT_NAMES ${WNAMES}"
      echo "WEIGHT_MIN ${WMIN}"
      echo "WEIGHT_MAX ${WMAX}"
      echo "WEIGHT_OUTFLAGS ${WFLAG}"
      #
      echo "FLAG_NAMES ${TEMPDIR}/flag_${CHIP}_$$.fits"
      echo 'FLAG_MASKS "0x0"'
      echo 'FLAG_WMASKS "0x0"'
      echo 'FLAG_OUTFLAGS "0"'
    } > ${TEMPDIR}/${CHIP}.ww_$$
    #
    if [ -f "${INSTRUMENT}_${CHIP}.reg" ]; then
      {
        echo "POLY_NAMES ${INSTRUMENT}_${CHIP}.reg"
        echo "POLY_OUTFLAGS 1"
      } >> ${TEMPDIR}/${CHIP}.ww_$$
    else
      {
        echo 'POLY_NAMES ""'
        echo 'POLY_OUTFLAGS ""'
      } >> ${TEMPDIR}/${CHIP}.ww_$$
    fi
    #
    {
      echo "OUTWEIGHT_NAME ${RESULTDIR}/globalweight_${CHIP}.fits"
      echo "OUTFLAG_NAME ${RESULTDIR}/globalflag_${CHIP}.fits"
    } >> ${TEMPDIR}/${CHIP}.ww_$$
  done

  ${P_WW} -c ${TEMPDIR}/${CHIP}.ww_$$
done

# clean up and bye:
cleanTmpFiles "${!#}"
theli_end "${!#}"
exit 0;
