#!/bin/bash

infile=$1
manualmask=$2
outfile=$3

inbase=`basename $infile .fits`
temp_dir=`dirname $outfile`

#gunzip -c $infile > $temp_dir/$inbase.tmp$$.fits
if [ "$outfile" == "$infile" ]
then 
  #the input is overwritten by outfile
  mv $infile $temp_dir/$inbase.tmp$$.fits
else 
  #the input is different and kept 
  ln -sf $infile $temp_dir/$inbase.tmp$$.fits
fi 

# run weight watcher on the manual mask
@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ww_theli \
 -c @RUNROOT@/@CONFIGPATH@/MAKEFLAGMASK.default.ww \
 -WEIGHT_NAMES $temp_dir/$inbase.tmp$$.fits \
 -WEIGHT_OUTFLAGS 0 \
 -WEIGHT_MIN -1 \
 -WEIGHT_MAX -1 \
 -POLY_NAMES $manualmask \
 -FLAG_NAMES "" \
 -POLY_OUTFLAGS 128 \
 -VERBOSE_TYPE FULL \
 -OUTFLAG_NAME $temp_dir/manual_mask.tmp$$.fits \
 -OUTWEIGHT_NAME ""

# ic the weight watcher FITS output and infile > outfile
@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic -p 16 '%1 %2 +' \
   $temp_dir/$inbase.tmp$$.fits \
   $temp_dir/manual_mask.tmp$$.fits \
   > $outfile

#gzip -f $outfile

#rm $temp_dir/*tmp$$.fits
