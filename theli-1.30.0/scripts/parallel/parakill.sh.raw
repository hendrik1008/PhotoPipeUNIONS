#!/bin/bash

# script that kills a process including all the childs and child's
# childs; the command line argument is a (sub)string of the jobname
# which allows its identification in the process list
#
# parts of the code were inspired by the 'zap' shellscript from
# Kerninghan and Pike: "The Unix Programming Environment"

# HISTORY COMMENTS:
# =================
#
# 10.09.2008:
# I rewrote the script to accept (parts of) the job name instead of
# the job number.

#$1: (sub)string of the process to be killed

# FUNCTION definitions
#
# 'killchild' recursively determines all jobs started by the job ID
# given as first argument.  It modifies the 'global' variable
# G_KILLJOBS
function killchild() {
  i=1
  JOBS=""
  JOBS=`ps -eaf | awk '{if($3=='$1') printf("%d ", $2)}'`
  test -z "${JOBS}" || echo "Job $1 invoked the tasks: ${JOBS}"

  if [ -z "${JOBS}" ]; then
    return
  else
    G_KILLJOBS="${G_KILLJOBS} ${JOBS}"
    for JOB in ${JOBS}
    do
      killchild ${JOB}
    done
  fi
}

# the function 'pick' asks the user for y/q/n for each of its
# arguments. 'y' gives back the argument, 'n' continues without any
# further action on the argument, 'q' leaves the program completely
# (taken from Kerninghan and Pike: The UNIX Programming Environment).
function pick() {
  for i
  do
      echo -n "$i? " >/dev/tty
      read response
      case $response in
      y*)    echo $i ;;
      q*)    exit 0;
      esac
  done </dev/tty
}

SCRIPT=`basename $0`

# A temporary change of IFS to only a newline character is necessary
# so that complete lines are treated as an entity and not splitted by
# the sheall
OLDIFS=${IFS}
IFS='
'

# get all JOB IDs from tasks which should be recursively killed:
KILLCANDIDATES=`pick \`ps -e -o pid,command |\
                       grep $1 | sed '/grep '$1'/d' |\
                       sed '/'${SCRIPT}'/d'\` |\
                awk '{print $1}'`

# We need to normally split the KILLCANDIDATES variable below; hence
# reverse to the old IFS:
IFS=${OLDIFS}

for JOB in ${KILLCANDIDATES}
do
  G_KILLJOBS="${JOB} "
  killchild ${JOB}

  # To make sure that jobs are killed in the correct order (from top
  # to bottom) we kill them one after the other (I did not find a
  # specification of kill respects the order of jobs given when
  # invoked with multiple jobs)
  for KILLJOB in ${G_KILLJOBS}
  do
    kill ${KILLJOB}
  done
done

exit 0;



