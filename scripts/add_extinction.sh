#!/bin/bash

wd=$1
incat=$2
outcat=$3

ldactoasc -i $incat -t OBJECTS -b -k ALPHA_J2000 DELTA_J2000 > $wd/tmp1_$$

python @RUNROOT@/@SCRIPTPATH@/eq2gal_list.py $wd/tmp1_$$ > $wd/tmp2_$$

orig_dir=`pwd`

cd $wd

~/src/Schlegel/CodeC/dust_getval infile=tmp2_$$ \
                                 outfile=tmp3_$$\
                                 ipath=~/src/Schlegel/ noloop=y interp=n 

cd $orig_dir

{
    echo "COL_NAME  = l_gal"
    echo "COL_TTYPE = FLOAT"
    echo "COL_HTYPE = FLOAT"
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = "deg"'
    echo "COL_DEPTH = 1"
    echo "COL_NAME  = b_gal"
    echo "COL_TTYPE = FLOAT"
    echo "COL_HTYPE = FLOAT"
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = "deg"'
    echo "COL_DEPTH = 1"
    echo "COL_NAME  = EXTINCTION"
    echo "COL_TTYPE = FLOAT"
    echo "COL_HTYPE = FLOAT"
    echo 'COL_COMM = ""'
    echo 'COL_UNIT = "mag"'
    echo "COL_DEPTH = 1"
} > $wd/asctoldac_ext.conf_$$
  
asctoldac -i $wd/tmp3_$$ \
          -o $wd/tmp4.cat_$$ -t OBJECTS \
          -c $wd/asctoldac_ext.conf_$$
   
ldacjoinkey -i $incat -o $outcat \
            -p $wd/tmp4.cat_$$ -t OBJECTS -k EXTINCTION

rm $wd/*_$$
