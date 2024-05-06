#!/bin/bash

# script to prepare Seb's redshift catalogue for a given tile
#
# Input:
# - main directory, field name, RA, Dec
#
# Output:
# - spec-z catalogue in FITS LDAC format
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2024-01-23 v1

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

RAmin=`echo  $RA $DEC | awk '{print $1-0.3/cos($2/180*3.141)}'`
RAmax=`echo  $RA $DEC | awk '{print $1+0.3/cos($2/180*3.141)}'`
DECmin=`echo $DEC | awk '{print $1-0.3}'`
DECmax=`echo $DEC | awk '{print $1+0.3}'`

echo $FIELD $RAmin $RAmax $DECmin $DECmax

awk 'BEGIN{i=0}
    {if ($1>'$RAmin' && $1<'$RAmax' && $2>'$DECmin' && $2<'$DECmax' && $3==2 && $4>0 && $5<0.01) 
    	{
		i+=1
		print i, $1, $2, $4, $5, 1.033333e-4, 1.033333e-4, 0., 0
	}
    }' @ZCAT@ \
	    > $md/${FIELD}_redshifts-2024-01-04.asc 

NLINES=`wc -l $md/${FIELD}_redshifts-2024-01-04.asc | awk '{print $1}'`
if [ -f $md/${FIELD}_redshifts-2024-01-04.asc ] && [ ${NLINES} -gt 0 ]
then
    asctoldac -a $md/${FIELD}_redshifts-2024-01-04.asc \
	      -o $md/${FIELD}_redshifts-2024-01-04.tmp.cat_$$ \
	      -c @RUNROOT@/@CONFIGPATH@/asctoldac_redshifts-2024-01-04.conf
    ldacaddkey -i $md/${FIELD}_redshifts-2024-01-04.tmp.cat_$$ \
	       -o $md/${FIELD}_redshifts-2024-01-04.cat -t FIELDS \
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
