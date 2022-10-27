#!/bin/bash

# Script to convert GAaP fluxes to magnitudes
#
# Input: ASCII file with GAaP measurements (averaged)
#
# Output: FITS LDAC catalogue with GAaP magnitudes

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

gaapcat=$1
KIDScat=$2
outcat=$3
band=$4
ZP=$5
RA_key=$6
Dec_key=$7

dir=`dirname $gaapcat`
base=`basename $gaapcat .gaap`

gawk '{if ($2>0 && $4==0) print $1, $2, $3, '$ZP'-2.5*log($2)/log(10), sqrt((2.5*$3/log(10)/$2)**2), $4, $5, $6
      if ($2<0 && $4==0)  print $1, $2, $3, 99.0, sqrt((2.5*$3/log(10)/$2)**2), $4, $5, $6
      if ($2==0 || $4!=0) print $1, $2, $3, -99.0, -99.0, $4, $5, $6
     }' $gaapcat > $dir/$base.mag.tmp_$$

{
    echo 'COL_NAME = SeqNr'
    echo 'COL_TTYPE = LONG'
    echo 'COL_HTYPE = INT'
    echo 'COL_COMM = "sequence number"'
    echo 'COL_UNIT = " "'
    echo 'COL_DEPTH = 1'
    echo '#'
    echo 'COL_NAME = FLUX_GAAP_'$band
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = "'$band' flux"'
    echo 'COL_UNIT = "count"'
    echo 'COL_DEPTH = 1'
    echo '#'
    echo 'COL_NAME = FLUXERR_GAAP_'$band
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = "'$band' flux error"'
    echo 'COL_UNIT = "count"'
    echo 'COL_DEPTH = 1'
    echo '#'
    echo 'COL_NAME = MAG_GAAP_'$band
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = "'$band' magnitude"'
    echo 'COL_UNIT = "mag"'
    echo 'COL_DEPTH = 1'
    echo '#'
    echo 'COL_NAME = MAGERR_GAAP_'$band
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = "'$band' magnitude error"'
    echo 'COL_UNIT = "mag"'
    echo 'COL_DEPTH = 1'
    echo '#'
    echo 'COL_NAME = FLAG_GAAP'
    echo 'COL_TTYPE = SHORT'
    echo 'COL_HTYPE = INT'
    echo 'COL_COMM = "GAAP photometry Flag"'
    echo 'COL_UNIT = " "'
    echo 'COL_DEPTH = 1'
    echo '#'
    echo 'COL_NAME = GAAP_nexp'
    echo 'COL_TTYPE = SHORT'
    echo 'COL_HTYPE = INT'
    echo 'COL_COMM = "GAAP number of exposures"'
    echo 'COL_UNIT = " "'
    echo 'COL_DEPTH = 1'
    echo '#'
    echo 'COL_NAME = GAAP_chi_sq_dof'
    echo 'COL_TTYPE = FLOAT'
    echo 'COL_HTYPE = FLOAT'
    echo 'COL_COMM = "GAAP chi^2/dof"'
    echo 'COL_UNIT = " "'
    echo 'COL_DEPTH = 1'
    echo '#'
}> $dir/asctoldac_gaap_mag_${base}_$band.conf_tmp_$$

asctoldac -a $dir/$base.mag.tmp_$$ -o $dir/$base.mag.ldac.tmp_$$ \
	       -c $dir/asctoldac_gaap_mag_${base}_$band.conf_tmp_$$

ldacjoinkey -i $dir/$base.mag.ldac.tmp_$$ -o $dir/$base.cat.tmp_$$ \
		 -p $KIDScat -t OBJECTS \
		 -k $RA_key $Dec_key A_WORLD B_WORLD THETA_J2000 Flag

ldacaddkey -i $dir/$base.cat.tmp_$$ -o $dir/$base.cat \
    -t FIELDS -k CRVAL1 0.0 DOUBLE "" CRVAL2 0.0 DOUBLE "" CRPIX1 0.0 DOUBLE "" \
    CRPIX2 0.0 DOUBLE "" CDELT1 0.0 DOUBLE "" CDELT2 0.0 DOUBLE "" MAPNAXS1 0.0 \
    LONG "" MAPNAXS2 0.0 LONG ""

rm $dir/*tmp_$$
