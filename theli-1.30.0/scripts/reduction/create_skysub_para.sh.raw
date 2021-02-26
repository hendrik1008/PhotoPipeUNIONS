#!/bin/bash -u

# ----------------------------------------------------------------
# File Name:           create_skysub_para.sh
# Author:              Thomas Erben (terben@astro.uni-bonn.de)
# Last modified on:    05.03.2013
# Description:         Performs sky subtraction of FITS images
#                      obtained in optical wavelengths
# ----------------------------------------------------------------

# Script history information:
# 24.09.2007:
# Script started (significant rewrite from an old script)
#
# 26.09.2007:
# - update of documentation
# - more robust treatment for a wrong skysubtraction mode
# - The subtraction modes are called ONEPASS and TWOPASS now
#
# 28.09.2007:
# - I corrected bugs in the deletion of temporary files
# - I test whether maindir/sciencedir exists at all
#
# 19.10.2007:
# The special cleanTmpFiles function (arguments)
# for this script required
# a rearrangement of the trap function handling program
# interruption.
#
# 02.10.2008:
# The saturation level keyword (SATLEVEL) is adapted
# if necessary.
#
# 12.10.2008:
# I make sure that the SATLEVEL keyword is always a float with two
# digits
#
# 26.01.2012:
# If the original image has a header keyword 'BACKGR' giving the
# skybackground value, this value is stored to a new key-value 'BACKORIG'
# and the new skybackground is given with 0.0.
#
# 08.02.2012:
# - statistics now only consider pixels between -60000 and 60000. We set
#   object pixels to -70000 and hence this couldm influence statistics if
#   large parts of an image are influenced by bright stars etc.
# - The 'correction' for small biases in the SExtractor abckground subtraction
#   is only applied if the correction is small (smaller than 1 ADU). The
#   correction is determined with global statistics which might be influenced
#   too much by large features. Hence, the 'local estimation' by SExtractor
#   with a possible small global bias is better in this case.
#
# 09.01.2013:
# I made the script more robust to non-existing files
#
# 06.02.2013:
# I added a TWOPASS_X_COLLAPSE mode to cosmetically correct remaining
# 'ramps' in the lower/upper parts of KIDS@OMEGACAM images.
#
# 05.03.2013:
# I modified deletion of temporary files so that error messages
# are avoided when no files are to be deleted.

# File inclusions:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# define various variables because of the '-u' script flag
# (the use of undefined variables will be treated as errors!)
# several variables might be used uninitialised
# in the cleanTmpFiles function.
THELI_DEBUG=${THELI_DEBUG:-""}
CHIP=${CHIP:-""}
BASE=${BASE:-""}
i=1
while [ ${i} -le ${NCHIPS} ]
do
  RESULTDIR[${i}]=${RESULTDIR[${i}]:-""}
  i=$(( $i + 1 ))
done

##
## function definitions:
##
function printUsage
{
  echo "SCRIPT NAME:"
  echo "    create_skysub_para.sh"
  echo ""
  echo "    RUN/SET processing script"
  echo ""
  echo "ARGUMENTS:"
  echo "    1. main_dir."
  echo "    2. set_dir."
  echo "    3. image_extension"
  echo "    4. extension_addition"
  echo "    5. ONEPASS or TWOPASS (TWOPASS_X_COLLAPSE)"
  echo "   (6. chips to be processed)"
  echo ""
  echo "DESCRIPTION: "
  echo "    The script subtracts the sky background from astronomical"
  echo "    images obtained in optical wavelengths.  It supports two/three"
  echo "    different modes, a one-pass and a two-pass sky subtraction"
  echo "    (5th command line argument):"
  echo ""
  echo "    - ONEPASS:"
  echo "      We subtract the sky background with the standard SExtractor"
  echo "      procedure. The relevant SExtractor parameters in this case "
  echo "      are:"
  echo ""
  echo "      BACK_SIZE 128, BACK_FILTERSIZE 3"
  echo ""
  echo "    - TWOPASS:"
  echo "      In the two-pass case we first detect objects with SExtractor,"
  echo "      relpace pixels occupied by objects with the 'mode' of the image"
  echo "      and run SExtractor skybackground subtractionon the new, masked"
  echo "      image. The estimated background is then subtracted from the"
  echo "      original image."
  echo "      The relevant SExtractor parameters for this procedure are:"
  echo " "
  echo "      - First object detection:"
  echo "        DETECT_MINAREA 50, DETECT_THRESH   1.3 "
  echo ""
  echo "        The large detection minarea together with a low threshold"
  echo "        ensures that also halos around bright stars are masked."
  echo ""
  echo "      - Sky subtraction of the masked image:"
  echo "        BACK_SIZE 90, BACK_FILTERSIZE 5"
  echo "    "
  echo "    - TWOPASS_X_COLLAPSE:"
  echo "      This mode is very special to KIDS@OMEGACAM data and probably"
  echo "      only temporary. It has the same functionality as the TWOPASS"
  echo "      mode.  In addition, after the TWOPASS processing, a clipped mean"
  echo "      from all lines of the image is estimated and the mean is"
  echo "      subtracted from the corresponding line. For KIDS@OMEGACAM data"
  echo "      a 'ramp' is still visible in the lower/upper parts in several"
  echo "      chips after all preprocessing. This ramp is corrected for"
  echo "      'cosmetics' with the TWOPASS_X_COLLAPSE mode. However it should"
  echo "      be corrected multiplicatively. "
  echo ""
  echo "    The TWOPASS mode avoids that the background around bright objects "
  echo "    is biased too high and hence is preferable to the ONEPASS mode."
  echo "    However, check your images creafully whether it can be applied:"
  echo ""
  echo "    THE INITIAL IMAGE MUST BE REASONABLY FLAT FOR THE TWOPASS"
  echo "    MODE."
  echo ""
  echo "    If it shows large scale gradients or fringe residuals it will give"
  echo "    very wrong results and spoil photometric measurements later on."
  echo "    Image imperfections might errorneously be identified as objects"
  echo "    and wrongly be masked in the first pass or the calculated mode for"
  echo "    the whole image is not a good estimate for the whole image which"
  echo "    leads to a wrong background subtraction in the second pass.  "
  echo ""
  echo "    If your images suffer from a non-flat background or other "
  echo "    high-frequency residuals (such as fringes) use the ONEPASS "
  echo "    sky-subtraction."
  echo ""
  echo "EXAMPLE:"
  echo "    parallel_manager.sh create_skysub_new_para.sh \\"
  echo "                        /aibn85_2/terben/TEST/ \\"
  echo "                        set_1 OFCSF .sub ONEPASS"
  echo ""
  echo "    Subtract the skybackground in one pass from all images"
  echo "    in the directory /aibn85_2/terben/TEST/set_1 with"
  echo "    the ending 'OFCSF'. The ending of the result images"
  echo "    will be 'OFCSF.sub' ."
  echo ""
  echo "    (e. g. WFI.2000-12-27T07:57:29.376_1OFCSF.fits ->"
  echo "           WFI.2000-12-27T07:57:29.376_1OFCSF.sub.fits)"
  echo ""
  echo "AUTHOR:"
  echo "    Thomas Erben (terben@astro.uni-bonn.de)"
}

function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"

    for CHIP in ${!#}
    do
      test -f ${TEMPDIR}/skysub_images_${CHIP}_$$ && \
           rm ${TEMPDIR}/skysub_images_${CHIP}_$$

      ${P_FIND} ${RESULTDIR[${CHIP}]} -maxdepth 1\
                -name \*_${CHIP}$1_backsub\*.fits | xargs -r rm
      ${P_FIND} ${RESULTDIR[${CHIP}]} -maxdepth 1\
                -name \*_${CHIP}$1_noobj\*.fits | xargs -r rm
      ${P_FIND} ${RESULTDIR[${CHIP}]} -maxdepth 1\
                -name \*_${CHIP}$1_skyback.fits | xargs -r rm
    done
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

##
## initial sanity checks
##

# check validity of command line arguments:
if [ $# -ne 6 ]; then
  theli_error "Wrong number of command line arguments!" "${!#}"
  exit 1
fi

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles $3 \"${!#}\"; exit 1" INT


# Existence of image directory:
test -d /$1/$2 || { theli_error \
                    "Can't find directory /$1/$2" "${!#}"; exit 1; }

##
## The main script starts here:
##
for CHIP in ${!#}
do
  ${P_FIND} /$1/$2/ -name \*_${CHIP}$3.fits > \
            ${TEMPDIR}/skysub_images_${CHIP}_$$

  if [ -s ${TEMPDIR}/skysub_images_${CHIP}_$$ ]; then
    # determine where the FITS files (result and intermediary)
    # finally go (THELI link structure)
    FILE=`${P_GAWK} '(NR==1) {print $0}' ${TEMPDIR}/skysub_images_${CHIP}_$$`

    if [ -L ${FILE} ]; then
      LINK=`${P_READLINK} ${FILE}`
      RESULTDIR[${CHIP}]=`dirname ${LINK}`
    else
      RESULTDIR[${CHIP}]="/$1/$2"
    fi

    # do the real job here:
    while read FILE
    do
      BASE=`basename ${FILE} .fits`

      if [ "$5" = "ONEPASS" ]
      then
        # In the one-pass case we 'just' subtract the background
        # with the standard SExtractor procedure:
        ${P_SEX} ${FILE} \
                 -c ${DATACONF}/skysub.conf.sex\
                 -BACK_SIZE 128 -BACK_FILTERSIZE 3\
                 -CHECKIMAGE_TYPE -BACKGROUND\
                 -CHECKIMAGE_NAME ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits"

      elif [ "$5" = "TWOPASS" ] || [ "$5" = "TWOPASS_X_COLLAPSE" ]
      then
        # In the two-pass case we first detect objects, relpace pixels
        # occupied by objects with the 'mode' of the image and run
        # SExtractor on the new, masked image. Note that the initial
        # image needs to be reasonably flat for this to work properly.

        # First SExtractor to remove objects:
        ${P_SEX} ${FILE} -c ${DATACONF}/image-objects.sex\
                 -CHECKIMAGE_NAME ${RESULTDIR[${CHIP}]}/${BASE}"_noobj.fits"

        # Now the mode of the original image. To obtain a good
        # estimate we use the original image with the objects removed
        # already:
        MODE=`${P_IC} '%1 -70000 %2 fabs 1.0e-06 > ?' \
                      ${FILE} \
                      ${RESULTDIR[${CHIP}]}/${BASE}"_noobj.fits" | \
                      ${P_STATS} -t -60000 60000 |\
                      ${P_GAWK} '($1=="mode") {print $3}'`

        # substitute object pixels in the original image with the mode
        # and run SExtractor background subtraction on that image.
        # Afterwards this background needs to be subtrackted from the
        # original image:
        ${P_IC} '%1 '${MODE}' %2 fabs 1.0e-06 > ?' \
                ${FILE} \
                ${RESULTDIR[${CHIP}]}/${BASE}"_noobj.fits" > \
                ${RESULTDIR[${CHIP}]}/${BASE}"_noobj_mode.fits"

        ${P_SEX} -c ${DATACONF}/skysub.conf.sex \
              ${RESULTDIR[${CHIP}]}/${BASE}"_noobj_mode.fits" \
              -CHECKIMAGE_NAME ${RESULTDIR[${CHIP}]}/${BASE}"_skyback.fits" \
              -BACK_SIZE 90

        ${P_IC} '%1 %2 -' ${FILE} \
                          ${RESULTDIR[${CHIP}]}/${BASE}"_skyback.fits" >\
                          ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits"

        if [ "$5" = "TWOPASS_X_COLLAPSE" ]; then
          ${P_IC} '%1 -70000 %2 fabs 1.0e-06 > ?' \
              ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits" \
              ${RESULTDIR[${CHIP}]}/${BASE}"_noobj.fits" > \
              ${RESULTDIR[${CHIP}]}/${BASE}"_noobj_coll.fits"

          ${P_FITSCOLLAPSE} -c x \
              -i ${RESULTDIR[${CHIP}]}/${BASE}"_noobj_coll.fits" \
              -o ${RESULTDIR[${CHIP}]}/${BASE}"_noobj_coll_res.fits"

          ${P_IC} '%1 %2 -' ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits" \
              ${RESULTDIR[${CHIP}]}/${BASE}"_noobj_coll_res.fits" > \
              ${RESULTDIR[${CHIP}]}/${BASE}"_backsub_coll.fits"

          mv ${RESULTDIR[${CHIP}]}/${BASE}"_backsub_coll.fits" \
             ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits"
        fi
      else
        echo "Unknown background subtraction mode: $5; Exiting !!"
        exit 1
      fi
      # We again estimate the mode of the 'skysubtracted' images and
      # subtract it. SExtractor seems to be biased low by a small
      # margin. We only do this additional 'global correction' if it
      # is small (less than 1 ADU).  Bright stars and other 'large
      # features' might influence global statistics too much:
      MODE=`${P_STATS} -t -60000 60000 < \
              ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits" |\
            ${P_GAWK} '($1=="mode") {print $3}'`

      DOCORR=`${P_GAWK} 'BEGIN {if ('${MODE}' < 1.0 && '${MODE}' > -1.0) {
                                  print 1;
                                } else {
                                  print 0;
                                }}'`

      if [ ${DOCORR} -eq 1 ]; then
        ${P_IC} '%1 '${MODE}' -' \
           ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits" > \
           ${RESULTDIR[${CHIP}]}/${BASE}"$4.fits"
      else
        mv ${RESULTDIR[${CHIP}]}/${BASE}"_backsub.fits" \
           ${RESULTDIR[${CHIP}]}/${BASE}"$4.fits"
      fi

      # get saturation level of image (if present);
      # it will be modified by sky-background subtraction:
      SATLEVEL=`${P_DFITS} ${FILE} | ${P_FITSORT} -d SATLEVEL |\
                ${P_GAWK} '{print $2}'`

      # update saturation level if necessary:
      if [ "${SATLEVEL}" != "KEY_N/A" ]; then
        ORIGMODE=`${P_STATS} < ${FILE} | ${P_GAWK} '($1=="mode") {print $3}'`
        NEWSATLEVEL=`${P_GAWK} 'BEGIN {
                 printf("%.2f", '${SATLEVEL}'-('${ORIGMODE}')-('${MODE}'))
                                }'`
        value ${NEWSATLEVEL}
        writekey ${RESULTDIR[${CHIP}]}/${BASE}"$4.fits" SATLEVEL \
                 "${VALUE} / Saturation Level" REPLACE
      fi

      # if a header key for the skybackground is present, store its
      # value to header key 'BACKORIG' and set the current skybackground
      # header value to 0.0:
      BACKGR=`${P_DFITS} ${FILE} | ${P_FITSORT} -d BACKGR |\
              ${P_GAWK} '{print $2}'`

      if [ "${BACKGR}" != "KEY_N/A" ]; then
        value "${BACKGR}"
        writekey ${RESULTDIR[${CHIP}]}/${BASE}"$4.fits" BACKORIG \
                 "${VALUE} / original sky background value (ADU)" REPLACE
        value "0.0"
        writekey ${RESULTDIR[${CHIP}]}/${BASE}"$4.fits" BACKGR \
                 "${VALUE} / sky background value (ADU)" REPLACE
      fi

      # and finally a link if necessary:
      if [ "${RESULTDIR[${CHIP}]}" != "/$1/$2" ]; then
        ln -s ${RESULTDIR[${CHIP}]}/${BASE}"$4.fits" /$1/$2/${BASE}"$4.fits"
      fi

    done < ${TEMPDIR}/skysub_images_${CHIP}_$$
  else
    theli_warn "No images to process for Chip ${CHIP}"
  fi
done

# clean up and bye:
cleanTmpFiles $3 "${!#}"
theli_end "${!#}"
exit 0;
