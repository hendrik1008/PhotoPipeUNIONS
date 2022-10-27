#!/bin/bash -u

# the script merges info from the singleastrom and the
# check_science_PSF step. It transfers e1, e2 from the KSB cats to the
# SExtractor cats giving the "new" catalogs the same .cat as the old
# ones. Hence these catalogs can be used in the further processing
# having KSB info. Also this step is not absolutely necssary in the
# processing and can be omitted if this PSF information is not
# important.
#
# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 17.01.2007:
# - If KSB catalogues for certain files are not present
#   they will be filled up with dummy values for the four
#   keys e1, e2, rh and cl (e1=e2=-2.0; rh=0.0; cl=0).
#   We no longer quit the script
#   if KSB information is not present for all files.
#   In this way we still can use the present lensing
#   information.
# - filenames for temporary files are unique now including
#   the process number.
#
# 07.02.2007:
# I substituted two 'ls' commands with 'find' equivalents
# to avoid 'argument list too long' errors.
#
# 28.07.2007:
# I corrected a bug when seeting the BASE variable in a loop.
#
# 06.05.2008:
# I now copy catalogues always; before they were only copied if the
# 'copy' subdiretory (below 'cat') did not yet exist.  It could then
# happen that a faulty previous run of this script led to an empty
# 'copy' directory and to no transfer of KSB info!
#
# 05.10.2008:
# I make sure that temporary files are removed at the end of
# the script.
#
# 16.07.2009:
# The name of the catalogue subdirectory now has to be provided as
# command line argument.
#
# 23.10.2009:
# bug fix in the addition of dummy lensing keywords (when appropriate
# values could not be determined)
#
# 18.04.2012:
# The script now works in parallel moerging several catalogues
# simultaneously
#
# 07.01.2013:
# I made the script more robust to non-existing files.
#
# 13.05.2016:
# a temporary file did not go into TEMPDIR - fixed.

#$1: main dir.
#$2: science dir.
#$3: image extension (ext) on ..._iext.fits (i is the chip number)
#$4: catalogue directory

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# define THELI_DEBUG because of the '-u' script flag (the use of
# undefined variables will be treated as errors!)  THELI_DEBUG is used
# in the cleanTmpFiles function.
#
THELI_DEBUG=${THELI_DEBUG:-""}

# clean temporary script files:
function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"
    test -f ${TEMPDIR}/mergecats_$$ && rm ${TEMPDIR}/mergecats_$$
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

# first make a security copy of catalogues:
test -d /$1/$2/$4/copy || mkdir /$1/$2/$4/copy
${P_FIND} /$1/$2/$4/ -maxdepth 1 -name \*$3.cat -exec cp {} /$1/$2/$4/copy \;

# now transfer KSB info
${P_FIND} /$1/$2/$4/copy/ -name \*$3.cat > ${TEMPDIR}/mergecats_$$

# see whether we need to do something at all:
if [ -s ${TEMPDIR}/mergecats_$$ ]; then
  # test if all KSB cats are there; write missing ones to a file!
  test -f ${TEMPDIR}/fail_$$ && rm ${TEMPDIR}/fail_$$

  while read CAT
  do
    BASE=`basename ${CAT} .cat`
    if [ ! -f "/$1/$2/$4/${BASE}_ksb_tmp.cat2" ]; then
      echo "/$1/$2/$4/${BASE}_ksb_tmp.cat2 is not there" >> \
        ${TEMPDIR}/fail_$$
    fi
  done < ${TEMPDIR}/mergecats_$$

  # distribute the task on different processors/cores:
  NCATS=`wc -l ${TEMPDIR}/mergecats_$$ | ${P_GAWK} '{print $1}'`

  if [ ${NPARA} -gt ${NCATS} ]; then
    NPARA=${NCATS}
  fi

  PROC=0
  while [ ${PROC} -lt ${NPARA} ]
  do
    NCATS[${PROC}]=0
    CATS[${PROC}]=""
    PROC=$(( ${PROC} + 1 ))
  done

  i=1
  while read CAT
  do
    PROC=$(( $i % ${NPARA} ))
    CATS[${PROC}]="${CATS[${PROC}]} ${CAT}"
    NCATS[${PROC}]=$(( ${NCATS[${PROC}]} + 1 ))
    i=$(( $i + 1 ))
  done < ${TEMPDIR}/mergecats_$$

  PROC=0
  while [ ${PROC} -lt ${NPARA} ]
  do
    j=$(( ${PROC} + 1 ))
    echo -e "Starting Job $j. It has ${NCATS[${PROC}]} catalogues to process!\n"
    {
      for CAT in ${CATS[${PROC}]}
      do
        # The following ldacfilter is necessary because a
        # corresponding cut was applied when running the KSB
        # algorithm (TO BE CHANGED because we would like to
        # keep all objects!)
        ${P_LDACFILTER} -i ${CAT} -o ${TEMPDIR}/tmp1_${j}_$$.cat\
                        -t LDAC_OBJECTS\
                        -c "(FLUX_RADIUS>0.0)AND(FLUX_RADIUS<10.0);"
        BASE=`basename ${CAT} .cat`
        if [ -f "/$1/$2/$4/${BASE}_ksb_tmp.cat2" ]; then
          ${P_LDACRENTAB} -i /$1/$2/$4/${BASE}_ksb_tmp.cat2 \
           	            -o ${TEMPDIR}/tmp_${j}_$$.cat \
                          -t OBJECTS LDAC_OBJECTS
          ${P_LDACJOINKEY} -i ${TEMPDIR}/tmp1_${j}_$$.cat \
                           -o /$1/$2/$4/${BASE}.cat \
      	             -p ${TEMPDIR}/tmp_${j}_$$.cat \
      	             -t LDAC_OBJECTS \
                           -k e1 e2 rh cl snratio
          rm ${TEMPDIR}/tmp_${j}_$$.cat ${TEMPDIR}/tmp1_${j}_$$.cat
        else
          # if we have no lensing information we add dummy
          # information to have consistent catalogues
          ${P_LDACADDKEY} -i ${TEMPDIR}/tmp1_${j}_$$.cat \
                          -o /$1/$2/$4/${BASE}.cat \
                          -t LDAC_OBJECTS \
                          -k e1 -2.0 FLOAT "dummy value" \
                             e2 -2.0 FLOAT "dummy value" \
                             rh 0.0 FLOAT "dummy value" \
                             cl 0 SHORT "dummy value" \
                             snratio 0 FLOAT "dummy value"
          rm ${TEMPDIR}/tmp1_${j}_$$.cat
        fi
      done
    } &
    PROC=$(( ${PROC} + 1 ))
  done

  # wait for all processes to end:
  wait;
else
  theli_error "No catalogues to process! Nothing to do!"
  cleanTmpFiles
  exit 1;
fi

# clean up and bye:
cleanTmpFiles
theli_end
exit 0;
