#!/bin/bash

tile_no_start=$1
tile_no_end=$2
shift
shift

source @RUNROOT@/INSTALL/anaconda2/bin/activate

awk '{if (NR>='$tile_no_start' && NR<='$tile_no_end') print $0}' \
    @RUNROOT@/r_tiles_rest.txt \
    > @RUNROOT@/r_tiles_rest_${tile_no_start}t${tile_no_end}.txt
bash @RUNROOT@/run_PhotoPipe.sh \
     @RUNROOT@/r_tiles_rest_${tile_no_start}t${tile_no_end}.txt \
     ${tile_no_start}t${tile_no_end} \
     CONVERT PREPARE GAUSSIANISE GAAP SDSSPREP ZPREP COMPTILE MERGE \
     BPZ COMPTILEZ MASK QC CLEAN COPY \
     >& @RUNROOT@/PhotoPipe_r_tiles_rest_${tile_no_start}t${tile_no_end}.log

#awk '{if (NR>='$tile_no_start' && NR<='$tile_no_end') print $0}' \
#    @RUNROOT@/ugriz_tiles.txt \
#    > @RUNROOT@/ugriz_tile_${tile_no_start}t${tile_no_end}.txt
#bash @RUNROOT@/run_PhotoPipe.sh \
#     @RUNROOT@/ugriz_tile_${tile_no_start}t${tile_no_end}.txt \
#     ${tile_no_start}t${tile_no_end} \
#     CONVERT PREPARE GAUSSIANISE GAAP SDSSPREP ZPREP COMPTILE MERGE \
#     BPZ COMPTILEZ MASK QC CLEAN COPY \
#     >& @RUNROOT@/PhotoPipe_ugriz_tile_${tile_no_start}t${tile_no_end}.log
