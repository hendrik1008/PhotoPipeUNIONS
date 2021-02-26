#!/bin/bash -u

# This is a cousin script to 'parallel_manager.sh'. While
# parallel_manager.sh performs parallelisation on the chip-level of
# individual exposures, this script parallelises tasks that need all
# chips of an exposure available.

# HISTORY:
# ========
#
# 03.11.2017:
# temporary files are deleted at the end of the script

# scripts that are parallelised on the exposure level must fulfil the
# following criteria:

# - The first three arguments of the script MUST be:
#   (1) main directory; (2) science directory; (3) image ending
# - After the first three mandatory arguments, an arbitrary number
#   of arguments can follow
# - The last argument is a text file listing an exposure name
#   on each line. If individual chips are named like bla_1ENDING.fits,
#   bla_2ENDING.fits, .... the exposure name in that case would be 'bla'

. ./${INSTRUMENT:?}.ini

# Per default we use simple bash techniques to parallelise THELI jobs.
# If you need other schemes for specific machines, you can append
# modes in a natural way.

# NOTE:
# Up to now I did not get to work the script with GNU-parallel
# if one if the script arguments is an empty string!

: ${THELI_PARALLEL:="THELIPARA"} # default value
if [ "${THELI_PARALLEL}" = "THELIPARA" ]; then
  # script and mandatory script arguments:
  SCRIPT=$1
  MD=$2
  SD=$3
  ENDING=$4

  # The follwing lines obtain the exposure names and split them
  # into files exp_para_i.txt_$$ where i runs from 1 up to the number
  # of parallel channels:
  ${P_FIND} ${MD}/${SD} -maxdepth 1 -mindepth 1 -name \*${ENDING}.fits | \
    ${P_GAWK} -F/ '{print $NF}' | sed -e 's/_[0-9]\+'${ENDING}'.fits//' | \
    sort | uniq > ${TEMPDIR}/exposures.txt_$$

  NEXPOSURES=$(wc -l ${TEMPDIR}/exposures.txt_$$ | cut -d ' ' -f1)

  # just make sure that we get at least 1 exposure per parallel channel!
  if [ ${NEXPOSURES} -lt ${NPARA} ]; then
    NPARA=${NEXPOSURES}
  fi

  ${P_GAWK} '{print $0 > sprintf("'${TEMPDIR}'/exp_para_%d.txt_%d",
                         1 + NR%'${NPARA}', '$$')}' \
    ${TEMPDIR}/exposures.txt_$$


  # and finally launch the jobs:
  for FILE in $(ls ${TEMPDIR}/exp_para*.txt_$$)
  do
    ./${SCRIPT} "${@:2}" ${FILE} &
  done

  wait;
# else ... # other parallel modes may be implemented here
fi

# celan up and bye:
rm ${TEMPDIR}/*_$$
exit 0;
