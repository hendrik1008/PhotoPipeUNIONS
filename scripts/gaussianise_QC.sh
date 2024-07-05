#!/bin/bash -xv

# Script to create QC plots for gaussianisation
#
# Input:
# - GAaP checkplots and input images
#
# Output:
# - A4 page with checkplots
#
# Author: Hendrik Hildebrandt
#
# Version history:
# 2024-01-22 V1.0

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/tex/bin/x86_64-linux/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/:@RUNROOT@/INSTALL/anaconda2/lib/
set -e
#}}}

### command line parameters

wd=$1   # work directory
tile=$2 # tile name
band=$3 # band of the image, i.e. u, g, r, i, z

python @RUNROOT@/@SCRIPTPATH@/chimney_plot.py \
       $wd $tile $band

album -b 10 1 1 $wd/${tile}_${band}.fits > $wd/${tile}_${band}.small.fits
album -b 10 1 1 $wd/${tile}_${band}.weight.fits > $wd/${tile}_${band}.weight.small.fits

ic '1 0 %1 0 > ? %2 *' \
   $wd/${tile}_${band}.weight.small.fits \
   $wd/${tile}_${band}.small.fits \
   > $wd/${tile}_${band}.cleaned.small.fits
fits2bitmap --min_percent 1 --max_percent 95 \
	    $wd/${tile}_${band}.cleaned.small.fits
fits2bitmap --min_percent 1 --max_percent 99 \
	    $wd/${tile}_${band}.weight.small.fits

test -s $wd/${tile}_${band}_smart_ggpsf_map.ps && \
    ps2pdf $wd/${tile}_${band}_smart_ggpsf_map.ps $wd/${tile}_${band}_smart_ggpsf_map.pdf
test -s $wd/${tile}_${band}_smart_ker.map.ps && \
    ps2pdf $wd/${tile}_${band}_smart_ker.map.ps $wd/${tile}_${band}_smart_ker.map.pdf
test -s $wd/${tile}_${band}_smart_ggpsfstarpos.ps && \
    ps2pdf $wd/${tile}_${band}_smart_ggpsfstarpos.ps $wd/${tile}_${band}_smart_ggpsfstarpos.pdf
test -s $wd/${tile}_${band}_smart_ggpsfsticks.ps && \
    ps2pdf $wd/${tile}_${band}_smart_ggpsfsticks.ps $wd/${tile}_${band}_smart_ggpsfsticks.pdf

cp @RUNROOT@/@CONFIGPATH@/QC_landscape.tex $wd/${tile}_${band}_QC.tex

for OPT in tile band
do
    sed -i "s#\@${OPT}\@#${!OPT}#g" $wd/${tile}_${band}_QC.tex
done

cd $wd
pdflatex -interaction=batchmode ${tile}_${band}_QC.tex
