#!/bin/bash -u

# ----------------------------------------------------------------
# File Name:           create_binnedmosaics.sh
# Author:              Thomas Erben (terben@astro.uni-bonn.de)
# Last modified on:    13.02.2013
# Description:         creation of binned mosaic images for
#                      regularly shaped multi-chip cameras
# ----------------------------------------------------------------

# Script history information:
#
# 20.09.2007:
# script written
#
# 01.01.2010:
# improvement of documentation and script maintainance
#
# 13.02.2013:
# I introduced the new THELI logging scheme into the script

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"


# define THELI_DEBUG because of the '-u' script flag
# (the use of undefined variables will be treated as errors!)
# THELI_DEBUG is used in the cleanTmpFiles function.
THELI_DEBUG=${THELI_DEBUG:-""}

# function definitions:
function printUsage
{
  echo "SCRIPT NAME:"
  echo "    create_binnedmosaics.sh - create binned mosaics of multi-chip exposures"
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
  echo "    files of a multi-chip instrument. The mosaic must be 'regualrly'"
  echo "    shaped, i. e. the mosaic must form a rectangle with each chip"
  echo "    having the SAME x- and y-dimensions. If this is not the case you"
  echo "    have to provide an own, instrument dependent version of this"
  echo "    script.  The created image mosaics allow us an easy evaluation of"
  echo "    prereduction quality and a comfortable detection of image defects"
  echo "    that might need manual intervention."
  echo ""
  echo "    The script takes its input images from /main_dir/science_dir/,"
  echo "    creates a subdirectory BINNED and puts its results therein."
  echo "    The script gets the chip geometry from the shell variable "
  echo "    CHIPGEOMETRY in the THELI instrument definition file. "
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
  echo '    If you want an arbitrary image_prefix, use "" as '
  echo "    command line argument, e.g."
  echo "    "
  echo "    create_binnedmosaics.sh /aibn85_2/terben/DATA \\"
  echo '       SCIENCE_R "" OFCSF 8 -32'
  echo "    "
  echo "AUTHOR:"
  echo "    Thomas Erben (terben@astro.uni-bonn.de)"
  echo ""
}

function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"

    rm ${TEMPDIR}/image_list_$$
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles; exit 1" INT

# check validity of command line arguments and necessary shell variables:
if [ $# -lt 5 ] || [ $# -gt 6 ] ; then
  printUsage
  exit 1;
fi

if [ -z "${CHIPGEOMETRY}" ]; then
  theli_error "CHIPGEOMETRY environment variable not defined! Exiting!"
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

# Decompose chip geometry variable:
NCHIPSX=`echo ${CHIPGEOMETRY} | ${P_GAWK} '{print $1}'`
NCHIPSY=`echo ${CHIPGEOMETRY} | ${P_GAWK} '{print $2}'`

# Crude check whether something is wrong with the CHIPGEOMETRY var.
test -z ${NCHIPSY} && \
     { theli_error "Malformed CHIPGEOMETRY variable! Exiting !!"; exit 1; }

cd ${MD}/${SD}\
 || { theli_error "Directory ${MD}/${SD} does not exist! Exiting"; exit 1; }

if [ ! -d BINNED ]; then
  mkdir BINNED
fi

test -f ${TEMPDIR}/image_list_$$ && rm ${TEMPDIR}/image_list_$$

# note that we assume that FITS images are always present for
# ALL chips; also if they are finally not usable:
${P_FIND} . -name ${PREFIX}\*_1${EXTENSION}.fits > ${TEMPDIR}/image_list_$$

while read file
do
  BASE=`basename ${file} _1$4.fits`
  echo "Working on mosaic ${BASE}"

  # build up image list for the album command:
  LIST=""
  i=1
  while [ ${i} -le ${NCHIPS} ]
  do
    LIST="${LIST} ${BASE}_${i}$4.fits"
    i=$(( $i + 1 ))
  done

  # and do the mosaicing:
  ${P_ALBUM} -l -p ${BITPIX} -b ${BINNING} ${NCHIPSX} ${NCHIPSY} ${LIST}\
             > BINNED/${BASE}_mos.fits

done < ${TEMPDIR}/image_list_$$

# clean temporary files and bye
cleanTmpFiles

theli_end
exit 0
