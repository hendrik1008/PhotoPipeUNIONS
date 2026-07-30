#!/bin/bash

#===============================================================================
#+
# NAME:#   getgsc
#
# PURPOSE:
#   Download stellar data from the gsc1 or gsc2 archive at 
#   http://galex.stsci.edu or http://gsss.stsci.edu
#
# COMMENTS:
#   The catalogs provides a V magnitude for the Northern, 
#   and J for the Southern hemisphere
#   
# USAGE:
#   getgsc hh:mm:ss.s +dd:mm:ss.sss radius mag_max catversion
#
# INPUTS:
#   RA Dec        # RA Dec either in format hh:mm:ss.s +dd:mm:ss.sss 
#                 # Or float value
#                 # radius in arcmin
#   mag_max       # mag_max: max. F (~R) band magnitude included
#   catversion    # catversion is 1 or 2
#
#
# OUTPUTS:
#                To STDOUT
# EXAMPLES:
#
# BUGS:
#   
#
# REVISION HISTORY:
#   2008-01-21     Started Schrabback (Leiden)
#   2008-05-09     merged getgsc1 and getgsc2 to one script (TE)
#   2008-07-24     bug fix to correctly treat negative declinations (TE)
#-
#===============================================================================

# parse the command line

RAin=$1
DECin=$2
radius=$3
mag_max=$4
CATVERSION=$5

# check if all fields have been entered:

if [ $# -ne 5 ]; then
    echo "usage: getgsc hh:mm:ss.s +dd:mm:ss.sss radius mag_max catversion" 
    exit 0;
fi

# split up Ra and Dec in hours, minutes and seconds (degrees, minutes, seconds):
RA_H=`echo ${RAin} | awk -F: '{print $1}'`
RA_M=`echo ${RAin} | awk -F: '{print $2}'`
RA_S=`echo ${RAin} | awk -F: '{if ($3=="60.0000") print 59.9999; else print $3}'`
RAin=$RA_H:$RA_M:$RA_S

DEC_D=`echo ${DECin} | awk -F: '{if($1 < 0) {print $1*(-1)} else {print $1}}'`
SIGN_D=`echo "${DECin}" | awk -F: '{if($1 ~ /-/) { print "-" } else { print "+" }}'`
DEC_M=`echo ${DECin} | awk -F: '{print $2}'`
DEC_S=`echo ${DECin} | awk -F: '{print $3}'`

radius_deg=`echo $radius|awk '{print $1/60}'`

if [ ${CATVERSION} -eq 1 ]; then
    #FILE="GSC1DataReturn.aspx?RAH=${RA_H}&RAM=${RA_M}&RAS=${RA_S}&DSN=${SIGN_D}&DD=${DEC_D}&DM=${DEC_M}&DS=${DEC_S}&EQ=2000&SIZE=${radius}&SRCH=Radius&FORMAT=Plain%20Text&CAT=GSC12&RGNID=&SEQID="
    FILE="CatalogSearch.aspx?RA=${RAin}&DEC=${DECin}&DSN=+&FORMAT=Text&SR=${radius_deg}&M2=${mag_max}&CAT=GSC11"
  wget https://gsss.stsci.edu/webservices/vo/${FILE} -q -O - |\
      awk 'BEGIN{maglim='${mag_max}'}
      {
	if (NF==16)
	{
          mag=$9;
          if ((NR>6)&&($16 ne "")&&(mag<maglim))
          {
            ra=15*($2+$3/60+$4/3600);
            if($5 ~ /-/)
            {
              dsign=-1;
            }
            else
            {
              dsign=1;
            }
            dec = dsign*($5*dsign+$6/60+$7/3600);
            printf "%.10f %.10f %.1f %d\n",ra,dec,mag,$12
          }
	}
	if (NF==17)
	{
          mag=$10;
          if ((NR>6)&&($17 ne "")&&(mag<maglim))
          {
            ra=15*($2+$3/60+$4/3600);
            if($5 ~ /-/)
            {
              dsign=-1;
            }
            else
            {
              dsign=1;
            }
            dec = dsign*($6+$7/60+$8/3600);
            printf "%.10f %.10f %.1f %d\n",ra,dec,mag,$13
          }
	}
      }' 
else
    #FILE="GSC2DataReturn.aspx?RAH=${RA_H}&RAM=${RA_M}&RAS=${RA_S}&DSN=${SIGN_D}&DD=${DEC_D}&DM=${DEC_M}&DS=${DEC_S}&EQ=2000&SIZE=${radius}&SRCH=Radius&FORMAT=Plain%20Text&CAT=GSC23&HSTID=&GSC1ID="
    FILE="CatalogSearch.aspx?RA=${RAin}&DEC=${DECin}&DSN=+&FORMAT=Text&CAT=GSC23&M2=${mag_max}&SR=${radius_deg}&"
   wget https://gsss.stsci.edu/webservices/vo/${FILE} -q -O - |\
       awk 'BEGIN{maglim='${mag_max}'}
       {
	if (NF==42)
	{
           mag=$12;
           if ((NR>6)&&($20 ne "")&&($12<maglim))
           {
             ra=15*($3+$4/60+$5/3600);
             if($6 ~ /-/) 
             {
               dsign=-1;
             }
             else
             {
               dsign=1;
             }
             dec = dsign*($6*dsign+$7/60+$8/3600);
             printf "%.10f %.10f %.1f %d\n",ra,dec,mag,$36
           }
	}
	if (NF==43)
	{
           mag=$13;
           if ((NR>6)&&($21 ne "")&&(mag<maglim))
           {
             ra=15*($3+$4/60+$5/3600);
             if($6 ~ /-/) 
             {
               dsign=-1;
             }
             else
             {
               dsign=1;
             }
             dec = dsign*($7+$8/60+$9/3600);
             printf "%.10f %.10f %.1f %d\n",ra,dec,mag,$37
           }
	}
       }'
fi
