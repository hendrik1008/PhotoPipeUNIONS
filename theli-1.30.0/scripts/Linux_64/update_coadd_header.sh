#!/bin/bash -u

# The script makes an update of headers from finally coadded
# images. For the moment, the added header keywords are: Total
# exposure time, total gain, magnitude zeropoint and coaddition
# condition.
#
# We assume that there is a catalog coaddidentifier.cat in the
# catalogs directory of the set and that keywords DUMMY0, DUMMY1 and
# so on are in the coadded image (how many of the DUMMY keywords are
# needed is not clear in the beginning)

#
# 29.01.2006:
# The table in the coaddidentifier.cat catalog in which
# to find EXPTIME keyword is now a command line argument.
#
# 02.02.2006:
# Bug fix: The table given as 4th command line argument
# actually was not used.
#
# 26.04.2006:
# The catalog table from which to read ($3.cat) was
# hardcoded to STATS instead of using the 4th command
# line argument.
#
# 30.04.2006:
# gawk was called directly one time instead via the
# P_GAWK variable.
#
# 06.11.2006:
# - The type of magnitude zeropoint (AB or Vega) is
#   now a command line argument. It is written to the
#   corresponding comment line; before always Vega was
#   written as comment.
# - some temporary files now get unique names including
#   the process number.
#
# 06.12.2006:
# The FITS key strings for coaddition conditions and
# photometric solutions are up to 70 characters now
# (instead of 20 as before).
#
# 16.02.2007:
# - The seeing is now calculated from object catalogues cleaned
#   for sources having an external flag unequal to zero.
#
# 24.04.2007:
# I make sure that EXPTIME, SEEING and GAINEFF are FLOAT FITS
# cards (if they could be represented as integers they were stored
# as INTEGER FITS cards).
#
# 10.06.2008:
# The length of the comment lines was reduced from 69 to 66
# (security margin; the old length led to wrong results of comment
# lines were longer than one line)
#
# 19.07.2008:
# I added command line arguments for astrometric reference catalogue,
# instrument and telescope. This information is then added to the
# image headers.
#
# 24.11.2008:
# I added information on the start and end date of observations
# for the co-added stack under consideration (GABODSIDS and civil dates)
#
# 05.05.2011:
# - The FILTER is now taken from the statistic tabel. The new format of the
#   the statistics catalogue no longer contains a FIELDS table from which
#   the FILTER was retrieved up to now.
# - I removed modification of the GAIN within this script. Swarp now
#   calculates and sets this quantity directly.
#
# 26.01.2012:
# I reintroduced modification of the GAIN keyword within this script.
# This is only done if swarp could not determine the gain for some reason
# (swarp puts a value of zero into the GAIN keyword in this case)
#
# 07.02.2013:
# I introduced the new THELI logging scheme into the script.
#
# 12.02.2013:
# If the image that should be updated is not present the script now
# exits with an error message.
#
# 13.06.2014:
# Command line arguments are now referenced with meaningful names and
# no longer via '$1' etc.
#
# 14.06.2015:
# For the ORIGIN keyword I substituted spaces with underscores to avoid
# any shell quotation problems in the 'value' - function. That function
# transforms the origin string into a FITS keyowrd string. However, I did
# not yet fully understand why the spaces are an issue!
#
# 18.07.2018:
# caldate was not called via its 'progs.ini'-variable - fixed.

# TODO:
# Include a proper calculation of the GAIN if the
# co-addition is not weighted mean.

#$1: main dir
#$2: science dir
#$3: coadd identifier
#$4: table in coaddidentifier catalogue to find statistic information
#$5: name of coadded image (without .fits extension)
#    we assume that the weight has the name $4.weight.fits
#$6: Mag Zeropoint; (-1.0 indicates that the image is not calibrated!)
#$7: 'AB' or 'Vega'. The type of the mnagnitude zeropoint.
#    It is used for the comment of the MAGZP header keyword.
#$8: string of coaddition condition
#$9: astrometric reference catalogue used for astrom. calibration
#$10: string for INSTRUME keyword
#$11: string for TELESCOP keyword

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# give meaningful names to command line arguments:
MD=$1
SD=$2
COADDIDENT=$3
STATSTAB=$4
COADDIMA=$5
MAGZP=$6
MAGSYS=$7

# set default values if the last command line arguments are not provided:
COADDCONDITION=${8:-"NONE"}
ASTROMREFCAT=${9:-"UNKNOWN"}
INSTRUMENTKEY=${10:-"UNKNOWN"}
TELESCOPEKEY=${11:-"UNKNOWN"}

# get EXPTIME and hence effective gain from the coaddition catalog.
EXPTIME=0.0
GAINEFF=0.0
NCOMBINE=-1
FILTER="'NOT SPECIFIED'"
OBJECT="'NOT SPECIFIED'"
DATEST="'NOT SPECIFIED'"
DATEEND="'NOT SPECIFIED'"
GABODSST=-1
GABODSEN=-1

if [ -f ${MD}/${SD}/cat/${COADDIDENT}.cat ]; then
  ${P_LDACFILTER} -i ${MD}/${SD}/cat/${COADDIDENT}.cat -t ${STATSTAB} \
                  -o ${TEMPDIR}/tmp_$$.cat -c "($3=1);"

  FILTER="'`${P_LDACTOASC} -s -b -i ${TEMPDIR}/tmp_$$.cat \
          -t ${STATSTAB} -k FILTER | awk 'NR==1'`'"

  OBJECT="'`${P_LDACTOASC} -s -b -i ${TEMPDIR}/tmp_$$.cat \
          -t ${STATSTAB} -k OBJECT | sort | uniq -c | sort -nr | \
          awk 'NR==1 {print $2}'`'"

  NCOMBINE=`${P_LDACTOASC} -b -i ${TEMPDIR}/tmp_$$.cat -t ${STATSTAB} \
            -k EXPTIME | wc -l`

  EXPTIME=`${P_LDACTOASC} -b -i ${TEMPDIR}/tmp_$$.cat -t ${STATSTAB} \
           -k EXPTIME |\
           ${P_GAWK} 'BEGIN {time = 0} {
                        time = time+$1
                      } END { printf("%.2f", time) }'`

  GAINEFF=`${P_GAWK} 'BEGIN {printf("%.2f", '${GAIN}'*'${EXPTIME}')}'`

  GABODSST=`${P_LDACTOASC} -b -i ${TEMPDIR}/tmp_$$.cat -t ${STATSTAB} \
            -k GABODSID |\
            ${P_SORT} -n | ${P_GAWK} '(NR == 1) { print $1 }'`

  GABODSEN=`${P_LDACTOASC} -b -i ${TEMPDIR}/tmp_$$.cat -t ${STATSTAB} \
            -k GABODSID |\
            ${P_SORT} -nr | ${P_GAWK} '(NR == 1) { print $1 }'`

  if [ "${GABODSST}_A" != "_A" ]; then
    DATEST=`${P_CALDATE} -d 31/12/1998 -i ${GABODSST} | ${P_GAWK} '{print $3}'`
  fi

  if [ "${GABODSEN}_A" != "_A" ]; then
    DATEEND=`${P_CALDATE} -d 31/12/1998 -i ${GABODSEN} | ${P_GAWK} '{print $3}'`
  fi

  rm ${TEMPDIR}/tmp_$$.cat
fi

#
# determine the seeing in the coadded image:
#
# first create a simple SExtractor parameter file. It
# only contains three entries and is very special for
# this script. Hence we do not create a new pipeline
# config file:
{
  echo "NUMBER"
  echo "FWHM_IMAGE"
  echo "IMAFLAGS_ISO"
} > ${TEMPDIR}/seeing_sexparam.asc_$$

if [ ! -s ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits ]; then
  theli_error "Something is wrong with Image ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits! Exiting!"
  exit 1;
else
  ${P_SEX} ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits \
        -c ${CONF}/postcoadd.conf.sex \
        -CATALOG_NAME ${TEMPDIR}/seeing_$$.cat \
        -FILTER_NAME ${DATACONF}/default.conv\
        -WEIGHT_IMAGE ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.weight.fits \
        -WEIGHT_TYPE MAP_WEIGHT \
        -FLAG_IMAGE ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.flag.fits \
        -FLAG_TYPE MAX \
        -DETECT_THRESH 10 -DETECT_MINAREA 10 \
        -ANALYSIS_THRESH 10\
        -PARAMETERS_NAME ${TEMPDIR}/seeing_sexparam.asc_$$

  ${P_LDACCONV}  -i ${TEMPDIR}/seeing_$$.cat -o ${TEMPDIR}/seeing_ldac.cat_$$\
                 -b 1 -c "sex" -f R

  ${P_LDACFILTER} -i ${TEMPDIR}/seeing_ldac.cat_$$ \
                  -o ${TEMPDIR}/seeing_ldac_filt.cat_$$ -c "(IMAFLAGS_ISO<1);"

  ${P_LDACTOASC} -b -i ${TEMPDIR}/seeing_ldac_filt.cat_$$ -t OBJECTS\
                 -k FWHM_IMAGE > ${TEMPDIR}/seeing_$$.asc

  NLINES=`wc ${TEMPDIR}/seeing_$$.asc | awk '{print $1}'`

  SEEING=`${P_GAWK} 'BEGIN {
                       binsize=10./'${NLINES}';
                       if(binsize<0.01) { binsize=0.01 }
            	     nbins=int(((3.0-0.3)/binsize)+0.5);
                       ndata = 0;
                       for(i=1; i<=nbins; i++) { bin[i]=0 }
                     }
                     {
                       if(($1*'${PIXSCALE}' > 0.3) &&
                          ($1*'${PIXSCALE}' < 3.0)) {
                         data[ndata++] = $1 * '${PIXSCALE}';
                         actubin=int(($1*'${PIXSCALE}'-0.3)/binsize);
  		       bin[actubin]+=1;
                       }
                     }
  	           END {
                     max=0; k=0;
  		     for(i=1;i<=nbins; i++) {
  		       if(bin[i]>max) {
  		         max=bin[i];
  		         k=i;
  		       }
  		     }
                     seeing = 0.3 + k * binsize;
                     asort(data);
                     nsmall = 0; i = 0;
                     while(data[i] <= seeing) { i++ }
                     low  = int(i - 0.34 * i)
                     high = int(i + 0.34 * i)
                     seeing_sigma = (data[high] - data[low]) / 2.;
  		     printf("%.2f %.2f", seeing, seeing_sigma)
                   }' ${TEMPDIR}/seeing_$$.asc`

  SEEING_MEAN=`echo ${SEEING} | ${P_GAWK} '{printf("%.2f", $1)}'`
  SEEING_SDEV=`echo ${SEEING} | ${P_GAWK} '{printf("%.2f", $2)}'`

  rm ${TEMPDIR}/seeing_sexparam.asc_$$
  rm ${TEMPDIR}/seeing_$$.cat
  rm ${TEMPDIR}/seeing_$$.asc
  rm ${TEMPDIR}/seeing_ldac.cat_$$
  rm ${TEMPDIR}/seeing_ldac_filt.cat_$$

  #
  # write keywords in header

  # TEXPTIME key (total integrated exposure time)
  value ${EXPTIME}
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits \
      TEXPTIME "${VALUE} / total Exposure Time" REPLACE

  # EXPTIME keyword (effective exposure time in the co-added images;
  # up to now the scaling is always for 1s!
  value "1.00"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits EXPTIME \
        "${VALUE} / effective Exposure Time (s)" REPLACE

  # see if we need to modify the GAIN key. We do this only if the header
  # value is zero. In this case swarp could not determine it for some reason:
  DOGAIN=0
  GAINIMAGE=`${P_DFITS} ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits |\
             ${P_FITSORT} -d GAIN | ${P_GAWK} '{print $2}'`

  if [ "${GAINIMAGE}" = "KEY_N/A" ]; then
    DOGAIN=1
  else
    DOGAIN=`${P_GAWK} 'BEGIN {if ('${GAINIMAGE}' < 1.0e-02) {
                                print 1
                              } else {
                                print 0
                              }}'`
  fi

  if [ ${DOGAIN} -eq 1 ]; then
    # the following expression for the GAIN is only valid if the unit of the
    # co-added image is ADU/sec and if a mean (weighted mean) co-addition
    # was performed.
    value ${GAINEFF}
    writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits GAIN \
             "${VALUE} / effective GAIN for SExtractor" REPLACE
  fi


  # ORIGIN keyword
  value "'Argelander_Institute_for_Astronomy_/_Bonn_(Germany)'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits ORIGIN \
           "${VALUE}" REPLACE

  # TELESCOP keyword
  value "'${TELESCOPEKEY}'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits TELESCOP \
           "${VALUE} / Where the data was observed" REPLACE

  # INSTRUME keyword
  value "'${INSTRUMENTKEY}'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits INSTRUME \
           "${VALUE} / Instrument Name" REPLACE

  # FILTER key
  value "${FILTER}"
  writekey  ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits FILTER \
           "${VALUE} / Filter Name" REPLACE

  # OBJEKT key
  value "${OBJECT}"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits OBJECT \
           "${VALUE} / Target Name" REPLACE

  # ASTROMREFCAT key
  value "'${ASTROMREFCAT}'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits ASTRCAT \
           "${VALUE} / Astrometric reference catalogue" REPLACE

  # ASTRERR keyword (fixed for all catalogues for the moment)
  value "0.30"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits ASTRERR \
           "${VALUE} / estimated absolute astrometric error (arcsec)" REPLACE

  # round MAGZP to two decimal points:
  MAGZP=`echo ${MAGZP} | ${P_GAWK} '{printf("%.2f", $1)}'`
  if [ "${MAGZP}" != "-1.00" ]; then
    value ${MAGZP}
  else
    value "-1.00"
  fi
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits MAGZP \
      "${VALUE} / ${MAGSYS} Magnitude Zeropoint" REPLACE

  # photomeric zeropoint error (fixed for the moment)
  MAGZPERR=`${P_GAWK} 'BEGIN { if('${MAGZP}' > 0) {
                                 printf("%.2f", 0.10)
                               } else {
                                 printf("%.2f", -1.0)
                               }}'`
  value ${MAGZPERR}
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits MAGZPERR \
      "${VALUE} / estimated Magnitude Zeropoint Error" REPLACE

  # MAGSYS keyword (magnitude system: AB or Vega)
  value "'${MAGSYS}'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits MAGSYS \
        "${VALUE} / Magnitude System" REPLACE

  #
  # BUNIT keyword
  value "'ADU/s'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits BUNIT \
           "${VALUE} / Unit of pixel values" REPLACE

  # SEEING key
  value ${SEEING_MEAN}
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits SEEING \
      "${VALUE} / measured image Seeing (arcsec)" REPLACE

  # SEEINGERR key
  value ${SEEING_SDEV}
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits SEEINERR \
      "${VALUE} / measured image Seeing error (arcsec)" REPLACE

  # NCOMBINE key
  value ${NCOMBINE}
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits NCOMBINE \
           "${VALUE} / number of individual frames in this stack" REPLACE

  # GABODSST key
  value ${GABODSST}
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits GABODSST \
           "${VALUE} / first GABODSID of images in this stack" REPLACE

  # GABODSEN key
  value ${GABODSEN}
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits GABODSEN \
           "${VALUE} / last GABODSID of images in this stack" REPLACE

  # DATE-ST key
  value "'${DATEST}'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits DATE-ST \
           "${VALUE} / first date of stack data (local time at tel.)" REPLACE

  # DATE-END key
  value "'${DATEEND}'"
  writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits DATE-END \
           "${VALUE} / last date of stack data (local time at tel.)" REPLACE


  #
  # add information on the condition on the input images
  # to enter the stacking process.
  writecommhis ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits COMMENT ""
  writecommhis ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits COMMENT \
               "Conditions on the input images:"

  #
  # split the coaddition condition in strings of length 20 if necessary
  echo ${COADDCONDITION} | ${P_GAWK} 'BEGIN {len=66}
                        {
                          if((n=length($1)/len)-(int(length($1)/len))>0.001) {
                            n=n+1;
                          }
                          pos=1;
                          for(i=1; i<=n; i++) {
                            print substr($1,pos,len);
                            pos+=len;
                          }
                        }' > ${TEMPDIR}/coaddcondition_$$.txt

  j=1
  while read STRING
  do
    writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits \
             COND${j} "'${STRING}'" REPLACE
    j=$(( $j + 1 ))
  done < ${TEMPDIR}/coaddcondition_$$.txt

  rm ${TEMPDIR}/coaddcondition_$$.txt

  #
  # add information about the photometric solutions entering the
  # coadded image
  ${P_LDACTESTEXIST} -i /${MD}/${SD}/cat/chips_phot.cat5 -t SOLPHOTOM
  if [ "$?" -eq "0" ]; then
    ${P_LDACTOASC} -i /${MD}/${SD}/cat/chips_phot.cat5 -b -t SOLPHOTOM | \
    ${P_GAWK} '{printf $0"; "}' | \
    ${P_GAWK} 'BEGIN {len=66}
               {
                 if ((n = length($0) / len) - (int(length($0)/len)) > 0.001) {
                   n = n + 1;
                 }
                 pos = 1;
                 for (i = 1; i <= n; i++) {
                   print substr($0,pos,len);
                   pos+=len;
                 }
               }' > ${TEMPDIR}/nights_$$.txt
    j=1
    while read STRING
    do
      writekey ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits \
               SOLPH${j} "'${STRING}'" REPLACE
      j=$(( $j + 1 ))
    done < ${TEMPDIR}/nights_$$.txt

    rm ${TEMPDIR}/nights_$$.txt
  fi
fi # if [ ! -s ${MD}/${SD}/coadd_${COADDIDENT}/${COADDIMA}.fits ] ...

theli_end
exit 0;
