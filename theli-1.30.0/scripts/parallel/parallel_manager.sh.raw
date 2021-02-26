#!/bin/bash -u

# HISTORY INFORMATION:
#
# 16.05.2010:
# sanity check for the relation between available chips and parallel
# processes. Missing it led to too many launched processes if the
# camera has fewer chips than parallel channels
#
# 21.11.2013:
# I included some more sanity checks. The script tried to launch more
# processes than necessary if the number of chips was not a multiple
# of the number of parallel channels!
#
# 21.03.2014:
# The default parallel scheme is now 'GNU parallel'. If you still want to
# use the old THELI parallel scheme set the environment variable
# 'THELI_PARALLEL' to something except 'GNUPARA'.
#
# Advantages of GNU parallel:
# - 'much' simpler to implement
# - better load balancing if some chip(s) need more processing time than
#   others for some reason.
# - output from different jobs is better ordered and no longer a complete
#   mess.
#
# 07.05.2014:
# I suppress the GNU parallel welcome message. It was just 'noise' in
# debugging output.
#
# 16.01.2017:
# I make sure that the instrument specific config file is sourced from the
# current directory.

. ./${INSTRUMENT:?}.ini

# sanity check:
if [ ${NPARA} -gt ${NCHIPS} ]; then
  NPARA=${NCHIPS}
fi

# Per default we use the GNU paprallel script to parallelise
# THELI jobs. If you do not want this set the environment
# variable 'THELI_PARALLEL' to another value then GNUPARA.

: ${THELI_PARALLEL:="GNUPARA"} # default value
if [ -x ./parallel.pl ] && [ "${THELI_PARALLEL}" = "GNUPARA" ]; then
  # script and script arguments:
  SCRIPT=$1
  shift
  ARGS=$*

  seq 1 ${NCHIPS} | ./parallel.pl --no-notice \
                     -j ${NPARA} ./${SCRIPT} ${ARGS} {1}
else # old parallel_manager THELI parallel scheme:
  # do some initialisations
  i=1
  while [ ${i} -le ${NPARA} ]
  do
    PARA[$i]=""
    i=$(( $i + 1 ))
  done

  # now divide the chips to the processors:
  NPROC=1   # total number of distributed processes
  while [ ${NPROC} -le ${NCHIPS} ]
  do
    ACTUPARA=1
    while [ ${ACTUPARA} -le ${NPARA} ] && [ ${NPROC} -le ${NCHIPS} ]
    do
      PARA[${ACTUPARA}]="${PARA[${ACTUPARA}]} ${NPROC}"
      NPROC=$(( ${NPROC} + 1 ))
      ACTUPARA=$(( ${ACTUPARA} + 1 ))
    done
  done

  # finally start all the processes
  SCRIPT=$1
  shift
  ARGS=$*

  ACTUPARA=1
  while [ ${ACTUPARA} -le ${NPARA} ]
  do
    if [ "${PARA[${ACTUPARA}]}" != "" ]; then
      ( ./${SCRIPT} ${ARGS} "${PARA[${ACTUPARA}]}" ) &
    fi
    ACTUPARA=$(( ${ACTUPARA} + 1 ))
  done

  # wait until all subprocesses have finished
  wait
fi

exit 0;
