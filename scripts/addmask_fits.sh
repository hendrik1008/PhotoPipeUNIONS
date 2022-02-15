#!/bin/bash

incat=$1
outcat=$2
maskfits=$3
maskkey=$4
comment=$5
longshort=$6

ldactoasc -b -i $incat -t OBJECTS -k Xpos Ypos > pos_$$

python @RUNROOT@/@SCRIPTPATH@/addmask_fits.py pos_$$ $maskfits > mask_$$

{
echo 'COL_NAME = '$maskkey
echo 'COL_TTYPE = '$longshort
echo 'COL_HTYPE = INT'
echo 'COL_COMM = "'$comment'"'
echo 'COL_UNIT = ""'
echo 'COL_DEPTH = 1'
}>asctoldac_$$.conf

asctoldac -a mask_$$ -o mask_$$.cat -c asctoldac_$$.conf

ldacjoinkey -i $incat -o $outcat -t OBJECTS -p mask_$$.cat -k $maskkey

rm pos_$$
rm mask_$$
rm mask_$$.cat
rm asctoldac_$$.conf
