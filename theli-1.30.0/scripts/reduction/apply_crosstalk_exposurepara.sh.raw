#!/bin/bash

# created by Axel Buddendiek, Argelander Institut fuer Astronomie
# Auf dem Huegel 71, Bonn
# abuddend@astro.uni-bonn.de

# HISTORY COMMENTS
# ================
#
# 26.03.2013:
# - first version from Axel Buddendiek
# - Integration into the THELI data flow (TE)
#
# 04.03.2013:
# I corrected a bug in the construction of an image file name.
#
# 14.08.2013:
# Axel Buddendiek corrected some bugs in his crosstalk procedures.
#
# 23.09.2013:
# I added a test whether an image to be corrected is present at all;
# if not it could happen that files of zero byte length ended up in
# the image directories (images can disappear of they are marked
# as bad/unusable lateron).
#
# 30.01.2014:
# parallel script version and code cleanup
#
# 20.02.2018:
# The deletion of temporary files was not done correctly (assuming implicitely
# that TEMPDIR is the pwd) - corrected.

# This script reads in image names and crosstalk coefficients
# and applies them.

# Arguments are in the following order:
#
#$1: basedir
#$2: science dir.
#$3: crosstalk coefficients (creeated with the script
#    create_crosstalk_coefficients.sh)
#$4: output directory for result images
#$5: suffix of crosstalk corrected images

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

BASEDIR=$1
SCIENCEDIR=$2
MASTERFILE=$3
ENDDIR=$4
SUFFIX=$5

MD=${BASEDIR}/${SCIENCEDIR}

# create target directory if not yet present:
test -d ${ENDDIR} || mkdir -p ${ENDDIR}

# first we need to know how many chips are involved
${P_GAWK} '($0 !~ /#/) {print $1}' ct_default_${INSTRUMENT}.txt |\
  sort -n | uniq -c > ${TEMPDIR}/num_robbed_per_chip_$$.txt

# now we save the number of chips with which every chip interacts in a
# variable
while read NUM CHIP
do
  eval NUMBER_${!CHIP}=${NUM}
done < ${TEMPDIR}/num_robbed_per_chip_$$.txt

# the correction for crosstalk can be done in parallel on the robbed images:

# remove comment lines from the masterfile if any:
${P_GAWK} '($0 !~ /#/)' ${MASTERFILE} > tmp.txt_$$
NFILES=`wc -l tmp.txt_$$ | cut -f1 -d" "`

SPLITLINES=`expr ${NFILES} / ${NPARA}`
if [ ${SPLITLINES} -eq 0 ]; then
  SPLITLINES=1
fi

split --lines=${SPLITLINES} tmp.txt_$$ \
    "${TEMPDIR}/tmp_$$_"

PARACHAN=1
for FILE in ${TEMPDIR}/tmp_$$_*
do
  {
    # create a unique process identifier for parallel channels:
    PROCID=$$_${PARACHAN}

    # reading in names and chips in order to correct; in the following
    # we do not need the contents of the 'REST' variable.
    while read NAME ROBBED REST
    do
      # manipulating name strings for later use
      BASE=`basename ${NAME} _${ROBBED}.fits`

      # check whether images to be corrected are present at all;
      # It might happen that we created crosstalk coefficients for
      # some files that were deleted lateron:
      if [ -s ${MD}/${NAME} ]; then
        # run SE the first time in order to get a model for the bg
        # We need to store the background itself because we need to add
        # it again to the image at the very end:

        # The STR variable is only there to keep line lengths below 80
        # columns in the following call:
        STR="${TEMPDIR}/${BASE}_${ROBBED}_sub_${PROCID}.fits,"
        STR="${STR}${TEMPDIR}/${BASE}_${ROBBED}_bg_${PROCID}.fits"
        ${P_SEX} ${MD}/${NAME} \
          -CHECKIMAGE_TYPE -BACKGROUND,BACKGROUND \
          -CHECKIMAGE_NAME ${STR} \
          -c ${CONF}/crosstalk.sex \
          -PARAMETERS_NAME ${CONF}/crosstalk.param \
          -FILTER_NAME ${CONF}/crosstalk.conv \
          -CATALOG_NAME ${TEMPDIR}/test_${PROCID}.cat

        # number of thief chips:
        AMOUNTTHIEF=$NUMBER_${!ROBBED}

        # for loop to read in chips and coefficients and to correct
        # images
        for (( C=1; C<=${AMOUNTTHIEF}; C++ ))
        do
          # The THIEFARRAY below has the first index at 0; therefore
          # the subtraction of 1 in the follwing command:
          COLUMN=$((C * 3 - 1))

          # read in thief chip correction data:
          read -a THIEFARRAY <<< "`grep ${NAME} ${MASTERFILE}`"
          THIEF=${THIEFARRAY[((COLUMN++))]}
          SIGN=${THIEFARRAY[((COLUMN++))]}
          COEFF=${THIEFARRAY[COLUMN]}

          # run SE in order to get a model for the bg of the thief chip
          ${P_SEX} ${MD}/${BASE}_${THIEF}.fits \
            -CHECKIMAGE_TYPE -BACKGROUND \
            -CHECKIMAGE_NAME ${TEMPDIR}/${BASE}_${THIEF}_sub_${PROCID}.fits \
            -c ${CONF}/crosstalk.sex \
            -PARAMETERS_NAME ${CONF}/crosstalk.param \
            -FILTER_NAME ${CONF}/crosstalk.conv \
            -CATALOG_NAME ${TEMPDIR}/test_${PROCID}.cat

          # correct image
          ${P_IC} -p -32 "%1 %2 ${COEFF} * ${SIGN}" \
            ${TEMPDIR}/${BASE}_${ROBBED}_sub_${PROCID}.fits \
            ${TEMPDIR}/${BASE}_${THIEF}_sub_${PROCID}.fits > \
            ${TEMPDIR}/pre_corrected_image_${PROCID}.fits

          #move corrected image in order to use it again with the next chip
          mv ${TEMPDIR}/pre_corrected_image_${PROCID}.fits \
             ${TEMPDIR}/${BASE}_${ROBBED}_sub_${PROCID}.fits
        done

        # Add background again
        ${P_IC} -p -32 '%1 %2 +' \
          ${TEMPDIR}/${BASE}_${ROBBED}_sub_${PROCID}.fits \
          ${TEMPDIR}/${BASE}_${ROBBED}_bg_${PROCID}.fits > \
          ${ENDDIR}/${BASE}_${ROBBED}${SUFFIX}.fits

        rm ${TEMPDIR}/${BASE}_${ROBBED}_sub_${PROCID}.fits
        rm ${TEMPDIR}/${BASE}_${ROBBED}_bg_${PROCID}.fits
      else
        theli_warn "Image ${MD}/${NAME} not present!"
      fi
    done < ${FILE}
  } 2>&1 &
  ((PARACHAN++)) # short for PARACHAN=$(( ${PARACHAN} + 1 ))
done

# wait until all parallel channels have finished.
wait;

# clean up and bye:
for FILE in ${TEMPDIR}/*_$$*
do
  test -f ${FILE} && rm ${FILE}
done

theli_end
exit 0;
