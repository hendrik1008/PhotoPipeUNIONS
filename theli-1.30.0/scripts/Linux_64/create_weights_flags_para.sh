#!/bin/bash -u

# the script creates weights and flags for for science and photometric
# standard star frames.  It assumes the global weight images in the
# WEIGHT directory and the reg files in the sciencedir/reg directory.
#
# The current script version is a merging of the 'old'
# 'create_weights_para.sh' and 'create_weights_flags_para.sh' scripts.
#
# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 24.08.04:
# the SExtractor call to produce the cosmic ray mask now produces a
# DUMMY catalog in TEMPDIR (avoiding a lot of stdout messages during
# the processing)
#
# 25.04.2006:
# temporarily created FITS files (cosmic ray masks) are removed at the
# end of the script.
#
# 17.07.2006:
# the temporary file cosmic.cat is renamed to a unique name including
# the process number. It is also removed at the end of image
# processing.
#
# 22.09.2006:
# - I introduced the possibility to use specific cosmic ray mask if
#   necessary. The location of such masks has to be given as
#   (optional) command line argument. If a mask is not provided the
#   standard 'cosmic.ret.sex' mask is used.
# - I made the listing of files more robust if many files are involved
#   (by using 'find' instead of 'ls')
#
# 01.08.2007:
# some cleaning of not needed temporary files added
#
# 03.08.2007:
# I merged the two scripts 'create_weights_para.sh' and
# 'create_weights_flags_para.sh'.
#
# 08.08.2011:
# Bug fix concerning the usage of TEMPDIR (The error became apparent
# when the TEMPDIR was not identical to the reduction directory anymore).
#
# 01.01.2013:
# I made the script more robust to non-existing files and directories
#
# 07.04.2014:
# A new command line option to specify whether we have low S/N or high
# S/N data. In the case of low S/N the science data can have valid
# negative values (e.g. very low exposed u-band data in the ATLAS
# survey). In the case of high S/N such pixels are masked and give a
# weight of zero. This is the default for the large majority of our
# data.
#
# 19.08.2016:
# - Bug Fix: If the temporary directory is now cwd, temporary files
#   were not cleaned up correctly at the end of the script.
# - command line arguments now get meaningful names.

# $1: main directory
# $2: science dir.
# $3: image extension (ext) on ..._iext.fits (i is the chip number)
# $4: WEIGHTS, FLAGS OR WEIGHTS_FLAGS
#     determines whether weights, flags or weights AND flags
#     are produced
# $5: LOWSN / HIGHSN (for LOWSN images science images are allowed to
#     have valid, negative values; for HIGHSN images those pixels are
#     masked and given a weight of zero).
# $6: Filter to use for cosmic ray detection (OPTIONAL; if the argument
#     is not given, the default filter 'cosmic.ret.sex' is used)
# ${!#}: chips to be processed

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# give meaninigful names to command line arguments:
if [ $# -ne 6 ] && [ $# -ne 7 ]; then
  theli_error "USAGE $0 MD SD ENDING PRODUCTS LOWSN/HIGHSN [FILT] CHIPS"
  exit 1;
fi

MD=$1
SD=$2
ENDING=$3
PRODUCTS=$4
DATAKIND=$5
# set cosmic ray mask to use:
MASKFILE=${CONF}/cosmic.ret.sex
if [ $# -eq 7 ]; then
  MASKFILE=$6
fi

# check first whether a WEIGHTS directory exists. If not, probably
# something went wrong in preceding scripts and we do not need to
# continue:
if [ -d ${MD}/WEIGHTS ]; then
  # what needs to be done?
  WEIGHTSPROD=0
  FLAGSPROD=0

  if [ "${PRODUCTS}" = "WEIGHTS" ] || \
     [ "${PRODUCTS}" = "WEIGHTS_FLAGS" ]; then
    WEIGHTSPROD=1
  fi

  if [ "${PRODUCTS}" = "FLAGS" ] || \
     [ "${PRODUCTS}" = "WEIGHTS_FLAGS" ]; then
    FLAGSPROD=1
  fi

  if [ "${WEIGHTSPROD}" = 0 ] && [ "${FLAGSPROD}" = 0 ]; then
    theli_error "Nothing to do! Probably you gave the wrong mode!" "${!#}"
    exit 1;
  fi

  if [ "${DATAKIND}" = "LOWSN" ]; then
    WEIGHTVAL="-WEIGHT_MIN  0.1,-1e9,-100"
  else
    WEIGHTVAL="-WEIGHT_MIN  0.1,-1e9,0.0"
  fi

  for CHIP in ${!#}
  do
    ${P_FIND} ${MD}/${SD} -maxdepth 1 -name \*_${CHIP}${ENDING}.fits \
              -print > ${TEMPDIR}/crw_images_$$

    if [ -s ${TEMPDIR}/crw_images_$$ ]; then
    TESTFILE=`${P_GAWK} '(NR==1) {print $0}' ${TEMPDIR}/crw_images_$$`

    if [ -L ${TESTFILE} ]; then
      LINK=`${P_READLINK} ${TESTFILE}`
      RESULTDIR=`dirname ${LINK}`
    else
      RESULTDIR="${MD}/WEIGHTS"
    fi

    while read FILE
    do
      BASE=`basename ${FILE} ${ENDING}.fits`

      if [ -r "${MD}/${SD}/reg/${BASE}.reg" ]; then
        INPOLYS="-POLY_NAMES \"${MD}/${SD}/reg/${BASE}.reg\""
      else
        INPOLYS="-POLY_NAMES \"\""	
      fi

      INFLAGS="-FLAG_NAMES \"\""
      OUTFLAG="-OUTFLAG_NAME \"\""
      INWEIGHTS="\"\""
      OUTWEIGHT="-OUTWEIGHT_NAME \"\""

      if [ "${WEIGHTSPROD}" = "1" ]; then
        if [ -s ${RESULTDIR}/globalweight_${CHIP}.fits ]; then
          INWEIGHTS="${RESULTDIR}/globalweight_${CHIP}.fits"
          OUTWEIGHT="-OUTWEIGHT_NAME ${RESULTDIR}/${BASE}${ENDING}.weight.fits"
        else
          theli_error "Can't find ${RESULTDIR}/globalweight_${CHIP}.fits!" \
            "${!#}"
          exit 1;
        fi
      fi

      if [ "${FLAGSPROD}" = "1" ]; then
        if [ -s ${RESULTDIR}/globalflag_${CHIP}.fits ]; then
          INFLAGS="-FLAG_NAMES ${RESULTDIR}/globalflag_${CHIP}.fits"
          OUTFLAG="-OUTFLAG_NAME ${RESULTDIR}/${BASE}${ENDING}.flag.fits"
        else
          theli_error "Can't find ${RESULTDIR}/globalflag_${CHIP}.fits!" "${!#}"
          exit 1;
        fi
      fi

      # first run sextractor to identify cosmic rays:
      ${P_SEX} ${FILE} \
               -c ${CONF}/cosmic.conf.sex \
               -CHECKIMAGE_NAME ${TEMPDIR}/cosmic_${CHIP}_$$.fits\
               -FILTER_NAME ${MASKFILE} \
               -CATALOG_NAME ${TEMPDIR}/cosmic.cat_$$

      # then run weightwatcher:
      if [ "${INWEIGHTS}" = "" ]; then
        INWEIGHTS="${TEMPDIR}/cosmic_${CHIP}_$$.fits"
      else
        INWEIGHTS="${INWEIGHTS},${TEMPDIR}/cosmic_${CHIP}_$$.fits"
      fi
      INWEIGHTS="${INWEIGHTS},${FILE}"

      ${P_WW} -c ${CONF}/weights_flags.ww\
              ${WEIGHTVAL} \
              -WEIGHT_NAMES ${INWEIGHTS} ${INPOLYS} ${INFLAGS} \
                            ${OUTWEIGHT} ${OUTFLAG}

      if [ "${RESULTDIR}" != "${MD}/WEIGHTS" ]; then
        if [ "${WEIGHTSPROD}" = "1" ]; then
          ln -s ${RESULTDIR}/${BASE}${ENDING}.weight.fits \
                ${MD}/WEIGHTS/${BASE}${ENDING}.weight.fits
        fi
        if [ "${FLAGSPROD}" = "1" ]; then
          ln -s ${RESULTDIR}/${BASE}${ENDING}.flag.fits \
                ${MD}/WEIGHTS/${BASE}${ENDING}.flag.fits
        fi
      fi

      # clean up temporary files:
      test -f ${TEMPDIR}/cosmic_${CHIP}_$$.fits &&\
              rm ${TEMPDIR}/cosmic_${CHIP}_$$.fits

      test -f ${TEMPDIR}/cosmic.cat_$$ && rm ${TEMPDIR}/cosmic.cat_$$
    done < ${TEMPDIR}/crw_images_$$
    else
      theli_warn "No images to process for Chip ${CHIP}!" "${!#}"
    fi

    test -f ${TEMPDIR}/crw_images_$$ && rm  ${TEMPDIR}/crw_images_$$
  done
else
  theli_error "Directory ${MD}/WEIGHTS does not exist!" "${!#}"
  exit 1;
fi

# and bye:
# delete temporary files of necessary:
for FILE in ${TEMPDIR}/*_$$*
do
  test -f ${FILE} && rm ${FILE}
done

theli_end "${!#}"
exit 0;
