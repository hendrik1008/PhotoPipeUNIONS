#!/bin/bash

# Gets the standard catalogue for all standard fields

# $1 - main directory of runs
# $2 - type of files (STANDARD, SCIENCE or SCIENCESHORT)
# $3 - directory where to save catalogue
# $5 - name of the final catalogue

MD=$1
TYPES="STANDARD SCIENCE SCIENCESHORT"
#TYPES=$2
CATDIR=$3
RADIUS=$4
FINALCAT=$5
FILTERS="g_SDSS i_SDSS r_SDSS u_SDSS z_SDSS"
#GETSDSS=/vol/science01/scratch/dklaes/data/SDSSR9_query/SDSSR7_objects.py
GETSDSS=/users/dklaes/git/SDSS-catalogue-creator/SDSS_dataquery.py
PWD=`pwd`
# for FILTER in ${FILTERS}
# do
#  cd $1/${FILTER}/
#  for DIR in `ls -1`
#  do
#    echo "Getting coordinates from filter ${FILTER} in directory ${DIR} ..."
#    for TYPE in ${TYPES}
#    do
# 	if [  -d ${DIR}/${TYPE}_${FILTER}/ORIGINALS/ ]; then
# 	  cd ${DIR}/${TYPE}_${FILTER}/ORIGINALS/
# 	  for IMAGE in `ls -1`
# 	  do
# 	    COORDS=`dfits ${IMAGE} | fitsort -d RA DEC | awk '{print $2, $3}'`
# 	    echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-'${RADIUS}', $1+'${RADIUS}', $2-'${RADIUS}', $2+'${RADIUS}'; else print $1-'${RADIUS}'+360.0, $1+'${RADIUS}', $2-'${RADIUS}', $2+'${RADIUS}'}' >> ${CATDIR}/fields.tmp
# 
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-1.0, $1-0.5, $2-1.0, $2-0.5; else print $1-1.0+360.0, $1-0.5, $2-1.0, $2-0.5}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-0.5, $1+0.0, $2-1.0, $2-0.5; else print $1-0.5+360.0, $1+0.0, $2-1.0, $2-0.5}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.0, $1+0.5, $2-1.0, $2-0.5; else print $1+0.0+360.0, $1+0.5, $2-1.0, $2-0.5}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.5, $1+1.0, $2-1.0, $2-0.5; else print $1+0.5+360.0, $1+1.0, $2-1.0, $2-0.5}' >> ${CATDIR}/fields.tmp
#     # # 
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-1.0, $1-0.5, $2-0.5, $2+0.0; else print $1-1.0+360.0, $1-0.5, $2-0.5, $2+0.0}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-0.5, $1+0.0, $2-0.5, $2+0.0; else print $1-0.5+360.0, $1+0.0, $2-0.5, $2+0.0}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.0, $1+0.5, $2-0.5, $2+0.0; else print $1+0.0+360.0, $1+0.5, $2-0.5, $2+0.0}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.5, $1+1.0, $2-0.5, $2+0.0; else print $1+0.5+360.0, $1+1.0, $2-0.5, $2+0.0}' >> ${CATDIR}/fields.tmp
#     # # 
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-1.0, $1-0.5, $2+0.0, $2+0.5; else print $1-1.0+360.0, $1-0.5, $2+0.0, $2+0.5}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-0.5, $1+0.0, $2+0.0, $2+0.5; else print $1-0.5+360.0, $1+0.0, $2+0.0, $2+0.5}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.0, $1+0.5, $2+0.0, $2+0.5; else print $1+0.0+360.0, $1+0.5, $2+0.0, $2+0.5}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.5, $1+1.0, $2+0.0, $2+0.5; else print $1+0.5+360.0, $1+1.0, $2+0.0, $2+0.5}' >> ${CATDIR}/fields.tmp
#     # # 
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-1.0, $1-0.5, $2+0.5, $2+1.0; else print $1-1.0+360.0, $1-0.5, $2+0.5, $2+1.0}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1-0.5, $1+0.0, $2+0.5, $2+1.0; else print $1-0.5+360.0, $1+0.0, $2+0.5, $2+1.0}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.0, $1+0.5, $2+0.5, $2+1.0; else print $1+0.0+360.0, $1+0.5, $2+0.5, $2+1.0}' >> ${CATDIR}/fields.tmp
#     # # 	echo "${COORDS}" | awk '{if ($1-'${RADIUS}' >= 0.0) print $1+0.5, $1+1.0, $2+0.5, $2+1.0; else print $1+0.5+360.0, $1+1.0, $2+0.5, $2+1.0}' >> ${CATDIR}/fields.tmp
# 	  done
# 	  cd $1/${FILTER}/
# 	fi
#    done
#  done
# done

# echo "Getting fields..."

cd ${CATDIR}
# awk '{if (a[$0]==0) {a[$0]=1; print}}' fields.tmp | awk '!/^$/' > fields.list
# 
# IFS=$'\n'
# for LINE in `cat fields.list`
# do
#   RAMIN=`echo ${LINE} | awk '{print $1}'`
#   RAMAX=`echo ${LINE} | awk '{print $2}'`
#   DECMIN=`echo ${LINE} | awk '{print $3}'`
#   DECMAX=`echo ${LINE} | awk '{print $4}'`
#   echo "Downloading area ${LINE} ..."
#   ${GETSDSS} DR9 STARS ${RAMIN} ${RAMAX} ${DECMIN} ${DECMAX} > catalog_${RAMIN}_${RAMAX}_${DECMIN}_${DECMAX}.csv
#   sleep 1
# done
# cat catalog_*.csv > catalog.tmp
# echo "Getting rid of doubled objects..."
 
awk '{if (a[$0]==0) {a[$0]=1; print}}' catalog.tmp | sed -ne '/^[[:digit:]]/p' | awk '{print $0, "SDSSDR9"}' > catalog.tmp2

echo "VERBOSE = DEBUG"		>  ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = SeqNr"	>> ./asctoldac_tmp.conf
echo "COL_TTYPE = LONG" 	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = INT"		>> ./asctoldac_tmp.conf
echo 'COL_COMM = ""'		>> ./asctoldac_tmp.conf
echo 'COL_UNIT = ""'		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = Ra"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = DOUBLE"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "ra"'		>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = Dec"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = DOUBLE"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "dec"'		>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = RaErr"	>> ./asctoldac_tmp.conf
echo "COL_TTYPE = DOUBLE"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "Ra Error"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = DecErr"	>> ./asctoldac_tmp.conf
echo "COL_TTYPE = DOUBLE"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "Dec Error"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = umag"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "umag"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = uerr"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "uerr"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = gmag"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "gmag"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = gerr"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "gerr"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = rmag"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "rmag"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = rerr"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "rerr"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = imag"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "imag"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = ierr"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "ierr"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = zmag"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "zmag"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = zerr"		>> ./asctoldac_tmp.conf
echo "COL_TTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = FLOAT"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "zerr"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 1'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf
echo "COL_NAME  = fromcat"	>> ./asctoldac_tmp.conf
echo "COL_TTYPE = STRING"	>> ./asctoldac_tmp.conf
echo "COL_HTYPE = STRING"	>> ./asctoldac_tmp.conf
echo 'COL_COMM = "fromcat"'	>> ./asctoldac_tmp.conf
echo "COL_UNIT = ''"		>> ./asctoldac_tmp.conf
echo 'COL_DEPTH = 64'		>> ./asctoldac_tmp.conf
echo "#"			>> ./asctoldac_tmp.conf



echo "Converting asc to ldac..."
asctoldac -i catalog.tmp2 -o catalog.tmp3 -c asctoldac_tmp.conf -t STDTAB -b 1 -n "sdss ldac cat"
ldacfilter -i catalog.tmp3 -o catalog.tmp4 -t STDTAB -c "(ierr>0);"
ldacfilter -i catalog.tmp4 -o catalog.tmp5 -t STDTAB -c "(uerr>0);"
ldacfilter -i catalog.tmp5 -o catalog.tmp6 -t STDTAB -c "(gerr>0);"
ldacfilter -i catalog.tmp6 -o catalog.tmp7 -t STDTAB -c "(rerr>0);"
ldacfilter -i catalog.tmp7 -o catalog.tmp8 -t STDTAB -c "(zerr>0);"
ldaccalc -i catalog.tmp8 -o catalog.tmp9 -t STDTAB -c "(umag-gmag);" -n umg "" -k FLOAT \
						   -c "(gmag-rmag);" -n gmr "" -k FLOAT \
						   -c "(rmag-imag);" -n rmi "" -k FLOAT \
						   -c "(imag-zmag);" -n imz "" -k FLOAT \
						   -c "(sqrt((uerr*uerr)+(gerr*gerr)));" -n umgerr "" -k FLOAT \
						   -c "(sqrt((gerr*gerr)+(rerr*rerr)));" -n gmrerr "" -k FLOAT \
						   -c "(sqrt((rerr*rerr)+(ierr*ierr)));" -n rmierr "" -k FLOAT \
						   -c "(sqrt((ierr*ierr)+(zerr*zerr)));" -n imzerr "" -k FLOAT
#ldacaddkey -i catalog.tmp9 -o ${FINALCAT}.cat -t STDTAB -k Epoch 2000.0 FLOAT "" n 0 SHORT "" m 0 SHORT "" A_WCS 0.0002 FLOAT "" \
#							  B_WCS 0.0002 FLOAT "" THETAWCS 0.0 FLOAT "" Flag 0 SHORT ""
ldacaddkey -i catalog.tmp9 -o ${FINALCAT}.cat -t STDTAB -k Epoch 2000.0 FLOAT "" A_WCS 0.0002 FLOAT "" \
							  B_WCS 0.0002 FLOAT "" THETAWCS 0.0 FLOAT "" Flag 0 SHORT ""
# ldacaddkey -i catalog.tmp10 -o catalog.tmp11 -t STDTAB -k n 0 SHORT ""
# ldacaddkey -i catalog.tmp11 -o catalog.tmp12 -t STDTAB -k m 0 SHORT ""
# ldacaddkey -i catalog.tmp12 -o catalog.tmp13 -t STDTAB -k A_WCS 0.0005 FLOAT ""
# ldacaddkey -i catalog.tmp13 -o catalog.tmp14 -t STDTAB -k B_WCS 0.0005 FLOAT ""
# ldacaddkey -i catalog.tmp14 -o catalog.tmp15 -t STDTAB -k THETAWCS 0.0 FLOAT ""
# ldacaddkey -i catalog.tmp15 -o ${FINALCAT}.cat -t STDTAB -k Flag 0 SHORT ""


echo "Creating skycat file..."
ldactoskycat -i ${FINALCAT}.cat -t STDTAB -k SeqNr Ra Dec imag -l id_col SeqNr ra_col Ra dec_col Dec mag_col imag > ${FINALCAT}.skycat

echo "Creating ASCII file..."
ldactoasc -s -i ${FINALCAT}.cat -t STDTAB -k SeqNr Ra Dec RaErr DecErr umag uerr gmag gerr rmag rerr imag ierr zmag zerr umg gmr rmi imz umgerr gmrerr rmierr imzerr A_WCS B_WCS THETAWCS Flag fromcat > ${FINALCAT}.asc
# 
# #rm catalog.tmp* fields.list fields.tmp asctoldac_tmp.conf

cd ${PWD}
