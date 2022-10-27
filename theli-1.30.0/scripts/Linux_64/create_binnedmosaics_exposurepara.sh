#!/bin/bash -u

# ----------------------------------------------------------------
# File Name:           create_binnedmosaics_exposurepara.sh
# Author:              Thomas Erben (terben@astro.uni-bonn.de)
# Last modified on:    22.07.2014
# Description:         creation of binned mosaic images for
#                      multi-chip cameras (parallel version)
# ----------------------------------------------------------------

# Script history information:
#
# 29.07.2011:
# script written - a modification of create_binnedmosaic.sh
#
# 09.01.2012:
# I made the script more robust to non-existing files
#
# 22.07.2014:
# I extended the script also for non-regularly shaped CCD mosaics.
# In this case a configuration file for album has to be provided.
# If a configuartion file with the name 'alub_${INSTRUMENT}.conf'
# is present in the pwd we use the information therein instead of
# assuming a regularly shaped CCD mosaic (the geometry for the first
# ones is specified within the CHIPGEOMETRY environment variable. This
# variable does not need to be set in case a configuration file is
# present.
#
# 07.11.2017:
# The script now also works if not all chips from an image are present.
# The missing chips are substituted with a dummy of zeros in that case.

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# define variables because of the '-u' script flag
# (the use of undefined variables will be treated as errors!)
# THELI_DEBUG is used in the cleanTmpFiles function.
THELI_DEBUG=${THELI_DEBUG:-""}

# CHIPGEOMETRY does not need to be set if an album configuration file
# is provided:
CHIPGEOMETRY=${CHIPGEOMETRY:-""}

# function definitions:
function printUsage
{
  echo "SCRIPT NAME:"
  echo "    create_binnedmosaics_exposurepara.sh - create binned mosaics"
  echo ""
  echo "ARGUMENTS:"
  echo "    1. main_dir."
  echo "    2. science_dir."
  echo "    3. image_prefix"
  echo "    4. image_extension"
  echo "    5. binning_factor"
  echo "    6. BITPIX of output images (16 or -32)"
  echo "       (OPTIONAL; default is 16)"
  echo ""
  echo "DESCRIPTION:"
  echo "    The script creates binned mosaic images from individual chip FITS"
  echo "    files of a multi-chip instrument. The mosaic must either be"
  echo "    'regualrly' shaped, i. e. the mosaic must form a rectangle with"
  echo "    each chip having the SAME x- and y-dimensions. In this case the"
  echo "    chip geometry is defined within a CHIPGEOMETRY variable in the"
  echo "    instrument definition file. If the shape is not regular you must"
  echo "    provide an album_\${INSTRUMENT}.conf configuration file for the"
  echo "    'album' program. In addition, the (unbinned) dimensions of the"
  echo "    output file must be specified within an ALBUMOUTSIZE variable in"
  echo "    the instrument definition file. See the instrument DECAM for an"
  echo "    example. The created image mosaics allow us an easy evaluation of"
  echo "    prereduction quality and a comfortable detection of image defects"
  echo "    that might need manual intervention."
  echo ""
  echo "    The script takes its input images from /main_dir/science_dir/,"
  echo "    creates a subdirectory BINNED and puts its results therein. The"
  echo "    script gets the chip geometry from the shell variable CHIPGEOMETRY"
  echo "    in the THELI instrument definition file."
  echo ""
  echo "    Given that individual chip FITS files have names like"
  echo "    prefix*_'CHIPNUMBER'image_extension.fits (The '*' stands for an"
  echo "    arbitrary character string) the output images get the names"
  echo "    prefix*.fits"
  echo "     "
  echo "EXAMPLES:"
  echo "    create_binnedmosaics.sh /aibn85_2/terben/DATA \\"
  echo "       SCIENCE_R WFI OFCSF 8 -32"
  echo ""
  echo "    This bins all images in /aibn85_2/terben/DATA/SCIENCE_R"
  echo "    with names WFI*_'CHIP'OFCSF.fits, where 'CHIPS' stands "
  echo "    for numbers ranging from 1 to the number of chips for"
  echo "    the considered instrument.    "
  echo "    (e.g. WFI.2000-12-27T07:50:38.198_1OFCSF.fits,"
  echo "          WFI.2000-12-27T07:50:38.198_1OFCSF.fits and so on)"
  echo "    The name of the output images would be"
  echo "    WFI.2000-12-27T07:50:38.198.fits and so on."
  echo ""
  echo "    If you want an arbitrary image_prefix, use "" as "
  echo "    command line argument, e.g."
  echo "    "
  echo "    create_binnedmosaics.sh /aibn85_2/terben/DATA \\"
  echo "       SCIENCE_R "" OFCSF 8 -32"
  echo "    "
  echo "AUTHOR:"
  echo "    Thomas Erben (terben@astro.uni-bonn.de)"
  echo ""
}

function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"

    for FILE in ${TEMPDIR}/*_$$
    do
      rm ${FILE}
    done
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles; exit 1" INT

# check validity of command line arguments and necessary shell variables:
if [ $# -lt 5 ] || [ $# -gt 6 ] ; then
  theli_error "Wrong number of command line arguments! Exiting!"
  exit 1
fi

if [ ! -f album_${INSTRUMENT}.conf ] && [ -z "${CHIPGEOMETRY}" ]; then
  theli_error "Before using this script, you need to define the CHIPGEOMETRY variable in your instrument definition file for ${INSTRUMENT} or you need to provide an album configuration file. Exiting!"
  exit 1;
fi

# set command line arguments to meaningful names:
MD=$1
SD=$2
PREFIX=$3
EXTENSION=$4
BINNING=$5

# set default for output BITPIX to 16:
if [ $# -lt 6 ]; then
  BITPIX=16
else
  BITPIX=$6
fi

# Here the main tasks of the script begin:

# check for an album configuration file. If present we treat
# the mosaic as irregular:
if [ -f album_${INSTRUMENT}.conf ]; then
  ALBUMCONF="-f `pwd`/album_${INSTRUMENT}.conf"
  NCHIPSX=""
  NCHIPSY=""
else
  ALBUMCONF=""
  ALBUMOUTSIZE="" # for irregularly shaped CCD mosaics this is defined
                  # in the instrument definition file.
  # Decompose chip geometry variable.
  NCHIPSX=`echo ${CHIPGEOMETRY} | ${P_GAWK} '{print $1}'`
  NCHIPSY=`echo ${CHIPGEOMETRY} | ${P_GAWK} '{print $2}'`

  # Crude check whether something is wrong with the CHIPGEOMETRY var.
  test -z ${NCHIPSY} && \
     { theli_error "Malformed CHIPGEOMETRY variable! Exiting !!"; exit 1; }
fi

cd ${MD}/${SD} \
 || { theli_error "Directory ${MD}/${SD} does not exist! Exiting"; exit 1; }

if [ ! -d BINNED ]; then
  mkdir BINNED
fi

# note that we assume that FITS images are always present for
# ALL chips; also if they are finally not usable:
${P_FIND} . -name ${PREFIX}\*${EXTENSION}.fits | \
  sed -e 's/_[0-9]\+'${EXTENSION}'.fits//' | sort | uniq > \
  ${TEMPDIR}/exposure_list_$$

# do we need to do something at all?
if [ -s ${TEMPDIR}/exposure_list_$$ ]; then
  # distribute task on different processors/cores:
  NIMA=`wc -l ${TEMPDIR}/exposure_list_$$ | ${P_GAWK} '{print $1}'`

  if [ ${NPARA} -gt ${NIMA} ]; then
    NPARA=${NIMA}
  fi

  PROC=0
  while [ ${PROC} -lt ${NPARA} ]
  do
    NFIELDS[${PROC}]=0
    FIELDS[${PROC}]=""
    PROC=$(( ${PROC} + 1 ))
  done

  i=1
  while read FIELD
  do
    PROC=$(( $i % ${NPARA} ))
    FIELDS[${PROC}]="${FIELDS[${PROC}]} ${FIELD}"
    NFIELDS[${PROC}]=$(( ${NFIELDS[${PROC}]} + 1 ))
    i=$(( $i + 1 ))
  done < ${TEMPDIR}/exposure_list_$$

  PROC=0
  while [ ${PROC} -lt ${NPARA} ]
  do
    j=$(( ${PROC} + 1 ))
    echo -e "Starting Job $j. It has ${NFIELDS[${PROC}]} files to process!\n"
    {
      k=1
      for FIELD in ${FIELDS[${PROC}]}
      do
        echo "Working on mosaic ${FIELD}"

        # build up image list for the album command:
        LIST=""
        for CHIP in $(seq 1 ${NCHIPS})
        do
          # if a chip is not present, we substitute it with a fake image
          # of zeros:
          if [ -s ${FIELD}_${CHIP}${EXTENSION}.fits ]; then
            LIST="${LIST} ${FIELD}_${CHIP}${EXTENSION}.fits"
          else
            if [ ! -s ${TEMPDIR}/dummy_${CHIP}.fits_$$ ]; then
              ${P_IC} -c ${SIZEX[${CHIP}]}  ${SIZEY[${CHIP}]} '0' > \
                 ${TEMPDIR}/dummy_${CHIP}.fits_$$
            fi
            LIST="${LIST} ${TEMPDIR}/dummy_${CHIP}.fits_$$"
          fi
        done

        # and do the mosaicing:
        ${P_ALBUM} ${ALBUMCONF} -l -p ${BITPIX} \
                   -b ${BINNING} ${NCHIPSX} ${NCHIPSY} \
                   ${ALBUMOUTSIZE} ${LIST} \
                   > BINNED/${FIELD}_mos.fits
        k=$(( $k + 1 ))
      done
    } &
    PROC=$(( ${PROC} + 1 ))
  done

  # only continue once all processes have finished
  wait;
else
  theli_error "No images to process!"
  cleanTmpFiles
  exit 1;
fi

# clean temporary files and bye
cleanTmpFiles
theli_end

exit 0
