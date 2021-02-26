#!/bin/bash -u

# The script merges astrometrically calbrated standardstar observations
# with a standardstar catalogue. It represents the second of a necessary
# to-stage process. This script cannot be used without having successfully
# run 'create_stdphotom_prepare_para.sh' before.

# SCRIPT HISTORY:
# ===============
#
# 05.06.2013:
# script started
#
# 18.01.2015:
# ldacdesc is now called via its program variable and bo longer
# directly (bug fix).
#
# 13.05.2016:
# several temporary files did not go into TEMPDIR - fixed.
#
# 26.09.2016:
# The standard star catalogue is now filtered before being matched
# with individual pointings- For DECam the association of the standardstar
# catalogue with individual exposures took very long (very large standard
# star catalogue). We therefore extract Ra/Dec ranges from the individual
# exposures and filter the standardstar source list accordingly before the
# association. This speeds up the whole process very significantly!
#
# 17.07.2017:
# Refinement of initial catalogue selection: We only consider catalogues
# directly in the 'cat' directory but not in subdiretories of it. We now
# can have a BAD_PHOTOM subdiretory and hence the change was necessary.

#$1: main directory
#$2: science dir. (the catalogues are assumed to be in $1/$2/cat)
#$3: image extension
#$4: photometric standard star catalogue with full path

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# check number of command line arguments:
if [ $# -ne 4 ]; then
  theli_error "Number of command line argument not correct!"
  exit 1;
fi

# give meaningful names to command line arguments:
MD=$1
SD=$2
EXTENSION=$3
PHOTCAT=$4

# create configuration file 'make_ssc' (merging of extracted sources
# with standard star catalogue below) on the fly:
grep "COL_INPUT"  ${DATACONF}/stdphotom_prepare_make_ssc.conf |\
  ${P_GAWK} '{print $3}' > ${TEMPDIR}/confraw.txt_$$

echo "SeqNr" >> ${TEMPDIR}/confraw.txt_$$

# get keys that need to be added to stdphotom_prepare_make_ssc.conf
# from the standardstar catalogue. For simplicity we consider all keys
# present in the standardstar catalogue but not in our source lists:
${P_LDACDESC} -i ${PHOTCAT} -t STDTAB |\
   grep "Key name:" | ${P_GAWK} -F. '{print $NF}' > \
   ${TEMPDIR}/photcat.txt_$$

cat ${TEMPDIR}/confraw.txt_$$  ${TEMPDIR}/photcat.txt_$$ | \
   sort | uniq -c | ${P_GAWK} '($1 == 1) {print $2}' > \
   ${TEMPDIR}/uniq.txt_$$

cat ${TEMPDIR}/photcat.txt_$$ ${TEMPDIR}/uniq.txt_$$ | \
   sort | uniq -c | ${P_GAWK} '($1 == 2) {print $2}' > \
   ${TEMPDIR}/add.txt_$$

cat ${DATACONF}/stdphotom_prepare_make_ssc.conf > \
  ${TEMPDIR}/make_ssc.conf_$$

while read COL
do
  {
    echo "#"
    echo "COL_NAME  = ${COL}"
    echo "COL_INPUT = ${COL}"
    echo "COL_MERGE = AVE_REG"
    echo "COL_CHAN  = 1"
  } >>  ${TEMPDIR}/make_ssc.conf_$$
done < ${TEMPDIR}/add.txt_$$

# do the real work now:

# get a list of all the exposures:
${P_FIND} ${MD}/${SD}/cat -maxdepth 1 -name \*${EXTENSION}_photprep.cat |\
  ${P_GAWK} -F/ '{print $NF}' | sed -e 's/'${EXTENSION}'_photprep\.cat//' | \
  ${P_GAWK} -F_ '{$NF = ""; $1 = $1; print $0}' | sort | uniq >  \
    ${TEMPDIR}/exposures.txt_$$

# do we need to do something at all?
if [ -s ${TEMPDIR}/exposures.txt_$$ ]; then
  # distribute task on different processors/cores:
  NIMA=`wc -l ${TEMPDIR}/exposures.txt_$$ | ${P_GAWK} '{print $1}'`

  if [ ${NPARA} -gt ${NIMA} ]; then
    NPARA=${NIMA}
  fi

  PROC=0
  while [ ${PROC} -lt ${NPARA} ]
  do
    NFIELDS[${PROC}]=0
    FIELDS[${PROC}]=""
    PROC=$(( ${PROC} + 1 ))
  done

  i=1
  while read FIELD
  do
    PROC=$(( $i % ${NPARA} ))
    FIELDS[${PROC}]="${FIELDS[${PROC}]} ${FIELD}"
    NFIELDS[${PROC}]=$(( ${NFIELDS[${PROC}]} + 1 ))
    i=$(( $i + 1 ))
  done < ${TEMPDIR}/exposures.txt_$$

  PROC=0
  while [ ${PROC} -lt ${NPARA} ]
  do
    j=$(( ${PROC} + 1 ))
    echo -e "Starting Job $j. It has ${NFIELDS[${PROC}]} files to process!\n"
    {
      k=1
      for FIELD in ${FIELDS[${PROC}]}
      do
        echo "Working on exposure ${FIELD}"
        ${P_LDACPASTE} -i /${MD}/${SD}/cat/${FIELD}*${EXTENSION}_photprep.cat \
                       -o ${TEMPDIR}/tmp.cat0_${j}_$$ -t STDTAB

        # note that ldacpaste does not update SeqNr. So we have to do this
        # manually:
        ${P_LDACDELKEY} -i ${TEMPDIR}/tmp.cat0_${j}_$$ \
                        -o ${TEMPDIR}/tmp.cat1_${j}_$$ \
                        -t STDTAB -k SeqNr

        ${P_LDACADDKEY} -i ${TEMPDIR}/tmp.cat1_${j}_$$ \
                        -o /${MD}/${SD}/cat/${FIELD}_all_photprep.cat \
                        -t STDTAB -k SeqNr 1 COUNT "running object number"

        # filter standardstar catalogue for objects in Ra/Dec-range of
        # the photprep catalogue. First the Ra-range filtering. A bit
        # of fiddeling is necessary for pole-fields.
        ${P_LDACTOASC} -b -i ${MD}/${SD}/cat/${FIELD}_all_photprep.cat \
                       -t STDTAB -k ALPHA_J2000 | \
                       ${P_GAWK} '{printf("%.5f\n", $0)}' | sort -n > \
                       ${TEMPDIR}/ra_${j}_$$

        POLE_FIELD=$(${P_GAWK} '{if (NR == 1) {small = $0}} END {
                                 if (($0 - small) > 180.0) {
                                   print 1;
                                 } else {
                                   print 0;
                                 }}' ${TEMPDIR}/ra_${j}_$$)

        if [ ${POLE_FIELD} -eq 0 ]; then
          MIN_RA=$(${P_GAWK} '(NR == 1) {print $0}' ${TEMPDIR}/ra_${j}_$$)
          MAX_RA=$(${P_GAWK} 'END {print $0}' ${TEMPDIR}/ra_${j}_$$)

          ${P_LDACFILTER} -i ${PHOTCAT} -t STDTAB \
                          -c '(Ra>='${MIN_RA}')AND(Ra<='${MAX_RA}');' \
                          -o ${TEMPDIR}/${FIELD}_std_tmp.cat_${j}_$$
        else
          MIN_RA=$(${P_GAWK} '($0 > 180.0)' ${TEMPDIR}/ra_${j}_$$ |\
                   ${P_GAWK} '(NR == 1) {print $0}')
          MAX_RA=$(${P_GAWK} '($0 < 180.0)' ${TEMPDIR}/ra_${j}_$$ |\
                   ${P_GAWK} 'END {print $0}')

          ${P_LDACFILTER} -i ${PHOTCAT} -t STDTAB \
                          -c '(Ra>='${MIN_RA}')OR(Ra<='${MAX_RA}');' \
                          -o ${TEMPDIR}/${FIELD}_std_tmp.cat_${j}_$$
        fi

        # now the Dec-range filtering:
        ${P_LDACTOASC} -b -i ${MD}/${SD}/cat/${FIELD}_all_photprep.cat \
                       -t STDTAB -k DELTA_J2000 | \
                       ${P_GAWK} '{printf("%.5f\n", $0)}' | sort -n > \
                       ${TEMPDIR}/dec_${j}_$$

        MIN_DEC=$(${P_GAWK} '(NR == 1) {print $0}' ${TEMPDIR}/dec_${j}_$$)
        MAX_DEC=$(${P_GAWK} 'END {print $0}' ${TEMPDIR}/dec_${j}_$$)

        ${P_LDACFILTER} -i ${TEMPDIR}/${FIELD}_std_tmp.cat_${j}_$$ -t STDTAB \
                        -c '(Dec>='${MIN_DEC}')AND(Dec<='${MAX_DEC}');' \
                        -o ${TEMPDIR}/${FIELD}_std.cat_${j}_$$

        # Now we can finally associate:
        ${P_ASSOCIATE} -i /${MD}/${SD}/cat/${FIELD}_all_photprep.cat \
                          ${TEMPDIR}/${FIELD}_std.cat_${j}_$$ \
		       -o ${TEMPDIR}/tmp.cat3_${j}_$$ \
                          ${TEMPDIR}/tmp.cat4_${j}_$$ \
                       -t STDTAB \
                       -c ${DATACONF}/stdphotom_prepare_associate.conf

        ${P_MAKESSC} -i ${TEMPDIR}/tmp.cat3_${j}_$$ \
                        ${TEMPDIR}/tmp.cat4_${j}_$$ \
       	             -o ${TEMPDIR}/tmp.cat5_${j}_$$ -t STDTAB \
                     -c ${TEMPDIR}/make_ssc.conf_$$

        ${P_LDACFILTER} -i ${TEMPDIR}/tmp.cat5_${j}_$$ \
                        -o /${MD}/${SD}/cat/${FIELD}_all_photprep_merg.cat\
                        -c "((N_00=1)AND(N_01=1));" -t PSSC

        k=$(( $k + 1 ))
      done
    } &
    PROC=$(( ${PROC} + 1 ))
  done

  # only continue once all processes have finished
  wait;
else
  theli_error "No images to process!"
  rm ${TEMPDIR}/*_$$
  exit 1;
fi

# clean up and bye:
for FILE in ${TEMPDIR}/*_$$
do
  test -f ${FILE} && rm ${FILE}
done

theli_end
exit 0;
