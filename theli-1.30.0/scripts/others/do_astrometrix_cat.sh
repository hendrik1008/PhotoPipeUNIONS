#!/bin/bash -u

# ----------------------------------------------------------------
# File Name:           do_astrometrix_cat.sh
# Author:              Thomas Erben (terben@astro.uni-bonn.de)
# Last modified on:    22.08.2007
# Description:         Extracts astrometric catalogues from FITS data
#                      for ASTROMETRIX
# Version:             $$VERSION$$
# ----------------------------------------------------------------

# Script history information:
# 18.10.2006:
# I extended the script to take into account a
# weight image and to output an additional skycat
# catalogue.
#
# 06.11.2006:
# The SExtractor threshhold and minarea parameters
# are command line arguments now.
#
# 02.02.2007:
# - user documentation included
# - possible use of FLAG images included
#   If FLAG images are used all objects having an external
#   flag larger than 0 are rejected.
# - configuration files for SExtracor are created 'on the fly'
#
# 22.08.2007:
# I now test whether the input image is present and not
# empty. This avoids creating empty catalogue files later on.
#
# 30.08.2007:
# The SExtractor executable is now called 'sex_theli'. I changed
# calls to that program.

# preliminary stuff:
. sexconfig_include

# define THELI_DEBUG because of the '-u' script flag
# (the use of undefined variables will be treated as errors!)
THELI_DEBUG=${THELI_DEBUG:-""}

# function definitions
function printUsage
{
    echo "SCRIPT NAME:"
    echo -e "    do_astrometrix_cat.sh\n"
    echo "SYNOPSIS:"
    echo "    do_astrometrix_cat.sh image"
    echo "                          weight_image"
    echo "                          flag image"
    echo "                          sexthreshhold"
    echo "                          sexarea"
    echo -e "                          output_catalogue\n"
    echo "DESCRIPTION:"
    echo "    The script runs SExtractor on an astronomic image"
    echo "    and creates a reference catalog ready for use with the"
    echo -e "    ASTROMETRIX tool.\n"
    echo "    The input image (first script argument) must have meaningful"
    echo "    astrometric information in its image header."
    echo "    If the input image is accompanied by"
    echo "    a weight map give it as second argument. Otherwise, set this"
    echo "    argument to 'NONE'. The same applies to a possible FLAG map"
    echo "    which can be specified as third argument. Fourth and fith"
    echo "    command line arguments are"
    echo "    detection threshold and minimum area for SExtractor detections."
    echo "    The sixth argument is the (base)name of the output catalogues."
    echo "    It is appended with '.asc' to give the catalogue to be used"
    echo "    with ASTROMETRIX. Another catalogue with the ending '_skycat.asc'"
    echo -e "    can be used to overlay detections with the 'skycat' tool.\n"
    echo "    The initial SExtractor catalogues are filtered for internal and"
    echo "    external (if present) flags. Only objects with flags equal to"
    echo -e "    zero are accepted.\n"
    echo "    The script needs THELI versions of the programs 'sex', "
    echo "    'ldacconv', 'ldacfilter', 'ldactoasc', 'ldactoskycat', "
    echo -e "    'decimaltohms' and 'decimaltodms'.\n"
    echo "AUTHOR"
    echo -e "    Thomas Erben (terben@astro.uni-bonn.de)\n"
}

function cleanTmpFiles
{
    if [ -z ${THELI_DEBUG} ]; then
        echo "Cleaning temporary files for script $0"
        test -f astrometrix_sex_$$.cat  && rm astrometrix_sex_$$.cat
        test -f astrometrix_ldac_$$.cat && rm astrometrix_ldac_$$.cat
        test -f astrometrix_good_$$.cat && rm astrometrix_good_$$.cat
        test -f ras_$$.txt              && rm ras_$$.txt
        test -f decs_$$.txt             && rm decs_$$.txt
        test -f mags_$$.txt             && rm mags_$$.txt

        test -f default.sex_$$          && rm default.sex_$$
        test -f default.param_$$        && rm default.param_$$
        test -f gauss_4.0_7x7.conv_$$   && rm gauss_4.0_7x7.conv_$$
        test -f default.nnw_$$          && rm default.nnw_$$
        test -f tmp_$$.asc              && rm tmp_$$.asc
    else
        echo "Variable THELI_DEBUG set! No cleaning of temp. files in script $0"
    fi
}

# Handling of program interruption by CRTL-C
trap "echo 'Script $0 interrupted!! Cleaning up and exiting!'; \
      cleanTmpFiles; exit 0" INT

# check validity of command line arguments:
if [ $# -ne 6 ]; then
    printUsage
    exit 1
fi

# check the presence of the primary image:
if [ ! -s "$1" ]; then
    echo "Image $1 not present or empty!! Exiting!"
    exit 1
fi

# create config files for SExtractor
writeSexConf                . default.sex_$$
writeSexParamAstrometrixCat . default.param_$$
writeSexFilterGauss_4.0_7x7 . gauss_4.0_7x7.conv_$$
writeSexNNW                 . default.nnw_$$

SEXLINE="-c ./default.sex_$$ \
         -PARAMETERS_NAME ./default.param_$$ \
         -FILTER_NAME ./gauss_4.0_7x7.conv_$$ \
         -STARNNW_NAME ./default.nnw_$$ \
         -CATALOG_NAME astrometrix_sex_$$.cat\
         -DETECT_THRESH $4 -DETECT_MINAREA $5"

# take into account a weight map if present
if [ "$2" != "NONE" ]; then
    SEXLINE="${SEXLINE} -WEIGHT_IMAGE $2 -WEIGHT_TYPE MAP_WEIGHT"
fi

# take into account a flag map if present. In this case also
# the parameter file needs to be updated
if [ "$3" != "NONE" ]; then
    SEXLINE="${SEXLINE} -FLAG_IMAGE $3 -FLAG_TYPE MAX"
    echo "IMAFLAGS_ISO" >> default.param_$$
fi

SEXLINE="${SEXLINE} $1"

# first run sextractor
sex_theli ${SEXLINE}


# now ldacconv, ldacfilter to select only objects with clean
# SExtractor flags.
ldacconv -i astrometrix_sex_$$.cat -o astrometrix_ldac_$$.cat -f I\
    -c "test" -b 1

# If a flag map is present take it into account when filtering objects
# Objects that are considered ok have external flag=0, all others
# a flag not equal to zero.
FILTERCOND="(Flag < 1);"
if [ "$3" != "NONE" ]; then
    FILTERCOND="(IMAFLAGS_ISO<1)AND${FILTERCOND}"
fi

ldacfilter -i astrometrix_ldac_$$.cat -o astrometrix_good_$$.cat \
    -c "${FILTERCOND}"

#
# create skycat catalogue:
ldactoskycat -i astrometrix_good_$$.cat -t OBJECTS \
    -k SeqNr ALPHA_J2000 DELTA_J2000 MAG_AUTO\
    -l id_col SeqNr ra_col ALPHA_J2000 dec_col DELTA_J2000\
    mag_col MAG_AUTO > $6_skycat.asc

#
# create ASTROMETRIX catalogue. It needs a bit of work because
# coordinates have to be provided in sexagesimal notation and we
# get decimal numbers!
ldactoasc -b -i astrometrix_good_$$.cat -t OBJECTS \
    -k ALPHA_J2000 DELTA_J2000 MAG_AUTO\
    > tmp_$$.asc

awk '{print $1}' tmp_$$.asc | decimaltohms -f - > ras_$$.txt
awk '{print $2}' tmp_$$.asc | decimaltodms -f - > decs_$$.txt
awk '{print $3}' tmp_$$.asc > mags_$$.txt

# we assume that the user wants to replace an exisiting result file
# (No checking or asking!!)
test -f $6.asc && rm $6.asc

paste ras_$$.txt decs_$$.txt mags_$$.txt > $6.asc

# clean up temporary files
cleanTmpFiles

exit 0;
