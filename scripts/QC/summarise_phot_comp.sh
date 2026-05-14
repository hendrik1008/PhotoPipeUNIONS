#!/bin/bash

export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/

md=/arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS_DR6/
wd=/arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS_DR6_QC/phot_comp/

test ! -d $wd && mkdir -p $wd

for survey in PGM PS SDSS
do
    echo $survey
    for MERGE in MERGE preMERGE
    do
	for filter in u g r i z z2
	do
	    echo $filter
	    rm $wd/${filter}_${survey}_offsets_tile_$MERGE.asc
	    if [ $MERGE = "MERGE" ]
	    then
		for file in $md/UNIONS.*/phot_comp_$survey/UNIONS.???.???_ugriz_${survey}_${filter}_offset.asc
		do
		    tile=`dirname $file|cut -d "/" -f 10|cut -d "." -f 2-3`
		    WCS=`@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/translate_THELI2MP.py $tile|sed 's/\_/\ /g'`
		    { 
			echo -n $tile $WCS" "
	    		awk '!/nan/' $file
		    } |awk 'NF==8' >> $wd/${filter}_${survey}_offsets_tile_$MERGE.asc
		done
	    else
		for file in $md/UNIONS.*/$filter/UNIONS.???.???_${filter}_smart_full_${survey}_${filter}_offset.asc
		do
		    tile=`dirname $file|cut -d "/" -f 10|cut -d "." -f 2-3`
		    WCS=`@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/translate_THELI2MP.py $tile|sed 's/\_/\ /g'`
		    { 
			echo -n $tile $WCS" "
	    		awk '!/nan/' $file
		    } |awk 'NF==8' >> $wd/${filter}_${survey}_offsets_tile_$MERGE.asc
		done
	    fi
	done
	
	#@RUNROOT@/INSTALL/anaconda2/bin/python
	python3 @RUNROOT@/@SCRIPTPATH@/QC/summarise_phot_comp.py $wd $MERGE $survey \
					       > $wd/phot_comp_summary_${survey}_tile_${MERGE}.txt
    done
done
