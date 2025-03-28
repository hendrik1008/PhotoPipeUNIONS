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
wd=$md/PS/
test ! -d $wd && mkdir $wd
FIELD=$2

RA=$3
DEC=$4

python3 @RUNROOT@/@SCRIPTPATH@/retrieve_PS.py $wd $FIELD $RA $DEC

NLINES=`wc -l $wd/${FIELD}_PS1-DR2.asc | awk '{print $1}'`
if [ -f $wd/${FIELD}_PS1-DR2.asc ] && [ ${NLINES} -gt 1 ]
then
    awk '{if (NR>1 && $5-$13<0.05) print $0,1.033333e-4, 1.033333e-4, 0., 0}' \
	    $wd/${FIELD}_PS1-DR2.asc | \
	    awk 'BEGIN{i=1}!/#/{print i,$0;i++}' \
	    > $wd/${FIELD}_PS1-DR2.tmp.dat_$$
    asctoldac -a $wd/${FIELD}_PS1-DR2.tmp.dat_$$ \
	      -o $wd/${FIELD}_PS1-DR2.tmp.cat_$$ \
	      -c @RUNROOT@/@CONFIGPATH@/asctoldac_PS1-DR2.conf
    ldacaddkey -i $wd/${FIELD}_PS1-DR2.tmp.cat_$$ \
	       -o $wd/${FIELD}_PS1-DR2.cat -t FIELDS \
	       -k CRVAL1 0.0 DOUBLE ""\
	       CRVAL2 0.0 DOUBLE ""\
	       CRPIX1 0.0 DOUBLE ""\
	       CRPIX2 0.0 DOUBLE ""\
	       CDELT1 0.0 DOUBLE ""\
	       CDELT2 0.0 DOUBLE ""\
	       MAPNAXS1 0.0 LONG ""\
	       MAPNAXS2 0.0 LONG ""
fi

rm $wd/*_$$
