#!/bin/bash

# created by Axel Buddendiek, Argelander Institut fuer Astronomie Auf
# dem Huegel 71, Bonn abuddend@astro.uni-bonn.de

# UPDATES
# =======
#
# 14.02.2013: First version is running
# 25.02.2013: Using progs.ini, changed var names to capital
#		  letters, using variable for OB keyword, changes in
#		  $MD, using _$$ for temp files and passing it as
#		  an argument to subscripts
# 26.02.2013: No hardcoding to create master file anymore
# 04.03.2013: Using coefficient CUTOFF in order to avoid
#		  obviously wrong results; default for
#		  OMEGACAM should be 0.01
# 26.03.2013: Integration into the THELI data flow (TE)
# 03.04.2013: We quit with an error message if the default CT coeffients
#             file is not present.
# 04.04.2013: some comments added to the source code (thanks to Axel)
# 24.04.2013: I transfered code from Axel to reject strong outlier
#             measurements in the crosstalk coefficients.
# 03.05.2013: I perform sanity checks on the existence of necessary
#             directories and made a sorting command more robust.
# 08.07.2013: The CUTOFF is now read in from the ct_default file
#             and is different for every chip pair. Also, the
#             outlier rejection is obsolete and has been removed (AB)
# 13.08.2013: Found two bugs. Crosstalk sign was read in for images
#             of the wrong OB. This lead in rare cases to the wrong
#             sign. Second one is the default_OMEGACAM file.
#             In case we need the default value those need to have signs,
#             too. This is fixed now. Still working on SCIENCESHORT images.
# 06.09.2013: Implemented different S/N cuts for SCIENCESHORT (<100s exposures)
#             in measure_crosstalk.sh. SCIENCESHORT problems are solved. (AB)
# 24.01.2014: - I transformed Axels original create_crosstalk_coefficients.sh
#               and measure_crosstalk.sh script pair into a parallel version.
#               The script create_crosstalk_coefficients.sh is renamed to
#               create_crosstalk_coefficients_exposurepara.sh.
#               I also added some 'code cleanup'.
#             - images for that a valid CT correction could not be determined
#               by measure_crosstalk.sh now have a CT value of '-1'. In this
#               case I do not consider them in the mean run CT correction.
#               If none of the images in a run has a valid CT correction
#               we fall back to the default CT correction.
# 04.02.2014: I changed the treatment in the case that an OB has no valid
#             CT correction could be obtained. In this case the OB gets as
#             CT correction the mean value of all images in that run. Only
#             if for no image in the run a coefficient could be determined we
#             use the default value.
# 19.01.2015: I forgot to call fitsort and dfits via their program variables
#             at one point (bug fix).
# 17.05.2018: A temporary file was not handled correctly (TEMPDIR) - fixed.

# This script reads in chip numbers and an initial guess of chip pairs
# of a multi chip camera which are suspected to do crosstalk. It then
# calls another script, which measures the crosstalk. In the end the
# mean crosstalk coefficient of an OB is computed. The final product
# is a file with all images (single chip exposures) and their
# corresponding coefficients
#
# Arguments are in the following order:
#
# $1: base dir.
# $2: science dir.
# $3: OBSTART keyword
# $4: output coefficient file

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

BASEDIR=$1
SCIENCEDIR=$2
OBKEYWORD=$3
OUTPUTFILE=$4

MD=${BASEDIR}/${SCIENCEDIR}

# test for the obligatory default coefficients file:
if [ ! -f ct_default_${INSTRUMENT}.txt ]; then
  theli_error "CT coeffients file ct_default_${INSTRUMENT}.txt not present!"
  exit 1;
fi

# check whether the directory for the output file exists at all:
OUTPUTDIR=`dirname ${OUTPUTFILE}`
if [ ! -d ${OUTPUTDIR} ]; then
  theli_warn "${OUTPUTDIR} does not exits. I try to create it!"
  mkdir -p ${OUTPUTDIR}

  if [ ! -d ${OUTPUTDIR} ]; then
    theli_error "Could not create ${OUTPUTDIR}. Exiting!"
    exit 1;
  fi
fi


# delete old result file if present:
test -e ${OUTPUTFILE} && rm ${OUTPUTFILE}

# we read in every pair of chips with crosstalk plus an initial guess
# for the coefficient
while read ROBBED THIEF WANTED_GUESS CUTOFF
do
  # remove  files from previous steps if necessary
  test -f ${TEMPDIR}/coefficients_$$.txt && rm ${TEMPDIR}/coefficients_$$.txt
  test -f ${TEMPDIR}/statistics_$$.txt && rm ${TEMPDIR}/statistics_$$.txt

  # We create a file with the columns
  # imagename OBSTART robbed_chip
  # We later append information about every thief chip. At the very
  # end we delete the not-needed information on the OBSTART.
  if [ ! -f ${TEMPDIR}/robbed_${ROBBED}_$$.txt ]; then
    find ${MD} -name \*_${ROBBED}.fits > ${TEMPDIR}/tmp.txt_$$

    # Lateron, we implicitely make use of the fact that dfits only
    # writes out the basename of the files:
    ${P_DFITS} `cat ${TEMPDIR}/tmp.txt_$$` | ${P_FITSORT} -d OBSTART | \
        ${P_GAWK} '{print $0, '${ROBBED}'}' > \
        ${TEMPDIR}/robbed_${ROBBED}_$$.txt
  fi

  # find all images of our chip pair at the MD
  ls ${MD}/*_${ROBBED}.fits > ${TEMPDIR}/names_${ROBBED}_$$.txt
  ls ${MD}/*_${THIEF}.fits > ${TEMPDIR}/names_${THIEF}_$$.txt

  # merge those files
  pr -m -s -t ${TEMPDIR}/names_${ROBBED}_$$.txt \
              ${TEMPDIR}/names_${THIEF}_$$.txt > \
              ${TEMPDIR}/names_${ROBBED}_${THIEF}_$$.txt

  # reading in the image names and passing them to a script, which
  # measures the crosstalk; the following code segment results in a file
  # 'statistics_$$.txt' that is used below. We can do crosstalk
  # estimation in parallel:
  NFILES=`wc -l ${TEMPDIR}/names_${ROBBED}_${THIEF}_$$.txt | cut -f1 -d" "`

  SPLITLINES=`expr ${NFILES} / ${NPARA}`
  if [ ${SPLITLINES} -eq 0 ]; then
    SPLITLINES=1
  fi

  split --lines=${SPLITLINES} ${TEMPDIR}/names_${ROBBED}_${THIEF}_$$.txt \
      "${TEMPDIR}/tmp_${ROBBED}_${THIEF}_$$_"

  PARACHAN=1
  for FILE in ${TEMPDIR}/tmp_${ROBBED}_${THIEF}_$$_*
  do
    # The following script calls to measure_crosstalk.sh just append
    # to the file ${TEMPDIR}/statistics_$$_${PARACHAN}.txt. Delete it
    # if it is present already:
    test -f ${TEMPDIR}/statistics_$$_${PARACHAN}.txt && \
         rm ${TEMPDIR}/statistics_$$_${PARACHAN}.txt
    {
      while read NAME1 NAME2
      do
        ./measure_crosstalk.sh ${NAME1} ${NAME2} $$_${PARACHAN} \
                               ${OBKEYWORD} ${CUTOFF}
      done < ${FILE}
    } 2>&1 &
    ((PARACHAN++)) # short for PARACHAN=$(( ${PARACHAN} + 1 ))
  done

  # wait until all parallel channels have finished.
  wait;

  cat ${TEMPDIR}/statistics_$$_*.txt | sort > ${TEMPDIR}/statistics_$$.txt

  # now calculate mean CT correction values for each run. The result
  # of the following awk command is a file with the columns
  # OBNAME  mean_CT_value  PM
  # where PM is -1 * sgn(mean_CT_value)
  sort -k3,3  ${TEMPDIR}/statistics_$$.txt |\
     ${P_GAWK} 'BEGIN {runmean = 0.0; nrun = 0; mean[""] = 0.0; n[""] = 0;} {
                if($4 > -0.9) {
                  mean[$3] += $4;
                  n[$3] += 1;
                  runmean += $4;
                  nrun += 1;
                } else { # to deal with OBs having no valid CT value
                  mean[$3] += 0.0;
                  n[$3] += 0;
                }} END {
                if(nrun > 0) {
                  runmean /= nrun;
                } else {
                  runmean = '${WANTED_GUESS}';
                }
                for(m in mean) {
                  if(m != "") {
                    if(n[m] > 0) {
                      mean[m] = mean[m] / n[m];
                    } else {
                      mean[m] = runmean;
                    }
                    printf("%s %.11f %s\n",
                           m, sqrt(mean[m] * mean[m]),
                           ((mean[m] > 0) ? "-" : "+"));
                  }
                }}' > ${TEMPDIR}/coefficients_$$.txt

  # append the current robbed/thief information to the respective statistics
  # file:
  ${P_GAWK} '{coeff[$1] = $2; pm[$1] = $3} END {
             while(getline < "'${TEMPDIR}'/robbed_'${ROBBED}'_'$$'.txt") {
               printf("%s %d %s %.11f\n", $0, '${THIEF}', pm[$2], coeff[$2]);
             }}' ${TEMPDIR}/coefficients_$$.txt > ${TEMPDIR}/tmp.txt_$$

  mv ${TEMPDIR}/tmp.txt_$$ ${TEMPDIR}/robbed_${ROBBED}_$$.txt
done < ct_default_${INSTRUMENT}.txt

# merge all the result files from the robbed chips to the output file:
cat ${TEMPDIR}/robbed_*_$$.txt | awk '{$2 = ""; print $0}' > ${OUTPUTFILE}

# remove all relics of this run
rm ${TEMPDIR}/*_$$*

theli_end
exit 0;
