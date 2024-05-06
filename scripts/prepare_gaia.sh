#!/bin/bash

# script to prepare Gaia catalogue
#
# Input:
# - RA, Dec
#
# Output:
# - Gaia star catalogue in ASCII format for GAaP
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2023-10-05 V1.0

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

# Retrieve Gaia catalogue and save as $md/$FIELD_Gaia.asc

python @RUNROOT@/@SCRIPTPATH@/retrieve_gaia.py $md $FIELD $RA $DEC

# Transform magnitudes and apply some cuts

awk '{
      if (NR==1) print "#", $0, "g r i"
      else
	{
            G=$4
      	    bp_rp=$5
	    G_g = 0.13518 - 0.46245*bp_rp - 0.25171*bp_rp**2 + 0.021349*bp_rp**3
	    G_r = - 0.12879 + 0.24662*bp_rp - 0.027464*bp_rp**2 - 0.049465*bp_rp**3
	    G_i = - 0.29676 + 0.64728*bp_rp - 0.10141*bp_rp**2
      	    g = G - G_g
      	    r = G - G_r
      	    i = G - G_i
      	    print $0, g, r, i
	}
     }' \
	 $md/${FIELD}_Gaia.asc > \
	 $md/${FIELD}_Gaia_SDSS.asc

awk '{
      if (NR>1)
            G=$4
      	    bp_rp=$5
	    G_g = 0.13518 - 0.46245*bp_rp - 0.25171*bp_rp**2 + 0.021349*bp_rp**3
      	    g = G - G_g
      	    if (g>15 && g<18.5 && $6>0.9999) print $2, $3
     }' \
	 $md/${FIELD}_Gaia.asc > \
	 $md/${FIELD}_Gaia_u.wcs

awk '{
      if (NR>1)
            G=$4
      	    bp_rp=$5
	    G_g = 0.13518 - 0.46245*bp_rp - 0.25171*bp_rp**2 + 0.021349*bp_rp**3
      	    g = G - G_g
      	    if (g>17 && g<21 && $6>0.9999) print $2, $3
     }' \
	 $md/${FIELD}_Gaia.asc > \
	 $md/${FIELD}_Gaia_g.wcs

awk '{
      if (NR>1)
            G=$4
      	    bp_rp=$5
	    G_r = - 0.12879 + 0.24662*bp_rp - 0.027464*bp_rp**2 - 0.049465*bp_rp**3
      	    r = G - G_r
      	    if (r>16.5 && r<20 && $6>0.9999) print $2, $3
     }' \
	 $md/${FIELD}_Gaia.asc > \
	 $md/${FIELD}_Gaia_r.wcs

awk '{
      if (NR>1)
            G=$4
      	    bp_rp=$5
	    G_i = - 0.29676 + 0.64728*bp_rp - 0.10141*bp_rp**2
      	    i = G - G_i
      	    if (i>14 && i<17.5 && $6>0.9999) print $2, $3
     }' \
	 $md/${FIELD}_Gaia.asc > \
	 $md/${FIELD}_Gaia_i.wcs

for filter in u g r i
do
    sky2xy $md/../$filter/${FIELD}_${filter}.fits @$md/${FIELD}_Gaia_${filter}.wcs \
	   | awk 'BEGIN{i=1}{if (NF==6) {print $5,$6,1000,10,3,3,3,0,i,0; i+=1}}' \
	   > $md/../$filter/${FIELD}_${filter}_star_cat_GAaP.asc
done

#rm $md/*_$$
