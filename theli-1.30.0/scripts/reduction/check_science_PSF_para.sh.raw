#!/bin/bash -u

# the script runs analyseldac over the objects previously used in the
# singleastrom step.

# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 21.11.2006:
# - The script is more general and can be used also
#   in cases where a .cat1 catalogue is not yet present
#   (e.g. if we use only ASTROMETRIX for astrometric calibration
#   and hence do not create all intermediate catalogues
#   of the LDAC pipeline).
# - 'analyseldac' runs with the option '-p' which uses the SExtractor
#   centre for shape estimation instead of determining a new one.
#   For PSF checkplots this is good enough and it speeds up the
#   script tremendously.
#
# 16.03.2007:
# I delete some temporary files
#
# 01.08.2007:
# some more deletion of temporary files
#
# 16.07.2009:
# The name of the catalogue subdirectory now has to be provided as
# command line argument.
#
# 04.01.2013:
# I made the script more robust to missing files.
#
# 20.01.2013:
# Some temporary files were not deleted if no catalogues were
# to be processed.
#
# 21.03.2013:
# I again improved deletion of temporary files.
#
# 17.05.2013:
# A 'for' loop to clean up temporary files missed the 'do' command.
#
# 08.09.2013:
# I corrected a bug in the inclusion of 'bash_functions.include'.
#
# 14.06.2015:
# We saw for the new 40-chip setup of MEGAPRIME that the PSF
# there is so narrow that the default step size for preanisotopy
# (0.15 pixels) is too large there. We therefore make this step
# size an optional command line parameter.
#
# 20.05.2016:
# A temporary file was not put correctly into TEMPDIR - fixed.

# $1: main directory
# $2: science dir.
# $3: image extension (ext) on ..._iext.fits (i is the chip number)
# $4: catalogue directory
# $5: step size for size coordinate (OPTIONAL; default: 0.15)
# $#: chips to be processed

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*" "${!#}"

STEPSIZE=0.15
if [ $# -eq 6 ]; then
  STEPSIZE=$5
fi

for CHIP in ${!#}
do
  ls -1 /$1/$2/*_${CHIP}$3.fits > ${TEMPDIR}/psfimages_${CHIP}_$$

  if [ -s ${TEMPDIR}/psfimages_${CHIP}_$$ ]; then
    while read FILE
    do
      BASE=`basename ${FILE} .fits`
      #
      # now convert the SExtractor cat to one readable by analyseldac:
      if [ ! -f /$1/$2/$4/${BASE}.cat0 ] && [ -f /$1/$2/$4/${BASE}.cat ]; then
          ${P_LDACCONV} -b 1 -c R -i /$1/$2/$4/${BASE}.cat \
                        -o /$1/$2/$4/${BASE}.cat0
      fi

      ${P_LDACADDKEY} -i /$1/$2/$4/${BASE}.cat0 -o ${TEMPDIR}/tmp11.cat_$$ \
                            -t OBJECTS -k nu 1.0 FLOAT ""

      ${P_LDACCALC} -i ${TEMPDIR}/tmp11.cat_$$ \
                    -o ${TEMPDIR}/tmp12.cat_$$ -t OBJECTS \
                    -c "(FLUX_RADIUS);" -n "rg" "" -k FLOAT \
                    -c "(Xpos-1.0);" -n "x" "" -k LONG \
                    -c "(Ypos-1.0);" -n "y" "" -k LONG \
                    -c "(Xpos);" -n "xbad" "" -k FLOAT \
                    -c "(Ypos);" -n "ybad" "" -k FLOAT

      ${P_LDACTOASC} -i ${TEMPDIR}/tmp12.cat_$$ -b -t FIELDS \
                     -k FITSFILE SEXBKGND SEXBKDEV -s > ${TEMPDIR}/tmp.asc_$$

      # create a config file for asctoldac on the fly

      {
        echo "VERBOSE = DEBUG"
        echo "COL_NAME  = IMAGE"
        echo "COL_TTYPE = STRING"
        echo "COL_HTYPE = STRING"
        echo 'COL_COMM = ""'
        echo 'COL_UNIT = ""'
        echo 'COL_DEPTH = 128'
        echo "COL_NAME  = SKY_MODE"
        echo "COL_TTYPE = FLOAT"
        echo "COL_HTYPE = FLOAT"
        echo 'COL_COMM = ""'
        echo 'COL_UNIT = ""'
        echo 'COL_DEPTH = 1'
        echo "COL_NAME  = SKY_SIGMA"
        echo "COL_TTYPE = FLOAT"
        echo "COL_HTYPE = FLOAT"
        echo 'COL_COMM = ""'
        echo 'COL_UNIT = ""'
        echo 'COL_DEPTH = 1'
      } > ${TEMPDIR}/asctoldac_tmp.conf_$$

      ${P_ASCTOLDAC} -i ${TEMPDIR}/tmp.asc_$$ \
                     -c ${TEMPDIR}/asctoldac_tmp.conf_$$ -t HFINDPEAKS \
        	           -o ${TEMPDIR}/hfind.cat_$$ -b 1 -n "KSB"
      rm ${TEMPDIR}/asctoldac_tmp.conf_$$

      # now transfer the HFINDPEAKS table to the SEX catalog
      ${P_LDACADDTAB} -i ${TEMPDIR}/tmp12.cat_$$ \
                      -o /$1/$2/$4/${BASE}_tmp1.cat1 \
                      -t HFINDPEAKS -p ${TEMPDIR}/hfind.cat_$$

      ${P_LDACFILTER} -i /$1/$2/$4/${BASE}_tmp1.cat1 \
                      -o /$1/$2/$4/${BASE}_tmp.cat1 \
                      -c "(rg>0.0)AND(rg<10.0);"

      # now run analyseldac
      ${P_ANALYSELDAC} -i /$1/$2/$4/${BASE}_tmp.cat1 \
                       -o /$1/$2/$4/${BASE}_ksb.cat1 \
                       -p -x 1 -r -3 -f ${FILE}

      ${P_PREANISOTROPY} -i /$1/$2/$4/${BASE}_ksb.cat1 -t OBJECTS \
                         -k rh mag -s ${STEPSIZE} \
                         -c rh 1.0 100000000.0 >& ${TEMPDIR}/tmp1.asc_$$
      ANISOLINE=`${P_GAWK} '($2 == "propose") {
                   print $10, $8, $12, $16, $14, $18
                 }' ${TEMPDIR}/tmp1.asc_$$`

      ${P_ANISOTROPY} -i /$1/$2/$4/${BASE}_ksb.cat1 -c ${ANISOLINE} \
                      -o /$1/$2/$4/${BASE}_ksb_tmp.cat2 -j 12.25 -e 2.0

      ${P_LDACFILTER} -i /$1/$2/$4/${BASE}_ksb_tmp.cat2 \
                      -o /$1/$2/$4/${BASE}_ksb.cat2 -c "(cl=2);"
    done < ${TEMPDIR}/psfimages_${CHIP}_$$

    rm ${TEMPDIR}/psfimages_${CHIP}_$$
  else # if [-s ... ]
    theli_warn "No images to process for Chip ${CHIP}"
  fi
done

# clean up and bye:
for FILE in ${TEMPDIR}/*_$$
do
  rm ${FILE}
done

theli_end "${!#}"
exit 0;
