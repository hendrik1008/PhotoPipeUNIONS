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

md=$1
wd=$md/PGM/
test ! -d $wd && mkdir $wd
FIELD=$2

RA=$3
DEC=$4

RAmin=`echo $RA $DEC|awk '{print $1-0.3/cos($2/180*3.141)}'`
RAmax=`echo $RA $DEC|awk '{print $1+0.3/cos($2/180*3.141)}'`
DECmin=`echo $DEC|awk '{print $1-0.3}'`
DECmax=`echo $DEC|awk '{print $1+0.3}'`

cadc-tap query \
	 "SELECT raPS,decPS,gHSC,rMega,iPS,zPS,zHSC FROM pgm.ps_gaia_merged WHERE raPS>"$RAmin" AND raPS<"$RAmax" AND decPS>"$DECmin" and decPS<"$DECmax \
	 >$wd/${FIELD}_PGM.asc

NLINES=`wc -l $wd/${FIELD}_PGM.asc | awk '{print $1}'`
if [ -f $wd/${FIELD}_PGM.asc ] && [ ${NLINES} -gt 1 ]
then
    awk '{if (NR>2 && NF==7) print $0,1.033333e-4, 1.033333e-4, 0., 0}' \
	    $wd/${FIELD}_PGM.asc | \
	    awk 'BEGIN{i=1}!/"/{print i,$0;i++}' \
	    > $wd/${FIELD}_PGM.tmp.dat_$$
    asctoldac -a $wd/${FIELD}_PGM.tmp.dat_$$ \
	      -o $wd/${FIELD}_PGM.tmp.cat_$$ \
	      -c @RUNROOT@/@CONFIGPATH@/asctoldac_PGM.conf
    ldacaddkey -i $wd/${FIELD}_PGM.tmp.cat_$$ \
	       -o $wd/${FIELD}_PGM.cat -t FIELDS \
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
