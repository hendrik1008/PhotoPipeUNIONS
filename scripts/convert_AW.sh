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

md=$1
field=$2
od=$3

base=${field}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_GAaP_@REFERENCE@
cat=$md/${base}.fits 

python @RUNROOT@/@SCRIPTPATH@/convert_AW.py \
       $cat $od/$base.asc.tmp_$$ $od/asctoldac.conf_$$ 2>> @LOGFILE@

awk 'NR>1' $od/$base.asc.tmp_$$ \
    | sed 's/KIDS\ /KIDS\_/g' \
    | sed 's/\"//' \
    | sed 's/\"//' > $od/$base.asc

asctoldac \
    -a $od/$base.asc \
    -o $od/$base.ldac.tmp.cat_$$ \
    -c $od/asctoldac.conf_$$

python @RUNROOT@/@SCRIPTPATH@/add_header.py \
       $od/$base.ldac.tmp.cat_$$ \
       $od/$base.ldac.cat \
       $cat

rm $od/*_$$
rm $od/$base.asc
