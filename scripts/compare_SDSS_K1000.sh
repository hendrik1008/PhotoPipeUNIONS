#!/bin/bash

# Script to compare GAaP measurements with SDSS
#
# Input: SDSS catalogue and GAaP catalogue
#
# Output: - Merged catalogue
#         - Photometric offset
#         - Comparison plot

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-1.6.1/bin/Linux_64/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/lib/
set -e 
#}}}

wd=$1
SDSS_cat=$2
cat=$3
band=$4
mag_min=$5
mag_max=$6

base=`basename $cat .cat`

case $band in
    "Z") band2=z;;
esac

associate -i $cat ${SDSS_cat} \
               -o $wd/tmp1.cat_$$ $wd/tmp2.cat_$$ \
               -c @RUNROOT@/@CONFIGPATH@/associate_K1000.conf

bash @RUNROOT@/@SCRIPTPATH@/make_make_ssc_conf -i $wd/tmp1.cat_$$ -c 0 > $wd/make_ssc.conf_$$
bash @RUNROOT@/@SCRIPTPATH@/make_make_ssc_conf -i $wd/tmp2.cat_$$ -c 1 | \
    gawk 'BEGIN{FS="="}{if ($1=="COL_NAME") printf "%s_SDSS\n",$0; else print $0}' \
	>> $wd/make_ssc.conf_$$

ldacfilter -i $wd/tmp1.cat_$$ -o $wd/tmp3.cat_$$ -t OBJECTS -c "Pair_1>0;"
echo
ldacfilter -i $wd/tmp2.cat_$$ -o $wd/tmp4.cat_$$ -t OBJECTS -c "Pair_0>0;"
echo

make_ssc -i ${wd}/tmp3.cat_$$ ${wd}/tmp4.cat_$$ \
             -o ${wd}/merg_SDSS_comp.cat_$$ \
             -c $wd/make_ssc.conf_$$

ldacrentab -i $wd/merg_SDSS_comp.cat_$$ -o $wd/${base}_SDSS.cat \
		-t PSSC OBJECTS

rm $wd/*_$$

ldactoasc -i $wd/${base}_SDSS.cat -t OBJECTS -s -b -k \
    RA \
    DEC \
    MAG_GAAP_$band \
    MAGERR_GAAP_$band \
    ${band2}_SDSS \
    | gawk '{if ($3>0 && $3<99 && $4<0.5 && $5>0 && $5<99) print $0}' \
    > $wd/${base}_SDSS_$band.asc

python @RUNROOT@/@SCRIPTPATH@/phot_offset.py $wd/${base}_SDSS_$band.asc $mag_min $mag_max \
       > $wd/${base}_SDSS_${band}_offset.asc

median=`awk '{printf "%1.3f\n", $1}' $wd/${base}_SDSS_${band}_offset.asc`
NMAD=`awk   '{printf "%1.3f\n", $2}' $wd/${base}_SDSS_${band}_offset.asc`
mean=`awk   '{printf "%1.3f\n", $3}' $wd/${base}_SDSS_${band}_offset.asc`
std=`awk    '{printf "%1.3f\n", $4}' $wd/${base}_SDSS_${band}_offset.asc`

{
    echo 'unset key'
    echo 'set xlabel "'$band'\_SDSS"'
    echo 'set ylabel "'$band'\_VIKING - '$band'\_SDSS"'
    echo 'set term postscript eps color'
    echo 'set output "'$wd/${base}_SDSS_${band}_smart.eps'"'
    echo 'set arrow from '$mag_min',-2 to '$mag_min',2 nohead'
    echo 'set arrow from '$mag_max',-2 to '$mag_max',2 nohead'
    echo 'set label "median='$median'" at 15.1,1.5'
    echo 'set label "NMAD='$NMAD'" at 15.1,1.3'
    echo 'set label "mean='$mean'" at 15.1,1.1'
    echo 'set label "std='$std'" at 15.1,0.9'
    echo 'plot [15:19.5][-2:2]"'$wd/${base}_SDSS_${band}.asc'" u ($5):($3-$5), 0'
}|gnuplot
epstopdf $wd/${base}_SDSS_${band}_smart.eps
rm $wd/${base}_SDSS_${band}_smart.eps
