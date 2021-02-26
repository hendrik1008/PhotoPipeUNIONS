#!/bin/bash

# Top-level script for processing KiDS-Legacy Photometry
# Usage: 
#        bash run_PhotoPipe.sh [MODE]
#
# (Running without specifying any [MODE] will print available modes)
#
# Required Inputs:
# - Background-subtracted VISTA chips, returned from the VIKING
#   reduction pipeline: 
#   https://github.com/AngusWright/VIKINGProcessingPipeline.git
#   (Written by Angus H. Wright)
# - KiDS 4-band catalogues from AstroWISE.
#
# Output:
# - 9-band photometric catalogues with photoz 
# - 9-band photometric masks 
#
# Author: Angus H Wright
# Adapted from "meta_wrapper_K1000.sh" written by H. Hildebrandt
#
# Version history:
# 2020-08-05 V1.0

#Set Stop-On-Error {{{
abort()
{
  echo -e "\033[0;31m --- !FAILED! ----" >&2
  echo -e "\033[0;34m An error occured during the step above \033[0m" >&2
  echo -e "\033[0;34m Check the relevant logging file for this step.\033[0m" >&2
  echo >&2
  exit 1
}
trap 'abort' 0
set -e 
#}}}

#Set up the PATH {{{
export PYTHONPATH=@RUNROOT@/INSTALL/anaconda2/bin/python2:@RUNROOT@/INSTALL/anaconda2/lib/
export NUMERIX=numpy
export PATH=@RUNROOT@/INSTALL/anaconda2/bin/:${PATH}
export PATH=@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/:${PATH}
export PATH=@RUNROOT@/INSTALL/wcstools-3.9.6/bin/:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:@RUNROOT@/INSTALL/anaconda2/lib/
#}}}

#Check for the MODE specification {{{
ALLMODES=`echo CONVERT COLLECT LINK PREPARE GAUSSIANISE GAAP COMBINEPAW COMBINETILE 2MASSPREP SDSSPREP COMPPAW \
               COMPTILE MERGE COMPTILEVST STACK BPZ COMPTILEZ COMPTILE2DF MASK4 MASK RAND COPY `
MODELIST=""
while [ $# -gt 0 ]
do 
  for MODE in $1
  do 
    if [ "$MODE" == "ALL" ] 
    then 
      echo -e "Using\033[0;31m Mode Set\033[0;34m ALL\033[0m"
      MODELIST=`echo ${MODELIST} \
        CONVERT COLLECT LINK PREPARE GAUSSIANISE GAAP COMBINEPAW COMBINETILE 2MASSPREP SDSSPREP COMPPAW \
        COMPTILE MERGE COMPTILEVST STACK BPZ COMPTILEZ COMPTILE2DF MASK4 MASK RAND COPY `
    elif [ "$MODE" == "REQUIRED" ]
    then 
      echo -e "Using\033[0;31m Mode Set\033[0;34m REQUIRED\033[0m"
      MODELIST=`echo ${MODELIST} \
        CONVERT COLLECT LINK PREPARE GAUSSIANISE GAAP COMBINETILE MERGE BPZ MASK4 MASK COPY`
    elif [ "$MODE" == "PHOTOMETRY" ]
    then 
      echo -e "Using\033[0;31m Mode Set\033[0;34m PHOTOMETRY\033[0m"
      MODELIST=`echo ${MODELIST} \
      CONVERT COLLECT LINK PREPARE GAUSSIANISE GAAP COMBINETILE MERGE`
    elif [ "$MODE" == "PZ+MASK" ]
    then 
      echo -e "Using\033[0;31m Mode Set\033[0;34m PZ+MASK\033[0m"
      MODELIST=`echo ${MODELIST} \
      BPZ MASK4 MASK`
    elif [ "$MODE" == "QC" ]
    then 
      echo -e "Using\033[0;31m Mode Set\033[0;34m QC\033[0m"
      MODELIST=`echo ${MODELIST} \
      2MASSPREP SDSSPREP COMPTILE COMPTILEVST STACK COMPTILEZ COMPTILE2DF `
    else 
      found=0
      for mode in ${ALLMODES}
      do 
        if [ "$mode" == "$MODE" ]
        then
          found=1
        fi 
      done
      if [ "$found" == "1" ] 
      then 
        MODELIST=`echo ${MODELIST} ${MODE}`
      else 
        >&2 echo -e "\033[0;31mERROR:\033[0m Requested mode\033[0;34m ${MODE}\033[0m does not exist!"
        >&2 echo -e "Run\033[0;34m 'bash run_PhotoPipe.sh'\033[0m to see the list of available modes"
        exit 1
      fi 
    fi 
  done 
  shift 
done 
MODES=`echo $MODELIST | uniq`
#}}}

#Define the Number of Threads {{{
NTHREAD=@NTHREAD@
#}}}

#Define the refresh rate {{{
REFRESHRATE=@REFRESHRATE@
#}}}

#Define the Pointing List file {{{
POINTINGLIST=@POINTINGLIST@
#}}}

#Define the DryRun variable {{{
DRYRUN=@DRYRUN@
#}}}

#Move to the RUNROOT directory {{{
cd @RUNROOT@
#}}}

#Starting Prompt {{{
echo -e "\033[0;34m=========================================\033[0m"
echo -e "\033[0;34m== \033[0;31m Photometry Pipeline Master Script \033[0;34m ==\033[0m"
echo -e "\033[0;34m=========================================\033[0m"
sleep 1
echo -e "Welcome to the photometry pipeline, \033[0;31m`whoami`\033[0m!" 
sleep .2
#Check the input MODES {{{
if [ "$MODES" != "" ] 
then 
  #Print the modes to be run {{{
  echo -e "I will be looping over each of the requested modes: \033[0;31m${MODES}\033[0m" 
  sleep .2
  #}}}
else 
  #Print the available MODES {{{
  echo -e "\033[0;31mERROR:\033[0m There are no MODES provided! I have nothing to do!" 
  echo -e "The available modes (and the order in which they should be called) are:"
  echo -e "\033[0;31m   1. \033[0;34m CONVERT:\033[0m Convert AW catalogues."
  echo -e "\033[0;31m   2. \033[0;34m COLLECT:\033[0m Collect VISTA chips."
  echo -e "\033[0;31m   3. \033[0;34m LINK:\033[0m Link VISTA chips."
  echo -e "\033[0;31m   4. \033[0;34m PREPARE:\033[0m Create directories."
  echo -e "\033[0;31m   5a.\033[0;34m GAUSSIANISE:\033[0m Gaussianise the VIKING chips."
  echo -e "\033[0;31m   5. \033[0;34m GAAP:\033[0m Extract GaAP photometry."
  echo -e "\033[0;31m   6. \033[0;34m COMBINEPAW:\033[0m Combine flux measurements of all chips per pawprint. (OPTIONAL)"
  echo -e "\033[0;31m   7. \033[0;34m COMBINETILE:\033[0m Combine flux measurements of all chips per tile. "
  echo -e "\033[0;31m   8. \033[0;34m 2MASSPREP:\033[0m Preparation of 2MASS catalogue."
  echo -e "\033[0;31m   9. \033[0;34m SDSSPREP:\033[0m Preparation of SDSS catalogue."
  echo -e "\033[0;31m   10.\033[0;34m COMPPAW:\033[0m Comparisons to 2MASS (JHKs bands) and SDSS (Z band). Individual pawprints. (OPTIONAL)"
  echo -e "\033[0;31m   11.\033[0;34m COMPTILE:\033[0m Comparisons to 2MASS (JHKs bands) and SDSS (Z band). Full tile."
  echo -e "\033[0;31m   12.\033[0;34m MERGE:\033[0m Paste the measurements from individual bands into a full 9-band catalogue."
  echo -e "\033[0;31m   13.\033[0;34m COMPTILEVST:\033[0m Comparisons to SDSS (ugri-bands). Full tile."
  echo -e "\033[0;31m   14.\033[0;34m STACK:\033[0m Create a stack and sum image of all chips that went into the photometry."
  echo -e "\033[0;31m   15.\033[0;34m BPZ:\033[0m Run BPZ."
  echo -e "\033[0;31m   16.\033[0;34m COMPTILEZ:\033[0m Comparison to SDSS redshifts. Full tile."
  echo -e "\033[0;31m   17.\033[0;34m COMPTILE2DF:\033[0m Comparison to 2dFLenS redshifts. Full tile."
  echo -e "\033[0;31m   18.\033[0;34m MASK4:\033[0m Create the 4-band MASK."
  echo -e "\033[0;31m   19.\033[0;34m MASK:\033[0m Create the 9-band MASK."
  echo -e "\033[0;31m   20.\033[0;34m RAND:\033[0m Create a new random catalogue. (OPTIONAL)"
  echo -e "\033[0;31m   21.\033[0;34m COPY:\033[0m Copy data products into THELI tree."
  echo -e "\033[0;31mALTERNATIVELY:\033[0m You can specify the MODE as one of the below:"
  echo -e "\033[0;31m  [Set]\033[0;34m ALL\033[0m: Runs all modes (1-21)"
  echo -e "\033[0;31m  [Set]\033[0;34m REQUIRED\033[0m: Runs all modes required to produce science-data (1-5,7,12,15,18-19,21)"
  echo -e "\033[0;31m  [Set]\033[0;34m PHOTOMETRY\033[0m: Runs modes 1-5,7,12"
  echo -e "\033[0;31m  [Set]\033[0;34m PZ+MASK\033[0m: Runs modes 15,18-19"
  echo -e "\033[0;31m  [Set]\033[0;34m QC\033[0m: Runs modes 8-9,11,13-14,16-17"
  exit 1 
  #}}}
fi 
#Check the input pointing filelist {{{
if [ -f ${POINTINGLIST} ] 
then 
  NPOINTINGS=`wc -l ${POINTINGLIST}`
  #Print the number of pointings to be run 
  echo -e "There are \033[0;31m${NPOINTINGS}\033[0m pointing(s) in the provided Pointing Filelist: \033[0;31m${POINTINGLIST}\033[0m" 
  sleep .2
else 
  echo -e "\033[0;31mERROR:\033[0m The provided Pointing Filelist does not exist: \033[0;31m${POINTINGLIST}\033[0m" 
  exit 1 
fi
#}}}
#Print whether we're doing a dry run 
if [ "${DRYRUN}" == "0" ]
then 
  echo -e "This\033[0;31m is not\033[0m a dry run! I will attempt to execute each of the requested modes!" 
  sleep .2
  if [ "${NTHREAD}" == "" ]
  then
    NTHREAD=1
  fi
  echo -e "The maximum number of threads that I will use is \033[0;31m${NTHREAD}\033[0m"
  sleep .2
else 
  echo -e "This\033[0;31m is\033[0m a dry run! I will not attempt to execute the modes, but will just make the command lists." 
  sleep .2
fi 
#}}}
#Wait for 5 seconds {{{
echo -en "\033[0;34mYou have 5 sec until I continue with these settings... \033[0m  "
spinner()
{
  _pid=$! # Process Id of the previous running command
  _spin='-\|/'
  _i=0
  while kill -0 $_pid 2>/dev/null 1>&2 
  do
    _i=$(( (_i+1) %4 ))
    printf "[${_spin:$_i:1}]\b\b\b"
    sleep .1
  done
}

sleep 5 & spinner 
#}}}
# Start! {{{
echo " OK!"
sleep 1
echo -e "\033[0;34mRunning Pipeline now. \033[0m  "
sleep .5
echo -e "\033[0;34m=======================================\033[0m"
#}}}
#}}}

#Loop over MODES {{{
for MODE in ${MODES} 
do 
  #Check that no executable list exists {{{
  if [ -f ${MODE}_commandlist.sh ] 
  then 
    echo -e "\033[0;31mWARNING:\033[0m Removing previous command list!"
    rm ${MODE}_commandlist.sh 
  fi 
  if [ -f ${MODE}_commandlist.log ] 
  then 
    echo -e "\033[0;31mWARNING:\033[0m Removing previous command log!"
    rm ${MODE}_commandlist.log 
  fi 
  joblist=( ${MODE}_job*.sh )
  if [ ${#joblist[@]} -gt 1 ] 
  then 
    echo -e "\033[0;31mWARNING:\033[0m Removing previous job lists!"
    rm ${MODE}_job*.sh
  fi 
  #}}}
  #Construct the executable list {{{
  echo -e "Starting Mode \033[0;31m${MODE}\033[0m (`date`)" 
  while read field ra dec 
  do
    #Check that the required files exist: 
    if [ -f @ASTROWISEPATH@/${field}_@THELIFILTER@.@THELIVERSION@_@AWSURVEYNAME@_GAaP_@REFERENCE@.fits ]
    then
      #Construct the executable list {{{
  	  bash @SCRIPTPATH@/construct_commands_KiDSLegacy.sh \
           -md @RUNROOT@/@WORKINGDIR@/ \
           -cd @ASTROWISEPATH@/ \
           -bd @VIKINGROOT@/@VIKINGTYPE@/ \
           -id @RUNROOT@/@WORKINGDIR@/@RAWDIR@/ \
           -fi ${field} \
           -fn ${field} \
        	 -lg @RUNROOT@/@WORKINGDIR@/@LOGFILE@ \
        	 -ma @RUNROOT@/@WORKINGDIR@/${field}/${field}_AW_THELI.flags.fits \
  	       -th @THELIDATAPATH@/ \
           -m $MODE \
           >> ${MODE}_commandlist.sh 2>> ${MODE}_commandlist.log
       #}}}
       #Remove any duplicated commands {{{ 
       cat ${MODE}_commandlist.sh | sort | uniq > tmp_${MODE}_commandlist.sh 
       mv tmp_${MODE}_commandlist.sh ${MODE}_commandlist.sh 
       #}}}
    fi
  done < ${POINTINGLIST}
  #}}}
  #Launch the commands {{{
  if [ "$DRYRUN" == "0" ]
  then 
    #Check that there is anything to run:
    if [ "`wc -l ${MODE}_commandlist.sh | awk '{print $1}'`" != "0" ]
    then
      #Run the jobs and wait until they are completed {{{
      #Construct the NTHREAD job commands {{{
      ####Splits subsequent lines across NTHREAD files
      ###awk -v MODE=${MODE} -v NPROC=${NTHREAD} \
      ###  '{print $0 > MODE"_job"NR%NPROC+1"_of_"NPROC".sh"}' ${MODE}_commandlist.sh
      #Constructs NTHREAD chunks of the original commandlist
      split --numeric-suffixes=01 -e -n l/${NTHREAD} --additional-suffix=_of_${NTHREAD}.sh ${MODE}_commandlist.sh ${MODE}_job
      #}}}
      #Distribute the executables and wait for their completion {{{
      for i in `seq -w $NTHREAD`
      do
        if [ -f ${MODE}_job${i}_of_${NTHREAD}.sh ]
        then 
          screen -L -Logfile ${MODE}_job${i}_of_${NTHREAD}.log -S ${MODE}_job${i}_of_${NTHREAD}.sh -d -m bash ${MODE}_job${i}_of_${NTHREAD}.sh
        fi 
      done
      #}}}
      #Check if we can continue to the next MODE {{{
      while [ `ps au | grep -v "bash -c " | grep -v grep | grep -c ${MODE}_job` -ge 1 ]
      do
        #If this is the first loop of the wait, then print what is running  /*fold*/ {{{
        if [ "${prompt}" != "${MODE}" ]
        then
          echo -e "Pipeline paused while waiting for \033[0;31m${MODE}\033[0m to be completed (`date`)"
          prompt=${MODE}
        fi
        sleep ${REFRESHRATE}
        #/*fend*/}}}
      done
      #}}}
      echo "Mode ${MODE} is complete! Moving to next mode (`date`)" 
      #Move the commands and logs to storage {{{
      mkdir -p ${MODE}_logs 
      mv -f ${MODE}_*.* ${MODE}_logs/
      #}}}
      #}}}
    else
      #There is nothing to do?! Probably a bug... {{{
      echo "Mode ${MODE} resulted in no commands?!! This is probably not right... I will stop here (`date`)"
      exit 1
      #}}}
    fi
  else 
    #Prompt and continue {{{
    echo "Command Construction for mode ${MODE} is complete! Moving to next mode (This is a Dry Run!)" 
    #}}}
  fi 
  #}}}
  #Loop to the next MODE
done 
#}}}

#Closing Prompt {{{
echo -e "\033[0;34m=======================================\033[0m"
#Finished! 
trap : 0
echo -e "\033[0;34m==============================\033[0m"
echo -e "\033[0;34m==\033[31m  Pipeline Run Complete!  ==\033[0m"
echo -e "\033[0;34m==============================\033[0m"
#}}}
