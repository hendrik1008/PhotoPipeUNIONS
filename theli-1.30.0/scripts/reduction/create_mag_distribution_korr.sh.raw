#!/bin/bash -u

# last update: 14.01.2013
# Authors     : Marco Hetterscheidt & Thomas Erben
#

# 23.01.2006:
# The script was adapted to changes in mag_distribution_korr2.sm.
# The instrument to be used (WFI or CFH12K for now)
# has to be given as script argument now
#
# 30.08.2006:
# The script was adapted to changes in mag_distribution_korr.sm
# (which was formerly mag_distribution_korr2.sm). We included
# MEGAPRIME@CFHT as new instrument and the describing string
# for comparing number counts is a command line argument now.
#
# 07.02.2013:
# - I introduced the new THELI logging scheme into the script.
# - For KIDS fields, a string that 'sm' was given could become
#   too long - fixed.
#
# 12.02.2013:
# If the initial object catalogue is not present we quit with
# an error message.
#
# 14.01.2014:
# change in the device name of the sm output plot to avoid too long
# strings for 'sm'.

#$1: main dir.
#$2: science dir.
#$3: name of coadded image (without FITS extension)
#$4 colourindex to compare with McCracken et al. 2004 (r,b,v,i bands)
#$5 CLASS_STAR  -
#$6 FLUX_RADIUS -- values to separate between stars & gal.+ fit interval ($4)
#$7 MAG_AUTO    -
#$8: string for marking comparison number counts

# The script calls the sm-script "mag_distribution_korr2.sm". The
# sm-program expects a asc-file:Xpos Ypos MAG_AUTO ISO0 FLUX_RADIUS CLASS_STAR

# It calculates the number of galaxies and stars per 0.5 mag and sq. degree.
# It takes into account that each object occupies an area, so that darker
# objects can not be detected in this area. For this corr. the isophotal area
# ISO0 (isophote above analysis threshold) and the pixel size of pz is taken .

# The total area is calculated from the position of detected objects.

# To separate stars & gal., CLASS_STAR is taken. Objects with CLASS_STAR > cs
# are defined as stars, if additionally the FLUX_RADIUS < fr (close to half
# light radius of stars) and MAG_AUTO < ma (close to saturation level).

# The fit of the logarithmic number counts is an error weighted liner regres.
# of the data points between mag ma and the 4th last point (detection limit).

# By default the separate parameters are: cs=0.04, fr=2.5, ma=17.5 and
# the default filter colour is R

# If you want to change at least one parameter, you have to type all four
# values.

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# check whether an object catalogue is present at all:
if [ ! -s /$1/$2/postcoadd/cats/$3_sex_ldac.asc ]; then
  theli_error "Catalogue /$1/$2/postcoadd/cats/$3_sex_ldac.asc not present!"
  exit 1;
else
  # The following 'cp' is necessary as sm cannot
  # deal with too long strings
  #
  cp /$1/$2/postcoadd/cats/$3_sex_ldac.asc ./tmp_$$.asc

  # Note: We give in the following sm-commands in the 'dev' line
  # a filename that is later moved to another one. It seems that
  # 'sm' has issues when strings get too long ...
  {
    echo "define TeX_strings 0"
    echo 'macro read "'${SMMACROS}'/mag_distribution_korr.sm"'
    echo 'dev postencap "/'$1'/'$2'/postcoadd/plots/tmp.ps_'$$'"'
    echo 'plot tmp_'$$'.asc '${INSTRUMENT}' "'$8'"'

    COLOUR=${4}
    CLASS_STAR=${5}
    FLUX_RADIUS=${6}
    MAG_AUTO=${7}

    echo "$3"
    echo "${COLOUR}"
    echo "${CLASS_STAR}"
    echo "${FLUX_RADIUS}"
    echo "${MAG_AUTO}"
    echo "${PIXSCALE}"
  } | ${P_SM}

  mv /$1/$2/postcoadd/plots/tmp.ps_$$ \
     /$1/$2/postcoadd/plots/$3_mag_distribution.ps
fi # if [ ! -s /$1/$2/postcoadd/cats/$3_sex_ldac.asc ]

rm tmp_$$.asc

theli_end
exit 0;
