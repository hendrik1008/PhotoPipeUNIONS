#!/bin/bash -u

# script to estimate absolute photometric calibration on a night
# basis. It substitutes the old create_photoinfo.sh script.

# HISTORY COMMENTS
# ================
#
# 03.11.2004 Filter measured magnitudes for nonsense
#
# 25.11.2004:
# we adapted the script to the new output of fitsort
# that no longer gives the file including the whole
# but only the filename.
#
# 07.12.2004:
# I corrected a bug in the writing of ZP and COEFF keyowrds (FITS
# standard issues)
#
# 10.12.2004:
# Now passes the new label argument to phot_abs.py
#
# 26.02.2005:
# I introduced a new argument to account for a different 'filter' name
# and the name for that filter in the standard star catalog.
#
# 01.03.2005:
# I introduced temporary output files for files created by
# photo_abs.py. photo_abs.py uses pgplot that cannot handle too long
# file names.
#
# 11.03.2005:
# We now write all photometric solutions to the header.  In case the
# night was estimated non-photometric the ZPCHOICE keyword 0
#
# 13.03.2005:
# In the header updates the next free dummy keyword is now searched
# automatically. This part is written more modular now.
#
# 17.03.2005:
# If the night is marked as non-photometric, ZPCHOICE is set to 0 now.
#
# 19.03.2005:
# I corrected a syntax error in an if statement (that probably
# was introduced in one of the last commits)
#
# 14.08.2005:
# The call of the UNIX 'sort' program is now done via a variable
# 'P_SORT'.
#
# 05.12.2005
# Chips whose NOTUSE or NOTPROCESS flag is set are not considered in
# the determination of absolute photometric zeropoints.
#
# 23.01.2006:
# I introduced a new command line argument giving the minimum
# magnitude for standard stars to be considered. This argument is
# optional. This change should help to better reject bright objects
# with saturated features.
#
# 08.02.2008:
# - I removed the 'fifo' concept to ask for user input
# - I introduced an automatic calibration mode. If it is chosen
#   over the interactive one (10th command line argument) the
#   script checks whether the zeropoint of the second solution
#   (fit of zeropoint and colour term) is within predefined values
#   (11th and 12th command line argument). If this condition is
#   fulfilled this solution is taken.
#
# 18.05.2009:
# Modification of the script to reflect changes in the photo_abs.py
# module. There, we now use matplotlib instead of ppgplot.
#
# 03.02.2010:
# some code cleaning
#
# 28.03.2010:
# BUG FIX: The condition for accepting a zeropoint in the AUTOMATIC
# mode was wrong.
#
# 26.11.2012:
# - calibrations can now be done on night basis (this is how the script
#   worked up to now), or on a run basis. In the latter case all standard
#   stars from all present nights are used to determine one single set
#   of photometric quantities for all science images.
# - the photo_abs.py script no longer requires a 'night' argument.
#
# 30.12.2012:
# In RUNCALIBRATION mode we now use the dummy night 0 to indicate this.
# Before we used the string 'RUN' but it was problematic to have a string
# or a numeric value as indicator for the calibration mode.
#
# 31.12.2012:
# I made the script more robust against non-existing files
#
# 18.01.2013:
# I renamed the variable CHOICE to the more meaningful ZPCHOICE.
#
# 17.06.2013:
# Slight adaptions to the script because now catalogues, merged from
# individual exposures, not from individual chips, enter this script.
#
# 27.08.2013:
# the temporary file 'photo_res' did not have the process number included
# (potential problems if this script was called multiple times with the
# same 'TEMPDIR' setting).
#
# 07.09.2013:
# I substituded 'direct listings' of directories with the 'find' command
# (command too long issues with very big directories).
#
# 23.03.2014:
# I introduced a new command line options indicating whether science
# images should be updated with magnitude zeropoint information.
# For many images this updates takes some time and we do not always need it
# (e.g. 'science' image processing for z-band in KIDS).
#
# 06.05.2016:
# We reject obvious non-photometric data from the absolute photometric
# calibration process. For this we estimate the (extinction
# uncorrected) median zeropoint of the sources and we reject all
# sources deviating by more then 1 mag from this median. Only the
# remianing sources enter the fit at all. We noticed for DECam data
# that these strong outliers prevented the fit to succeed in several
# cases.
#
# 12.09.2016:
# - I removed the interactive script mode and the possibility to update science
#   images with the zeropoint. The first feature was not used for years and
#   the second feature id performed by the script create_zp_correct_header....
# - temporary files are now deleted at the end of the script. All temporary
#   files have now the PID at the end of their name. They had it partly in
#   the middle of their name before.
# - All command line arguments are now referenced with meaningful names.
#
# 17.07.2017:
# Refinement of initial catalogue selection: We only consider catalogues
# directly in the 'cat' directory but not in subdiretories of it. We now
# can have a BAD_PHOTOM subdiretory and hence the change was necessary.

# $1 main dir
# $2 standard dir
# $3 science dir
# $4 image extension
# $5 filter
# $6 filter name in standardstar catalog
# $7 color index (e.g. VmI)
# $8 Extinction for fits with constant extinction
# $9 color for fit with constant color term
# $10 NIGHTCALIB or RUNCALIB (determination of photometric
#                             quantities on night- or runbasis)
# $11 lower limit for magnitudes to be considered
#     (standard star catalog) (OPTIONAL argument;
#      set to 0.0 by default, i.e. we use all objects)


. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# give meaningful names to some command line arguments:
MD=$1
STD=$2
SD=$3
ENDING=$4
FILTER=$5
STD_FILTER=$6
STD_CI=$7
DEFAULT_EXT=$8
DEFAULT_COLOR=$9
CALIB_METHOD=${10}

if [ $# -eq 11 ]; then
  MINMAG=${11}
else
  MINMAG=0.0
fi

# list the exposure catalogues that were merged with the
# a standardstar catalogue:
CATS=`${P_FIND} ${MD}/${STD}/cat/ -maxdepth 1 -name \*all_photprep_merg.cat`

# check whether something needs to be done at all:
if [ "${CATS}" != "" ]; then
  ${P_LDACPASTE} \
      -i ${CATS} -t PSSC -o ${TEMPDIR}/tmp_exp.cat_$$

  # first filters for suitable objects:
  # Filter for the magnitude in the standard star catalogue:
  COND="${STD_FILTER}mag < 99"
  COND="(${STD_FILTER}mag > ${MINMAG})AND(${COND})"
  # reasonable standard star colour term:
  COND="(${STD_CI} < 10)AND(${COND})"
  COND="(${STD_CI} > -10)AND(${COND})"
  # The input magnitude of our object  (normalised to 1s and
  # with a 0.0 zeropoint) must be negative:
  COND="(Mag < 0)AND(${COND});"

  ${P_LDACFILTER} \
      -i ${TEMPDIR}/tmp_exp.cat_$$ -t PSSC \
      -c "${COND}" -o ${MD}/${STD}/cat/allexp_tmp.cat

  LABEL=`echo ${STD_CI} | sed 's/m/-/g'`

  # Get a list of all nights:
  NIGHTS=`${P_LDACTOASC} -i ${MD}/${STD}/cat/allexp_tmp.cat  -t PSSC\
      -b -k GABODSID | ${P_SORT} | uniq | ${P_GAWK} '{printf("%s ", $1)}'`

  # In case of run calibration we do not loop through each night
  if [ "${CALIB_METHOD}" = "RUNCALIB" ]; then
    CALIBNIGHT="${NIGHTS}" # variable that is  written as CALIBNIGHT
                           # to image header
    NIGHTS="0" # we use the dummy night 0 to indicate RUN calibration.
               # The value of this variable is used below for the filename
               # of the result file.
  fi

  for NIGHT in ${NIGHTS}
  do
    if [ "${CALIB_METHOD}" = "RUNCALIB" ]; then
      echo
      echo "       ---==== Calibrating the whole RUN ====---"
      echo
    else
      echo
      echo "       ---==== Calibrating night ${NIGHT} ====---"
      echo
    fi

    if [ "${CALIB_METHOD}" = "RUNCALIB" ]; then
      cp ${MD}/${STD}/cat/allexp_tmp.cat ${MD}/${STD}/cat/night_${NIGHT}.cat
    else
      ${P_LDACFILTER} -i ${MD}/${STD}/cat/allexp_tmp.cat  -t PSSC \
    	              -o ${MD}/${STD}/cat/night_${NIGHT}.cat \
                      -c "(GABODSID=${NIGHT});"
    fi

    ${P_LDACTOASC} -i ${MD}/${STD}/cat/night_${NIGHT}.cat  -t PSSC \
  	-b -k Mag ${STD_FILTER}mag ${STD_CI} AIRMASS > \
        ${TEMPDIR}/night_${FILTER}_${NIGHT}.asc_$$

    # get rid of obvious problematic sources having an (extinction uncorrected)
    # zeropoint with a deviation from 1 or more from the median zeropoint.
    ${P_GAWK} '{line[NR] = $0; zp[NR] = $2 - $1; zp_copy[NR] = $2 - $1} END {
      asort(zp);
      if (NR % 2) {
        median = zp[(NR + 1) / 2];
      } else {
        median = (zp[(NR / 2)] + zp[(NR / 2) + 1]) / 2.0
      }
      for (i = 1; i <= NR; i++) {
        if (zp_copy[i] > median - 1.0 && zp_copy[i] < median + 1.0) {
          print line[i]
        }
      }}' ${TEMPDIR}/night_${FILTER}_${NIGHT}.asc_$$ > \
          ${TEMPDIR}/use_phot.asc_$$

    # finally do photometric fits:
    ${S_PHOTOABS} \
        --input=${TEMPDIR}/use_phot.asc_$$ \
        --output=${TEMPDIR}/photo_res_$$ --extinction="${DEFAULT_EXT}" \
        --color="${DEFAULT_COLOR}" --label=${LABEL} --batch

    # in 'photo_abs.py' we 'abuse' the return value to give the interactive
    # choice back to the shell:
    ZPCHOICE=$?

    test -d ${MD}/${STD}/calib || mkdir ${MD}/${STD}/calib

    mv ${TEMPDIR}/photo_res_$$.asc \
       ${MD}/${STD}/calib/night_${NIGHT}_${FILTER}_result.asc
    mv ${TEMPDIR}/photo_res_$$.png \
       ${MD}/${STD}/calib/night_${NIGHT}_${FILTER}_result.png
  done
else
  theli_error "No catalogues available for absolute phot. calibration!"
  exit 1;
fi

# clean up and bye:
for FILE in ${TEMPDIR}/*_$$
do
  test -f ${FILE} && rm ${FILE}
done

theli_end
exit 0;
