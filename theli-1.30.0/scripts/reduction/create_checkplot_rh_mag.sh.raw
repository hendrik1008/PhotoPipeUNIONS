#!/bin/bash -u

# The script creates a mag-rh plot showing objects that SExtractor
# classifies as stars as red dots.
#

# last update: 13.09.2013
# authors     : Marco Hetterscheidt / Thomas Erben

# 30.05.04:
# temporary files go to a TEMPDIR directory
#
# 05.07.2004:
# corrected an error in giving the TEMPDIR to sm
#
# 07.02.2013:
# I introduced the new THELI logging scheme into the script
#
# 12.02.2013:
# If the initial object catalogue is not present we quit with
# an error message.
#
# 13.09.2013:
# Temporary files now get a unique identifier with the process
# number.
#
# 14.01.2014:
# change in the device name of the sm output plot to avoid too long
# strings for 'sm'.
#
# 11.05.2015:
# The output plot now contains a top-labell indicating the image the
# plot belongs to. I adapted this script accordingly.

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

#$1: main dir.
#$2: science dir.
#$3: name of coadded image (without FITS extension)
#$4: CLASS_STAR value    -
#$5: FLUX_RADIUS value   --- for separation between stars and galaxies and
#$6: MAG_AUTO value      -   fitting range (MAG_AUTO)

# The sm-script only plots MAG_AUTO against half light radius (FLUX_RADIUS),
# where the red dots indicate the stars to check separation values.

# check whether we have an objects catalogue:
if [ ! -s /$1/$2/postcoadd/cats/$3_sex_ldac.cat ]; then
  theli_error "Catalogue /$1/$2/postcoadd/cats/$3_sex_ldac.cat not present!"
  exit 1;
else
  ${P_LDACFILTER} -i /$1/$2/postcoadd/cats/$3_sex_ldac.cat \
           -o ${TEMPDIR}/tmp.cat_$$ -c '(CLASS_STAR > '$4');'
  ${P_LDACFILTER} -i ${TEMPDIR}/tmp.cat_$$ \
           -o /$1/$2/postcoadd/cats/$3_sex_ldac_stars.cat \
           -c '((FLUX_RADIUS < '$5') OR (MAG_AUTO < '$6'))AND(MAG_AUTO > 16);'

  ${P_LDACTOASC} -i /$1/$2/postcoadd/cats/$3_sex_ldac.cat -t OBJECTS \
                 -k FLUX_RADIUS MAG_AUTO -b > ${TEMPDIR}/tmp.asc_$$
  ${P_LDACTOASC} -i /$1/$2/postcoadd/cats/$3_sex_ldac_stars.cat \
                 -t OBJECTS -k FLUX_RADIUS MAG_AUTO -b > ${TEMPDIR}/tmp2.asc_$$

  {
    echo 'macro read "'${SMMACROS}'/magrh.sm"'
    echo 'dev postencap "/'$1'/'$2'/postcoadd/plots/tmp.ps_'$$'"'
    echo 'plot "'${TEMPDIR}'/tmp.asc_'$$'" "'${TEMPDIR}'/tmp2.asc_'$$'" "'$3'"'
  } | ${P_SM}
fi

mv /$1/$2/postcoadd/plots/tmp.ps_$$ /$1/$2/postcoadd/plots/$3_magrh.ps

# clean up and bye:
rm ${TEMPDIR}/*_$$

theli_end
exit 0;


