#!/bin/bash

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

incat=$1
md=`dirname $1`
outcat=$2
maskfits=$3
maskkey=$4
comment=$5
longshort=$6

ldactoasc -b -i $incat -t OBJECTS -k Xpos Ypos > $md/pos_$$

python @RUNROOT@/@SCRIPTPATH@/addmask_fits.py $md/pos_$$ $maskfits > $md/mask_$$

{
echo 'COL_NAME = '$maskkey
echo 'COL_TTYPE = '$longshort
echo 'COL_HTYPE = INT'
echo 'COL_COMM = "'$comment'"'
echo 'COL_UNIT = ""'
echo 'COL_DEPTH = 1'
}>$md/asctoldac_$$.conf

asctoldac -a $md/mask_$$ -o $md/mask_$$.cat -c $md/asctoldac_$$.conf

ldacjoinkey -i $incat -o $outcat -t OBJECTS -p $md/mask_$$.cat -k $maskkey

rm $md/pos_$$
rm $md/mask_$$
rm $md/mask_$$.cat
rm $md/asctoldac_$$.conf
