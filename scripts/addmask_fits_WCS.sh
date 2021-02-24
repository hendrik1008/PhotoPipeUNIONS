#!/bin/bash

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-1.6.1/bin/Linux_64/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/lib/
set -e 
#}}}


incat=$1
outcat=$2
maskfits=$3
maskkey=$4
comment=$5
longshort=$6
tmpdir=$7

${P_LDACTOASC} -b -i $incat -t OBJECTS -k ALPHA_J2000 DELTA_J2000 > $tmpdir/pos_$$

echo

python ./addmask_fits_WCS.py $tmpdir/pos_$$ $maskfits > $tmpdir/mask_$$

{
echo 'COL_NAME = '$maskkey
echo 'COL_TTYPE = '$longshort
echo 'COL_HTYPE = INT'
echo 'COL_COMM = "'$comment'"'
echo 'COL_UNIT = ""'
echo 'COL_DEPTH = 1'
}>$tmpdir/asctoldac_$$.conf

echo

${P_ASCTOLDAC} -a $tmpdir/mask_$$ -o $tmpdir/mask_$$.cat -c $tmpdir/asctoldac_$$.conf
echo
${P_LDACJOINKEY} -i $incat -o $outcat -t OBJECTS -p $tmpdir/mask_$$.cat -k $maskkey
echo
rm $tmpdir/pos_$$
rm $tmpdir/mask_$$
rm $tmpdir/mask_$$.cat
rm $tmpdir/asctoldac_$$.conf
