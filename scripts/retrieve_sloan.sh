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
# Update: Angus H Wright
#
# Version history:
# 2017-03-02 V1.0
# 2021-02-25 V1.1 Updated for PhotoPipe 

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/:@RUNROOT@/INSTALL/anaconda2/lib/
set -e
#}}}

md=$1
FIELD=$2

RA=$3
DEC=$4

RAmin=`echo  $RA $DEC | awk '{print $1-0.6/cos($2/180*3.141)}'`
RAmax=`echo  $RA $DEC | awk '{print $1+0.6/cos($2/180*3.141)}'`
DECmin=`echo $DEC | awk '{print $1-0.6}'`
DECmax=`echo $DEC | awk '{print $1+0.6}'`

echo $FIELD $RAmin $RAmax $DECmin $DECmax

python @RUNROOT@/@SCRIPTPATH@/SDSS_dataquery.py SDSSDR10 STARS $RAmin $RAmax $DECmin $DECmax \
		    > $md/${FIELD}_sdssdr8_stars.dat 

NLINES=`wc -l $md/${FIELD}_sdssdr8_stars.dat | awk '{print $1}'`
if [ -f $md/${FIELD}_sdssdr8_stars.dat ] && [ ${NLINES} -gt 3 ]
then
    awk '{if (NR>1) print $0,1.033333e-4, 1.033333e-4, 0., 0}' \
	    $md/${FIELD}_sdssdr8_stars.dat | \
	    awk 'BEGIN{i=1}!/#/{print i,$0;i++}' \
	    > $md/${FIELD}_sdssdr8_stars.tmp.dat_$$
    asctoldac -a $md/${FIELD}_sdssdr8_stars.tmp.dat_$$ \
	      -o $md/${FIELD}_sdssdr8_stars.tmp.cat_$$ \
	      -c @RUNROOT@/@CONFIGPATH@/asctoldac_sdssdr8_stars_K1000.conf
    ldacaddkey -i $md/${FIELD}_sdssdr8_stars.tmp.cat_$$ \
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

python @RUNROOT@/@SCRIPTPATH@/SDSS_dataquery.py SDSSDR10 GALZ $RAmin $RAmax $DECmin $DECmax \
		    > $md/${FIELD}_sdssdr8_galz.dat 

NLINES=`wc -l $md/${FIELD}_sdssdr8_galz.dat | awk '{print $1}'`
if [ -f $md/${FIELD}_sdssdr8_galz.dat ] && [ ${NLINES} -gt 3 ]
then
    awk '{if (NR>1) print $0,1.033333e-4, 1.033333e-4, 0., 0}' \
	    $md/${FIELD}_sdssdr8_galz.dat | \
	    awk 'BEGIN{i=1}!/#/{print i,$0;i++}' \
	    > $md/${FIELD}_sdssdr8_galz.tmp.dat_$$
    asctoldac -a $md/${FIELD}_sdssdr8_galz.tmp.dat_$$ \
	      -o $md/${FIELD}_sdssdr8_galz.tmp.cat_$$ \
	      -c @RUNROOT@/@CONFIGPATH@/asctoldac_sdssdr8_galz.conf
    ldacaddkey -i $md/${FIELD}_sdssdr8_galz.tmp.cat_$$ \
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
