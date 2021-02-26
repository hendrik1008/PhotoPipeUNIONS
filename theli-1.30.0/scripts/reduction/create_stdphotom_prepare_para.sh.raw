#!/bin/bash -u

# First step to create catalogues suitable for absolute photometric
# calibration with 'create_abs_photo_info.sh'. The script takes
# astrometric information from a scamp processing and calculates
# 'correct' sky coordinates from objects catalogues (extracted from
# standard star fields). Afterwards the standard star object
# catalogues are merged with a reference standard star sources.
# (e.g. from Landolt, Stetson, Sloan ....)
# This last step is to be performed with the script
# 'create_stadphotom_merge.sh'

# SCRIPT HISTORY:
#
# 23.11.2012:
# The merging of extarcted source lists and the standard star catalogue
# needed an update because names of magnitudes are different for
# different filter systems. They are now read dynamically from the
# standardstar catalogue and a config file for 'make_ssc' is created on
# the fly.
#
# 07.01.2013:
# I made the script more robust to non-existing files.
#
# 01.06.2013:
# Bug fix: objects that were associated with the standardstarcatalogue
# but who had associations with more than one object entered the processing.
# This caused huge problems in using long-exposed KIDS fields for photometric
# calibration with SDSS. Up to now this probably never showed up when we
# calibrated with short exposed standard star fields who only had 'isolated'
# objects listed as standard sources.
#
# 04.06.2013:
# I completely rewrote this script by splitting it. The task of estimating
# correct astrometry for standard star observations and merging them
# with a standard star catalogue is now donw in two separate scripts.
# The merging is no longer performed chip-by-chip but on an exposure level.
# This dramatically reduces the number of necessary associations. The
# associations became a very long process when we used also KiDS science
# data (overlapping with SDSS) in the calibration process.
#
# 15.12.2016:
# - I speeded up the script by collecting the estimation of
#   'correct astrometric' coordinates to a single call of the
#   Python interpreter.
# - If the THELI_CLEAN variable is set, intermediate catalogues, which
#   are not needed for further processing, are deleted at the end of the
#   script.
#
# 17.07.2017:
# Refinement of initial catalogue selection: We only consider catalogues
# directly in the 'cat' directory but not in subdiretories of it. We now
# can have a BAD_PHOTOM subdiretory and hence the change was necessary.

#$1: main directory
#$2: science dir. (the catalogues are assumed to be in $1/$2/cat)
#$3: image extension
#$4: astrometry standardstar catalogue used for the scamp calibration
#    (the script needs the scamp headers which are assumed to be in
#    $1/$2/headers_scamp_$4)
#$5: chips to work on

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# define THELI_CLEAN because of the '-u' script flag (the use of
# undefined variables would be treated as errors!)
#
THELI_CLEAN=${THELI_CLEAN:-""}

# check number of command line arguments:
if [ $# -ne 5 ]; then
  theli_error "Number of command line argument not correct!" "${!#}"
  exit 1;
fi

# give meaningful names to command line arguments:
MD=$1
SD=$2
EXTENSION=$3
STANDARDCAT=$4

# start script task:
for CHIP in ${!#}
do
  CATS=$(${P_FIND} ${MD}/${SD}/cat -maxdepth 1 \
           -name \*_${CHIP}${EXTENSION}.cat)

  if [ "${CATS}" != "" ]; then
    # first estimate correct astrometric coordinates for all the
    # catalogues. We collect this into a single call of the Python interpreter.
    # With the short running time per catalogue, the many Python-calls
    # became the time determininjg factor of the whole script!
    test -f ${TEMPDIR}/cats.txt_$$ && rm ${TEMPDIR}/cats.txt_$$
    for CAT in ${CATS}
    do
      BASE=$(basename ${CAT} .cat)
      HEADBASE=$(basename ${CAT} ${EXTENSION}.cat)
      DIR=$(dirname ${CAT})
      HEADDIR=${DIR%/cat}/headers_scamp_${STANDARDCAT}

      echo "${CAT} ${HEADDIR}/${HEADBASE}.head ${DIR}/${BASE}_astrom.cat" >> \
        ${TEMPDIR}/cats.txt_$$
    done
    ${S_APLASTROMSCAMP} \
        -f ${TEMPDIR}/cats.txt_$$ --processes=1 \
        -t LDAC_OBJECTS -p X_IMAGE Y_IMAGE -r Ra Dec

    for CAT in ${CATS}
    do
      BASE=$(basename ${CAT} .cat)
      ${P_LDACCONV} -i ${MD}/${SD}/cat/${BASE}_astrom.cat \
                    -o ${MD}/${SD}/cat/${BASE}_conv.cat \
                    -b ${CHIP} -c ${INSTRUMENT} -f dummy

      # transfer some keys that are needed in follow-up scripts
      # from the FIELDS to the OBJECTS table:
      ${P_MAKEJOIN} -i ${MD}/${SD}/cat/${BASE}_conv.cat \
                    -o ${TEMPDIR}/tmp.cat0_$$ \
    		    -c ${DATACONF}/stdphotom_prepare_make_join.conf

      # calculate object magnitudes with a magzeropoint of 0 and
      # an exposure time normalisation of 1s (1.08574 is 2.5 / log(10)):
      ${P_LDACCALC} -i ${TEMPDIR}/tmp.cat0_$$ \
                    -o ${TEMPDIR}/tmp.cat00_$$ \
                    -t OBJECTS -c "(-1.08574*log(FLUX_AUTO/EXPTIME));" \
                    -n MAG_AUTO_corr "exposure time corr. MAG_AUTO" -k FLOAT
      ${P_LDACRENTAB} -i ${TEMPDIR}/tmp.cat00_$$\
                      -o ${TEMPDIR}/tmp.cat1_$$ -t OBJECTS STDTAB
      ${P_LDACDELTAB} -i ${TEMPDIR}/tmp.cat1_$$\
                      -o ${TEMPDIR}/tmp.cat11_$$ -t FIELDS
      ${P_LDACRENKEY} -i ${TEMPDIR}/tmp.cat11_$$ \
                      -o ${MD}/${SD}/cat/${BASE}_photprep.cat \
                      -t STDTAB -k A_WORLD A_WCS B_WORLD B_WCS \
                                   THETA_J2000 THETAWCS

    done
  else # if [ "${CATS}" != "" ]
    theli_warn "No catalogues for Chip ${CHIP} available!" "${!#}"
  fi
done

# clean up and bye
# delete intermediate catalogues that are not needed anymore
# if cleaning is activated:
if [ ! -z "${THELI_CLEAN}" ]; then
  ${P_FIND} ${MD}/${SD}/cat -name \*_${CHIP}${EXTENSION}_astrom.cat | \
    xargs rm
  ${P_FIND} ${MD}/${SD}/cat -name \*_${CHIP}${EXTENSION}_conv.cat | \
    xargs rm
fi

for FILE in ${TEMPDIR}/*_$$
do
  test -f ${FILE} && rm ${FILE}
done

theli_end "${!#}"
exit 0;
