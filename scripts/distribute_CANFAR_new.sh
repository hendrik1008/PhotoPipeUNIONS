#!/bin/bash

#     CONVERT PREPARE GAUSSIANISE GAAP SDSSPREP ZPREP PSPREP COMPTILE \
#     COMPTILEPS MERGE COMPTILEPOSTMERGE COMPTILEPSPOSTMERGE BPZ \
#     COMPTILEZ MASK QC CLEAN COPY COPYBACK ERASE \

iterstart=$1
iterend=$2
nsession=$3
ntile=$4
tile_list=$5
acronym=$6
waitshort=$7
waitlong=$8
nthread=$9
shift
shift
shift
shift
shift
shift
shift
shift
shift

for j in `seq $[$iterstart-1] $[$iterend-1]`
do
    date
    echo Starting iteration $[$j+1] of $[$iterend-$iterstart+1].
    for i in `seq 0 $[$nsession-1]`
    do
	tile_no_start=$[$[$ntile*$nsession]*$j+$ntile*$i+1]
	tile_no_end=$[$[$ntile*$nsession]*$j+$ntile*$i+$ntile]
	tile_list_base=`basename $tile_list .txt`
	awk '{if (NR>='$tile_no_start' && NR<='$tile_no_end') print $0}' \
	    $tile_list \
	    > @RUNROOT@/${tile_list_base}_${tile_no_start}t${tile_no_end}.txt
	echo Launching session with tiles ${tile_no_start} to ${tile_no_end}.
	canfar launch headless skaha/improc:latest --cpu 1 --memory 4 --  \
	       bash @RUNROOT@/run_PhotoPipe.sh \
	       @RUNROOT@/${tile_list_base}_${tile_no_start}t${tile_no_end}.txt \
	       ${tile_no_start}t${tile_no_end} $nthread \
	       $*\
	       >& @RUNROOT@/PhotoPipe_${tile_list_base}_${tile_no_start}t${tile_no_end}.log
	#curl -k -E ~/.ssl/cadcproxy.pem \
    	#     https://ws-uv.canfar.net/skaha/v0/session \
    	#     -d "name=${acronym}-$[$[$ntile*$nsession]*$j+$ntile*$i+1]t$[$[$ntile*$nsession]*$j+$ntile*$i+$ntile]" \
    	#     -d "image=images.canfar.net/skaha/improc:24.04" \
    	#     -d "cmd=@RUNROOT@/CANFAR_launch.sh" \
    	#     -d "args=  @RUNROOT@/$list ${acronym}-$[$[$ntile*$nsession]*$j+$ntile*$i+1]t$[$[$ntile*$nsession]*$j+$ntile*$i+$ntile] $nthread $*" \
    	#     -d "type=headless" \
	#     -d "cores=2" \
	#     -d "ram=8"
	sleep $waitshort
    done
    date
    echo Waiting for $waitlong seconds before launching next iteration.
    sleep $waitlong
done
