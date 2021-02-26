#!/bin/bash -u

# The script estimates an absolute photometric zeropoint for the
# coadded image of a set. It relies on presence of information on
# relative photometry (PHOTOMETRIX) and photometric zeropoints.  If
# the script is called with the first two arguments only it uses the
# default zeropoints to estimate the zeropoint of a stacked image.

# 29.01.2006:
# The table in which to find info. on the abolute photometry
# is now called STATS instead of IMAGES.
#
# 25.05.2007:
# The calculation of the final zeropoint has been refined.
# If more than 20 images contribte to the final zeropoint calculation
# we now perform a sigmaclipping on the sample before estimating the
# final zeropoint. This was done because in CFHTLS Deep processings
# we noted photometrically marked frames that falsified the result.
# The reason in the actual case was a wrong listed exposure time in
# an image header.
#
# 20.04.2008:
# I refined again the estimation of final zeropoints. I now
# filter images having a 'bad' relative zeropoint which is defined
# as being -0.1 w.r.t. the median of relative zeropoints.
#
# 14.10.2008:
# Temporary files are cleaned at the end of the script
#
# 12.03.2010:
# MAJOR BUG FIX: A number (-1) was compared to the string "-1"
# instead numerically. This led to errors if the number was not
# represented by an integer.
#
# 31.03.2010:
# correction of a typo bug introduced in the fixing of the bug from
# 12.03.2010
#
# 02.02.2011:
# I forgot to remove one of the temporary files.
#
# 07.02.2013:
# I introduced the new THELI logging scheme into the script.
#
# 30.04.2013:
# I corrected a bug in the preselction pf photometric frames.
# I assumed that non-photometric frames always have a ZP of -1
# but there is no reason why they should have it. In contrast,
# ZPCHOICE=0 is a certain indicator for a non-photometric (or not
# photometrically calibrated) frame.
#
# 07.03.2014:
# I now use photometric groups in the estimation of photometric
# zeropoints. Different photometric groups appear if the survey
# contains several 'islands' of data and not a contiguous patch.


# TODO:
# - The sigmaclipping on the zeropoint calculation (introduced
#   on (25.05.2007) might be refined. It is now a simple rejection
#   of points deviating three sigmas from the mean. This rejection
#   is done three times at most.
# - Better checking of catalogues entering this script. For instance
#   we should react on cases where relative zeropoints are not found
#   in the input 'chips.cat5' catalogue.

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"


# $1: main dir.
# $2: science dir. (the cat dir is a subdirectory of this)
# $3-$#: pairs of GABODSID and photometric solution that should
#        be considered in the photometric solution

#
# check consistency of command line arguments
if [ $# -gt 2 ] && [ $(( ($# - 2) % 2 )) -ne 0 ]; then
  theli_error "wrong command line syntax !!"
  exit 1;
fi

#
# Generate a catalog containing the photometric information
# requested by the user.
test -f /$1/$2/cat/absphot.asc   && rm /$1/$2/cat/absphot.asc
test -f ${TEMPDIR}/nights_$$.asc && rm ${TEMPDIR}/nights_$$.asc

PHOTGRP=0 # do we have a key indicating photometric groups in the
          # STATS table? (1=YES)

${P_LDACTESTEXIST} -i /$1/$2/cat/chips.cat5 -t STATS -k PHOTGROUP
if [ "$?" -eq "0" ]; then
  PHOTGRP=1
fi

# if we have nights to calibrate on the command line take this info,
# otherwise calibrate with all preselected photometric information
if [ $# -gt 2 ]; then
  NAME=""
  NIGHT=3
  CHOICE=4

  KEYS="IMAGENAME ZP${!CHOICE} COEFF${!CHOICE} AIRMASS RZP"
  if [ ${PHOTGRP} -eq 1 ]; then
    KEYS="${KEYS} PHOTGROUP"
  fi

  while [ ${NIGHT} -le $(( $# - 1 )) ]
  do
    ${P_LDACFILTER} -i /$1/$2/cat/chips.cat5 -o ${TEMPDIR}/zp_$$.cat \
                    -t STATS -c "(GABODSID=${!NIGHT});"

    ${P_LDACTOASC} -b -i ${TEMPDIR}/zp_$$.cat -s -t STATS \
                   -k ${KEYS} | \
                   ${P_GAWK} '{if ($1 !~ /#/ && $2 > 0) {
                                 if ('${PHOTGRP}' == 1) {
                                   print $0;
                                 } else {
                                   print $0 " 1";
                              }}' >\
                             ${TEMPDIR}/zp_$$.asc

    if [ -s ${TEMPDIR}/zp_$$.asc ]; then
      cat ${TEMPDIR}/zp_$$.asc >> /$1/$2/cat/absphot.asc
      echo ${!NIGHT} ${!CHOICE} >> ${TEMPDIR}/nights_$$.asc
    else
      theli_error "problem with night ${!NIGHT} and solution ${!CHOICE}; aborting !!"
      exit 1
    fi
    NAME="${NAME}_${!NIGHT}_${!CHOICE}"
    NIGHT=$(( ${NIGHT} + 2 ))
    CHOICE=$(( ${CHOICE} + 2 ))
  done
else
  KEYS="IMAGENAME ZP COEFF AIRMASS RZP ZPCHOICE"
  if [ ${PHOTGRP} -eq 1 ]; then
    KEYS="${KEYS} PHOTGROUP"
  fi


  ${P_LDACTOASC} -b -i /$1/$2/cat/chips.cat5 \
                 -s -t STATS -k ${KEYS} | \
      ${P_GAWK} '{if ($1 !~ /#/ && $6 > 0) {
                    if ('${PHOTGRP}' == 1) {
                      print $1, $2, $3, $4, $5, $7;
                    } else {
                      print $1, $2, $3, $4, $5, " 1";
                    }
                  }}' > /$1/$2/cat/absphot.asc

  if [ ! -s /$1/$2/cat/absphot.asc ]; then
    theli_error "No default zeropoints and extinction coefficients available !!"
    exit 1;
  fi

  ${P_LDACTOASC} -b -i /$1/$2/cat/chips.cat5 -t STATS -k GABODSID ZPCHOICE |\
      ${P_SORT} -g | uniq | ${P_GAWK} '($2>0) {print $0}' \
        > ${TEMPDIR}/nights_$$.asc
fi

# now estimate the absolute photometric zeropoints - one for each
# photometric group. If more than 20 photometric images for a group
# are available we perform a sigmaclipping algorithm before estimating
# the zeropoint by a straight mean.
#
# First we filter entries with a 'bad' relative zeropoint. We
# define this as being 0.1 below the median of relative zeropoints:
test -f ${TEMPDIR}/phottmp_$$.asc && rm ${TEMPDIR}/phottmp_$$.asc

PHOTGROUPS=`${P_GAWK} '{print $NF}' /$1/$2/cat/absphot.asc | sort | uniq`

for GROUP in ${PHOTGROUPS}
do
  ${P_GAWK} '{if ($NF == '${GROUP}') {print $0}}' /$1/$2/cat/absphot.asc >\
     ${TEMPDIR}/tmp_$$.txt

  ${P_GAWK} 'BEGIN {i = 1} {
               line[i] = $0; rzp[i] = $5; rzpcopy[i] = $5; i++
             }
    END {n = asort(rzpcopy);
         if(n > 2) {
           if(n%2==0) {
             reject = (rzpcopy[n / 2] + rzpcopy[n / 2 + 1]) / 2. - 0.1
           } else {
             reject = rzpcopy[int(n / 2)] - 0.1;
           }
         } else {
           reject = -1.0;
         }
         for(i = 1; i <= n; i++) {
           if(rzp[i] > reject) {
             print line[i];
           }
         }
        }' ${TEMPDIR}/tmp_$$.txt |\
  ${P_GAWK} '{oldline[NR] = $0;
              newline[NR] = $0;
              oldzp[NR]   = $2+$3*$4-$5;
              newzp[NR]   = $2+$3*$4-$5;}
    END { if (NR > 20) {maxiter = 3} else {maxiter = 0}
         oldnelem = 0;
         newnelem = NR;
         actuiter = 0;
         while (actuiter <= maxiter && oldnelem != newnelem) {
           zpmean = 0.0; zpsdev = 0.0;
           for(i = 1; i <= newnelem; i++) {
             zpmean += newzp[i];
             zpsdev += newzp[i] * newzp[i];
           } zpmean /= newnelem;
           if(newnelem > 2) {
             zpsdev = sqrt(zpsdev / newnelem - zpmean * zpmean)
           } else {
             zpsdev = 0.0
           }

           oldnelem = newnelem;
           if(zpsdev > 0.0 && actuiter < maxiter)
           {
             newnelem = 0;
             for(i = 1; i <= oldnelem; i++) {
               oldzp[i]   = newzp[i];
               oldline[i] = newline[i];
             }
             for(i = 1; i <= oldnelem; i++) {
               if(sqrt((oldzp[i] - zpmean) * (oldzp[i] - zpmean)) < 3.0 * zpsdev)
               {
                 newzp[++newnelem] = oldzp[i];
                 newline[newnelem] = oldline[i];
               }
             }
           }
           actuiter++;
         }
         for(i=1; i<=oldnelem; i++) {
           print newline[i], newzp[i], zpmean, zpsdev
         }
        }' >> ${TEMPDIR}/phottmp_$$.asc
done

#
# create tables SOLPHOTOM and ABSPHOTOM containing info on the nights and
# solutions that finally went into the photometric calibration.
if [ -s ${TEMPDIR}/nights_$$.asc ] && [ -s ${TEMPDIR}/phottmp_$$.asc ]; then
  ${P_ASCTOLDAC} -a ${TEMPDIR}/nights_$$.asc -o ${TEMPDIR}/nights_$$.cat \
                 -t SOLPHOTOM -c ${DATACONF}/asctoldac_solphotom.conf

  ${P_ASCTOLDAC} -a ${TEMPDIR}/phottmp_$$.asc -o ${TEMPDIR}/phottmp_$$.cat \
                 -t ABSPHOTOM -c ${DATACONF}/asctoldac_absphotom.conf

  ${P_LDACADDTAB} -i /$1/$2/cat/chips.cat5 -o /$1/$2/cat/chips_tmp.cat5 \
                  -p ${TEMPDIR}/nights_$$.cat -t SOLPHOTOM

  ${P_LDACADDTAB} -i /$1/$2/cat/chips_tmp.cat5 -o /$1/$2/cat/chips_phot.cat5 \
                  -p ${TEMPDIR}/phottmp_$$.cat -t ABSPHOTOM
fi

# clean up temporary files and bye:
test -f /$1/$2/cat/absphot.asc    && rm /$1/$2/cat/absphot.asc
test -f /$1/$2/cat/chips_tmp.cat5 && rm /$1/$2/cat/chips_tmp.cat5
test -f ${TEMPDIR}/tmp_$$.txt  && rm ${TEMPDIR}/tmp_$$.txt
test -f ${TEMPDIR}/nights_$$.asc  && rm ${TEMPDIR}/nights_$$.asc
test -f ${TEMPDIR}/nights_$$.cat  && rm ${TEMPDIR}/nights_$$.cat
test -f ${TEMPDIR}/phottmp_$$.asc && rm ${TEMPDIR}/phottmp_$$.asc
test -f ${TEMPDIR}/phottmp_$$.cat && rm ${TEMPDIR}/phottmp_$$.cat

theli_end
exit 0;

