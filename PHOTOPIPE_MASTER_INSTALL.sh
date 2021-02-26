#
# Shell script for installation of the KiDS Photomety Pipeline
# Written by A.H.Wright (2020-08-05)
#
#Set Stop-On-Error {{{
abort()
{
  echo -e "\033[0;31m - !FAILED!" >&2
  echo -e "\033[0;34m An error occured during the step above \033[0m" >&2
  echo -e "\033[0;34m Check the relevant logging file for this step.\033[0m" >&2
  echo >&2
  exit 1
}
trap 'abort' 0
set -e 
#}}}

#Set the default variables to determine the installation paths  {{{
#Do we want to run the configure file? (1=NO, else YES)
NOCONFIG=0
#Package directory (default: `pwd`)
PACKROOT=`pwd`
#Root directory for software & reduce folder storage (default: `pwd`)
RUNROOT=/net/home/fohlen13/hendrik/PhotoPipe/RUNDIR_CLEAN/
#RUNROOT=/net/home/fohlen13/awright/PhotoPipe/RUNDIR_CLEAN/
#Directory for runtime script storage
RUNTIME=RUNTIME
#Survey ID  
SURVEY=KiDS-Legacy 
#Survey ID in AstroWISE
AWSURVEYNAME=KiDS-1347 
#Patch catalogue suffix 
FILESUFFIX=_v2_good
#Username (default: `whoami`) 
USER=`whoami`
#Directory for work
WORKINGDIR=work_clean_${SURVEY}
#Directory for Raw Images 
RAWDIR=raw_chips
#Path to configuration files
CONFIGPATH=RUNTIME/config/
#Path to modified script files
SCRIPTPATH=RUNTIME/scripts/
#Path to VIKING data 
VIKINGROOT=/path/to/VIKING/
VIKINGROOT=/net/fohlen11/home/awright/KiDS_VIKING_1350/
#Name of the chip-type for VIKING data 
VIKINGTYPE=native_bsub
#Path to THELI data 
THELIDATAPATH=/path/to/THELI/Data/
THELIDATAPATH=/net/fohlen13/home/hendrik/KIDSCOLLAB_V1.3.0A/
#THELIDATAPATH=/net/fohlen13/home/awright/KiDS/DR5/THELI/
#Do we want to do a DRYRUN (!=0 := YES)
DRYRUN=0
#Define the Pointing Filelist 
POINTINGLIST=/path/to/pointinglist.txt 
POINTINGLIST=pointing_filelist.dat
POINTINGLIST=KiDS-Legacy_pointings_1.txt
#Path to AstroWISE catalogues 
ASTROWISEPATH=/path/to/AstroWISE/catalogues/
ASTROWISEPATH=/net/fohlen12/home/awright/KiDS/DR5/multiband/
#The THELI Filter for photometry 
THELIFILTER=r_SDSS
#The THELI Version that was used 
THELIVERSION=V1.3.0A
#The Survey used for Photometric Reference
REFERENCE=Gaia
#File containing u-band zero-point corrections
UBANDCORRECTIONSFILE=KiDS_DR4.0-GaiaDR2-DMAGcorr-to-subtract.txt
#Machine type
MACHINE=Linux_64 # can be seen using `uname`
#THELI Path 
THELIPATH=${RUNROOT}/INSTALL/theli-1.6.1/bin/${MACHINE}/
#File with pointing WCS limits
POINTINGLIMITSFILE=${RUNROOT}/${CONFIGPATH}/KIDS_ra_dec_cuts_corr2.txt
#Number of threads 
NTHREAD=32
#File with Catalogue Keywords, units, comments, and order
CATALOGUEKEYSFILE=${CONFIGPATH}/Legacy_keyword_order_comments.csv
#2DFLens Redshift Catalogue (For QC) 
TWODFLENSCATALOGUE=/net/fohlen11/home/hendrik/data/2dFLenS/2dflens_bestredshifts_lrgs_goodz_final_kidss.cat
#Set the wait time between completion checks 
REFRESHRATE=5
#Logfile name 
LOGFILE=PhotoPipe.log
#}}}
#Full list of options {{{
OPTLIST="NOCONFIG PACKROOT RUNROOT RUNTIME SURVEY AWSURVEYNAME FILESUFFIX USER \
  WORKINGDIR RAWDIR CONFIGPATH SCRIPTPATH VIKINGROOT VIKINGTYPE DRYRUN \
  POINTINGLIST POINTINGLIMITSFILE ASTROWISEPATH THELIFILTER THELIVERSION \
  REFERENCE UBANDCORRECTIONSFILE THELIPATH THELIDATAPATH NTHREAD \
  CATALOGUEKEYSFILE REFRESHRATE LOGFILE TWODFLENSCATALOGUE THELIPACKVERS \
  MACHINE THELIPACKSUFFIX"
#}}}

#Read any command line options  {{{
while [ $# -gt 0 ]
do 
  case $1 in 
    "--noconfig") shift; NOCONFIG=1;;
    "--packroot") shift; PACKROOT=$1; shift;;
    "--runroot") shift; RUNROOT=$1; shift;;
    "--runtime") shift; RUNTIME=$1; shift;;
    "--survey") shift; SURVEY=$1; shift;;
    "--filesuffix") shift; FILESUFFIX=$1; shift;;
    "--user") shift; USER=$1; shift;;
    "--workingdir") shift; WORKINGDIR=$1; shift;;
    "--configpath") shift; CONFIGPATH=$1; shift;;
    "--scriptpath") shift; SCRIPTPATH=$1; shift;;
    "--vikingpath") shift; VIKINGPATH=$1; shift;;
    *) echo "ERROR - unknown option $1!"; exit 1;;
  esac
done
#}}}

#Starting Prompt {{{
echo -e "\033[0;34m======================================================\033[0m"
echo -e "\033[0;34m== \033[0;31m Photometry Pipeline Installation Master Script \033[0;34m ==\033[0m"
echo -e "\033[0;34m======================================================\033[0m"
sleep 1
echo -e "Welcome to the installation script, \033[0;31m`whoami`\033[0m!" 
sleep .2
echo -e "I will be running with many \033[0;31m pre-defined \033[0m variables! A sample are below:"
echo -e "(These can be edited now in the MASTER_INSTALL \033[0;31m or \033[0m later in your configure.sh)"
sleep .2
echo -e "    RUNROOT\033[0;34m=\033[0;31m$RUNROOT \033[0m"
echo -e "    SURVEY\033[0;34m=\033[0;31m$SURVEY \033[0m"
echo -e "    AWSURVEYNAME\033[0;34m=\033[0;31m$AWSURVEYNAME \033[0m"
echo -e "    FILESUFFIX\033[0;34m=\033[0;31m$FILESUFFIX \033[0m"
echo -e "    USER\033[0;34m=\033[0;31m$USER \033[0m"
echo -e "    SCRIPTPATH\033[0;34m=\033[0;31m$SCRIPTPATH \033[0m"
echo -e "    WORKINGDIR\033[0;34m=\033[0;31m$WORKINGDIR \033[0m"
echo -e "    CONFIGPATH\033[0;34m=\033[0;31m$CONFIGPATH \033[0m"
sleep .2
echo -e ""
echo -e "If you want to update these now then you \033[0;31m may kill the script now \033[0m and"
echo -e "edit the PHOTOPIPE_MASTER_INSTALL.sh script variables (at the top of the file). Otherwise you " 
echo -e "will need to edit and rerun the configure script after the MASTER_INSTALL is completed. " 
sleep .2
echo -en "\033[0;34mYou have 10 sec to decide... \033[0m  "
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

#sleep 10 & spinner 

echo " OK!"
sleep 1
echo -e "\033[0;34mStarting Installation now. \033[0m  "
sleep .5
echo -e "\033[0;34m=======================================\033[0m"
#}}}

#Install Packages and useful scripts {{{
#If the functions are already installed, skip {{{
if [ ! -d ${RUNROOT}/INSTALL ]
then 
  #Move into the install directory {{{
  #echo -e "   >\033[0;31m ERROR: There is a previous pipeline installation in \033[0m" 
  #echo -e "   >\033[0;34m ${RUNROOT}/INSTALL \033[0m" 
  #echo -e "   >\033[0;31m If you want to rerun the installation, then you must delete it!\033[0m" 
  #echo -e "\033[0;34m=======================================\033[0m"
  #trap : 0 
  #exit 1 
  #rm -r ${RUNROOT}/INSTALL
  echo -en "   >\033[0;34m Creating Initial Directory structure \033[0m" 
  mkdir -p ${RUNROOT}/INSTALL 
  cd ${RUNROOT}/INSTALL
  echo -e "\033[0;31m - Done! \033[0m" 
  #}}}

  #Run the Script and Package Installations {{{
  #Install Local Python Version {{{
  echo -en "   >\033[0;34m Installing Local Anaconda Python2.7 \033[0m" 
  echo > ${RUNROOT}/INSTALL/yesdoc.txt <<EOF
  yes
EOF
  wget http://repo.continuum.io/archive/Anaconda2-4.3.0-Linux-x86_64.sh > python_wget.log 2>&1
  bash Anaconda2-4.3.0-Linux-x86_64.sh -b -p ./anaconda2/ > Anaconda_install.log 2>&1
  export PYTHONPATH=${RUNROOT}/INSTALL/anaconda2/bin/python2:${RUNROOT}/INSTALL/anaconda2/lib/
  export PATH=${RUNROOT}/INSTALL/anaconda2/bin/:${PATH}
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${RUNROOT}/INSTALL/anaconda2/lib/
  echo -e "\033[0;31m - Done! \033[0m" 
  echo -en "   >\033[0;34m Setting Anaconda configuration \033[0m" 
  ##${RUNROOT}/INSTALL/anaconda2/bin/conda config --set channel_priority strict
  ${RUNROOT}/INSTALL/anaconda2/bin/conda update conda  > python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt
  echo -e "\033[0;31m - Done! \033[0m" 
  echo -en "   >\033[0;34m Installing Python modules \033[0m" 
  ${RUNROOT}/INSTALL/anaconda2/bin/pip install tdqm numpy astroquery==0.4.0 astropy pyfits > python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt 
  echo -e "\033[0;31m - Done! \033[0m" 
  #echo -en "   >\033[0;34m Installing cfitsio, pgplot, source-extractor, gfortran, libxcb, tcsh, swarp \033[0m" 
  #${RUNROOT}/INSTALL/anaconda2/bin/conda install -c conda-forge tcsh screen cfitsio pgplot astromatic-source-extractor gfortran_linux-64 \
  #  libxcb astromatic-swarp >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt
  echo -en "   >\033[0;34m Installing cfitsio, pgplot, gfortran, libxcb, tcsh \033[0m" 
  ${RUNROOT}/INSTALL/anaconda2/bin/conda install -c conda-forge tcsh screen cfitsio pgplot gfortran_linux-64 \
    libxcb >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt
  echo -e "\033[0;31m - Done! \033[0m" 
  #echo -en "   >\033[0;34m Installing gfortran \033[0m" 
  #${RUNROOT}/INSTALL/anaconda2/bin/conda install  >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt
  #echo -e "\033[0;31m - Done! \033[0m" 
  ##echo -en "   >\033[0;34m Installing additional conda-forge tools \033[0m" 
  ##${RUNROOT}/INSTALL/anaconda2/bin/conda install -c conda-forge cfitsio pgplot astromatic-source-extractor openmp >> python_packages.log 2>&1 <<EOF
  #echo -en "   >\033[0;34m Installing cfitsio \033[0m" 
  #timeout 120s ${RUNROOT}/INSTALL/anaconda2/bin/conda install -c conda-forge cfitsio >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt || echo -en "- timeout"
  #echo -e "\033[0;31m - Done! \033[0m" 
  #echo -en "   >\033[0;34m Installing pgplot \033[0m" 
  #timeout 120s ${RUNROOT}/INSTALL/anaconda2/bin/conda install -c conda-forge pgplot >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt || echo -en "- timeout"
  #echo -e "\033[0;31m - Done! \033[0m" 
  #echo -en "   >\033[0;34m Installing source extractor \033[0m" 
  #timeout 120s ${RUNROOT}/INSTALL/anaconda2/bin/conda install -c conda-forge astromatic-source-extractor >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt || echo -en "- timeout"
  #echo -e "\033[0;31m - Done! \033[0m" 
  ##}}}
  #Install THELI LDAC tools {{{
  echo -en "   >\033[0;34m Installing THELI LDAC tools\033[0m" 
  if [ ! -d ${PACKROOT}/theli-1.30.0 ]
  then 
    if [ -f ${RUNROOT}/../INSTALL/theli-1.6.1.tgz ]
    then 
      ln -s ${RUNROOT}/../INSTALL/theli-1.6.1.tgz .
    else 
      wget https://marvinweb.astro.uni-bonn.de/data_products/theli/theli-1.6.1.tgz > ${RUNROOT}/INSTALL/THELI_wget.log 2>&1
    fi 
    tar -xf theli-1.6.1.tgz >> THELI_install.log 2>&1
    rm -f theli-1.6.1.tgz  >> THELI_install.log 2>&1
    cd theli-1.6.1/pipesetup
    THELIPACKVERS=1.6.1
    THELIPACKSUFFIX=   
  else 
    cd ${RUNROOT}/INSTALL
    cp -r ${PACKROOT}/theli-1.30.0 .
    cd theli-1.30.0/pipesetup
    THELIPACKVERS=1.30.0
    THELIPACKSUFFIX="_theli"
  fi 
  bash install.sh -m ALL >> THELI_install.log 2>&1
  if [ "${THELIPACKSUFFIX}" != "" ]
  then 
    cd ${RUNROOT}/INSTALL/theli-${THELIPACKVERS}/bin/${MACHINE}/
    for file in `ls *${THELIPACKSUFFIX}`
    do
      ln -s ${file} ${file//${THELIPACKSUFFIX}/}
    done
  fi
  cd ${RUNROOT}/INSTALL
  echo -e "\033[0;31m - Done! \033[0m" 
  #}}}
  #Install GAAP {{{
  echo -en "   >\033[0;34m Installing GAAP \033[0m" 
  cd ${RUNROOT}/INSTALL
  cp -r ${PACKROOT}/gapphot_TE . > ${RUNROOT}/INSTALL/gaap_copy.log 2>&1
  #Compile the kk directory 
  cd ${RUNROOT}/INSTALL/gapphot_TE/kk
  make clean  > ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  make >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  sed -i "s@^F77 =@F77 = ${RUNROOT}/INSTALL/anaconda2/bin/x86_64-conda_cos6-linux-gnu-gfortran \#@" makefile 
  sed -i "s@^libs =@libs = -L ${RUNROOT}/INSTALL/anaconda2/lib/ -lpgplot -lcfitsio -L. -lshape -lutil \#@" makefile 
  make all >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  #compile the bigim directory 
  cd ${RUNROOT}/INSTALL/gapphot_TE/kk/bigim/
  make clean  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  #This binary isn't removed in the clean
  rm -f kermapm2rot 
  make >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  sed -i "s@^F77 =@F77 = ${RUNROOT}/INSTALL/anaconda2/bin/x86_64-conda_cos6-linux-gnu-gfortran \#@" makefile 
  sed -i "s@^libs =@libs = -L ${RUNROOT}/INSTALL/anaconda2/lib/ -lpgplot -lcfitsio -L. -lshape -lutil \#@" makefile 
  sed -i "s@^all: @all: set gapphot fitkermaptwk imxshmapwithtweak kermapm2rot pix2g8 pixpsfxshcpts8 psfcat2gauskerwithtweak psfcat2gauskerwithtweak_no_recentre showdxdy showpsfmaptwk kermapm2rot @" makefile 
  echo "" >> makefile 
  echo "psfcat2gauskerwithtweak_no_recentre: psfcat2gauskerwithtweak_no_recentre.o libshape.a libutil.a" >> makefile 
  echo '	$(F77)  psfcat2gauskerwithtweak_no_recentre.o $(libs) -o psfcat2gauskerwithtweak_no_recentre' >> makefile
  echo "" >> makefile 
  echo "kermapm2rot: kermapm2rot.o libshape.a libutil.a" >> makefile 
  echo '	$(F77)  kermapm2rot.o $(libs) -o kermapm2rot' >> makefile
  make all >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  cd ${RUNROOT}
  echo -e "\033[0;31m - Done! \033[0m"
  #}}}
  #Install WCSTools {{{
  echo -en "   >\033[0;34m Installing WCStools \033[0m" 
  cd ${RUNROOT}/INSTALL/
  wget http://tdc-www.harvard.edu/software/wcstools/wcstools-3.9.6.tar.gz > ${RUNROOT}/INSTALL/wcstools_wget.log 2>&1
  tar -xf wcstools-3.9.6.tar.gz 
  cd wcstools-3.9.6
  make all > ${RUNROOT}/INSTALL/wcstools_make.log 2>&1
  echo -e "\033[0;31m - Done! \033[0m"
  #}}}
  #}}}
  echo -e "\033[0;31m   ##Script Installations all done!##\033[0m" 
fi 
#}}}
#Check that THELI Package version is defined {{{
if [ "${THELIPACKVERS}" == "" ]
then
  if [ -d ${RUNROOT}/INSTALL/theli-1.30.0 ]
  then 
    THELIPACKVERS=1.30.0
    THELIPACKSUFFIX="_theli"
  else 
    THELIPACKVERS=1.6.1
    THELIPACKSUFFIX=""
  fi 
fi 
#}}}
cd ${RUNROOT}
##}}}

#Add useful Functions to Python Lib {{{
echo -en "   >\033[0;34m Adding usefull functions to python lib \033[0m" 
cd ${RUNROOT}/INSTALL/anaconda2/lib/
cp -f ${PACKROOT}/scripts/ldac.py ${PACKROOT}/scripts/sqlcl.py . > ${RUNROOT}/INSTALL/PythonLib_link.log 2>&1
echo -e "\033[0;31m - Done! \033[0m" 
#}}}

#Update the run script for this run {{{
echo -en "   >\033[0;34m Update the configure script \033[0m" 
#PYTHONBIN=${RUNROOT}/INSTALL/anaconda2/bin/
cp ${PACKROOT}/scripts/run_PhotoPipe_raw.sh ${RUNROOT}/run_PhotoPipe.sh 
#Make the Script, Config, and Runtime directories 
mkdir -p ${RUNROOT}/${SCRIPTPATH}/ ${RUNROOT}/${CONFIGPATH}/ ${RUNROOT}/${WORKINGDIR}/
cp -r ${PACKROOT}/scripts/* ${RUNROOT}/${SCRIPTPATH}/
cp ${PACKROOT}/config/* ${RUNROOT}/${CONFIGPATH}/
for OPT in $OPTLIST
do 
  sed -i "s#\@${OPT}\@#${!OPT}#g" ${RUNROOT}/run_PhotoPipe.sh ${RUNROOT}/${SCRIPTPATH}/*.*
done 
echo -e "\033[0;31m - Done! \033[0m" 
#}}}

#Closing Prompt {{{
echo -e "\033[0;34m=======================================\033[0m"
#Finished! 
trap : 0
echo -e "\033[0;34m=======================================\033[0m"
echo -e "\033[0;34m==\033[31m  Pipeline Installation Complete!  ==\033[0m"
echo -e "\033[0;34m=======================================\033[0m"
#}}}

