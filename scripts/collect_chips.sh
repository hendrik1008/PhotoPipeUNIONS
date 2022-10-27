#!/bin/bash

# Script to collect all VISTA chips in a rectangular
# sky region.
#
# Input:
# - list of chips with RA and Dec positions.
# - Min. and max. RA and Dec.
#
# Output:
# - list of chips (to STDOUT)
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2017-03-13 V1.0

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

chip_list=$1
RA_min=$2
RA_max=$3
Dec_min=$4
Dec_max=$5

if [ "$chip_list" == "" ] || [ $# -lt 5 ] 
then 
  >&2 echo "ERROR: Incorrect arguments provided"
  exit 1
fi 

if [ ! -f $chip_list ] 
then 
  >&2 echo "ERROR: The chip list ${chip_list} does not exist!"
  exit 1
fi 

#Check the chip RADec limits, being careful of the RA=0 limit
gawk '{
if ('$RA_min'<'$RA_max')
  {
    if ( $2>'$RA_min'-0.1/cos('$Dec_min'/180*3.141) && 
      $2<='$RA_max'+0.1/cos('$Dec_min'/180*3.141) && 
      $3>'$Dec_min'-0.1 &&
        $3<='$Dec_max'+0.1 ) 
      print $1
    }
    if ('$RA_min'>'$RA_max')
      {
        if ( ($2>'$RA_min'-0.1/cos('$Dec_min'/180*3.141) || 
          $2<='$RA_max'+0.1/cos('$Dec_min'/180*3.141)) && 
          $3>'$Dec_min'-0.1 &&
            $3<='$Dec_max'+0.1 ) 
          print $1
        }
      }' \
        $chip_list
