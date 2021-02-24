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

md=$1
field=$2
mask=$3

NAXIS1=21000
NAXIS2=21000

awk 'BEGIN{srand(); for (i=0;i<1E6;i++) print '$NAXIS1'*rand(), '$NAXIS2'*rand()}' \
    > $md/${field}_rand_tmp.pos_$$

xy2sky -d $mask @$md/${field}_rand_tmp.pos_$$ | \
    awk '{print NR,$1,$2,$4,$5}' \
	> $md/${field}_rand_tmp2.pos_$$

echo
asctoldac -a $md/${field}_rand_tmp2.pos_$$ \
	  -o $md/${field}_rand_tmp.cat_$$ \
	  -t OBJECTS -c asctoldac_rand.conf

./addmask_fits.sh $md/${field}_rand_tmp.cat_$$ \
		  $md/${field}_rand.cat $mask \
		  MASK "" SHORT OBJECTS

rm $md/*tmp*_$$

