#!/bin/bash

# Script to compare BPZ measurements with DEEP specz
#
# Input: DEEP catalogue and GAaP catalogue
#
# Output: - Merged catalogue
#         - Photometric offset
#         - Comparison plot

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

wd=$1
DEEP_cat=$2
cat=$3
field=$4

base=`basename $cat .cat`

ldacaddkey -i $cat \
		-o ${cat}_tmp_$$ \
		-t FIELDS \
		-k CRVAL1 0.0 DOUBLE ""\
		CRVAL2 0.0 DOUBLE ""\
		CRPIX1 0.0 DOUBLE ""\
		CRPIX2 0.0 DOUBLE ""\
		CDELT1 0.0 DOUBLE ""\
		CDELT2 0.0 DOUBLE ""\
		MAPNAXS1 0.0 LONG ""\
		MAPNAXS2 0.0 LONG ""

associate -i ${cat}_tmp_$$ ${DEEP_cat} \
               -o $wd/tmp1.cat_$$ $wd/tmp2.cat_$$ \
               -c @RUNROOT@/@CONFIGPATH@/associate_UNIONS.conf

bash @RUNROOT@/@SCRIPTPATH@/make_make_ssc_conf -i $wd/tmp1.cat_$$ -c 0 > $wd/make_ssc.conf_$$
bash @RUNROOT@/@SCRIPTPATH@/make_make_ssc_conf -i $wd/tmp2.cat_$$ -c 1 | \
    gawk 'BEGIN{FS="="}{if ($1=="COL_NAME") printf "%s_DEEP\n",$0; else print $0}' \
	>> $wd/make_ssc.conf_$$

make_ssc -i ${wd}/tmp1.cat_$$ ${wd}/tmp2.cat_$$ \
             -o ${wd}/merg_DEEP_comp.cat_$$ \
             -c $wd/make_ssc.conf_$$

ldacfilter -i $wd/merg_DEEP_comp.cat_$$ -o $wd/merg_DEEP_comp2.cat_$$ \
		-t PSSC -c "((((RICHNESS>1)AND(MAG_GAAP_u>0))AND(MAG_GAAP_g>0))AND(MAG_GAAP_r>0))AND(MAG_GAAP_i>0);"
echo

ldacrentab -i $wd/merg_DEEP_comp2.cat_$$ -o $wd/${base}_DEEP.cat \
		-t PSSC OBJECTS

python @RUNROOT@/@SCRIPTPATH@/zz_plot.py \
       $wd/${base}_DEEP.cat \
       z_spec_DEEP \
       Z_B \
       $field \
       $wd/${base}_DEEP_zz.png \
       $wd/${base}_DEEP_zz.pdf

python @RUNROOT@/@SCRIPTPATH@/zz_stats.py \
       $wd/${base}_DEEP.cat \
       z_spec_DEEP \
       Z_B \
       > $wd/${base}_DEEP_zz.txt

rm $wd/*_$$

