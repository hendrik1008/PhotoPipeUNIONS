#!/bin/bash -u

# script to merge scamp catalogues.  create_scamp_astrom_photom.sh can
# run in different MODES to produce optimal output for astrometric and
# relative photometric calibration. This script can merge the header
# outputs of such runs, i.e. produce headers with astrometric
# info. from the astrometric run and respectively for photometry.

# HISTORY COMMENTS:
# =================
#
# 07.02.2013:
# I introduced the new THELI logging scheme into the script.

#$1: MD
#$2: science dir.
#$3: dir. with astrom scamp headers (subdir. of /$1/$2)
#$4: dir. with photom scamp headers (subdir. of /$1/$2)
#$5: dir. with merged scamp headers (subdir. of /$1/$2)

. ./${INSTRUMENT:?}.ini
. ./bash_functions.include
theli_start "$*"

# These fields are substituted in the astrometric headers
# and replaced with values from the photometric run:
FIELDS="MAGZEROP PHOTIRMS PHOTINST PHOTLINK RZP FLXSCALE"

test -d /$1/$2/$3 || { theli_error "/$1/$2/$3 not present!"; exit 1; }
test -d /$1/$2/$4 || { theli_error "/$1/$2/$4 not present!"; exit 1; }
test -d /$1/$2/$5 && \
     { theli_warn "removing old /$1/$2/$5"; rm -rf /$1/$2/$5; }

mkdir /$1/$2/$5

FILES=`${P_FIND} /$1/$2/$3 -name \*.head`

if [ "${FILES}" != "" ]; then
  for FILE in ${FILES}
  do
    BASE=`basename ${FILE}`

    test -f /$1/$2/$4/${BASE} || \
      { echo "/$1/$2/$4/${BASE} not present"; exit 1; }

    cp /$1/$2/$3/${BASE} /$1/$2/$3/${BASE}.tmp_$$

    for FIELD in ${FIELDS}
    do
      LINE_OLD=`grep ${FIELD} /$1/$2/$3/${BASE}`
      LINE_NEW=`grep ${FIELD} /$1/$2/$4/${BASE}`

      if [ -n "${LINE_OLD}" ]; then
        sed "s!${LINE_OLD}!${LINE_NEW}!" /$1/$2/$3/${BASE}.tmp_$$ > \
          /$1/$2/$3/${BASE}.tmp2_$$
      else
        cp /$1/$2/$3/${BASE}.tmp_$$ /$1/$2/$3/${BASE}.tmp2_$$
      fi

      mv /$1/$2/$3/${BASE}.tmp2_$$ /$1/$2/$3/${BASE}.tmp_$$
    done
    mv /$1/$2/$3/${BASE}.tmp_$$ /$1/$2/$5/${BASE}
  done
else
  theli_error "No files to process! Exiting!"
  exit 1;
fi

theli_end
exit 0;
