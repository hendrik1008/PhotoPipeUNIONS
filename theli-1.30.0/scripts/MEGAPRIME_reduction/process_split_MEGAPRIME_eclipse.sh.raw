#!/bin/bash -u

# Splits Elixir processed CFHT Megaprime Fits extension images into
# the 36 (or 40) chips. Uses the eclipse utilities and also updates
# the image headers. Assumes that ONLY unsplit images are in the
# directory.
#
# If available, the script uses Skyprobe information to decide whether
# an image was obtained under photometric conditions and to transfer
# Elixir/QSO quality information to the image headers.  This is the
# case for CFHTLS data for instance.

# History comments:
#
# 20.07.06:
# I add Elixir/QSO quality information to the
# image headers.
#
# 25.06.2007:
# I merged the script with process_split_CFHTLS_eclipse.sh.
# That script split MEGAPRIME images and, in addition,
# transfered skyprobe information to the image headers.
# This task is done by this script if the optional,
# third command line argument is given. This argument
# points to an LDAC catalogue with skyprobe information
# on the images.
#
# 02.10.2008:
# We estimate a saturation level for Elixir processed images;
# we note that Elixir images carry a saturation level
# header keyword. However, this turned out to be useless
# (much too high values in many cases).
#
# 12.10.2008:
# I make sure that the SATLEVEL keyword is alwasys a float with
# two digits accuracy.
#
# 10.11.2008:
# I corrected the printing of SATLEVEL (two digits) at one more place
# in the script; if the estimated SATLEVEL is smaller than 50000 (for
# short exposures for instance) I set ot to 60000 by default.
#
# 25.11.2008:
# I add the civil date of observation (keyword DATE-OBS) to the
# headers. The date is given as 'local' time at the telescope, NOT
# as UT!
#
# 19.12.2008:
# - The program caldate is now called via a variable, not directly
# - For the u*-band filter we apply a zeropoint correction of 0.2
#   from 19/05/2006 - 28/09/2006
#
# 05.02.2009:
# We make sure that the seeing which is written to headers always
# is a float with two digits; we had problem with a seeing of exactly
# 1 arcsec which was stored as integer.
#
# 04.01.2010:
# The command line arguments now have to be given with a keyword.
# I introduced the possibility to give a second, optional argument.
# This did not permit anymore handling of command line arguments via
# simple positional variables. The new optional argument  is a file
# with image names, zeropoints and a 'is photometric' flag. It overrides
# photometric information in a photometric catalogue. This feature
# is probably only useful with the special Elixir data at Terapix!
#
# 26.03.2010:
# I make sure that the saturation files represent integer numbers (though
# given as float header keywords) as the input images come in Integer
# format.
#
# 28.09.2010:
# I made the script more efficient. Header entries are now treated within
# the mefsplit call whereever possible. The 'mefsplit' program was modified
# for that purpose. Also the estimation of saturation levels and their
# transfer to image headers was optimised.
#
# 28.11.2010:
# - I introduced an internal parallelisation to this script (simultaneous
#   splitting of files). Usefult e.g. on a RAMDISK
# - The header keyword GAIN is now transfered from the original images
#   to the splitted data.
#
# 16.12.2010:
# I made the construction of unique temporary filenames in subshells more
# robust. The old method of using BASHPID seems not to have worked properly!
#
# 19.04.2011:
# Possibility to use 'zeropoint files' provided by Jean-Charles to process
# 'non-official' Elixir processed images (used also for T07). The format
# of his files is 'imagename ZP ext.'; one image per line.
#
# 19.11.2012:
# adaption of the mefsplit call: The HEADTRANFER option now needs to
# specify the name of the output keyword.
#
# 21.03.2013:
# The Median Julian date (MJD-OBS) is now transfered to splitted
# images. It is needed for variability studies based on single frames.
#
# 10.06.2015:
# I significantly revised the script to also treat the new MEGAPRIME
# mode which operated the instrument with 40 chips!
#
# 20.06.2016:
# I introduced the '-d' command line option. If set, original, unsplit
# images are deleted after the splitting process.
#
# 04.05.2017:
# I removed the options yannickzp and jcczp. There is now one zpfile option.
# It gives a file listing zeropoint and extinction coefficients for images
# (identified by the Elixir basename). For images listed in that file,
# the zeropoint information in image headers is ignored.
#
# 23.10.2017:
# For Stephens pitcairn data we transfer the PHOTZP keyword to the splitted
# image headers.
#
# 09.11.2017:
# The RUN that should appear in the image headers can now be given on
# the command line. If it is not provided, we use the QRUNID keyword
# as before.
#
# 05.02.2018:
# I ensure that only split images enter statistics estimation for saturation
# level estimation. This makes the effects of this script less destructive
# if it is run unintentionally on directories which already contain
# processed data.

# -md : main directory
# -sd : science directory
# -cat: LDAC catalog with SkyProbe information (OPTIONAL)
# -zpfile: ASCII file with zeropoints and extinction coefficients;
#          If this option is given zeropoints and extinction coefficients
#          for listed files are taken from this list. Zeropoint infromation
#          in the headers is ignored in that case.

# Note that MEGAPRIME.ini is included here because we absolutely
# need some definitions from this file right now. Below we might
# overwrite these definitions with those from 'MEGAPRIME_FOURTYCHIP.ini'
# if we operate on new, 40-chip data:
. ./MEGAPRIME.ini
. ./bash_functions.include
theli_start "$*"

# On some systems we might consider to split data in parallel;
# e.g. on a RAMDISK. As the systems where this is possible without
# different mefsplit processes blocking each other we keep this
# functionality VERY internal!
NPROC=6

#
# parse command line arguments
#
MD=
SD=
PHOTCAT=
ZPFILE=
CLEANORIG=0 # do we delete original unsplit images after splitting
JCC=0
RUNPARAM="DEFAULT_RUN"
while [ $# -gt 0 ]
do
  case $1 in
  -d)
      CLEANORIG=1
      shift
      ;;
  -md)
      MD=${2}
      shift 2
      ;;
  -sd)
      SD=${2}
      shift 2
      ;;
  -r)
      RUNPARAM=${2}
      shift 2
      ;;
  -cat)
      PHOTCAT=${2}
      shift 2
      ;;
  -zpfile)
      ZPFILE=${2}
      shift 2
      ;;
   *)
      # there might be an 'empty string' argument which we
      # can ignore:
      if [ ! -z "$1" ]; then
        theli_error "Unknown command line option: ${1}"
        exit 1;
      else
        shift
      fi
      ;;
  esac
done

# sanity checks:
if [ -z ${MD} ] || [ -z ${SD} ]; then
  theli_error "You need to provide at least main directory and science directory!"
  exit 1;
fi

if [ ! -z ${PHOTCAT} ] && [ ! -f ${PHOTCAT} ]; then
  theli_error "Error reading ${PHOTCAT}. Exiting!"
  exit 1;
fi

if [ ! -z ${ZPFILE} ] && [ ! -f ${ZPFILE} ]; then
  theli_error "Error reading ${ZPFILE}. Exiting!"
  exit 1;
fi

# We first need to check whether we have the traditional 36 chip
# configuration or the new 40 chip configuration to deal with.
# We make the implicit assumption that ONLY data from one configuration
# are present!

# Just get the filter name from the first image present:
FILTER=`${P_FIND} /${MD}/${SD}/ORIGINALS/ -name \*p.fits | head -1 | \
   xargs ${P_DFITS} | ${P_FITSORT} -d FILTER | cut -f 2`

if [ "${FILTER}" = "" ]; then
  theli_error "Nothing to do! Exiting!"
  exit 1;
fi

if [ "${FILTER}" = "u.MP9302" ] || [ "${FILTER}" = "g.MP9402" ] || \
   [ "${FILTER}" = "r.MP9602" ] || [ "${FILTER}" = "i.MP9703" ] || \
   [ "${FILTER}" = "z.MP9901" ]; then
  . ./MEGAPRIME_FOURTYCHIP.ini
fi

# Create image list: we assume that ONLY unsplit images are in the
# directory.
if [ ! -z ${PHOTCAT} ]; then
  ${P_LDACTOASC} -b -i ${PHOTCAT} -t OBJECTS -s -k IMAGENAME PHOTOMETRIC \
                 > phot_$$.asc
  ${P_LDACTOASC} -b -i ${PHOTCAT} -t OBJECTS -s -k IMAGENAME QUALITY \
                 > qual_$$.asc

  # in cases where we 'only' have one seeing value for the whole mosaic,
  # we must fake the SEEINGCENTRE and SEEINGCORNER values with that,
  # one value:
  ${P_LDACTESTEXIST} -i ${PHOTCAT} -t OBJECTS -k SEEINGCENTRE

  if [ $? -eq 0 ]; then
    ${P_LDACTOASC} -b -i ${PHOTCAT} -t OBJECTS -s -k IMAGENAME SEEINGCENTRE \
                   > seeingcentre_$$.asc
    ${P_LDACTOASC} -b -i ${PHOTCAT} -t OBJECTS -s -k IMAGENAME SEEINGCORNER \
                   > seeingcorner_$$.asc
  else
    ${P_LDACTOASC} -b -i ${PHOTCAT} -t OBJECTS -s -k IMAGENAME SEEING \
                   > seeingcentre_$$.asc
    ${P_LDACTOASC} -b -i ${PHOTCAT} -t OBJECTS -s -k IMAGENAME SEEING \
                   > seeingcorner_$$.asc
  fi
fi

# build some convenience variables for the mefsplit calls below:
REFPIXX_VAR="${REFPIXX[1]}"
REFPIXY_VAR="${REFPIXY[1]}"
CD11_VAR="${CD11[1]}"
CD12_VAR="${CD12[1]}"
CD21_VAR="${CD21[1]}"
CD22_VAR="${CD22[1]}"
M11_VAR="${M11[1]}"
M12_VAR="${M12[1]}"
M21_VAR="${M21[1]}"
M22_VAR="${M22[1]}"
IMAGEID_VAR="${IMAGEID[1]}"

for i in `seq 2 ${NCHIPS}`
do
  REFPIXX_VAR="${REFPIXX_VAR},${REFPIXX[$i]}"
  REFPIXY_VAR="${REFPIXY_VAR},${REFPIXY[$i]}"
  CD11_VAR="${CD11_VAR},${CD11[$i]}"
  CD12_VAR="${CD12_VAR},${CD12[$i]}"
  CD21_VAR="${CD21_VAR},${CD21[$i]}"
  CD22_VAR="${CD22_VAR},${CD22[$i]}"
  M11_VAR="${M11_VAR},${M11[$i]}"
  M12_VAR="${M12_VAR},${M12[$i]}"
  M21_VAR="${M21_VAR},${M21[$i]}"
  M22_VAR="${M22_VAR},${M22[$i]}"
  IMAGEID_VAR="${IMAGEID_VAR},${IMAGEID[$i]}"
done


# split files 'in parallel':
DIR=`pwd`
FILES=`${P_FIND} /${MD}/${SD}/ORIGINALS/ -name \*p.fits`

i=0
while [ ${i} -lt ${NPROC} ]
do
  NFIELDS[${i}]=0
  FIELDS[${i}]=""
  i=$(( $i + 1 ))
done

i=1
for FIELD in ${FILES}
do
  PROC=$(( $i % ${NPROC} ))
  FIELDS[${PROC}]="${FIELDS[${PROC}]} ${FIELD}"
  NFIELDS[${PROC}]=$(( ${NFIELDS[${PROC}]} + 1 ))
  i=$(( $i + 1 ))
done

cd /${MD}/${SD}/ORIGINALS

IPARA=0
while [ ${IPARA} -lt ${NPROC} ]
do
  JPARA=$(( ${IPARA} + 1 ))
  echo -e "Starting Job ${JPARA}. It has ${NFIELDS[${IPARA}]} files to process!\n"
  (
    for FILE in ${FIELDS[${IPARA}]}
    do
      BASEELIXIR=`basename ${FILE} p.fits`

      # If a catalogue with Skyprobe information is given the image
      # under consideration HAS to be listed in order to be processed!!
      ISPRESENT=""

      if [ ! -z ${PHOTCAT} ]; then
        ISPRESENT=`${P_GAWK} '($1 == "'${BASEELIXIR}'") {print $1}' \
                   ${DIR}/phot_$$.asc`
      fi

      if [ "${ISPRESENT}_A" != "_A" ] || [ -z ${PHOTCAT} ]; then
        FILTNAM=`${P_DFITS} -x 1 ${FILE} | ${P_FITSORT} -d FILTER  | cut -f 2`
        OBJECT=`${P_DFITS} -x 1  ${FILE} | ${P_FITSORT} -s -d OBJECT  |\
                cut -f 2`
        RA=`${P_DFITS} -x 1  ${FILE} | ${P_FITSORT} -d RA_DEG  | cut -f 2`
        DEC=`${P_DFITS} -x 1 ${FILE} | ${P_FITSORT} -d DEC_DEG | cut -f 2`
        LST=`${P_DFITS} -x 1 ${FILE} | ${P_FITSORT} -d LST-OBS | cut -f 2 |\
             ${P_GAWK} -F: '{print $1 * 3600 + $2 * 60 + $3}'`
        MJD=`${P_DFITS} -x 1 ${FILE} | ${P_FITSORT} -d MJD-OBS  | cut -f 2`
        EXPTIME=`${P_DFITS} -x 1 ${FILE} | ${P_FITSORT} -d EXPTIME | cut -f 2`
        AIRMASS=`${P_AIRMASS} -t ${LST} -e ${EXPTIME} \
                              -r ${RA} -d ${DEC} -l ${OBSLAT}`
        GABODSID=`${P_NIGHTID} -t ${REFERENCETIME} -d 31/12/1998 -m ${MJD} |\
                  ${P_GAWK} ' ($1 ~ /Days/) {print $6}' |\
                  ${P_GAWK} 'BEGIN{ FS = "."} {print $1}'`

        # we recalculate the civil observation date from the GABODSID because
        # we want the local time at the telescope, NOT UT.
        DATEOBS=`${P_CALDATE} -d 31/12/1998 -i ${GABODSID} | cut -d ' ' -f 3`

        # RUN information
        if [ "${RUNPARAM}" = "DEFAULT_RUN" ]; then
          RUN=`${P_DFITS} ${FILE} | ${P_FITSORT} -d QRUNID  | cut -f 2`
        else
          RUN="${RUNPARAM}"
        fi

        # Photometric information; all images will be marked as photometric!!
        PHOTINFO=1 # by default we have photometric information; this may not
                   # be the case for special filters
        PHOT_C=`${P_DFITS} ${FILE} | ${P_FITSORT} -d PHOT_C  | cut -f 2`
        PHOT_K=`${P_DFITS} ${FILE} | ${P_FITSORT} -d PHOT_K  | cut -f 2`

        # check if we have special zero point files from Yannick or
        # Jean-Charles:
        if [ ! -z ${ZPFILE} ]; then
          grep ${BASEELIXIR} ${ZPFILE} > /dev/null

          if [ $? -eq 0 ]; then
            PHOT_C=`grep ${BASEELIXIR} ${ZPFILE} | awk 'END {print $2}'`
            PHOT_K=`grep ${BASEELIXIR} ${ZPFILE} | awk 'END {print $3}'`
          fi
        fi

        if [ "${PHOT_C}" != "KEY_N/A" ] && [ "${PHOT_K}" != "KEY_N/A" ]; then
          # our magnitude zeropoint (at airmass 0) is PHOT_C-PHOT_K
          ZP=`${P_GAWK} 'BEGIN {print '${PHOT_C}' - ('${PHOT_K}')}'`

          # If the filter is 'u.MP9301' and the observation between
          # 19.05.2006 and 28.09.2006 we subtract 0.2 from the CFHT zeropoint!
          # we do this only if zeropoints were not given directly by Yannick
          # or Jean-Charles:
          if [ -z ${ZPFILE} ]; then
            if [ "${FILTNAM}" = "u.MP9301" ]; then
              if [ ${GABODSID} -ge 2695 ] && [ ${GABODSID} -le 2827 ]; then
                ZP=`${P_GAWK} 'BEGIN {print '${ZP}' - 0.2}'`
              fi
            fi
          fi
        else
          PHOTINFO=0
        fi

        # collect some more info for image header keywords:
        HEADERSTRING="-HEADER RUN ${RUN} THELI_Run_ID"
        PHOTOMETRIC=1 # default for all images!

        # get Skyprobe info if available:
        if [ ! -z ${PHOTCAT} ]; then
          # get Elixir/QSO data and save it together with
          # photometric infomration,to the headers:
          PHOTOMETRIC=`${P_GAWK} '($1 == "'${BASEELIXIR}'") {print $2}' \
                       ${DIR}/phot_$$.asc`

          if [ "${PHOTOMETRIC}_A" = "_A" ]; then
            PHOTOMETRIC=0
          fi

          ELQUAL=`${P_GAWK} '($1 == "'${BASEELIXIR}'") {print int($2)}' \
                  ${DIR}/qual_$$.asc`

          if [ "${ELQUAL}_A" = "_A" ]; then
            ELQUAL=-1
          fi

          ELSEECEN=`${P_GAWK} '($1 == "'${BASEELIXIR}'") {printf("%.2f", $2)}' \
                    ${DIR}/seeingcentre_$$.asc`

          if [ "${ELSEECEN}_A" = "_A" ]; then
            ELSEECEN=-1.0
          fi

          ELSEECOR=`${P_GAWK} '($1 == "'${BASEELIXIR}'") {printf("%.2f", $2)}' \
                    ${DIR}/seeingcorner_$$.asc`

          if [ "${ELSEECOR}_A" = "_A" ]; then
            ELSEECOR=-1.0
          fi

          HEADERSTRING="${HEADERSTRING} -HEADER ELQUAL ${ELQUAL} Elixir_Quality_Flag"
          HEADERSTRING="${HEADERSTRING} -HEADER ELSEECEN ${ELSEECEN} Elixir_Seeing_in_Image_Center"
          HEADERSTRING="${HEADERSTRING} -HEADER ELSEECOR ${ELSEECOR} Elixir_Seeing_in_Image_Corners"
        fi

        if [ ${PHOTINFO} -eq 0 ]; then
          # if no photometric information is available we set the
          # zeropoints to '-1.0' and the extinction to '0.0'
          ZP=-1.0
          PHOT_K=0.0
          PHOTOMETRIC=0
        fi

        HEADERSTRING="${HEADERSTRING} -HEADER ZP ${ZP} Magnitude_Zero_Point"
        HEADERSTRING="${HEADERSTRING} -HEADER COEFF ${PHOT_K} Extinction_Coefficient"

        for i in `seq 1 3`
        do
          HEADERSTRING="${HEADERSTRING} -HEADER ZP${i} ${ZP} Magnitude_Zero_Point"
          HEADERSTRING="${HEADERSTRING} -HEADER COEFF${i} ${PHOT_K} Extinction_Coefficient"
        done
        HEADERSTRING="${HEADERSTRING} -HEADER ZPCHOICE ${PHOTOMETRIC} Is_Image_Photometric_(1=Yes)"
        HEADERSTRING="${HEADERSTRING} -HEADER BADCCD 0 Is_CCD_Bad_(1=Yes)"
        HEADERSTRING="${HEADERSTRING} -HEADER DATE-OBS ${DATEOBS} obs._date_(YYYY-MM-DD;_local_time_at_tel.)"

        # Note: The PHOTZP keyword, which is transfered to the splitted
        # headers is only present in pitcairn processed CFIS data from
        # Stephen Gwyn. For other Elicir data, the option just leads
        # to a warning.
        ${P_FITSSPLIT_ECL} \
           -CRPIX1 "${REFPIXX_VAR}"  -CRPIX2 "${REFPIXY_VAR}" \
           -CD11 "${CD11_VAR}"       -CD22 "${CD22_VAR}" \
           -CD12 "${CD12_VAR}"       -CD21 "${CD21_VAR}" \
           -M11 "${M11_VAR}"         -M22 "${M22_VAR}" \
           -M12 "${M12_VAR}"         -M21 "${M21_VAR}" \
           -CRVAL1 ${RA}             -CRVAL2 ${DEC} \
           -EXPTIME ${EXPTIME}       -AIRMASS ${AIRMASS} \
           -GABODSID ${GABODSID}     -FILTER ${FILTNAM}  \
           -IMAGEID "${IMAGEID_VAR}" -OBJECT "${OBJECT}" \
           -HEADTRANSFER GAIN GAIN   -HEADTRANSFER MJD-OBS MJD-OBS\
           -HEADTRANSFER PHOTZP PHOTZP \
           ${HEADERSTRING}           -OUTPUT_DIR .. ${FILE}

        # update image headers of unsplit images with the saturation level:
        # the following is optimised for speed, not for readability!

        # saturation level:
        # The saturation level given in Elixir processed images is often
        # too high. Instead of using the value in the headers
        # we estimate the mode of high pixel values and subtract 5000.
        # This turned out empirically to be a reasonable estimate.
        BASE=`basename ${FILE} .fits`

        # as we are in a subshell '$$' still contains the process ID from
        # the calling script. We therefore include the parallel channel
        # to construct unique names for temporary files.
        IDENTIFIER=job_${JPARA}_$$
        ${P_IMSTATS} -t 50000 100000 ../${BASE}_*[0-9].fits > \
            satlevels.txt_${IDENTIFIER}
        ${P_GAWK} '$1 !~ /#/ {
                image = $1;
                if ($2 > 50000) {
                  satlev = sprintf(" \"SATLEVEL= %20.2f / saturation level\"",
                                   int($2) - 5000.);
                } else {
                  satlev = sprintf(" \"SATLEVEL= %20.2f / saturation level\"",
                                   60000.00);
                }
                system("'${P_REPLACEKEY}' " image satlev " DUMMY1");
                }' satlevels.txt_${IDENTIFIER}
        rm satlevels.txt_${IDENTIFIER}

        # if the Elixir quality flag is '-1' or larger than 2 we move
        # the image immedialtely to a 'BADELQUAL' subdirectory.
        if [ ! -z ${PHOTCAT} ]; then
          if [ ${ELQUAL} -eq -1 ] || [ ${ELQUAL} -gt 2 ]; then
            if [ ! -d ../BADELQUAL ]; then
      	      mkdir ../BADELQUAL
            fi

            i=1
            while [ ${i} -le ${NCHIPS} ]
            do
      	      mv ../${BASE}_${i}.fits ../BADELQUAL
      	      i=$(( $i + 1 ))
            done
          fi
        fi

        # delete original file if asked for:
        if [ ${CLEANORIG} -eq 1 ]; then
          rm ${FILE}
        fi
      else
        if [ ! -z ${PHOTCAT} ]; then
          echo "No Skyprobe info for ${FILE}."
          echo "No splitting performed!!"
        fi
      fi
    done
  ) 2> ${DIR}/process_split_job_${IPARA}.log &
  IPARA=$(( ${IPARA} + 1 ))
done

wait

cd ${DIR}

# clean up!!
test -f phot_$$.asc         && rm phot_$$.asc
test -f qual_$$.asc         && rm qual_$$.asc
test -f seeingcentre_$$.asc && rm seeingcentre_$$.asc
test -f seeingcorner_$$.asc && rm seeingcorner_$$.asc

# and bye
theli_end

exit 0;
