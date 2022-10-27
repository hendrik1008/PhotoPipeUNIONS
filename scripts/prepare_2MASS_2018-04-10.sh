#!/bin/bash

# Script to prepare a 2MASS FITS catalogue downloaded
# from http://vizier.u-strasbg.fr/viz-bin/VizieR?-source=II/246&-to=3
#
# Input:
# - 2MASS FITS catalogue
#
# Output:
# - 2MASS FITS LDAC catalogue
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2017-03-02 V1.0

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
KiDS_field=$2
RA=$3
Dec=$4

python @RUNROOT@/@SCRIPTPATH@/get_standard_cat_2MASS.py \
       -r $RA \
       -d $Dec \
       --ra-range 70. \
       --dec-range 70. \
       -o $md/${KiDS_field}_2MASS.cat

ldacaddkey -i $md/${KiDS_field}_2MASS.cat -o $md/tmp1_$$ -t OBJECTS \
		-k \
		Flag 0 SHORT "" \
		A_WORLD 0.000139 FLOAT "" \
		B_WORLD 0.000139 FLOAT "" \
		THETA_J2000 0.0 FLOAT "" \
		FIELD_POS 1 SHORT ""

no_obj=`ldacdesc -i $md/tmp1_$$ | grep elements | \
gawk '{if (NR==1) print $0}' | \
cut -d " " -f 3 | sed 's/\.//g' | cut -d ":" -f 2`

gawk 'BEGIN{for (i=1;i<='$no_obj';i++) print i}' \
	  > $md/SeqNr

asctoldac -a $md/SeqNr -o $md/SeqNr.cat \
	       -t OBJECTS -c @RUNROOT@/@CONFIGPATH@/asctoldac_SeqNr.conf

ldacjoinkey -i $md/tmp1_$$ -p $md/SeqNr.cat \
		 -o $md/tmp2_$$ -t OBJECTS -k SeqNr

ldacaddtab -i $md/tmp2_$$ -o $md/tmp3_$$ \
		-p /net/fohlen11/home/hendrik/data/KiDS/VOICE-COSMOS/KIDS-COSMOS_r_sci.cat \
		-t FIELDS

ldactoasc -i $md/tmp3_$$ -t OBJECTS -b -s -k Qflg | \
    gawk 'BEGIN{FS=""}{print $1,$2,$3}' \
    > $md/tmp4_$$

asctoldac -a $md/tmp4_$$ -o $md/tmp5_$$ -c @RUNROOT@/@CONFIGPATH@/asctoldac_Qflg.conf

ldacjoinkey -i $md/tmp3_$$ -o $md/tmp6_$$ -p $md/tmp5_$$ \
		 -t OBJECTS -k Qflg_J Qflg_H Qflg_K

ldactoasc -i $md/tmp6_$$ -t OBJECTS -b -s -k Rflg | \
    gawk 'BEGIN{FS=""}{print $1,$2,$3}' \
    > $md/tmp7_$$

asctoldac -a $md/tmp7_$$ -o $md/tmp8_$$ -c @RUNROOT@/@CONFIGPATH@/asctoldac_Rflg.conf

ldacjoinkey -i $md/tmp6_$$ -o $md/tmp9_$$ -p $md/tmp8_$$ \
		 -t OBJECTS -k Rflg_J Rflg_H Rflg_K

ldactoasc -i $md/tmp9_$$ -t OBJECTS -b -s -k Bflg | \
    gawk 'BEGIN{FS=""}{print $1,$2,$3}' \
    > $md/tmp10_$$

asctoldac -a $md/tmp10_$$ -o $md/tmp11_$$ -c @RUNROOT@/@CONFIGPATH@/asctoldac_Bflg.conf

ldacjoinkey -i $md/tmp9_$$ -o $md/tmp12_$$ -p $md/tmp11_$$ \
		 -t OBJECTS -k Bflg_J Bflg_H Bflg_K

ldactoasc -i $md/tmp12_$$ -t OBJECTS -b -s -k Cflg | \
    gawk 'BEGIN{FS=""}{print $1,$2,$3}' \
    > $md/tmp13_$$

asctoldac -a $md/tmp13_$$ -o $md/tmp14_$$ -c @RUNROOT@/@CONFIGPATH@/asctoldac_Cflg.conf

ldacjoinkey -i $md/tmp12_$$ -o $md/tmp15_$$ -p $md/tmp14_$$ \
		 -t OBJECTS -k Cflg_J Cflg_H Cflg_K

cp $md/tmp15_$$ $md/${KiDS_field}_2MASS.cat

rm $md/*_$$ 

rm $md/SeqNr*
