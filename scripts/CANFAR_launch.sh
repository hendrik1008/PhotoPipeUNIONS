#!/bin/bash

tile_no_start=$1
tile_no_end=$2
shift
shift

awk '{if (NR>='$tile_no_start' && NR<='$tile_no_end') print $0}' \
    @RUNROOT@/r_tiles.txt \
    > @RUNROOT@/r_tile_${tile_no_start}t${tile_no_end}.txt

source @RUNROOT@/INSTALL/anaconda2/bin/activate

bash @RUNROOT@/run_PhotoPipe.sh \
     @RUNROOT@/r_tile_${tile_no_start}t${tile_no_end}.txt \
     ${tile_no_start}t${tile_no_end} \
     CONVERT PREPARE GAUSSIANISE GAAP SDSSPREP ZPREP COMPTILE MERGE \
     BPZ BPZ5 COMPTILEZ MASK QC CLEAN COPY \
     >& @RUNROOT@/PhotoPipe_r_tile_${tile_no_start}t${tile_no_end}.log
