#!/bin/bash -u

# this script performs resampling of data with swarp preparing the
# final coaddition. It can perform its task in parallel mode.

# 07.11.2003:
# I corrected a bug in the listing of files belonging to
# a certain chip. The old line
# FILES=`ls $1/$2/coadd_$4/*_${CHIP}*$3.fits` did not work
# correctly for more than 10 chips.
#
# 25.11.2003:
# the location of the coadd.head file is now given as parameter
# (necessary for reductions on marvin where the reduction directories
# of the individual nodes is not the directory where the
# prepare_coadd_swarp script created it)
#
# 02.03.2004:
# The ending of the resampled FITS images is now
# "COADIDENT".resamp.fits instead of simply .resamp.fits
#
# 08.06.2006
# The file 'coadd.head' is no longer removed at the end of the
# processing. If being unlucky it is needed by another process exactly
# at the moment when it is removed by this script. It does not matter
# if it is not removed anyway.
#
# 25.07.2006:
# The 'coadd.head' file stored in the reduce directory gets a unique
# name consisting of science dir. and co-addition identifier. This
# allows the execution of several co-additions simultaneously.
#
# 23.09.2006:
# The filenames of images to be resampled are passed to swarp no
# longer on the command line but in a file instead.
#
# 31.10.2006:
# I corrected a major bug introduced in the changes of 23.09.2006!
# (the script was not functional!)
#
# 21.03.2007:
# I included a test to check whether chips need to be co-added
# at all. This cleanly treats cases where only individual chips
# of a mosaic should be co-added.
#
# 08.02.2008:
# I merged this script with 'resample_coadd_swarp_CFHTLS_para.sh'.
# A new command line argument determines if the resampled files
# are put where the unresampled files reside (links!) or whether
# they are always created in the 'coadd' directory.
#
# 03.10.2008:
# I add a saturation level keyword to the resampled keywords
# (transfer from the not-resampled ones - if present)
#
# 12.10.2008:
# I make sure that the SATLEVEL keyword is always a float with two
# digits
#
# 16.09.2010:
# I made the construction of the filename for the coadd.head file in
# the reduce directory more robust. There were problems if a pathname
# ended with a slash.
#
# 26.11.2010:
# - I adapted the call to swarp to the syntax of the new version (2.19.1).
#   This includes a different way to provide the image list and the
#   name of a header keyword for the saturation level.
# - Bug fix in the treatment of the SATLEVEL keyword. If it is not present
#   its value was errorneously treated as zero.
#
# 05.05.2011:
# There is now a different 'coadd_$$.head' file for each process of
# this script. Before there was only one coadd.head file. Instances
# of this script checked whether this file existed and if not, copied it.
# It seems that this lead to inconsistencies when several instances
# queried the existence simultaneously. This might be the reason for
# completely faulty coadditions in rare cases.
#
# 07.02.2013:
# I introduced the new THELI logging scheme into the script.
#
# 12.02.2013:
# If there are no images from a chip that should be resampled a
# warning is issued now.

#$1: main dir.
#$2: science dir.
#$3: image extension (ext) on ..._iext.fits (i is the chip number)
#$4: coaddition identifier
#$5: location of the global coadd.head file (complete absolute path)
#$6: FOLLOWLINK/NOFOLLOWLINK; If set to FOLLOWLINK, links from
#    the files to be co-added are followed and the resampled files
#    are put where the linked files are. Otherwise the resampled
#    files stay in the 'coadd' directory.

# preliminary work:
. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# construct a unique name for the coadd.head file
# of this co-addition:
#
# The following 'sed' ensures a 'unique' construction with the
# '/' character which can appear in arbitrary combinations in
# file- and pathnames.
TMPNAME_1=`echo ${1} | sed -e 's!/\{2,\}!/!g' -e 's!/*$!!' |\
           ${P_GAWK} -F/ '{print $NF}'`
TMPNAME_2=`echo ${2} | sed -e 's!/\{2,\}!/!g' -e 's!/*$!!' |\
           ${P_GAWK} -F/ '{print $NF}'`
COADDFILENAME=${TMPNAME_1}_${TMPNAME_2}_${4}

for CHIP in ${!#}
do
  RESULTDIR[${CHIP}]="/$1/$2/coadd_$4"
done

for CHIP in ${!#}
do
  ${P_FIND} /$1/$2/coadd_$4/ -maxdepth 1 \
            -name \*_${CHIP}$3.fits > ${TEMPDIR}/files_$$.list

  # test whether we have something to do at all! In case that only individual
  # chips need to be co-added it may be that the preceeding find command
  # returns an empty file.
  if [ -s ${TEMPDIR}/files_$$.list ]; then
    if [ "$6" = "FOLLOWLINK" ]; then
      while read FILE
      do
        LINK=`${P_READLINK} ${FILE}`
        if [ -L ${LINK} ]; then
          LINKREAL=`${P_READLINK} ${LINK}`
          BASE=`basename ${LINKREAL} .fits`
          DIR=`dirname ${LINKREAL}`
          ln -s ${DIR}/${BASE}.$4.resamp.fits \
                $1/$2/coadd_$4/${BASE}.$4.resamp.fits
          ln -s ${DIR}/${BASE}.$4.resamp.weight.fits \
                $1/$2/coadd_$4/${BASE}.$4.resamp.weight.fits
          RESULTDIR[${CHIP}]=`dirname ${LINKREAL}`
        fi
      done < ${TEMPDIR}/files_$$.list
    fi

    DIR=`pwd`

    # necessary if TEMPDIR is '.':
    cp ${TEMPDIR}/files_$$.list ${RESULTDIR[${CHIP}]}

    cd ${RESULTDIR[${CHIP}]}

    if [ ! -f coadd_$$.head ]; then
      if [ ! -s $5/coadd_${COADDFILENAME}.head ]; then
        theli_error "I need $5/coadd_${COADDFILENAME}.head to continue!"
        theli_error "Did you run 'prepare_coadd_swarp.sh'?"
        exit 1;
      else
        cp $5/coadd_${COADDFILENAME}.head ./coadd_$$.head
      fi
    fi

    # resample images:
    ${P_SWARP} -c ${DATACONF}/create_coadd_swarp.swarp \
               -IMAGEOUT_NAME coadd_$$.fits \
               -WEIGHTOUT_NAME coadd_$$.weight.fits \
               -RESAMPLE Y -COMBINE N \
               -RESAMPLE_SUFFIX .$4.resamp.fits \
               -RESAMPLE_DIR . \
               @./files_$$.list\
               -SATLEV_KEYWORD SATLEVEL \
               -NTHREADS 1 -VERBOSE_TYPE QUIET

    test -f ./files_$$.list && rm ./files_$$.list

    cd ${DIR}

    # deal with saturation level keyword if necessary:
    while read FILE
    do
      BASE=`basename ${FILE} .fits`
      IMAGEDIR=`dirname ${FILE}`

      SATLEVEL=`${P_DFITS} ${FILE} | ${P_FITSORT} -d SATLEVEL |\
                cut -f 2`

      if [ "${SATLEVEL}" != "KEY_N/A" ]; then
        value ${SATLEVEL}
        writekey ${IMAGEDIR}/${BASE}.$4.resamp.fits SATLEVEL \
             "${VALUE} / Saturation Level" REPLACE

        touch ${IMAGEDIR}/${BASE}.$4.resamp.fits
      fi
    done < ${TEMPDIR}/files_$$.list
  else
    theli_warn "No images to resample for chip ${CHIP}!"
  fi

  # clean up
  test -f ${TEMPDIR}/files_$$.list && rm ${TEMPDIR}/files_$$.list
done

theli_end "${!#}"
exit 0;
