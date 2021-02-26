#!/bin/bash

# created by Axel Buddendiek, Argelander Institut fuer Astronomie
# Auf dem Huegel 71, Bonn
# abuddend@astro.uni-bonn.de

# NOTE
# This script should not be called 'standalone'. It is used by
# 'create_crosstalk_coefficients.sh'.

#
# This script measures the flux and the flux in the
# neighbouring image of high signal to noise objects.
# The quotient of this is our crosstalk coefficient.
# Since it differs from object to object and there are some false detections
# we use the median of our coefficient catalog.

# UPDATES
# =======
#
# 25.02.2013: changed var names to capital letters, using progs.ini,
#		  deleted obsolete stuff like changing flux sign,
#		  using crosstalk.conv for SE
# 04.03.2013: Using coefficient CUTOFF in order to avoid
#		  obviously wrong results, default for
#		  OMEGACAM should be 0.01
# 26.03.2013: Integration into the THELI data flow (TE)
# 03.04.2013: I substituted Axels construct to isolate the file basename
#             with a simple 'basename' command. Axels construct led to errors
#             if the filepath contained double slashes (//)
# 09.07.2013: Searching for two objects at the same pixel position and
#             and rejecting those objects for the coefficient determination
#             has been implemented.
#             Fixed a bug which made awk use scientific notation in the output.
#             Since bash cannot read this awk prints out floats now. (AB)
# 14.08.2013: several 'cosmetic' changes (TE)
# 06.09.2013: Implemented different S/N cuts for SCIENCESHORT
#             (<100s exposures) (AB)
# 24.01.2014: - I transformed Axels original create_crosstalk_coefficients.sh
#               and measure_crosstalk.sh script pair into a parallel version.
#               I also added some 'code cleanup'.
#             - The script used the previous estimate for the crosstalk
#               correction if no valid one can be determined for the
#               current image. I do not think this was always correct, e.g.
#               when the last image originated from another run than the
#               current one. I now return an (invalid) CT correction of '-1.0'
#               in this case and leave it to create_crosstalk_coefficients.sh
#               to deal with it.

. ./${INSTRUMENT:?}.ini


# our two image addresses should have been passed as command line arguments
IM1=${1}
IM2=${2}
PROCESS_NUMBER=${3}
OBKEYWORD=${4}
CUTOFF=${5}

SIZE=10         # Size of apertures for ASSOCIATE; this value
                #turns out to be robust.

NEWNAME1_3=`basename ${IM1}`
NEWNAME2_3=`basename ${IM2}`

# run SE the first time in order to get a model for the bg
${P_SEX} ${IM1} -CHECKIMAGE_TYPE -BACKGROUND \
                -CHECKIMAGE_NAME  ${TEMPDIR}/sub_${PROCESS_NUMBER}.fits \
                -c ${CONF}/crosstalk.sex \
                -PARAMETERS_NAME ${CONF}/crosstalk_cross.param \
                -FILTER_NAME ${CONF}/crosstalk.conv \
                -CATALOG_NAME ${TEMPDIR}/test1_${PROCESS_NUMBER}.cat \
                -CATALOG_TYPE FITS_LDAC

# getting the ob start keyword
OBSTART=$(${P_DFITS} ${IM1} | ${P_FITSORT} -d ${OBKEYWORD} |\
          ${P_GAWK} '{print $2}')


# run sextractor for neighbour image
${P_SEX} ${IM2} -CHECKIMAGE_TYPE -BACKGROUND \
                -CHECKIMAGE_NAME ${TEMPDIR}/sub2_${PROCESS_NUMBER}.fits  \
                -c ${CONF}/crosstalk.sex \
                -PARAMETERS_NAME ${CONF}/crosstalk_cross.param \
                -FILTER_NAME ${CONF}/crosstalk.conv \
                -CATALOG_NAME ${TEMPDIR}/test2_${PROCESS_NUMBER}.cat \
                -CATALOG_TYPE FITS_LDAC

# look for objects at the same pixel position

# First we need to add the A_IMAGE and B_IMAGE keys to our catalogs.
# This defines the object sizes. A circle of 10 pixels diameter seems
# to be fine.
${P_LDACADDKEY} -i ${TEMPDIR}/test1_${PROCESS_NUMBER}.cat \
                -o ${TEMPDIR}/test13_${PROCESS_NUMBER}.cat \
                -t LDAC_OBJECTS \
                -k A_IMAGE ${SIZE} FLOAT 'A_IMAGE' \
                   B_IMAGE ${SIZE} FLOAT 'B_IMAGE' \
                   THETA_IMAGE 0.0 FLOAT 'THETA_IMAGE'

${P_LDACADDKEY} -i ${TEMPDIR}/test2_${PROCESS_NUMBER}.cat \
                -o ${TEMPDIR}/test23_${PROCESS_NUMBER}.cat \
                -t LDAC_OBJECTS \
                -k A_IMAGE ${SIZE} FLOAT 'A_IMAGE' \
                   B_IMAGE ${SIZE} FLOAT 'B_IMAGE' \
                   THETA_IMAGE 0.0 FLOAT 'THETA_IMAGE'

# Now we cross match the catalogs using associate and make_ssc
${P_ASSOCIATE} -p -i ${TEMPDIR}/test23_${PROCESS_NUMBER}.cat \
                     ${TEMPDIR}/test13_${PROCESS_NUMBER}.cat \
                  -o ${TEMPDIR}/assoc_1_${PROCESS_NUMBER}.cat \
                     ${TEMPDIR}/assoc_2_${PROCESS_NUMBER}.cat \
                  -t LDAC_OBJECTS \
                  -c ${CONF}/matching_CT.conf

${P_MAKESSC} -i ${TEMPDIR}/assoc_1_${PROCESS_NUMBER}.cat \
                ${TEMPDIR}/assoc_2_${PROCESS_NUMBER}.cat \
             -o ${TEMPDIR}/merged_${PROCESS_NUMBER}.cat \
             -c ${CONF}/make_ssc_CT.conf \
             -t LDAC_OBJECTS \
             -SEQNR NUMBER

# In the end we need to find all objects, which are in both cats.
${P_LDACFILTER} -i ${TEMPDIR}/merged_${PROCESS_NUMBER}.cat \
                -t PSSC \
                -o ${TEMPDIR}/merged_clean_${PROCESS_NUMBER}.cat \
                -c "((N_00 = 1) AND (N_01 = 1));"

# ... and convert it to ascii
${P_LDACTOASC} -b -i ${TEMPDIR}/merged_clean_${PROCESS_NUMBER}.cat \
                  -t PSSC \
                  -k X_IMAGE Y_IMAGE \
                  > ${TEMPDIR}/double_objects_${PROCESS_NUMBER}.asc

# run dual image mode in order to measure our object fluxes
${P_SEX} ${TEMPDIR}/sub2_${PROCESS_NUMBER}.fits,\
         ${TEMPDIR}/sub_${PROCESS_NUMBER}.fits \
         -c ${CONF}/crosstalk.sex \
         -PARAMETERS_NAME ${CONF}/crosstalk.param \
         -FILTER_NAME ${CONF}/crosstalk.conv \
         -CATALOG_NAME ${TEMPDIR}/invertcat_${PROCESS_NUMBER}.txt\
         -CHECKIMAGE_TYPE NONE

${P_SEX} ${TEMPDIR}/sub2_${PROCESS_NUMBER}.fits,\
         ${TEMPDIR}/sub2_${PROCESS_NUMBER}.fits \
         -c ${CONF}/crosstalk.sex \
         -PARAMETERS_NAME ${CONF}/crosstalk.param \
         -FILTER_NAME ${CONF}/crosstalk.conv \
         -CATALOG_NAME ${TEMPDIR}/sub2cat_${PROCESS_NUMBER}.txt \
         -CHECKIMAGE_TYPE NONE

# merge catalogs
pr -m -s -t ${TEMPDIR}/invertcat_${PROCESS_NUMBER}.txt \
            ${TEMPDIR}/sub2cat_${PROCESS_NUMBER}.txt > \
            ${TEMPDIR}/matched_${PROCESS_NUMBER}.cat

# filter catalog using ${P_GAWK}
# we use the following cuts:
#	- no border objects
#	- 10000 < S/N < 50000
#	- |coefficient| < 1
BORDERLOWX=300
BORDERLOWY=300
BORDERHIGHX=`${P_DFITS} ${IM1} | ${P_FITSORT} -d NAXIS1 | \
             ${P_GAWK} '{print $2 - 300;}'`
BORDERHIGHY=`${P_DFITS} ${IM1} | ${P_FITSORT} -d NAXIS2 | \
             ${P_GAWK} '{print $2 - 300;}'`

 ### ### ### ### ###
 # Find exposure time
EXPTIME=$(${P_DFITS} $IM1 | ${P_FITSORT} -d EXPTIME \
            | ${P_GAWK} '{print $2}')

 # Check if EXPTIME is smaller than 100s
IFVARTIME=$(echo "${EXPTIME} < 100.0" | bc -l)


 # Apply different S/N cuts depending on the exposure time
if [ "${IFVARTIME}" -eq 1 ]; then
  LOWSN=20000
  HIGHSN=100000
else
  LOWSN=10000
  HIGHSN=50000
fi

cat ${TEMPDIR}/matched_${PROCESS_NUMBER}.cat | ${P_GAWK} -v limit=${CUTOFF} \
'(sqrt($13 * $13) > 0.001) {
   if ($1 > '${BORDERLOWX}' && $1 < '${BORDERHIGHX}' &&
       $2 > '${BORDERLOWY}' && $2 < '${BORDERHIGHY}' &&
       $13 / $14 > '${LOWSN}' && $13 / $14 < '${HIGHSN}' &&
       sqrt(($5 / $13) * ($5 / $13)) < limit) {
         printf "%.10f\t%.3f\t%.3f\t%.3f\t%.3f\t%.10f\n",
                $5 / $13, $1, $2, $5, $13, $5 / $13
      }}' > ${TEMPDIR}/matched_cor1_${PROCESS_NUMBER}.cat

### ### ### ### ###
# now we sort the catalog
sort -nr ${TEMPDIR}/matched_cor1_${PROCESS_NUMBER}.cat | \
  ${P_GAWK} '{print $1, $2, $3, $4, $5, $6}' > \
    ${TEMPDIR}/matched_cor_${PROCESS_NUMBER}.cat


# here we make sure there are no stars at the same pixel position in both chips

# check for cleaned catalog and delete if existent.
if [ -e ${TEMPDIR}/matched_cor_cleaned_${PROCESS_NUMBER}.cat ]; then
    rm ${TEMPDIR}/matched_cor_cleaned_${PROCESS_NUMBER}.cat
fi

# Create empty clean catalog in order to use it later if no objects
# were chosen to be in the clean catalog.
touch ${TEMPDIR}/matched_cor_cleaned_${PROCESS_NUMBER}.cat

# Two while loops to check if there are objects at the same pixel position for
# our crosstalk catalog.
while read NUMBER1 X Y NUMBER2 NUMBER3 NUMBER4
do
  TESTVAR=0

  while read X2 Y2
  do
    # Check if both objects are at the same pixel position
    IFX1=$(echo "sqrt ( (${X}-${X2})*(${X}-${X2}) + (${Y}-${Y2})*(${Y}-${Y2}) ) < ${SIZE}" \
    | bc -l)

    if [ "${IFX1}" -eq "1" ]; then
      TESTVAR=$(echo "${TESTVAR} + 1" | bc -l)
    fi
  done < ${TEMPDIR}/double_objects_${PROCESS_NUMBER}.asc

  # If there is no other object at this pixel position we will use it for
  # computing the coefficient
  if [ "${TESTVAR}" -eq "0" ]; then
    echo ${NUMBER1} ${X} ${Y} ${NUMBER2} ${NUMBER3} ${NUMBER4} \
      >> ${TEMPDIR}/matched_cor_cleaned_${PROCESS_NUMBER}.cat
  fi

done < ${TEMPDIR}/matched_cor_${PROCESS_NUMBER}.cat

# move cleaned catalog to normal catalog
# this saves us some changes in the old parts of the code
mv ${TEMPDIR}/matched_cor_cleaned_${PROCESS_NUMBER}.cat \
   ${TEMPDIR}/matched_cor_${PROCESS_NUMBER}.cat


# Check if there are matched objects (These are the crosstalk objects)
# (There should always be objects, because of dual image mode, right?)
IFVAR=$(wc -l ${TEMPDIR}/matched_cor_${PROCESS_NUMBER}.cat |\
        ${P_GAWK} '{print $1}')

if [ ${IFVAR} -ne 0 ]; then
  # calculate fraction of stolen flux (median of all values!)
  if [ ${IFVAR} -ne 1 ]; then
    MIDDLE=$(echo "${IFVAR} / 2 + 1" | bc)

    WANTED=$(cat ${TEMPDIR}/matched_cor_${PROCESS_NUMBER}.cat | \
      ${P_GAWK} -v mid=${MIDDLE} 'NR==mid {print $1}')
  else
    WANTED=$(cat ${TEMPDIR}/matched_cor_${PROCESS_NUMBER}.cat | \
      ${P_GAWK} '{print $1}')
  fi

  echo "Fraction of flux stolen:" $WANTED

  # save coefficients
  echo ${NEWNAME1_3} ${NEWNAME2_3} ${OBSTART} ${WANTED} >> \
    ${TEMPDIR}/statistics_${PROCESS_NUMBER}.txt
else
  # In case we cannot determine a valid CT correction for the
  # current image we return '-1.0' indicating 'no correction could'
  # be determined.
  echo ${NEWNAME1_3} ${NEWNAME2_3} ${OBSTART} -1.0 >> \
    ${TEMPDIR}/statistics_${PROCESS_NUMBER}.txt
fi

# clean up and bye:
rm ${TEMPDIR}/matched_cor_${PROCESS_NUMBER}.cat \
  ${TEMPDIR}/matched_${PROCESS_NUMBER}.cat
rm ${TEMPDIR}/matched_cor1_${PROCESS_NUMBER}.cat
rm ${TEMPDIR}/invertcat_${PROCESS_NUMBER}.txt
rm ${TEMPDIR}/sub_${PROCESS_NUMBER}.fits \
  ${TEMPDIR}/sub2_${PROCESS_NUMBER}.fits
rm ${TEMPDIR}/sub2cat_${PROCESS_NUMBER}.txt

exit 0;
