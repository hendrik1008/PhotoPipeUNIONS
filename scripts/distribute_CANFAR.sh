#!/bin/bash

iterstart=$1
iterend=$2
nsession=$3
ntile=$4
list=$5
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
	echo Launching session with tiles $[$[$ntile*$nsession]*$j+$ntile*$i+1] to $[$[$ntile*$nsession]*$j+$ntile*$i+$ntile].
	curl -k -E ~/.ssl/cadcproxy.pem \
    	     https://ws-uv.canfar.net/skaha/v0/session \
    	     -d "name=${acronym}-$[$[$ntile*$nsession]*$j+$ntile*$i+1]t$[$[$ntile*$nsession]*$j+$ntile*$i+$ntile]" \
    	     -d "image=images.canfar.net/skaha/improc:24.04" \
    	     -d "cmd=@RUNROOT@/CANFAR_launch.sh" \
    	     -d "args=$[$[$ntile*$nsession]*$j+$ntile*$i+1] $[$[$ntile*$nsession]*$j+$ntile*$i+$ntile] @RUNROOT@/$list ${acronym}-$[$[$ntile*$nsession]*$j+$ntile*$i+1]t$[$[$ntile*$nsession]*$j+$ntile*$i+$ntile] $nthread $*" \
    	     -d "type=headless" \
	     -d "cores=2" \
	     -d "ram=8"
	sleep $waitshort
    done
    date
    echo Waiting for $waitlong seconds before launching next iteration.
    sleep $waitlong
done
