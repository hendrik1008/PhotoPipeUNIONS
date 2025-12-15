#!/bin/bash

md=$1

tile=`basename $md`

# Check PREPARE failures.
for band in u g r i z z2
do
    if [ -s $md/$band/${tile}_${band}.fits ]
    then
	size=`ls -l $md/$band/${tile}_${band}.fits|awk '{print $5}'`
	sizew=`ls -l $md/$band/${tile}_${band}.weight.fits|awk '{print $5}'`
	if [ $size -lt 400000000 ] || [ $sizew -lt 400000000 ]
	then
	    rm -rf $md/$band
	    rm -rf $md/${tile}*
	    rm -rf $md/BPZ*
	    rm -rf $md/phot_comp*
	fi
    fi
done

