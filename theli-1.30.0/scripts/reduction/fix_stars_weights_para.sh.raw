#!/bin/bash -u

# script to 'fix' weight and flag maps around stars in 'very good'
# seeing images. We notices that the EyE cosmic ray detector tends
# to flag centres of stars in very good seeing conditions. This
# script tries to circumvent this. We see which stars would
# 'survive' a KSB PSF fitting process. Weight zero pixels in the
# centres of these stars are 'revalidated'. We choose this criterion
# because we are mostly interested in good stars for PSF fitting
# and lensing studies.

# HISTRORY COMMENTS
# =================
#
# 30.07.2009:
# - I introduced a seeing limit; frames above this seeing
#   are not corrected
# - I introduced an upper limit for the snratio up to which
#   objects are considered for correction. We saw that masks for
#   very big, saturated stars are altered otherwise.
#
# 14.09.2010:
# I introduced a fixed seeing limit of 0.6 to lower
# the S/N threshold for objects to be included in the KSB fitting
# process. We saw for CFHTLS data that for very good seeing images
# objects down to 22 can be affected by faulty cosmic ray masks.
#
# 20.05.2011:
# result files are now treated correctly if the original files
# are links (parallel processing).
#
# 25.07.2011:
# more robust treatment of a possible link structure.
#
# 10.01.2013:
# I introduced the new THELI logging and error reporting scheme into
# the script.
#
# 20.03.2013:
# For stellar selection withTims stellar_locus.pl I now reject objects
# with an rg smaller than 1 pixel. I saw for short exposed images
# that cosmics were confused with stars.
#
# 28.03.2018:
# I refined deletion of temporary files

#$1: main dir.
#$2: science dir.
#$3: image extension (ext) on ..._iext.fits (i is the chip number)
#$4: catalogue directory
#$5: weights directory
#$6: image extension for weight images
#$7: image extension for flag images
#$8: radius around star centers to fix weights/flags
#$9: seeing limit
#$10: chips to work on

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# define THELI_DEBUG because of the '-u' script flag (the use of
# undefined variables will be treated as errors!)  THELI_DEBUG is used
# in the cleanTmpFiles function.
#
THELI_DEBUG=${THELI_DEBUG:-""}
function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"
    for FILE in ${TEMPDIR}/*_$$
    do
      test -f ${FILE} && rm ${FILE}
    done
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles; exit 1" INT

# check number of command line arguments:
if [ $# -ne 10 ]; then
  theli_error "Wrong number of command line arguments! Exiting!" "${!#}"
fi

test -f ${TEMPDIR}/fixcats_$$ && rm ${TEMPDIR}/fixcats_$$

for CHIP in ${!#}
do
  ${P_FIND} /$1/$2/$4/ -maxdepth 1 -name \*_${CHIP}$3_ksb.cat1 >> \
    ${TEMPDIR}/fixcats_$$
done

# do we need to do something at all?
if [ -s ${TEMPDIR}/fixcats_$$ ]; then
  # we will move original weights and flags to an 'orig' directory:
  test -d /$1/$5/orig || mkdir /$1/$5/orig

  while read CAT
  do
    # first check whether we should do something at all
    # because of the seeing limit:
    DOIT=`${P_LDACTOASC} -b -i ${CAT} -t FIELDS -s -k SEXSFWHM |\
          ${P_GAWK} '{ if ($1 < '$9') {print 1;} else {print 0;} }'`

    # we saw that for extremly good seeing images stars with
    # magnitudes down to 22 can be affected by wrong cosmic
    # ray masking. For those images we lower the S/N ratio for
    # stars to be considered in the anisotropy fitting:
    #
    # !!TODO: CHANGE HARDCODING!!
    SNRATIOLIMIT=`${P_LDACTOASC} -b -i ${CAT} -t FIELDS -s -k SEXSFWHM |\
          ${P_GAWK} '{ if ($1 < 0.6) {print 10} else {print 20} }'`

    if [ ${DOIT} -eq 1 ]; then
      # check presence of necessary flags and weights:
      WBASE=`basename ${CAT} $3_ksb.cat1`

      if [ ! -f /$1/$5/${WBASE}$6.fits ] ||\
         [ ! -f /$1/$5/${WBASE}$7.fits ]; then
        echo "/$1/$5/${WBASE}$6.fits and/or /$1/$5/${WBASE}$7.fits missing."
        echo "Exiting!!"
        cleanTmpFiles
        exit 1;
      fi

      # get stellar locus of current catalogue with Tims stellar_locus script:
      # HIGH CUT IN SNRATIO:
      ${P_LDACTOASC} -b -i ${CAT} -t OBJECTS \
                     -k rg snratio |\
        ${P_GAWK} '($1 > 1.0 && $2 > 20.0 && $2 < 4000.0) {print $1}' > \
          ${TEMPDIR}/tmp.asc_$$

      ./stellar_locus.pl ${TEMPDIR}/tmp.asc_$$ 0.35 0.01 0.1 5 0.03 > \
        ${TEMPDIR}/locus.asc_$$

      # Tim says the lower limit for the minimum value should be relaxed:
      RMIN=`${P_GAWK} '{print $1 - 0.2}' ${TEMPDIR}/locus.asc_$$`
      RMAX=`${P_GAWK} '{print $2}' ${TEMPDIR}/locus.asc_$$`

      ${P_ANISOTROPY} -i ${CAT} -o ${TEMPDIR}/aniso.cat_$$ \
                      -t OBJECTS \
                      -c rg ${RMIN} ${RMAX} snratio ${SNRATIOLIMIT} 4000 \
                      -j 12.25 -e 2

      ${P_LDACFILTER} -i ${TEMPDIR}/aniso.cat_$$ \
                      -o ${TEMPDIR}/aniso_filt.cat_$$ \
                      -t OBJECTS -c "(cl=2);"

      ${P_LDACTOASC} -b -i ${TEMPDIR}/aniso_filt.cat_$$ -t OBJECTS \
                     -k Xpos Ypos |\
         ${P_GAWK} '{printf "%.2f %.2f\n", $1, $2}' > ${TEMPDIR}/stars.xy.asc_$$

      # determine directory where real files go and if links are necessary
      # (we assume here that weight and flag files are physically located
      # in the same directory):
      get_real_file /$1/$5/${WBASE}$6.fits
      single_slash_filename $1/$5

      RESULTDIR=${G_FILE}
      if [ "${G_REAL_FILE_DIR}" != "${RESULTDIR}" ]; then
        RESULTDIR=${G_REAL_FILE_DIR}
      fi

      # move original weights:
      mv /$1/$5/${WBASE}$6.fits /$1/$5/orig
      mv /$1/$5/${WBASE}$7.fits /$1/$5/orig

      # now 'fix' the weight and flag:
      ${P_FIXCR_ECL} /$1/$5/orig/${WBASE}$6.fits /$1/$5/orig/${WBASE}$7.fits \
                     $8 ${TEMPDIR}/stars.xy.asc_$$ \
                     ${RESULTDIR}/${WBASE}$6.fits ${RESULTDIR}/${WBASE}$7.fits

      # create links if necessary:
      single_slash_filename $1/$5
      if [ "${RESULTDIR}" != "${G_FILE}" ]; then
        ln -s ${RESULTDIR}/${WBASE}$6.fits $1/$5
        ln -s ${RESULTDIR}/${WBASE}$7.fits $1/$5
      fi
    fi
  done < ${TEMPDIR}/fixcats_$$
else
  theli_warn "No catalogues to process" "${!#}"
fi

# clean up and bye:
cleanTmpFiles
theli_end "${!#}"

exit 0;
