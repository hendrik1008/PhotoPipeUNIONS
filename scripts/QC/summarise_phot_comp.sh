#!/bin/bash

export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/

md=/arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/
wd=/arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000_QC/phot_comp/

test ! -d $wd && mkdir $wd

for survey in PS SDSS
do
    echo $survey
    for ending in "_" #_minaper1p0_ # _stars_ _stars0p7_ 
    do
	echo $ending
	#for filter in u g r i z 
	#do
	#    echo $filter
	#    rm $wd/${filter}${ending}${survey}_offsets_tile.asc
	#    for file in $md/UNIONS.*/phot_comp_$survey/UNIONS.???.???_ugriz_${survey}_${filter}_offset.asc
	#    do
	#	tile=`basename $file|cut -d "_" -f 1|cut -d "." -f 2-3`
	#	WCS=`@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/translate_THELI2MP.py $tile|sed 's/\_/\ /g'`
	#	{ 
	#	    echo -n $tile $WCS" "
	#    	    awk '!/nan/' $file
	#	} |awk 'NF==8' >> $wd/${filter}${ending}${survey}_offsets_tile.asc
	#    done
	#done
	
	@RUNROOT@/INSTALL/anaconda2/bin/python @RUNROOT@/@SCRIPTPATH@/QC/summarise_phot_comp.py $wd ${ending} $survey \
	       > $wd/phot_comp_summary${ending}_${survey}.txt
    done
done
