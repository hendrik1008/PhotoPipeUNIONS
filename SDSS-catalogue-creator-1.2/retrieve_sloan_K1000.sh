#!/bin/bash

# script to retrieve SDSS stars in a 1deg^2 region
#
# Input:
# - RA, Dec
#
# Output:
# - SDSS star catalogue in FITS LDAC format
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2017-03-02 V1.0

. progs.ini

md=$1
FIELD=$2

RA=$3
DEC=$4

RAmin=`echo  $RA $DEC | ${P_GAWK} '{print $1-0.6/cos($2/180*3.141)}'`
RAmax=`echo  $RA $DEC | ${P_GAWK} '{print $1+0.6/cos($2/180*3.141)}'`
DECmin=`echo $DEC | ${P_GAWK} '{print $1-0.6}'`
DECmax=`echo $DEC | ${P_GAWK} '{print $1+0.6}'`

echo $FIELD $RAmin $RAmax $DECmin $DECmax

./SDSS_dataquery.py SDSSDR10 STARS $RAmin $RAmax $DECmin $DECmax \
		    > $md/${FIELD}_sdssdr8_stars.dat 

NLINES=`wc -l $md/${FIELD}_sdssdr8_stars.dat | ${P_GAWK} '{print $1}'`
if [ -f $md/${FIELD}_sdssdr8_stars.dat ] && [ ${NLINES} -gt 3 ]
then
    ${P_GAWK} '{if (NR>1) print $0,1.033333e-4, 1.033333e-4, 0., 0}' \
	$md/${FIELD}_sdssdr8_stars.dat | \
	${P_GAWK} 'BEGIN{i=1}!/#/{print i,$0;i++}' \
	    > $md/${FIELD}_sdssdr8_stars.tmp.dat_$$
    ${P_ASCTOLDAC} -a $md/${FIELD}_sdssdr8_stars.tmp.dat_$$ \
	      -o $md/${FIELD}_sdssdr8_stars.tmp.cat_$$ \
	      -c asctoldac_sdssdr8_stars_K1000.conf
    ${P_LDACADDKEY} -i $md/${FIELD}_sdssdr8_stars.tmp.cat_$$ \
	       -o $md/${FIELD}_sdssdr8_stars.cat -t FIELDS \
	       -k CRVAL1 0.0 DOUBLE ""\
	       CRVAL2 0.0 DOUBLE ""\
	       CRPIX1 0.0 DOUBLE ""\
	       CRPIX2 0.0 DOUBLE ""\
	       CDELT1 0.0 DOUBLE ""\
	       CDELT2 0.0 DOUBLE ""\
	       MAPNAXS1 0.0 LONG ""\
	       MAPNAXS2 0.0 LONG ""
fi

./SDSS_dataquery.py SDSSDR10 GALZ $RAmin $RAmax $DECmin $DECmax \
		    > $md/${FIELD}_sdssdr8_galz.dat 

NLINES=`wc -l $md/${FIELD}_sdssdr8_galz.dat | ${P_GAWK} '{print $1}'`
if [ -f $md/${FIELD}_sdssdr8_galz.dat ] && [ ${NLINES} -gt 3 ]
then
    ${P_GAWK} '{if (NR>1) print $0,1.033333e-4, 1.033333e-4, 0., 0}' \
	$md/${FIELD}_sdssdr8_galz.dat | \
	${P_GAWK} 'BEGIN{i=1}!/#/{print i,$0;i++}' \
	    > $md/${FIELD}_sdssdr8_galz.tmp.dat_$$
    ${P_ASCTOLDAC} -a $md/${FIELD}_sdssdr8_galz.tmp.dat_$$ \
	      -o $md/${FIELD}_sdssdr8_galz.tmp.cat_$$ \
	      -c asctoldac_sdssdr8_galz.conf
    ${P_LDACADDKEY} -i $md/${FIELD}_sdssdr8_galz.tmp.cat_$$ \
	       -o $md/${FIELD}_sdssdr8_galz.cat -t FIELDS \
	       -k CRVAL1 0.0 DOUBLE ""\
	       CRVAL2 0.0 DOUBLE ""\
	       CRPIX1 0.0 DOUBLE ""\
	       CRPIX2 0.0 DOUBLE ""\
	       CDELT1 0.0 DOUBLE ""\
	       CDELT2 0.0 DOUBLE ""\
	       MAPNAXS1 0.0 LONG ""\
	       MAPNAXS2 0.0 LONG ""
fi

rm $md/*_$$
