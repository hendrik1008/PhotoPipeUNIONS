#!/bin/bash -u

# this script creates a STATS table from the catalogs created with
# 'create_astromcats_weights_para.sh' and, if present the headers
# created by ASTROMETRIX and PHOTOMETRIX. Similarily, the output
# of 'scamp' is treated if the astrometric/relative photometric
# calibration was done with that program.
#

# The STATS table contains 'global' information on the exposure level
# (e.g. seeing. zeropoints, sky background), i.e. condensed
# information from all the chips. Typically corresponding quantities
# from individual chips are averaged for the STATS table.

# HISTORY:
#
# 31.03.2011:
# I completely rewrote the old code. The new version creates a catalogue
# with only a 'STATS' table. The old script also contained the complete
# object catalogues from all contributing set images. It turned out that
# we never used this and for several surveys this produced unnecessarily
# HUGE catalogues.
#
# 27.04.2011:
# - Bug fix in the determination of present exposures.
# - The Filter name is added to the STATS table (as the catalogue containing
#   the STATS table no longer contains a FIELDS table the FILTER info would
#   not be available otherwise for header update).
#
#
# 08.09.2011:
# Bug Fix: The addition of the filter name was not done correctly (change
#          from 27.04.2011)
#
# 06.01.2012:
# I shifted AIRMASS to the keys that are included into the statistics tabel
# always. Before it was only added together with absolute photometric
# calibration information.
#
# 27.01.2012:
# LENSINGINFO needed to be initialised to 0
#
# 18.02.2012:
# Bug fix concerning TEMPDIR (a catalogue was read from the wrong place).
#
# 09.01.2013:
# I made the script more robust to missing files
#
# 18.01.2013:
# Some typos in the tmpfile removal function fixed.
#
# 31.10.2013:
# I added information on the 'scamp group' if this information is available
# in created 'headers'. This info is important for estimating photometric
# zeropoints lateron. Groups indicate 'isolated' groups of images and hence
# separate zeropoints need to be estimated for each such group.
#
# 12.02.2014:
# Extracting infos on relative photometyric zeropoints and photometric
# groups implicitely assumed that a scamp header from the first chip is
# present. However this could be a bad chip. I fixed this by taking the
# 'first header' that is found.
#
# 02.06.2016:
# I included the number of good chips (key GOODCCDS) in the statistics table.
#
# 28.03.2018:
# Some security if-statements added 8existing catalogue)

#
#$1: main directory
#$2: science directory
#$3: extension of the images for which
#    catalogs have been created (OFCSFF etc.)
#$4: headers directory (subdirectory of /$1/$2; OPTIONAL)

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# define THELI_DEBUG because of the '-u' script flag (the use of
# undefined variables will be treated as errors!)  THELI_DEBUG is used
# in the cleanTmpFiles function.
#
THELI_DEBUG=${THELI_DEBUG:-""}

function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"
    test -f ${TEMPDIR}/allfiles.txt_$$ && rm ${TEMPDIR}/allfiles.txt_$$
    test -f ${TEMPDIR}/e1e2.asc_$$ && rm ${TEMPDIR}/e1e2.asc_$$
    test -f ${TEMPDIR}/filt.cat_$$ && rm ${TEMPDIR}/filt.cat_$$
    test -f ${TEMPDIR}/table.dat_$$ && rm ${TEMPDIR}/table.dat_$$
    test -f ${TEMPDIR}/asctoldac.conf_$$ && rm ${TEMPDIR}/asctoldac.conf_$$
    test -f ${TEMPDIR}/statstable.txt_$$ && rm ${TEMPDIR}/statstable.txt_$$
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles; exit 1" INT

# For the very unlikely case that ${TEMPDIR}/statstable.txt_$$ already
# exists the script results would be nonsense:
test -f ${TEMPDIR}/statstable.txt_$$ && rm  ${TEMPDIR}/statstable.txt_$$

CATDIR=/$1/$2/cat
IMENDING=$3

HEADERDIR=""
if [ $# -eq 4 ]; then
  HEADERDIR=/$1/$2/$4
fi

# obtain a list of the present exposures:
${P_FIND} ${CATDIR} -maxdepth 1 -name \*${IMENDING}.cat |\
    ${P_GAWK} -F/ '{print $NF}' > \
    ${TEMPDIR}/allfiles.txt_$$

# do we need to do something at all?
if [ -s ${TEMPDIR}/allfiles.txt_$$ ]; then
  EXPOSURES=`sed -e 's/_[0-9]\+'${IMENDING}'.cat//' \
      ${TEMPDIR}/allfiles.txt_$$ | sort | uniq`

  # go exposure by exposure to build up the statistics table:
  KEYTEST=0 # was a test which keys are available already performed?
  ABSPHOTINFO=0
  RELPHOTINFO=0
  PHOTGROUPINFO=0 # scamp creates different 'groups' for isolated images
  LENSINGINFO=0
  KEYSTRING="OBJECT SEXBKGND SEXSFWHM GABODSID EXPTIME CRVAL1 CRVAL2"
  KEYSTRING="${KEYSTRING} FILTER AIRMASS"

  for EXPOSURE in ${EXPOSURES}
  do
    ALLCATS=`ls ${CATDIR}/${EXPOSURE}*${IMENDING}.cat`
    GOODCATS=""
    NGOODCATS=0  # number of good chips for the current exposure

    for CAT in ${ALLCATS}
    do
      BADCCD=`${P_LDACTOASC} -i ${CAT} \
                -t LDAC_IMHEAD -s | fold |\
                grep BADCCD | awk '{print $3}'`

      if [ "${BADCCD}" != "1" ]; then
        GOODCATS="${GOODCATS} ${CAT}"
        NGOODCATS=$(( ${NGOODCATS} + 1 ))
      fi
    done

    ${P_LDACCONV} -i ${GOODCATS} -o ${CATDIR}/${EXPOSURE}.cat \
        -c ${INSTRUMENT} -f dummy -b 1

    if [ -s ${CATDIR}/${EXPOSURE}.cat ]; then
      if [ ${KEYTEST} -eq 0 ]; then
        ${P_LDACTESTEXIST} -i ${CATDIR}/${EXPOSURE}.cat -t FIELDS -k ZPCHOICE

        if [ $? -eq 0 ]; then
          ABSPHOTINFO=1
          KEYSTRING="${KEYSTRING}
                     ZPCHOICE ZP COEFF ZP1 COEFF1 ZP2 COEFF2 ZP3 COEFF3"
        fi

        # see whether we have info on relative photometric calibration:
        # The following two lines ensure that we access a scamp header
        # that does exist (the first chip e.g. might be bad)
        CAT=`echo ${ALLCATS} | ${P_GAWK} '{print $1}'`
        BASE=`basename ${CAT} ${IMENDING}.cat`

        if [ -f ${HEADERDIR}/${BASE}.head ]; then
          RELPHOTINFO=`${P_GAWK} 'BEGIN {FS = "[=/]"; relzp = 0} {
                            if ($1 ~ /^RZP/) {
                              relzp = ($2 ~ /[0-9]+\.[0-9]*/)
                            }} END {print relzp}' ${HEADERDIR}/${BASE}.head`

          # check whether we have 'group information':
          grep -q FGROUPNO ${HEADERDIR}/${BASE}.head

          if [ $? -eq 0 ]; then
            PHOTGROUPINFO=1
          fi
        fi

        # see whether we have ellipticity (lensing) information on objects:
        ${P_LDACTESTEXIST} -i ${CATDIR}/${EXPOSURE}.cat -t OBJECTS -k e1 e2

        if [ $? -eq 0 ]; then
          LENSINGINFO=1
        fi

        KEYTEST=1
      fi

      ${P_LDACTOASC} -b -s -i ${CATDIR}/${EXPOSURE}.cat \
                     -t FIELDS -k ${KEYSTRING} > ${TEMPDIR}/table.dat_$$
      OBJECT=`${P_GAWK}   'NR==1 {print $1}' ${TEMPDIR}/table.dat_$$`
      GABODSID=`${P_GAWK} 'NR==1 {print $4}' ${TEMPDIR}/table.dat_$$`
      EXPTIME=`${P_GAWK}  'NR==1 {print $5}' ${TEMPDIR}/table.dat_$$`
      RA=`${P_GAWK}       'NR==1 {print $6}' ${TEMPDIR}/table.dat_$$`
      DEC=`${P_GAWK}      'NR==1 {print $7}' ${TEMPDIR}/table.dat_$$`
      FILTER=`${P_GAWK}   'NR==1 {print $8}' ${TEMPDIR}/table.dat_$$`
      AIRMASS=`${P_GAWK}  'NR==1 {print $9}' ${TEMPDIR}/table.dat_$$`

      BACK=`${P_GAWK} 'BEGIN {m = 0; n = 0} {
            m = m + ($2 / $5); n = n + 1} END {print m / n} ' \
            ${TEMPDIR}/table.dat_$$`
      BACKSDEV=`${P_GAWK} 'BEGIN {s = 0; n = 0; m = 0} {
                s = s + ($2 * $2) / ($5 * $5); m = m + ($2 / $5);
                n = n + 1} END {m = m / n; print sqrt(s / n - m * m)}' \
                ${TEMPDIR}/table.dat_$$`
      SEEING=`${P_GAWK} 'BEGIN {m = 0; n = 0} {
              m = m + $3; n = n + 1} END {print m / n}' ${TEMPDIR}/table.dat_$$`
      SEEINGSDEV=`${P_GAWK} 'BEGIN {s = 0; n = 0; m = 0} {
                s = s + ($3 * $3); m = m + $ 3;
                n = n + 1} END {m = m / n; print sqrt(s / n - m * m)}' \
                ${TEMPDIR}/table.dat_$$`

      if [ ${ABSPHOTINFO} -eq 1 ]; then
        ZPCHOICE=`${P_GAWK}   'NR==1 {print $10}' ${TEMPDIR}/table.dat_$$`
        ZP=`${P_GAWK}         'NR==1 {print $11}' ${TEMPDIR}/table.dat_$$`
        COEFF=`${P_GAWK}      'NR==1 {print $12}' ${TEMPDIR}/table.dat_$$`
        ZP1=`${P_GAWK}        'NR==1 {print $13}' ${TEMPDIR}/table.dat_$$`
        COEFF1=`${P_GAWK}     'NR==1 {print $14}' ${TEMPDIR}/table.dat_$$`
        ZP2=`${P_GAWK}        'NR==1 {print $15}' ${TEMPDIR}/table.dat_$$`
        COEFF2=`${P_GAWK}     'NR==1 {print $16}' ${TEMPDIR}/table.dat_$$`
        ZP3=`${P_GAWK}        'NR==1 {print $17}' ${TEMPDIR}/table.dat_$$`
        COEFF3=`${P_GAWK}     'NR==1 {print $18}' ${TEMPDIR}/table.dat_$$`
      fi

      if [ ${RELPHOTINFO} -eq 1 ]; then
        CAT=`echo ${ALLCATS} | ${P_GAWK} '{print $1}'`
        BASE=`basename ${CAT} ${IMENDING}.cat`

        RZP=`${P_GAWK} 'BEGIN {FS = "[=/]"} {
                        if ($1 ~ /^RZP/) {
                           print $2
                        }}' ${HEADERDIR}/${BASE}.head`
      fi

      if [ ${PHOTGROUPINFO} -eq 1 ]; then
        CAT=`echo ${ALLCATS} | ${P_GAWK} '{print $1}'`
        BASE=`basename ${CAT} ${IMENDING}.cat`

        PHOTGROUP=`${P_GAWK} 'BEGIN {FS = "[=/]"} {
                        if ($1 ~ /^FGROUPNO/) {
                           print $2
                        }}' ${HEADERDIR}/${BASE}.head`
      fi

      if [ ${LENSINGINFO} -eq 1 ]; then
        ${P_LDACFILTER} -i ${CATDIR}/${EXPOSURE}.cat -o ${TEMPDIR}/filt.cat_$$\
                        -t OBJECTS -c "(cl=2);"

        ${P_LDACTOASC} -b -i ${TEMPDIR}/filt.cat_$$ -t OBJECTS -k e1 e2 > \
            ${TEMPDIR}/e1e2.asc_$$

        E1=`${P_GAWK} 'BEGIN {m = 0; n = 0} {
                m = m + $1; n = n + 1} END {print m / n}' \
            ${TEMPDIR}/e1e2.asc_$$`
        E1SDEV=`${P_GAWK} 'BEGIN {s = 0; n = 0; m = 0} {
                s = s + ($1 * $1); m = m + $ 1;
                n = n + 1} END {m = m / n; print sqrt(s / n - m * m)}' \
                ${TEMPDIR}/e1e2.asc_$$`
        E2=`${P_GAWK} 'BEGIN {m = 0; n = 0} {
                m = m + $2; n = n + 1} END {print m / n}' \
            ${TEMPDIR}/e1e2.asc_$$`
        E2SDEV=`${P_GAWK} 'BEGIN {s = 0; n = 0; m = 0} {
                s = s + ($2 * $2); m = m + $ 2;
                n = n + 1} END {m = m / n; print sqrt(s / n - m * m)}' \
                ${TEMPDIR}/e1e2.asc_$$`

      fi

      EXPLINE="${EXPOSURE} ${OBJECT} ${EXPTIME} ${GABODSID} ${SEEING}"
      EXPLINE="${EXPLINE} ${SEEINGSDEV} ${BACK} ${BACKSDEV} ${RA} ${DEC}"
      EXPLINE="${EXPLINE} ${FILTER} ${AIRMASS} ${NGOODCATS}"

      if [ ${ABSPHOTINFO} -eq 1 ]
      then
        EXPLINE="${EXPLINE} ${ZPCHOICE} ${ZP} ${COEFF} ${ZP1} ${COEFF1}"
        EXPLINE="${EXPLINE} ${ZP2} ${COEFF2} ${ZP3} ${COEFF3}"
      fi

      if [ ${RELPHOTINFO} -eq 1 ]
      then
        EXPLINE="${EXPLINE} ${RZP}"
      fi

      if [ ${PHOTGROUPINFO} -eq 1 ]
      then
        EXPLINE="${EXPLINE} ${PHOTGROUP}"
      fi

      if [ ${LENSINGINFO} -eq 1 ]
      then
        EXPLINE="${EXPLINE} ${E1} ${E1SDEV} ${E2} ${E2SDEV}"
      fi

      echo ${EXPLINE} >> ${TEMPDIR}/statstable.txt_$$
    fi # [ -s ${CATDIR}/${EXPOSURE}.cat ]
  done

  {
    echo 'COL_NAME  = IMAGENAME'
    echo 'COL_TTYPE = STRING'
    echo 'COL_HTYPE = STRING'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 128'
    echo 'COL_NAME  = OBJECT'
    echo 'COL_TTYPE = STRING'
    echo 'COL_HTYPE = STRING'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 128'
    echo 'COL_NAME  = EXPTIME'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = GABODSID'
    echo 'COL_TTYPE = SHORT'
    echo 'COL_HTYPE = INT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = SEEING'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = SEEING_SDEV'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = BACKGR'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = BACKGR_SDEV'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = RA'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = DEC'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = FILTER'
    echo 'COL_TTYPE = STRING'
    echo 'COL_HTYPE = STRING'
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 128'
    echo 'COL_NAME  = AIRMASS'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = "airmass for this exposure"'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
    echo 'COL_NAME  = GOODCCDS'
    echo 'COL_TTYPE = SHORT'
    echo 'COL_HTYPE = INT'
    echo 'COL_COMM = "number of good chips in this exposure"'
    echo 'COL_UNIT = ""'
    echo 'COL_DEPTH = 1'
  } > ${TEMPDIR}/asctoldac.conf_$$

  if [ "${ABSPHOTINFO}" -eq 1 ]; then
    {
      echo 'COL_NAME  = ZPCHOICE'
      echo 'COL_TTYPE = SHORT'
      echo 'COL_HTYPE = INT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = ZP'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = COEFF'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = ZP1'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = COEFF1'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = ZP2'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = COEFF2'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = ZP3'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = COEFF3'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
    } >> ${TEMPDIR}/asctoldac.conf_$$
  fi

  if [ "${RELPHOTINFO}" -eq 1 ]; then
    {
      echo 'COL_NAME  = RZP'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
    } >> ${TEMPDIR}/asctoldac.conf_$$
  fi

  if [ "${PHOTGROUPINFO}" -eq 1 ]; then
    {
      echo 'COL_NAME  = PHOTGROUP'
      echo 'COL_TTYPE = SHORT'
      echo 'COL_HTYPE = INT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
    } >> ${TEMPDIR}/asctoldac.conf_$$
  fi

  if [ "${LENSINGINFO}" -eq 1 ]; then
    {
      echo 'COL_NAME  = e1'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = e1_SDEV'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = e2'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
      echo 'COL_NAME  = e2_SDEV'
      echo 'COL_TTYPE = FLOAT'
      echo 'COL_HTYPE = FLOAT'
      echo 'COL_COMM = ""'
      echo 'COL_UNIT = ""'
      echo 'COL_DEPTH = 1'
    } >> ${TEMPDIR}/asctoldac.conf_$$
  fi

  ${P_ASCTOLDAC} -i ${TEMPDIR}/statstable.txt_$$ \
                 -c ${TEMPDIR}/asctoldac.conf_$$ \
                 -o ${CATDIR}/chips.cat5 -t STATS

  # for compatibility reasons create a dummy 'chips.cat6' catalogue:
  test -f ${CATDIR}/chips.cat6 ||\
       ln -s ${CATDIR}/chips.cat5 ${CATDIR}/chips.cat6
else
  theli_error "No catalogues to process!"
  cleanTmpFiles
  exit 1;
fi

# clean up and bye:
cleanTmpFiles
theli_end
exit 0;
