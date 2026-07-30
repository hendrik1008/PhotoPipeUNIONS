#!/bin/sh -xv
#===============================================================================
#+
# NAME:#   maskstars.sh
#
# PURPOSE:
#   Create a mask for bright stars from an external reference star cat
#
# COMMENTS:
#   
#   Requires that sky2xy is installed (part of WCS tools)
#   In very few cases GSC1 has stars falsely marked as reflections.
#   These will be masked agressively if MASK_AGRESSIVELY=1 in the ini file
#   The weight is only needed if SExtractor is run to get a catalog (in addition 
#   to the GSC catalogs)
#
# USAGE:
#   maskstars.sh DIR IMAGE MASKINI
#
# INPUTS:
# 1  DIR        Directory with catalog
# 2  IMAGE      Image name
# 3  WEIGHT     Weight image name
#
#
# OUTPUTS:
#                Region files 
#                   ${BASE}.tight.reg
#                   ${BASE}.wide.reg
#                   ${BASE}.wide2.reg
#                   ${BASE}.reg   (combination of the 3)
#                   with  BASE=${IMAGE/.fits}_maskstars
#
# EXAMPLES:
#    maskstars.sh /data2/schrabback/CFHTLS/W3m1m3_V1.7A W3m1m3_i.V1.7A.swarp.cut.fits \
#      W3m1m3_i.V1.7A.swarp.cut.weight.fits
#
# BUGS:
#   
#
# REVISION HISTORY:
#   2008-01-22     Started Schrabback (Leiden)
#   2008-05-12     changes to reflect modifications in 
#                  mask_create_star_region.pl (see comments there)
#-
#===============================================================================

echo "*** maskstars.sh $1 $2 $3 $4 ***"

## preliminary stuff:
#. ${INSTRUMENT:?}_mask.ini
#
## path for sky2xy:
#BINPATH=/arc/home/hendrik/src/automask/ext_libs/Linux_64/bin
export PATH=@RUNROOT@/INSTALL/anaconda2/photopipe_env/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/photopipe_env/lib/:@RUNROOT@/INSTALL/anaconda2/lib/

# parse the command line

DIR=$1
IMAGE=$2
WEIGHT=$3
MASKINI=$4

BASE=${IMAGE/.fits}_maskstars

# check if all fields have been entered:

if [ "$4" == "" ]; then
    echo "usage: maskstars.sh DIR IMAGE WEIGHT MASKINI"
    exit 0;
fi

if [ ! -d ${DIR} ]; then
    echo "Error (maskstars.sh): directory ${DIR} not found!"
    exit 0;
fi

if [ -f ${DIR}/${IMAGE} ]; then
    echo "use image ${DIR}/${IMAGE}"
else
    echo "Error (maskstars.sh): image ${DIR}/${IMAGE} not found!"
    exit 0;
fi

if [ -f ${MASKINI} ]; then
    echo "use mask config file ${MASKINI}"
else
    echo "Error (maskstarcat.sh): mask config file ${MASKINI} not found!"
    exit 0;
fi

# source the ini file

. ${MASKINI}

if [ -f ${DIR}/${WEIGHT} ]; then
    echo "use weight ${WEIGHT}"
else
    echo "WARNING (maskstars.sh): weight ${WEIGHT} not found! Turn SExtractor run off."
    STAR_MASK_RUN_SEX=0
fi


# Read astrometric info
# Everything assumes North=up!

FIELD_CENTER_RA=`dfits_theli ${DIR}/${IMAGE} |\
                 fitsort_theli -d CRVAL1 | awk '{if ($2<360) print $2; else print $2-360}'`
FIELD_CENTER_RA=`decimaltohms ${FIELD_CENTER_RA}`
FIELD_CENTER_DEC=`dfits_theli ${DIR}/${IMAGE} |\
                 fitsort_theli -d CRVAL2 | awk '{print $2}'`
FIELD_CENTER_DEC=`decimaltodms ${FIELD_CENTER_DEC}`

# get the star catalog

# Compute the seeing dependent magnitude limit if requested
if [ ${APPLY_SEEING_DEP_GSC2_MAG_LIMIT} -eq '1' ]; then
    # get seeing:
    SEEING=`dfits_theli ${DIR}/${IMAGE} | fitsort_theli -d ${SEEING_KEY} | awk '{print $2}'`
    if [ "${SEEING}" == '' ]; then
	echo "WARNING: requested seeing-dependent magnitude limit for GSC2, 
              but image ${DIR}/${IMAGE} does not contain seeing key ${SEEING_KEY}. Skip!"
    else
	echo "Compute seeing-dependent magnitude limit for GSC2"
	echo "Original GSC2 magnitude limit for masking: ${MAX_MAG_STARS_TIGHT_GSC2} 
              (ref seeing: ${SEEING_DEP_GSC2_MAG_LIMIT_REFSEEING})"
	echo "Seeing: $SEEING"

	MAX_MAG_STARS_TIGHT_GSC2=`awk 'BEGIN {start='${MAX_MAG_STARS_TIGHT_GSC2}';
                                              seeing='${SEEING}';
                                              refseeing='${SEEING_DEP_GSC2_MAG_LIMIT_REFSEEING}';
                                              conversion='${SEEING_DEP_GSC2_MAG_LIMIT_CONVERSION}';
                                              limit=start-conversion*0.4343*log(seeing/refseeing);
                                              printf "%.2f", limit;}'`
	echo "Updated GSC2 magnitude limit for masking: ${MAX_MAG_STARS_TIGHT_GSC2}"
    fi
fi

echo "GSC2 magnitude limit for masking: ${MAX_MAG_STARS_TIGHT_GSC2}"

echo "download stellar catalog..."

echo "@RUNROOT@/@SCRIPTPATH@/masking/getgsc.sh ${FIELD_CENTER_RA} 
      ${FIELD_CENTER_DEC} ${BOXSIZE} ${MAX_MAG_STARS_TIGHT_GSC2} 2"

gsc2asc=$DIR/gsc2_${FIELD_CENTER_RA}_${FIELD_CENTER_DEC}_${BOXSIZE}_${MAX_MAG_STARS_TIGHT_GSC2}

if [ -f $gsc2asc ]; then
   echo "Catalog already found. Skip download."
else
   @RUNROOT@/@SCRIPTPATH@/masking/getgsc.sh ${FIELD_CENTER_RA} ${FIELD_CENTER_DEC} \
            ${BOXSIZE} ${MAX_MAG_STARS_TIGHT_GSC2} 2 |\
     awk '{if ($4==0){print $1, $2, $3}}' > $gsc2asc

fi

cp $gsc2asc $DIR/tmp_mask.asccat2

if [ -f $DIR/tmp_mask.asccat2 ]; then
    DUMMY=1
else 
   echo "Error: nothing downloaded. Abort."
    exit 0
fi

NOBJ=`wc $DIR/tmp_mask.asccat2 | awk '{print $1}'`

if [ ${NOBJ} -le 10 ]; then
    echo "Error: downloaded GSC2 catalog contains only  ${NOBJ} objects. Something wrong. Abort"
else
    echo "Downloaded GSC2 catalog contains ${NOBJ} objects."
fi

# get the GSC1 cat aswell:


echo "@RUNROOT@/@SCRIPTPATH@/masking/getgsc.sh ${FIELD_CENTER_RA} ${FIELD_CENTER_DEC} ${BOXSIZE} ${MAX_MAG_STARS_TIGHT_GSC1} 1"

gsc1asc=$DIR/gsc1_${FIELD_CENTER_RA}_${FIELD_CENTER_DEC}_${BOXSIZE}_${MAX_MAG_STARS_TIGHT_GSC1}

if [ -f $gsc1asc ]; then
  echo "Catalog already found. Skip download."
else
  @RUNROOT@/@SCRIPTPATH@/masking/getgsc.sh ${FIELD_CENTER_RA} ${FIELD_CENTER_DEC} \
           ${BOXSIZE} ${MAX_MAG_STARS_TIGHT_GSC1} 1 > $gsc1asc
fi

cp $gsc1asc $DIR/tmp_mask.asccat1allclass

CATNAME=tmp_bright

if [ "${STAR_MASK_RUN_SEX}" -eq '1' ]; then
##########################3
###########################
# This step is currently not used. The original plan was to unflag bright elliptical galaxies
# marked as stars in GSC, by matching GSC2 with our own catalog. However, with the saturated
# stellar pixels being set to 0, SExtractor often detects 2 objects for stars, with very similar magnitudes,
# FLUX_RADIUS, and sometimes even ELLIPTICITY as the elliptical galaxies, so a secure seperation seems very
# tedious. 
# For now we just have to live with the falsely flagged ellipticials

    echo "Requested an own Sextractor run to detect bright stars/galaxies"    
    rm sex.cat
    ${P_SEX}   -c ${MASKCONF}/mask_stars_sex.conf \
    	        ${DIR}/${IMAGE} \
    	        -CATALOG_NAME sex.cat \
    	        -WEIGHT_IMAGE ${DIR}/${WEIGHT}\
                -FLAG_IMAGE ""\
    	        -DETECT_MINAREA ${STAR_MIN_AREA} \
    	        -DETECT_THRESH ${STAR_MIN_THRESH} \
    	        -ANALYSIS_THRESH ${STAR_MIN_THRESH} \
                -DEBLEND_NTHRESH ${STAR_DEBLEND_NTHRESH} \
                -DEBLEND_MINCONT ${STAR_DEBLEND_MINCONT} \
    	        -BACK_TYPE AUTO \
                -FILTER Y\
    	        -FILTER_NAME ${MASKCONF}/${STAR_FILTER}\
                -INTERP_MAXXLAG ${STAR_INTERP_MAXXLAG}\
                -INTERP_MAXYLAG ${STAR_INTERP_MAXYLAG}\
                -PARAMETERS_NAME ${MASKCONF}/${STAR_PARAM_FILE}
    
    rm ${CATNAME}.cat
    ${P_LDACCONV} -i sex.cat -o ${CATNAME}.cat -b 1 -c "sex" -f R
    ${P_LDACTOSKYCAT} -i ${CATNAME}.cat -t OBJECTS \
                      -k SeqNr ALPHA_J2000 DELTA_J2000 MAG_AUTO \
                         FLUX_RADIUS ELLIPTICITY \
                      -l id_col SeqNr ra_col ALPHA_J2000 dec_col DELTA_J2000 \
                         mag_col MAG_AUTO > ${CATNAME}.skycat

####################################
####################################
####################################

else
     echo "An own Sextractor run to detect bright stars/galaxies has NOT been requested. Skip."
fi


if [ ${MASK_AGRESSIVELY_TIGHT} -eq '1' ]; then
    MAX_CLASS=30
    awk 'BEGIN{limit='${MAX_CLASS}'}{if ($4<=limit){print $1, $2, $3}}' \
      $DIR/tmp_mask.asccat1allclass > $DIR/tmp_mask.asccat1t
else
    MAX_CLASS=3
    if [ ${MASK_AGRESSIVELY_TIGHT_BRIGHT} -eq '1' ]; then
	awk 'BEGIN{limit='${MAX_CLASS}';
                   limit2=30;maglim='${MASK_AGRESSIVELY_TIGHT_BRIGHT_MAGLIM}';} {
             if (($4<=limit)||(($4<=limit2)&&($3<maglim)))
             {
               print $1, $2, $3
             }}' $DIR/tmp_mask.asccat1allclass > $DIR/tmp_mask.asccat1t
    else
	awk 'BEGIN{limit='${MAX_CLASS}'} {
             if ($4<=limit)
             {
               print $1, $2, $3
             }}' $DIR/tmp_mask.asccat1allclass > $DIR/tmp_mask.asccat1t
    fi
fi


if [ ${MASK_AGRESSIVELY_WIDE} -eq '1' ]; then
    MAX_CLASS=30
else
    MAX_CLASS=3
fi
awk 'BEGIN{limit='${MAX_CLASS}'} {
     if ($4<=limit)
     {
       print $1, $2, $3
     }}' $DIR/tmp_mask.asccat1allclass > $DIR/tmp_mask.asccat1


cat $DIR/tmp_mask.asccat2 $DIR/tmp_mask.asccat1t > $DIR/tmp_mask.asccat

# transform to xy coordinates (done in mask_create_star_region.pl)

sky2xy ${DIR}/${IMAGE} @$DIR/tmp_mask.asccat |\
  awk '{print $5, $6, $3}' > $DIR/tmp_mask.asccat.xy
sky2xy ${DIR}/${IMAGE} @$DIR/tmp_mask.asccat1 |\
  awk '{print $5, $6, $3}' > $DIR/tmp_mask.asccat.xy.gsc1

# We need the dimension of the FITS image for 
# mask_create_star_region.pl calls:
XWIDTH=`dfits_theli ${DIR}/${IMAGE} | fitsort_theli -d NAXIS1 | awk '{print $2}'`
YWIDTH=`dfits_theli ${DIR}/${IMAGE} | fitsort_theli -d NAXIS2 | awk '{print $2}'`

if [ ${DO_TIGHT_MASK} -eq '1' ]; then

   echo "${STAR_REGION_PARAMETERS_TIGHT}" > $DIR/tmp.star.region.paras

    @RUNROOT@/@SCRIPTPATH@/masking/mask_create_star_region.pl $DIR/tmp_mask.asccat.xy \
       ${XWIDTH} ${YWIDTH} $DIR/tmp.star.region.paras ${OFFSET_TIGHT} $DIR

    sed 's|physical;||' $DIR/auto_brightstars.reg > $DIR/tmp.reg
    awk '{if ($1 ~ /polygon/)
          {
            print $1, "# color=green"
          }
          else {print $0}}' $DIR/tmp.reg > $DIR/${BASE}.tight.reg
fi
# Now the wide masks:

if [ ${DO_WIDE_MASK} -eq '1' ]; then
   if [ ${WIDE_MASK_ONLY_WITH_GSC1} -eq '1' ]; then
       ASC=$DIR/tmp_mask.asccat.xy.gsc1
   else
       ASC=$DIR/tmp_mask.asccat.xy
   fi

   echo "${STAR_REGION_PARAMETERS_WIDE}" > $DIR/tmp.star.region.paras
   awk 'BEGIN{magmax='${MAX_MAG_STARS_WIDE}'} {
        if($3<magmax)
        {
          print $1, $2, $3, $4, $5
        }}' ${ASC} > $DIR/tmp_mask.asccat.xy.wide

    @RUNROOT@/@SCRIPTPATH@/masking/mask_create_star_region.pl $DIR/tmp_mask.asccat.xy.wide \
         ${XWIDTH} ${YWIDTH} $DIR/tmp.star.region.paras ${OFFSET_WIDE} $DIR
    sed 's|green|magenta|' $DIR/auto_brightstars.reg | sed 's|physical;||' > $DIR/tmp.reg
    awk '{if ($1 ~ /polygon/){print $1, "# color=magenta"}else{print $0}}' \
      $DIR/tmp.reg > $DIR/${BASE}.wide.reg

fi

if [ ${DO_WIDE_MASK2} -eq '1' ]; then
   if [ ${WIDE_MASK_ONLY_WITH_GSC1} -eq '1' ]; then
       ASC=$DIR/tmp_mask.asccat.xy.gsc1
   else
       ASC=$DIR/tmp_mask.asccat.xy
   fi

   echo "${STAR_REGION_PARAMETERS_WIDE2}" > $DIR/tmp.star.region.paras
   awk 'BEGIN{magmax='${MAX_MAG_STARS_WIDE2}'} {
        if($3<magmax)
        {
          print $1, $2, $3, $4, $5
        }}' ${ASC} > $DIR/tmp_mask.asccat.xy.wide

   @RUNROOT@/@SCRIPTPATH@/masking/mask_create_star_region.pl $DIR/tmp_mask.asccat.xy.wide \
         ${XWIDTH} ${YWIDTH} $DIR/tmp.star.region.paras ${OFFSET_WIDE} $DIR

    sed 's|green|cyan|' $DIR/auto_brightstars.reg | sed 's|physical;||'  > $DIR/tmp.reg
    awk '{if ($1 ~ /polygon/) 
          {
            print $1, "# color=cyan"
          }
          else { print $0 }}' $DIR/tmp.reg > $DIR/${BASE}.wide2.reg
fi
JOINT_REGION=$DIR/${BASE}.reg

cat $DIR/${BASE}.tight.reg $DIR/${BASE}.wide.reg $DIR/${BASE}.wide2.reg > ${JOINT_REGION}
cat $DIR/${BASE}.tight.reg $DIR/${BASE}.wide.reg > $DIR/${BASE}_tw.reg

# clean up

if [ ${CLEAN} -eq '1' ]; then
   rm $DIR/tmp* $DIR/gsc* $DIR/auto_brightstars.reg
fi
