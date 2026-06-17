#!/bin/bash

export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/

bd=/arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS_DR6/
md=$bd/../UNIONS_DR6_QC/

test ! -d $md/ && mkdir $md/

filters=ugriz

#pointings=ugriz_only_tiles100.txt
#pointings_tmp=ugriz_tiles.txt
#
#head -100 @RUNROOT@/$pointings_tmp > @RUNROOT@/$pointings
#
#FILES=""
#while read field
#do
#    FILES=$FILES" "$bd/$field/${field}_${filters}_photoz_ext.cat
#done<@RUNROOT@/$pointings
#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/paste_FITS_cats.py \
#				       $md/UNIONS100_${filters}_photoz_ext.cat \
#				       OBJECTS \
#				       $FILES

#@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/r_numbercounts.py \
#				       $md/UNIONS100_${filters}_photoz_ext.cat \
#				       Z_B \
#				       MAG_AUTO \
#				       9.9 \
#				       98.0 \
#				       UNIONS100 \
#				       $md/UNIONS100_${filters}_photoz_ext_nc

@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/r_numbercounts.py \
				       $md/UNIONS100_${filters}_photoz_ext.cat \
				       Z_B \
				       MAG_GAAP_i \
				       9.9 \
				       98.0 \
				       UNIONS100 \
				       $md/UNIONS100_${filters}_photoz_ext_nci
