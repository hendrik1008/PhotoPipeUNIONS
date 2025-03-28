#!/bin/bash

#### xifjy6-soccuz-papsiV

start=$1
end=$2
ntiles=$3
nthread=$4
list=$5
acronym=$6

for j in `seq $start $end`
do
    date
    echo Starting iteration $[$j+1] of 209.
    echo Launching session with tiles $[${ntiles}*$j+1] to $[${ntiles}*$j+${ntiles}].
    /arc/home/hendrik/PhotoPipeTest//CANFAR_launch.sh \
    	$[${ntiles}*$j+1] $[${ntiles}*$j+${ntiles}] $list $acronym-$[${ntiles}*$j+1]t$[${ntiles}*$j+${ntiles}] $nthread
    date
done
