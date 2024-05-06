md=/net/home/fohlen14/hendrik/UNIONS/PhotoPipe/work_UNIONS2000/

while read tile
do
    statsfile=$md/$tile/${tile}_ugri_photoz_ext_SDSS_zz.txt
    if [ -f $statsfile ]
    then
	awk '{
	n=$1
	b=$2
	s=$3
	nmad=$4
	o15=$5
	o25=$6
	if (n>20 && o25>0.5) print "'$tile'"
	}' $statsfile
    fi
done<$md/../ugri_tiles.txt_clean.txt
