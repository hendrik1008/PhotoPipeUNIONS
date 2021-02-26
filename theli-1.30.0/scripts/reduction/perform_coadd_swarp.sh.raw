#!/bin/bash -u

# the script coadds images with Emmanuels swarp program. It takes the
# resampled images created by resample_coadd_swarp_para.sh.  So far it
# cannot be parallelised!
#
# Note that a coaddition with another header than that created by
# prepare_coadd_swarp.sh and later used by
# resample_coadd_swarp_para.sh leads to false results !!!

# 02.03.2004:
# The ending of the resampled FITS images is now
# "COADIDENT".resamp.fits instead of simply .resamp.fits
#
# 08.06.2006:
# The existence of the file 'coadd.head' is checked before
# it is being copied.
#
# 17.07.2006:
# The swarp COMBINE_TYPE parameter can now be given as (optional)
# command line argument. If not provided, WEIGHTED is applied.
#
# 25.07.2006:
# The 'coadd.head' file stored in the reduce directory gets a unique
# name consisting of science dir. and co-addition identifier. This
# allows the execution of several co-additions simultaneously.
#
# 28.08.2006:
# The filenames to be co-added are passed to swarp no longer on the
# command line but in a file instead.
#
# 07.09.2006:
# I corrected a bug in the call to swarp. Images were passed on the
# command line AND via a file list.
#
# 17.02.2007:
# A flag map for the co-added image is created. It contains a '1'
# where the wqeight map is zero, i.e. bad pixels are marked.  All
# other pixels get a value of zero. The flag map is an eight bit FITS
# image.
#
# 04.09.2008:
# There are now four more command line arguments:
# - The user needs to provide the image ending for images
#   to be co-added
# - The user needs to provide an ending for the filename
#   of the final co-added image.
# - The user needs to provide the WEIGHT_TYPE of the input
#   weights; this allows us to use no weights at all
#   (WEIGHT_TYPE NONE)
# - The user decides whether a FLAG map is created or not
#   (it was created by default before)
#
# These changes ensure that the script is more general and can also be
# applied to derived products from resampled images.
#
# 16.09.2010:
# I made the construction of the filename for the coadd.head file
# in the reduce directory more robust. There were problems if
# a pathname ended with a slash.
#
# 26.11.2010:
# I adapted the call to swarp to the syntax of the new version (2.19.1).
# This includes a different way to provide the image list,
# name of a header keyword for the saturation level and trying to coadd
# data with multiple threads.
#
# 05.05.2011:
# new mandatory command line switch for the path of the coadd.head
# file used by this script. Before the treatment of the coadd.head
# file within this script was completely faulty!
#
# 07.02.2013:
# I introduced the new THELI logging scheme into the script.
#
# 28.05.2015:
# I removed the limitation of swarp threads to 16. The program does
# not have that limitation anymore. However, it was theoretical in any
# case because we never can make use of those high thread counts for
# swarp within this script! During co-addition we up to now only saw
# swarp running with about 200% (2 processors) in all circumstances.
#
# 11.01.2016:
# - command line arguments are now accessed with meaningful names
# - I make sure that a the flux scale keyword FLXSCALE is not taken into
#   account when creating a sum image. I do not understand that a problem
#   with that only occured now during DECan processing!

#$1: main dir.
#$2: science dir.
#$3: coadd identifier
#$4: image ending of frames to be co-added
#    (including a '.' before the final 'fits')
#$5: ending for the output image; it is defined
#    as coadd.${5}fits
#$6: location of the global coadd.head file (complete absolute path)
#$7: WEIGHT_TYPE of input WEIGHTS
#$8: FLAG/NOFLAG; create a FLAG image or not
#    (Pixels that are zero in the weight map, i.e. bad
#     are masked with a '1' in the flag map.)
#$9: swarp COMBINE_TYPE (OPTIONAL: WEIGHTED as default)

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

MD=$1
SD=$2
COADDIDENT=$3
ENDING=$4
ENDING_OUT=$5
HEAD_PATH=$6
WEIGHT_TYPE=$7
FLAG=$8

# set swarp COMBINE type:
COMBINETYPE="WEIGHTED"
if [ $# -eq 9 ]; then
  COMBINETYPE=$9
fi

DIR=`pwd`

# construct a unique name for the coadd.head file
# of this co-addition:
#
# The following 'sed' ensures a 'unique' construction with the
# '/' character which can appear in arbitrary combinations in
# file- and pathnames.
TMPNAME_1=`echo ${MD} | sed -e 's!/\{2,\}!/!g' -e 's!/*$!!' |\
           ${P_GAWK} -F/ '{print $NF}'`
TMPNAME_2=`echo ${SD} | sed -e 's!/\{2,\}!/!g' -e 's!/*$!!' |\
           ${P_GAWK} -F/ '{print $NF}'`
COADDFILENAME=${TMPNAME_1}_${TMPNAME_2}_${COADDIDENT}

cd ${MD}/${SD}/coadd_${COADDIDENT}

if [ ! -s coadd_$$.head ]; then
  if [ ! -s ${HEAD_PATH}/coadd_${COADDFILENAME}.head ]; then
    theli_error "I need ${HEAD_PATH}/coadd_${COADDFILENAME}.head to continue!"
    theli_error "Did you run 'prepare_coadd_swarp.sh'?"
    exit 1;
  else
    cp ${HEAD_PATH}/coadd_${COADDFILENAME}.head ./coadd_$$.head
  fi
fi

# collect the files to be co-added
test -f files.list_$$ && rm files.list_$$

${P_FIND} . -maxdepth 1 -name \*${ENDING}fits -print > files.list_$$

# If we create a sum image, we need to be sure not to
# do any photometric scaling of images:
FLXSCALE=""
if [ "${COMBINETYPE}" = "SUM" ]; then
  FLXSCALE="-FSCALE_KEYWORD NONE"
fi

# go if we have something to do at all:
if [ -s files.list_$$ ]; then
  ${P_SWARP} -c ${DATACONF}/create_coadd_swarp.swarp \
             -RESAMPLE N -COMBINE Y \
             -IMAGEOUT_NAME coadd_$$.fits \
             -WEIGHTOUT_NAME coadd_$$.weight.fits \
             -COMBINE_TYPE ${COMBINETYPE} \
             -WEIGHT_TYPE ${WEIGHT_TYPE} \
             -SATLEV_KEYWORD SATLEVEL ${FLXSCALE} \
             -NTHREADS ${NPARA} \
             @files.list_$$

  mv coadd_$$.fits coadd.${ENDING_OUT}fits
  mv coadd_$$.weight.fits coadd.${ENDING_OUT}weight.fits

  # create a flag map with name coadd.flag.fits.
  # Pixels that are zero in the weight map, i.e. bad
  # are masked with a '1' in the flag map.
  if [ "${FLAG}" = "FLAG" ]; then
    ${P_IC} -p 8 '1 0 %1 fabs 1.0e-06 < ?' \
            coadd.${ENDING_OUT}weight.fits > coadd.${ENDING_OUT}flag.fits
  fi
else
  theli_error "No images to coadd! Exiting!"
  exit 1;
fi


rm files.list_$$
cd ${DIR}

theli_end
exit 0;
