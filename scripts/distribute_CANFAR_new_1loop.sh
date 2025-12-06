#!/bin/bash

#     CONVERT PREPARE GAUSSIANISE GAAP SDSSPREP ZPREP PSPREP COMPTILE \
#     COMPTILEPS MERGE COMPTILEPOSTMERGE COMPTILEPSPOSTMERGE BPZ \
#     COMPTILEZ MASK QC CLEAN COPY COPYBACK ERASE \

nsession=$1
ntile=$2
tile_list=$3
acronym=$4
wait_sec=$5
nthread=$6
shift
shift
shift
shift
shift
shift

n_tile_list=`wc $tile_list|awk '{print $1}'`
niter=`echo $n_tile_list $ntile|awk '{printf "%d\n",$1/$2+0.5}'`

date

for i in `seq 0 $[$niter-1]`
do
    tile_no_start=$[$ntile*$i+1]
    tile_no_end=$[$ntile*$i+$ntile]
    tile_list_base=`basename $tile_list .txt`
    awk '{if (NR>='$tile_no_start' && NR<='$tile_no_end') print $0}' \
	$tile_list \
	> @RUNROOT@/${tile_list_base}_${tile_no_start}t${tile_no_end}.txt
    echo   "bash @RUNROOT@/run_PhotoPipe.sh \
	   @RUNROOT@/${tile_list_base}_${tile_no_start}t${tile_no_end}.txt \
	   ${tile_no_start}t${tile_no_end} $nthread \
	   $*\
	   >& @RUNROOT@/PhotoPipe_${tile_list_base}_${tile_no_start}t${tile_no_end}.log" \
	   >@RUNROOT@/PhotoPipe_${tile_list_base}_${tile_no_start}t${tile_no_end}.sh
    echo Launching session with tiles ${tile_no_start} to ${tile_no_end}.
    canfar launch headless skaha/improc:latest --cpu 1 --memory 4 --  \
	   bash @RUNROOT@/PhotoPipe_${tile_list_base}_${tile_no_start}t${tile_no_end}.sh
    sleep $wait_sec
    echo
    while :
    do
	nsession_run=`canfar ps -q -k headless -s Running|wc|awk '{print $1}'`
	nsession_pen=`canfar ps -q -k headless -s Pending|wc|awk '{print $1}'`
	nsession_live=$[$nsession_run+$nsession_pen]
	if [ $nsession_live -ge $nsession ]
	then
	   echo -e '\e[1A\e[KMaximum number of $nsession sessions running/pending. Waiting.'
	   sleep $wait_sec
	else
	    break
	fi
    done
    i=$[$i+1]
done

date
