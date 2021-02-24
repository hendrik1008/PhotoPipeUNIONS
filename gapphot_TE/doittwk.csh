#!/usr/bin/tcsh

# $1 input image
# $2 PSF catalogue
# $3 GPSF size
# $4 script directory
# $5 output kernel map
# $6 output Gaussianised FITS image

if $# != 6 then
   echo Usage: doit image PSFcat GPSFSIG SCRIPT_DIR output_kernel output_FITS
   exit
   endif

set sci = $1
set psf = $2
set sig = $3
set script_dir = $4
set ker = $5
set gps = $6

tcsh $script_dir/mkkermaptwk.csh $sci $psf $sig $ker $script_dir
tcsh $script_dir/im2gautwk.csh $sci $ker $gps $script_dir
tcsh $script_dir/chkgau.csh $gps $psf $script_dir

rm bgnoise.dat
rm noise-est.ps
rm -f inimage.fits
