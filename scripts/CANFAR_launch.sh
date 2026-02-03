#!/bin/bash

tile_no_start=$1
tile_no_end=$2
tile_list=$3
acronym=$4
nthread=$5
shift
shift
shift
shift
shift
# other parameters: modes to run

source @RUNROOT@/INSTALL/anaconda2/bin/activate base

#     CONVERT PREPARE GAUSSIANISE GAAP SDSSPREP ZPREP PSPREP COMPTILE \
#     COMPTILEPS MERGE COMPTILEPOSTMERGE COMPTILEPSPOSTMERGE BPZ \
#     COMPTILEZ MASK QC CLEAN COPY COPYBACK ERASE \

tile_list_base=`basename $tile_list .txt`

awk '{if (NR>='$tile_no_start' && NR<='$tile_no_end') print $0}' \
    $tile_list \
    > @RUNROOT@/${tile_list_base}_${tile_no_start}t${tile_no_end}.txt
bash @RUNROOT@/run_PhotoPipe.sh \
     @RUNROOT@/${tile_list_base}_${tile_no_start}t${tile_no_end}.txt \
     ${tile_no_start}t${tile_no_end} $nthread \
     $*\
     >& @RUNROOT@/PhotoPipe_${tile_list_base}_${tile_no_start}t${tile_no_end}.log
