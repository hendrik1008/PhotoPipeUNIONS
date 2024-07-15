#!/bin/bash -xv

# script to estimate photo-zs with the BPZ photometric
# redshift code

# HISTORY INFORMATION
# ===================
#
# 06.08.2008:
# - We now take into account objects which were not detected in some bands,
#   i.e. have larger magnitudes than the limiting magnitudes or large 
#   photometric errors. These are treated in BPZ as non-detected. Objects
#   which have an IMAFLAGS_ISO set are treated as if the corresponding
#   filter(s) were not observed.
# - The output catalogue has flags indicating how many (and which) filters were
#   observed/flagged or not detected.
#
# 17.08.2008:
# I introduced the possibility to use LDAC vector
# quantities for magnitudes and magnitude errors


#$1: catalog directory
#$2: multi-colour catalogue
#$3: filters to use for photo-z
#    estimation (to be given within double quotes)
#$4: magnitude quantity to use for photo-z
#    estimation (give in quotes if you want an 
#    element from a vector. E.g. give "MAG_APER 10" 
#    if you want the tenth element of the MAG_APER 
#    key); the same element will be taken from
#    the magnitude error key!
#$5: magnitude error quantity to use
#$6: limitimg magnitude quantity
#$7: Flag quantity
#$8: magnitude extinction quantity
#$9: AB or Vega
#$10: minimum magnitude error
#$11: file with zeropoint offsets (OPTIONAL)
#$12: acronym for the recalib directory (OPTIONAL)

INSTRUMENT=UNIONS

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/python2:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/
export PYTHONPATH=${PYTHONPATH}:@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/:@RUNROOT@/INSTALL/anaconda2/lib/
set -e 
#}}}

export BPZPATH=@RUNROOT@/INSTALL/bpz-1.99.3_expanded/

recalib=""

if [ $# -gt 10 ]; then
    recalib=${12}
fi

test -d $1/BPZ_photoz$recalib || mkdir -p $1/BPZ_photoz$recalib

if [ $# -gt 10 ]; then
    cp ${11} $1/BPZ_photoz$recalib
fi

DIR=`pwd`

cd $1/BPZ_photoz$recalib

# build up strings to create ASCII catalogue 
# and a configuration file for BPZ
ASCSTRING="SeqNr MAG_AUTO" # to hold first seqnr and magnitudes;
                           # later all quantities
ASCTMPMAGERR="" # to hold key names of magnitudes errors
ASCTMPMAGLIM="" # to hold key names of limiting magnitudes
ASCTMPFLAG=""   # to hold names of flag keys
ASCTMPEXT=""    # to hold names of extinction keys
BASE=`basename $2 .cat`

test -f ${BASE}_photoz.columns && rm ${BASE}_photoz.columns

MAGQUANT=`echo $4 | awk '{print $1}'`
MAGINDEX=`echo $4 | awk '{print $2}'` # will be empty if '$4' does not
                                      # contain two elements

if [ "${MAGINDEX}_A" != "_A" ]; then
  MAGINDEX="(${MAGINDEX})"
fi

NFILT=`echo $3 | gawk '{print NF}'`
MAGCOL=3
MAGERRCOL=$(( ${MAGCOL} + ${NFILT} ))

OFFSETS="" 
FLAGKEY=0 # we need to know below whether FLAG keys are
          # there or not (to determine the column position
          # of certain keys in the objects ASCII file)
EXTKEY=0  # we need similar information for the extinction key
for FILTER in $3
do
  ASCSTRING="${ASCSTRING} ${MAGQUANT}_${FILTER}${MAGINDEX}"
  ASCTMPMAGERR="${ASCTMPMAGERR} $5_${FILTER}${MAGINDEX}"
  ASCTMPMAGLIM="${ASCTMPMAGLIM} $6_${FILTER}"

  if [ "$7" != "NONE" ]; then
    ASCTMPFLAG="${ASCTMPFLAG} $7_${FILTER}"
    FLAGKEY=1 # we have FLAG keys in the catalogs
  fi

  if [ "$8" != "NONE" ]; then
    if [ "${FILTER}" == "i1" ] || [ "${FILTER}" == "i2" ]
    then 
      ASCTMPEXT="${ASCTMPEXT} ${8}_i"  
    else 
      ASCTMPEXT="${ASCTMPEXT} ${8}_${FILTER}"  
    fi 
    EXTKEY=1
  fi

  OFFSETACTU=0.0
  if [ $# -gt 10 ]; then
    OFFSETACTU=`gawk '$1 ~ /^'${FILTER}'$/ {print $2}' ${11}`

    if [ "A_${OFFSETACTU}" = "A_" ]; then
      OFFSETACTU=0.0
    fi
  fi
  OFFSETS="${OFFSETS} ${OFFSETACTU}"

  # Note that we do not apply magnitude offsets by giving them to the
  # BPZ 'columns' file. When doing this BPZ adapts magnitude errors
  # which can give significantly different results than applying
  # magnitude offsets manually (we want to treat offsets as known,
  # systematic errors, not as an additional contribution to
  # statistical errors).  Hence, the offsets are dealt with below.
  if [ "${FILTER}" == "i1" ] || [ "${FILTER}" == "i2" ] 
  then 
    echo "${INSTRUMENT}_i  ${MAGCOL},${MAGERRCOL}  $9  ${10}  0.0" \
         >> ${BASE}_photoz.columns
  elif [ "${FILTER}" == "Z" ]
  then 
    echo "${INSTRUMENT}_Z2 ${MAGCOL},${MAGERRCOL}  $9  ${10}  0.0" \
         >> ${BASE}_photoz.columns
  else 
    echo "${INSTRUMENT}_${FILTER}  ${MAGCOL},${MAGERRCOL}  $9  ${10}  0.0" \
         >> ${BASE}_photoz.columns
  fi 
  MAGCOL=$(( ${MAGCOL} + 1 ))
  MAGERRCOL=$(( ${MAGERRCOL} + 1 ))
done

ASCSTRING="${ASCSTRING} ${ASCTMPMAGERR} ${ASCTMPMAGLIM} \
           ${ASCTMPFLAG} ${ASCTMPEXT}" 

echo "ID  1" >> ${BASE}_photoz.columns
echo "M_0  2" >> ${BASE}_photoz.columns

# The following test is currently diabled because it fails
# for vector quantities in magnitues/magnitude errors
#
#${P_LDACTESTEXIST} -i $1/$2 -t OBJECTS \
#    -k ${ASCSTRING}

#Construct the temporary directory 
TEMPDIR=./BPZ_TEMPDIR/
mkdir -p ${TEMPDIR}

##if [ $? -eq 0 ]; then
  ldactoasc -s -b -i $1/$2 -t OBJECTS \
      -k ${ASCSTRING} > ${TEMPDIR}/tmp_$$.asc
  
  # take into account magnitude extinction if present. If present
  # we directly correct magnitude values for the extinction:
  gawk '{if('${EXTKEY}' == 1)
              {
                for(i = 3; i <= (2 + '${NFILT}'); i++)
                {
                  extcol = i + (3 + '${FLAGKEY}' ) * '${NFILT}'
                  if ( $i != -99 && $i != 99 )
                  {
                    $i = $i - $extcol;
                  }
                }
                if ( $2 != 99 )
                {
                  extcol = 5 + (3 + '${FLAGKEY}' ) * '${NFILT}'
                  $2 = $2 - $extcol
                }
              }
              print $0
             }' ${TEMPDIR}/tmp_$$.asc > ${TEMPDIR}/tmp1_$$.asc
  
  test -f ${TEMPDIR}/bpz_filters_$$.txt && rm ${TEMPDIR}/bpz_filters_$$.txt
  
  # create the final BPZ photometry file taking into
  # account possible magnitude offsets:
  gawk 'BEGIN {split("'"${OFFSETS}"'" ,offsets)} {
              bpzfilters = 0;   # encode in binary form which 
                                # filters are present 
              nbpzfilters = 0;  # how many filters are present
              bpznotdetfilters = 0;  # encode in binary form in which
                                     # filters the object was not
                                     # detected 
              nbpznotdetfilters = 0; # the number of filters in 
                                     # which the object was not detected
              bpzflagfilters = 0;  # encode in binary form in which
                                   # filters the object was not
                                   # observed 
              nbpzflagfilters = 0; # the number of filters in 
                                   # which the object was not observed
  
              for(i = 3; i <= 2 + '${NFILT}'; i++)
              {
                maglimcol = i + 2 * '${NFILT}'
                errorcol = i + '${NFILT}'
  
                $i = $i + offsets[i - 2];
  
                # objects being flagged are counted as
                # not observed in the current filter:
                if ('${FLAGKEY}' == 1)
                {
                  flagcol = i + 3 * '${NFILT}'
                  if( $flagcol > 0 || $i<0)
                  {
                    $i = -99
                    $errorcol = 0                    
                    bpzflagfilters += 2**(i-3);
                    nbpzflagfilters += 1;
                  }
                }
  
                # if an object is not flagged but has a magnitude
                # fainter than the limiting magnitude (or a large error)
                # we count it as not detected in the current
                # filter; in cas NO flag was provided we treat all
                # non-detected objects as if they were flagged!
                if (($i > 0) && ($i > $maglimcol || $errorcol > 1))
                {
                  if ('${FLAGKEY}' == 1)
                  {
                    $i = 99
                    $errorcol = $maglimcol;
                    bpznotdetfilters += 2**(i-3);
                    nbpznotdetfilters += 1;
                  }
                  else
                  {
                    $i = -99
                    $errorcol = 0                    
                    bpzflagfilters += 2**(i-3);
                    nbpzflagfilters += 1;
                  }
                }
                else if ($i > 0) # object is o.k.
                {
                  bpzfilters += 2**(i-3);
                  nbpzfilters += 1;
                }
              }

	      ##Define the GAAP-to-total aperture correction
              ##MAG_GAAP_r - MAG_AUTO
              #apcorr = $5 - $2
	      #
              ##BPZ prior is defined for the i-band, so use that if possible!
              #if ( $6!=99 && $6!=-99 ) # i1-band available?
              #{
              #  #Reference magnitude: MAG_GAAP_i1 - (MAG_GAAP_r-MAG_AUTO)
              #  $2 = $6 - apcorr
              #}
              #else if ( $7!=99 && $7!=-99 ) # i2-band available?
              #{
              #  #Reference magnitude: MAG_GAAP_i2 - (MAG_GAAP_r-MAG_AUTO)
              #  $2 = $7 - apcorr
              #}
	      ## otherwise MAG_AUTO (r-band) is used as M_0 in BPZ

              print bpzfilters, 
                    nbpzfilters, 
                    bpznotdetfilters,
                    nbpznotdetfilters,
                    bpzflagfilters, 
                    nbpzflagfilters >> "'${TEMPDIR}'/bpz_filters_'$$'.txt";
  
              out = ""
              for(i = 1; i <= 2 + 2*'${NFILT}'; i++)
              {
                out = out $i " "  
              }  
              print out;   # to STDOUT
             }' ${TEMPDIR}/tmp1_$$.asc > ${BASE}_photoz.asc
  
  @RUNROOT@/INSTALL/anaconda2/bin/python \
      $BPZPATH/bpz.py ${BASE}_photoz.asc -COLUMNS ${BASE}_photoz.columns \
                  -OUTPUT ${BASE}_bpz.asc -SPECTRA CWWSB_capak.list \
                  -PRIOR NGVS -ZMAX 7.0 -INTERP 10 -NEW_AB no \
		  -ODDS 0.68 -MIN_RMS 0.067 -PHOTO_ERRORS yes #\
                  #-PROBS_LITE ${BASE}_bpz_NGVS.prob
  
  # coonvert BPZ relevant quantities to LDAC format
  asctoldac -a ${BASE}_bpz.asc -o ${BASE}_bpz.cat -t OBJECTS \
                 -b 1 -n WFI -c @RUNROOT@/@CONFIGPATH@/asctoldac_bpz_68CI.conf
  
  # convert the filter encoding to LDAC format:
  {
    echo 'COL_NAME  = BPZ_FILT' 
    echo 'COL_TTYPE = LONG'     
    echo 'COL_HTYPE = INT'      
    echo 'COL_COMM = "filters with good photometry (BPZ)"'        
    echo 'COL_UNIT = " "'        
    echo 'COL_DEPTH = 1'        
    echo '#'
    echo 'COL_NAME  = NBPZ_FILT' 
    echo 'COL_TTYPE = LONG'     
    echo 'COL_HTYPE = INT'      
    echo 'COL_COMM = "number of filters with good phot. (BPZ)"'        
    echo 'COL_UNIT = " "'        
    echo 'COL_DEPTH = 1'        
    echo '#'
    echo 'COL_NAME  = BPZ_NONDETFILT' 
    echo 'COL_TTYPE = LONG'     
    echo 'COL_HTYPE = INT'      
    echo 'COL_COMM = "filters with faint photometry (BPZ)"'        
    echo 'COL_UNIT = " "'        
    echo 'COL_DEPTH = 1'        
    echo '#'
    echo 'COL_NAME  = NBPZ_NONDETFILT' 
    echo 'COL_TTYPE = LONG'     
    echo 'COL_HTYPE = INT'      
    echo 'COL_COMM = "number of filters with faint phot. (BPZ)"'        
    echo 'COL_UNIT = " "'        
    echo 'COL_DEPTH = 1'        
    echo '#'
    echo 'COL_NAME  = BPZ_FLAGFILT' 
    echo 'COL_TTYPE = LONG'     
    echo 'COL_HTYPE = INT'      
    echo 'COL_COMM = "flagged filters (BPZ)"'        
    echo 'COL_UNIT = " "'        
    echo 'COL_DEPTH = 1'        
    echo '#'
    echo 'COL_NAME  = NBPZ_FLAGFILT' 
    echo 'COL_TTYPE = LONG'     
    echo 'COL_HTYPE = INT'      
    echo 'COL_COMM = "number of flagged filters (BPZ)"'        
    echo 'COL_UNIT = " "'        
    echo 'COL_DEPTH = 1'        
  } > ${TEMPDIR}/asctoldac_bpzfilter.conf

  asctoldac -a ${TEMPDIR}/bpz_filters_$$.txt -o ${BASE}_bpzfilter.cat -t OBJECTS \
                 -b 1 -n WFI -c ${TEMPDIR}/asctoldac_bpzfilter.conf

  ldacjoinkey -i $1/$2 -o $1/${BASE}_photoz.cat.tmp \
                   -t OBJECTS \
                   -p ${BASE}_bpz.cat \
                   -k Z_B Z_B_MIN Z_B_MAX T_B ODDS \
                      Z_ML T_ML CHI_SQUARED_BPZ M_0

  ldacjoinkey -i $1/${BASE}_photoz.cat.tmp \
                   -o $1/${BASE}_photoz${recalib}.cat -t OBJECTS \
                   -p ${BASE}_bpzfilter.cat \
                   -k BPZ_FILT NBPZ_FILT BPZ_NONDETFILT NBPZ_NONDETFILT \
                      BPZ_FLAGFILT NBPZ_FLAGFILT

# disabled test (see above)
#fi

test -f $1/${BASE}_photoz.cat.tmp       && rm $1/${BASE}_photoz.cat.tmp
#test -f $1/${BASE}_photoz.cat.tmp2      && rm $1/${BASE}_photoz.cat.tmp2
test -f ${TEMPDIR}/tmp_$$.asc           && rm ${TEMPDIR}/tmp_$$.asc
test -f ${TEMPDIR}/tmp1_$$.asc          && rm ${TEMPDIR}/tmp1_$$.asc
#test -f ${TEMPDIR}/tmp2_$$.asc          && rm ${TEMPDIR}/tmp2_$$.asc
#test -f ${TEMPDIR}/tmp3_$$.asc          && rm ${TEMPDIR}/tmp3_$$.asc
#test -f ${TEMPDIR}/tmp4_$$.asc          && rm ${TEMPDIR}/tmp4_$$.asc
#test -f ${TEMPDIR}/tmp5_$$.asc          && rm ${TEMPDIR}/tmp5_$$.asc
#test -f ${TEMPDIR}/tmp6_$$.asc          && rm ${TEMPDIR}/tmp6_$$.asc
#test -f ${TEMPDIR}/tmp6_$$.cat          && rm ${TEMPDIR}/tmp6_$$.cat
test -f ${TEMPDIR}/bpz_filters_$$.txt   && rm ${TEMPDIR}/bpz_filters_$$.txt
test -f ${TEMPDIR}/asctoldac_bpzfilter.conf &&\
     rm ${TEMPDIR}/asctoldac_bpzfilter.conf

cd ${DIR}

exit 0;
