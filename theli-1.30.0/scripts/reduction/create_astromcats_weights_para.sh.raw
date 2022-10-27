#!/bin/bash -u

# the script creates catalogs used for astrometry with the
# ASTROMETRIX or SCAMP tools. To create clean catalogs we can use
# WEIGHT maps. We run SExtractor two times: The first run to
# determine a reasonable value for the image seeing that
# is used in the second run.

#
# 28.04.2006:
# - I commented the creation of 'clean' catalogs (all objects with
#   a SExtractor flag larger than 1 filtered from the raw detections).
#   Those catalogs are currently not used within the pipeline
# - I removed the errorneous removal of a temporary catalogue
#
# 29.09.2006:
# CCDs which are marked as BAD (keyword BADCCD) are not included
# in the catalogue extraction process.
#
# 01.08.2007:
# some cleaning of not needed temporary files added
#
# 07.10.2008:
# I introduced the possibility to extract objects with different
# detection threshhold/minarea parameters. The distinction is done
# according to exposure time; new command line arguments for default
# threshhold/minarea (always the same) and exptime/threshhold
# combinations.
#
# 15.07.2009:
# - I introduced a new. mandatory command line argument which specifies
#   the name of the catalogue subdirectory to be used for catalogues.
# - If we give NONE for the WEIGHTS directory, no weights are used
#   for object extraction.
#
# 02.02.2010:
# I introduced the possibility to take into account flag images.
#
# 31.12.2012:
# I made the script more robust against non-existing files/images
#
# 07.01.2013:
# I corrected a bug in the usage of the 'FWHM' variable. It was written
# 'fwhm' in one location.

#$1: main directory
#$2: science dir.
#$3: image extension (ext) on ..._iext.fits (i is the chip number)
#$4: cats directory
#$5: WEIGHTS/FLAGS directory (give some dummy value if neither
#    weights, nor flags are available)
#$6: image extension for weight images (give NONE if weights
#    should not be considered)
#$7: image extension for flag images (give NONE if flags
#    should not be considered)
#$8: default threshhold
#    used for all images having larger exposure time
#    than the last exptime/threshold combination below
#$9: exposure time (1)              (OPTIONAL)
#    images having exptime smaller than $8 will be
#    extracted with threshold $9
#$10: detect threshold / minarea (1) (OPTIONAL)
#$11: exposure time (2)              (OPTIONAL)
#     objects with exptime between $8 and $10 will be
#     extracted with threshold $11
#$12: detect threshold / minarea (2) (OPTIONAL)
# .
# .
#${!#}: chips to work on

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

# define THELI_DEBUG because of the '-u' script flag (the use of
# undefined variables will be treated as errors!)  THELI_DEBUG is used
# in the cleanTmpFiles function.
#
THELI_DEBUG=${THELI_DEBUG:-""}
function cleanTmpFiles
{
  if [ -z ${THELI_DEBUG} ]; then
    echo "Cleaning temporary files for script $0"
    test -f ${TEMPDIR}/astromimages_$$ && rm ${TEMPDIR}/astromimages_$$
    test -f ${TEMPDIR}/singleastrom.param.sex_$$ &&\
         rm ${TEMPDIR}/singleastrom.param.sex_$$
  else
    echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
  fi
}

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles; exit 1" INT

# we need to modify the SExtractor param file if we
# include FLAG image information
SEXPARAMFILE=${DATACONF}/singleastrom.param.sex

if [ "$7" != "NONE" ]; then
  # append FLAG image info to the SExtratcor parameters:
  cp ${SEXPARAMFILE} ${TEMPDIR}/singleastrom.param.sex_$$
  {
    echo "IMAFLAGS_ISO"
    echo "NIMAFLAGS_ISO"
  } >> ${TEMPDIR}/singleastrom.param.sex_$$

  SEXPARAMFILE=${TEMPDIR}/singleastrom.param.sex_$$
fi

for CHIP in ${!#}
do
  ${P_FIND} /$1/$2/ -maxdepth 1 \
            -name \*_${CHIP}$3.fits > ${TEMPDIR}/astromimages_$$

  if [ -s ${TEMPDIR}/astromimages_$$ ]; then
    # create catalogue directory if it does not yet exist:
    test -d /$1/$2/$4 || mkdir /$1/$2/$4

    while read FILE
    do
      # check for BADCCD; if an image has a BADCCD mark of '1' it is
      # NOT included in the catalogue extraction process
      BADCCD=`${P_DFITS} ${FILE} |\
              ${P_FITSORT} BADCCD |\
              ${P_GAWK} '($1!="FILE") {print $2}'`

      if [ "${BADCCD}" != "1" ]; then
        BASE=`basename ${FILE} .fits`
        WBASE=`basename ${FILE} $3.fits`
        #
        # now run sextractor to determine the seeing:

        # see if we have weight and/or flag images:
        WEIGHTSTR="-WEIGHT_TYPE NONE"
        if [ "$6" != "NONE" ]; then
          WEIGHTSTR="-WEIGHT_IMAGE /$1/$5/${WBASE}$6.fits -WEIGHT_TYPE MAP_WEIGHT"
        fi

        FLAGSTR=""
        if [ "$7" != "NONE" ]; then
          FLAGSTR="-FLAG_IMAGE /$1/$5/${WBASE}$7.fits -FLAG_TYPE MAX"
        fi

        ${P_SEX} ${FILE} -c ${DATACONF}/singleastrom.conf.sex \
                         -PARAMETERS_NAME ${SEXPARAMFILE} \
                         -CATALOG_NAME ${TEMPDIR}/seeing_$$.cat \
                         -FILTER_NAME ${DATACONF}/default.conv\
                         -CATALOG_TYPE "ASCII" \
                         -DETECT_MINAREA 5 \
                         -DETECT_THRESH 5.\
                         -ANALYSIS_THRESH 1.2 \
                         -PARAMETERS_NAME \
                         ${DATACONF}/singleastrom.ascii.param.sex\
                         ${WEIGHTSTR} ${FLAGSTR}

        NLINES=`wc ${TEMPDIR}/seeing_$$.cat | ${P_GAWK} '{print $1}'`
        FWHM=`${P_GAWK} 'BEGIN { binsize = 10. / '${NLINES}';
                               nbins = int(((3.0 - 0.3) / binsize) + 0.5);
                               for (i = 1; i <= nbins; i++) bin[i]=0
                         } {
                           if(($3 * '${PIXSCALE}' > 0.3) && \
                              ($3 * '${PIXSCALE}' < 3.0)) {
                               actubin=int(($3*'${PIXSCALE}' - 0.3) / binsize);
                               bin[actubin] += 1;
                           }
                         } # end of main loop
                         END { max = 0; k = 0
                             for(i = 1; i <= nbins; i++) {
                               if(bin[i] > max) {
                                 max = bin[i];
                                 k = i;
                               }
                             }
                             print 0.3 + k * binsize
                             }' ${TEMPDIR}/seeing_$$.cat`

        if [ "A${FWHM}" = "A0.0" ]; then
          FWHM=1.0
        fi

        THRESH=$8
        MINAREA=$8

        i=9
        j=10
        if [ $# -gt 9 ]; then
          # get exposure time and determine detection thressholds to
          # use:
          EXPTIME=`${P_DFITS} ${FILE} |\
                   ${P_FITSORT} EXPTIME |\
                   ${P_GAWK} '($1!="FILE") {print int($2)}'`

          while [ $i -lt $# ]
          do
            EXPTIMECURR=`echo ${!i} | awk '{print int($1)}'`
            if [ ${EXPTIME} -lt ${EXPTIMECURR} ]; then
              THRESH=${!j}
              MINAREA=${!j}
            fi
            i=$(( $i + 2 ))
            j=$(( $j + 2 ))
          done
        fi

        # now run sextractor to extract the objects.  The setting of
        # MAG_ZEROPOINT=0.0 is arbitrary.
        ${P_SEX} ${FILE} -c ${DATACONF}/singleastrom.conf.sex\
                         -PARAMETERS_NAME ${SEXPARAMFILE} \
                         -CATALOG_NAME /$1/$2/$4/${BASE}.cat\
                         -SEEING_FWHM ${FWHM} \
                         -DETECT_MINAREA ${THRESH} \
                         -DETECT_THRESH ${THRESH}\
                         -MAG_ZEROPOINT 0.0 \
                         ${WEIGHTSTR} ${FLAGSTR}

        test -f ${TEMPDIR}/seeing_$$.cat && rm ${TEMPDIR}/seeing_$$.cat
      fi
    done < ${TEMPDIR}/astromimages_$$
  else
    theli_error "No images to process for Chip ${CHIP}!; Exiting!" "${!#}"
    cleanTmpFiles
    exit 1;
  fi
done

# clean up and bye:
cleanTmpFiles
theli_end "${!#}"
exit 0;
