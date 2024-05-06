#!/bin/bash

md=/net/home/fohlen14/hendrik/UNIONS/PhotoPipe/work_UNIONS2000/

test ! -d $md/phot_comp && mkdir $md/phot_comp

for ending in "_" #_minaper1p0_ # _stars_ _stars0p7_ 
do
    #for filter in u g i z #r
    #do
    #	cat $md/UNIONS.*/$filter/*smart${ending}full_SDSS_${filter}_offset.asc \
    #	    > $md/phot_comp/${filter}${ending}SDSS_offsets_tile.asc
    #done
    
    python summarise_phot_comp.py $md ${ending} \
	   > $md/phot_comp/phot_comp_summary${ending}.txt
done
