#!/bin/bash

# Script to compare GAaP measurements with SDSS
#
# Input: SDSS catalogue and GAaP catalogue
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
SDSS_cat=$2
cat=$3
band=$4
field=$5

base=`basename $cat .cat`

associate -i $cat ${SDSS_cat} \
               -o $wd/tmp1.cat_$$ $wd/tmp2.cat_$$ \
               -c @RUNROOT@/@CONFIGPATH@/associate_UNIONS.conf

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

python @RUNROOT@/@SCRIPTPATH@/phot_offset_FITS.py \
       $wd/${base}_SDSS $wd/${base}_SDSS.cat $band $field \
       > $wd/${base}_SDSS_${band}_offset.asc
