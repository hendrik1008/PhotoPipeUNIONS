#!/bin/bash -u

# the script processes a set of Science frames the images are overscan
# corrected, debiased, flatfielded and stacked with mode rescaling
# (Superflat). We assume that we do NOT work with a normalised flat
# The script uses the 'preprocess' program to perform the reduction.
# It assumes that all images from ONE chip reside in the same physical
# directory
#

# HISTORY COMMENTS:
#
# 31.07.2016:
# The former 'process_science_eclipse_para.sh' script was split in two
# parts: (1). A new 'process_science_eclipse_para.sh' script which does now
# all the steps up to the initial flat-fielding with dome- or sky-flats.
# (2). A 'process_superflat_eclipse_para.sh' which creates the superflat
# image. The splitting was primarily done to allow special initial
# science processinf ig necessary, i.e. to write specialised
# 'process_science' scripts. This is for instance necessary for DECam.
#
# 02.08.2016:
# - superflat exclusion files are now located in ${MD}/${SD}/superflat_files
#   instead of in ${MD}.
# - The script now stops with an error if the wrong number of command line
#   arguments is given.
# - P_PYTHON -> P_PYTHON3 as THELI scripts are in python3 now.
#
# 12.09.2016:
# renaming of files: superflat_exclusion -> superfl_excl etc. The new names
# are still self-explanatory enough and the filenames get shorter.
#
# 20.09.2016:
# Major bug fix: A presence of a string as 'dec157560_4' also excluded files
# as 'dec157560_4X' (X arbitrary) from superflat creation - fixed!
#
# 25.09.2016:
# DECam has very low-level fringes so that we can use here the standard
# SExtractor parameters for detecting objects (those used for filters showing
# no fringes). I included this special case in this script. If this should
# occur for more instruments we need a better configuration setup for it.
#
# 17.05.2018:
# Bug fix: problematic single frames were not rejected correctly in the
# superflat creation step.

#$1: main directory (filter)
#$2: Science directory
#$3: image ending
#$4: FRINGE/NOFRINGE
#$5: SUPERTEST/NOSUPERTEST
#$6: chips to be processed

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

THELI_CLEAN=${THELI_CLEAN:-""}

if [ $# -ne 6 ]; then
  theli_error "USAGE: $0 MS SD ENDING FRINGEFLAG SUPTERTESTFLAG CHIPS"
  exit 1;
fi

# give meaningful names to command line arguments:
MD=$1
SD=$2
ENDING=$3
FRINGEFLAG=$4
SUPERTESTFLAG=$5

# do not create superflat images if they are present already.  We
# assume that superflat files exist for all chips if they exist for
# some:
DOSUPER=1
for CHIP in ${!#}
do
  if [ -s ${MD}/${SD}/${SD}_${CHIP}.fits ]; then
    theli_warn "Superflat ${MD}/${SD}/${SD}_${CHIP}.fits already exists!"
    DOSUPER=0
  fi
done

if [ ${DOSUPER} -eq 1 ]; then
  # we create object-subtracted images to avoid dark regions around
  # bright objects in later images (superflats)
  for CHIP in ${!#}
  do
    ${P_FIND} ${MD}/${SD} -maxdepth 1 -name \*_${CHIP}${ENDING}.fits > \
        ${TEMPDIR}/images-objects_$$

    RESULTDIR[${CHIP}]=${MD}/${SD}
    while read FILE
    do
      if [ -L ${FILE} ]; then
        LINK=$(${P_READLINK} ${FILE})
        RESULTDIR[${CHIP}]=$(dirname ${LINK})
      fi

      BASE=$(basename ${FILE} .fits)
      #
      # now run sextractor to subtract objects from the image
      #
      # if the SCIENCE images contain strong fringes ($6=FRINGE and
      # specific instruments) we have to use special detection/pixel
      # threshholds to keep the fringes.
      if [ "${FRINGEFLAG}" = "FRINGE" ] && \
         [ "${INSTRUMENT}" != "DECam" ]; then
        ${P_SEX} ${FILE} -c ${DATACONF}/image-objects.sex\
          -CHECKIMAGE_NAME ${RESULTDIR[${CHIP}]}/${BASE}"_sub.fits"\
                -DETECT_MINAREA 7 -DETECT_THRESH 5 -ANALYSIS_THRESH 5
      else
        ${P_SEX} ${FILE} -c ${DATACONF}/image-objects.sex\
          -CHECKIMAGE_NAME ${RESULTDIR[${CHIP}]}/${BASE}"_sub.fits"
      fi
      ${P_IC} '%1 -70000 %2 fabs 1.0e-06 > ?' ${FILE} \
          ${RESULTDIR[${CHIP}]}/${BASE}"_sub.fits" \
        > ${RESULTDIR[${CHIP}]}/${BASE}"_sub1.fits"

      mv ${RESULTDIR[${CHIP}]}/${BASE}"_sub1.fits" \
         ${RESULTDIR[${CHIP}]}/${BASE}"_sub.fits"

      if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${SD}" ]; then
        ln -s ${RESULTDIR[${CHIP}]}/${BASE}"_sub.fits" \
              ${MD}/${SD}/${BASE}"_sub.fits"
      fi
    done < ${TEMPDIR}/images-objects_$$
  done

  # see whether we need to exclude images (individual chips) from the
  # superflat creation process. We do not need to check for images to
  # be excluded if we already did this at some point. In this case
  # there should be a file 'superflat_exclusion' listing, on each
  # line, a chip to be excluded from the superflat. The lines in the
  # file should read as 'bla_CHIP' (bla is the basename of the image
  # and CHIP the chip number)
  if [ -f ${MD}/${SD}/superflat_files/super_excl ]; then
    cp ${MD}/${SD}/superflat_files/super_excl \
       ${MD}/${SD}/superflat_files/super_excl_$$
  else
    if [ "${SUPERTESTFLAG}" = "SUPERTEST" ]; then
      test -f ${TEMPDIR}/superflattest.txt_$$ && \
           rm ${TEMPDIR}/superflattest.txt_$$

      for CHIP in ${!#}
      do
        ${P_FIND} ${MD}/${SD}/ -maxdepth 1 \
            -name \*_${CHIP}${ENDING}_sub.fits >> \
            ${TEMPDIR}/superflattest.txt_$$
      done

      if [ -s ${TEMPDIR}/superflattest.txt_$$ ]; then
        ${P_PYTHON3} ${S_SUPERFLATEXCLUSION} \
            ${TEMPDIR}/superflattest.txt_$$ -50 0.08 1 > \
            ${TEMPDIR}/superflatexclude.txt_$$

        if [ -s ${TEMPDIR}/superflatexclude.txt_$$ ]; then
          test -d ${MD}/${SD}/superflat_files || \
            mkdir ${MD}/${SD}/superflat_files

          ${P_GAWK} -F/ '{print $NF}' ${TEMPDIR}/superflatexclude.txt_$$ |\
            sed -e 's/'${ENDING}'_sub.fits//' > \
            ${MD}/${SD}/superflat_files/super_excl_$$
        fi
      fi
    fi
  fi

  # create superflat
  for CHIP in ${!#}
  do
    if [ "${RESULTDIR[${CHIP}]}" != "${MD}/${SD}" ]; then
      ln -s ${RESULTDIR[${CHIP}]}/${SD}_${CHIP}.fits \
            ${MD}/${SD}/${SD}_${CHIP}.fits
    fi

    # do statistics from all science frames.  This is necessary to get
    # the mode of the combination from all images, so that the
    # combination where images have been excluded can be scaled
    # accordingly. At this stage only reject images where all the
    # chips need to be removed from the superflat process.
    ${P_FIND} ${MD}/${SD}/ -maxdepth 1 -name \*_${CHIP}${ENDING}_sub.fits > \
        ${TEMPDIR}/superflat_candidates_$$

    if [ -s ${TEMPDIR}/superflat_candidates_$$ ]; then
      if [ -f ${MD}/${SD}/superflat_files/super_compl_excl ]; then
        grep -vf ${MD}/${SD}/superflat_files/super_compl_excl  \
                 ${TEMPDIR}/superflat_candidates_$$ > \
                 ${TEMPDIR}/superflat_images_$$
      else
        mv ${TEMPDIR}/superflat_candidates_$$ ${TEMPDIR}/superflat_images_$$
      fi

      ${P_IMSTATS} $(cat ${TEMPDIR}/superflat_images_$$)\
                   -o ${TEMPDIR}/science_images_$$

      RESULTMODE=`${P_GAWK} 'BEGIN {mean = 0.0; n = 0} ($1!="#") {
                             n = n + 1;
                             mean = mean + $2}
                             END {print mean / n}' \
                             ${TEMPDIR}/science_images_$$`

      # modify the input list of images in case we have to reject files
      # for the superflat:
      if [ -s ${MD}/${SD}/superflat_files/super_excl_$$ ]; then
        # Note the '-w' option in the grep-call below.
        # It ensures 'exact matches'. Hence, the
        # filename 'dec157560_4O' would NOT be matched by 'dec157560_4'
        # as it should be!
        # By removing comments in the following the result file is empty
        # if the superflat exclusion should reject 'all' images:
        ${P_GAWK} '{print $0 "'${ENDING}'_sub.fits"}' \
          ${MD}/${SD}/superflat_files/super_excl_$$ > \
          ${TEMPDIR}/super_excl_tmp_$$
        grep -wvf ${TEMPDIR}/super_excl_tmp_$$ ${TEMPDIR}/science_images_$$ | \
          grep -v '#' > ${TEMPDIR}/science_coadd_images_$$
      else
        cp ${TEMPDIR}/science_images_$$ ${TEMPDIR}/science_coadd_images_$$
      fi

      # do the combination: first test whether a possible superflat
      # exclusion removed all the images. In this case just create a
      # constant value image with the mode '1' and mark the superflat as
      # bad:
      if [ -s ${TEMPDIR}/science_images_$$ ] && \
         [ ! -s ${TEMPDIR}/science_coadd_images_$$ ]; then
        ${P_IC} -c ${SIZEX[${CHIP}]} ${SIZEY[${CHIP}]} \
                -p -32 '1' > ${RESULTDIR[${CHIP}]}/${SD}_${CHIP}.fits
        ${P_REPLACEKEY} ${RESULTDIR[${CHIP}]}/${SD}_${CHIP}.fits \
              "BADCCD  =                    1 / Is_CCD_Bad_(1=Yes)" HISTORY
      else
        ${P_IMCOMBFLAT_IMCAT} \
              -i ${TEMPDIR}/science_coadd_images_$$ \
              -o ${RESULTDIR[${CHIP}]}/${SD}_${CHIP}.fits \
              -s 1 -e 0 1 -f 1.0 -m ${RESULTMODE} -k BADCCD
      fi

      if [ -z ${THELI_CLEAN} ]; then
        test -d ${MD}/${SD}/SUB_IMAGES || mkdir ${MD}/${SD}/SUB_IMAGES
        mv ${MD}/${SD}/*_${CHIP}${ENDING}_sub.fits ${MD}/${SD}/SUB_IMAGES/
      else
        rm ${MD}/${SD}/*_${CHIP}${ENDING}_sub.fits
      fi
    fi # if [ -s ${TEMPDIR}/superflat_candidates_$$ ]
  done
fi # if [ ${DOSUPER} ] ....

# clean up and bye
for FILE in ${TEMPDIR}/*_$$
do
  test -f ${FILE} && rm ${FILE}
done

theli_end "${!#}"
exit 0;
