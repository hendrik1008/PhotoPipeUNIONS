#!/bin/bash

# Script to compare GAaP measurements with 2MASS
#
# Input: 2MASS catalogue and GAaP catalogue
#
# Output: - Merged catalogue
#         - Photometric offset
#         - Comparison plot

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/lib/
set -e 
#}}}

wd=$1
TWOMASS_cat=$2
cat=$3
band=$4
mag_min=$5
mag_max=$6

base=`basename $cat .cat`

band2=$band
if [ $band = "Y" ] || [ $band = "Z" ]
then
    band2=J
fi
if [ $band = "Ks" ]
then
    band2=K
fi

case $band in
    Z)  EBcorr=0.370; ABcorr=0.521; CT=1.025;  band_col1=J; band_col2=H; const=0.0;; # v1.3
    Y)  EBcorr=0.140; ABcorr=0.618; CT=0.610;  band_col1=J; band_col2=H; const=0.0;; # v1.3
    J)  EBcorr=0.010; ABcorr=0.92;  CT=-0.077; band_col1=J; band_col2=H; const=0.0;; # v1.3
    H)  EBcorr=0.015; ABcorr=1.38;  CT=0.032;  band_col1=J; band_col2=H; const=0.0;; # v1.3
    Ks) EBcorr=0.005; ABcorr=1.84;  CT=0.010;  band_col1=J; band_col2=K; const=0.0;; # v1.3
    #Z)  ABcorr=0.521; CT=-0.077;  band_col1=J; band_col2=K; const=0.859;;  # v1.4
    #Y)  ABcorr=0.618; CT=-0.019;  band_col1=J; band_col2=K; const=0.457;;  # v1.4
    #J)  ABcorr=0.92;  CT=0.006;   band_col1=J; band_col2=K; const=-0.031;; # v1.4
    #H)  ABcorr=1.38;  CT=-0.005;  band_col1=J; band_col2=K; const=0.015;;  # v1.4
    #Ks) ABcorr=1.84;  CT=-0.007;  band_col1=J; band_col2=K; const=-0.006;; # v1.4
esac
#Average Schelgal dust value in KiDS
Schlegal=0.1

associate -i $cat ${TWOMASS_cat} \
          -o $wd/tmp1.cat_$$ $wd/tmp2.cat_$$ \
          -c @RUNROOT@/@CONFIGPATH@/associate_K1000.conf

bash @RUNROOT@/@SCRIPTPATH@/make_make_ssc_conf -i $wd/tmp1.cat_$$ -c 0 > $wd/make_ssc.conf_$$
bash @RUNROOT@/@SCRIPTPATH@/make_make_ssc_conf -i $wd/tmp2.cat_$$ -c 1 | \
   gawk \
	'BEGIN{FS="="}{if ($1=="COL_NAME") printf "%s_2MASS\n",$0; else print $0}' \
	>> $wd/make_ssc.conf_$$

ldacfilter -i $wd/tmp1.cat_$$ -o $wd/tmp3.cat_$$ -t OBJECTS -c "Pair_1>0;"
echo
ldacfilter -i $wd/tmp2.cat_$$ -o $wd/tmp4.cat_$$ -t OBJECTS -c "Pair_0>0;"
echo

make_ssc -i ${wd}/tmp3.cat_$$ ${wd}/tmp4.cat_$$ \
             -o ${wd}/merg_2MASS_comp.cat_$$ \
             -c $wd/make_ssc.conf_$$

ldacrentab -i $wd/merg_2MASS_comp.cat_$$ -o $wd/${base}_2MASS.cat \
		-t PSSC OBJECTS

rm $wd/*_$$

ldactoasc -i $wd/${base}_2MASS.cat -t OBJECTS -s -b -k \
    RAJ2000 \
    DECJ2000 \
    MAG_GAAP_$band \
    MAGERR_GAAP_$band \
    ${band2}mag_2MASS \
    Qflg_${band2}_2MASS \
    Rflg_${band2}_2MASS \
    Bflg_${band2}_2MASS \
    Cflg_${band2}_2MASS \
    Xflg_2MASS \
    Aflg_2MASS \
    FLAG_GAAP \
    Flag \
    ${band_col1}mag_2MASS \
    ${band_col2}mag_2MASS \
    | gawk \
    '{if ($3>0 && $3<90 && $6<=3 && $7>0 && $7<=3 && $8==2 && $9==1 && $10==0 && $11==0 && $12==0) print $1,$2,$3,$4,$5+('$CT'*($14-$15)+'$ABcorr'+('$EBcorr'*'$Schlegal')+'$const')}' \
    > $wd/${base}_2MASS_$band.asc
    #'{if ($3>0 && $3<99 && $4<0.5 && $6<=3 && $7>1 && $7<=4 && $8==2 && $9==1 && $10==0 && $11==0 && $12==0) print $1,$2,$3,$4,$5+('$CT'*($14-$15)+'$ABcorr'+('$EBcorr'*'$Schlegal')+'$const')}' \

python @RUNROOT@/@SCRIPTPATH@/phot_offset.py $wd/${base}_2MASS_$band.asc $mag_min $mag_max \
       > $wd/${base}_2MASS_${band}_offset.asc

median=`awk '{printf "%1.3f\n", $1}' $wd/${base}_2MASS_${band}_offset.asc`
NMAD=`awk   '{printf "%1.3f\n", $2}' $wd/${base}_2MASS_${band}_offset.asc`
mean=`awk   '{printf "%1.3f\n", $3}' $wd/${base}_2MASS_${band}_offset.asc`
std=`awk    '{printf "%1.3f\n", $4}' $wd/${base}_2MASS_${band}_offset.asc`

orig_dir=`pwd`

cd $wd

{
    echo 'unset key'
    echo 'set xlabel "'$band'\_2MASS"'
    echo 'set ylabel "'$band'\_VIKING - '$band'\_2MASS"'
    echo 'set term postscript eps color'
    echo 'set output "'${base}_2MASS_${band}_smart.eps'"'
    echo 'set arrow from '$mag_min',-2 to '$mag_min',2 nohead'
    echo 'set arrow from '$mag_max',-2 to '$mag_max',2 nohead'
    echo 'set label "median='$median'" at 14.1,1.5'
    echo 'set label "NMAD='$NMAD'" at 14.1,1.3'
    echo 'set label "mean='$mean'" at 14.1,1.1'
    echo 'set label "std='$std'" at 14.1,0.9'
    echo 'plot [14:18][-2:2]"'${base}_2MASS_${band}.asc'" u ($5):($3-$5), 0'
}|gnuplot
epstopdf ${base}_2MASS_${band}_smart.eps
#rm ${base}_2MASS_${band}_smart.eps

cd $orig_dir
