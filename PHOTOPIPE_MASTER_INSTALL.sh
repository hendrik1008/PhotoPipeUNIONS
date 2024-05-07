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
RUNROOT=/arc/home/hendrik/PhotoPipeTest/
#Directory for runtime script storage
RUNTIME=RUNTIME
#Survey ID  
SURVEY=UNIONS5000
#Directory of the MegaPipe catalogues
CATDIR=/net/home/fohlen14/hendrik/UNIONS/UNIONS2000/catalogues_MP_DR5/
#Directory of the images
IMDIR=/net/home/fohlen14/hendrik/UNIONS/UNIONS2000/
#Username (default: `whoami`) 
USER=`whoami`
#Directory for work
WORKINGDIR=work_${SURVEY}
#Path to configuration files
CONFIGPATH=RUNTIME/config/
#Path to modified script files
SCRIPTPATH=RUNTIME/scripts/
#Do we want to do a DRYRUN (!=0 := YES)
DRYRUN=0
#Define the Pointing Filelist 
#POINTINGLIST=W3_testtiles_new.txt
#POINTINGLIST=ugriz_tiles.txt1
#POINTINGLIST=specz_testtile.txt
POINTINGLIST=r_tiles.txt1
#File containing Deep Spec-z for photo-z comparison 
DEEPZCAT=/net/home/fohlen11/hendrik/data/DEEP2/DEEP2_specz.cat
#File containing Seb's spec-z for photo-z comparison 
ZCAT=/net/home/fohlen14/hendrik/UNIONS/redshifts-2024-01-04/redshifts-2024-01-04.asc 
#Machine type
MACHINE=Linux_64 # can be seen using `uname`
#THELI Path 
THELIPATH=${RUNROOT}/INSTALL/theli-1.6.1/bin/${MACHINE}/
#File with pointing WCS limits
POINTINGLIMITSFILE=${RUNROOT}/${CONFIGPATH}/KIDS_ra_dec_cuts.txt        #KIDS
#Number of threads 
NTHREAD=100
#Set the wait time between completion checks 
REFRESHRATE=5
#Logfile name 
LOGFILE=PhotoPipe.log
#}}}
#Full list of options {{{
OPTLIST="NOCONFIG PACKROOT RUNROOT RUNTIME SURVEY USER \
  WORKINGDIR CONFIGPATH SCRIPTPATH DRYRUN \
  POINTINGLIST POINTINGLIMITSFILE \
  THELIPATH NTHREAD \
  REFRESHRATE LOGFILE \
  MACHINE DEEPZCAT ZCAT \
  CATDIR IMDIR"
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
  echo -en "   >\033[0;34m Creating Initial Directory structure \033[0m" 
  mkdir -p ${RUNROOT}/INSTALL 
  cd ${RUNROOT}/INSTALL
  echo -e "\033[0;31m - Done! \033[0m" 
  #}}}

  #Run the Script and Package Installations {{{
  #Install Local Python Version {{{
  echo -en "   >\033[0;34m Installing Local Anaconda Python2.7 \033[0m" 
  echo > ${RUNROOT}/INSTALL/yesdoc.txt `cat <<EOF
  y
EOF`
  wget http://repo.continuum.io/archive/Anaconda2-4.3.0-Linux-x86_64.sh > python_wget.log 2>&1
  bash Anaconda2-4.3.0-Linux-x86_64.sh -b -p ./anaconda2/ > Anaconda_install.log 2>&1
  export PYTHONPATH=${RUNROOT}/INSTALL/anaconda2/bin/python2:${RUNROOT}/INSTALL/anaconda2/lib/
  export PATH=${RUNROOT}/INSTALL/anaconda2/bin/:${PATH}
  export LD_LIBRARY_PATH=${RUNROOT}/INSTALL/anaconda2/lib/:${LD_LIBRARY_PATH}
  echo -e "\033[0;31m - Done! \033[0m" 
  echo -en "   >\033[0;34m Setting Anaconda configuration \033[0m" 
  echo step1 > python_packages.log
  conda config --set ssl_verify no
  conda create -p ${RUNROOT}/INSTALL/anaconda2/photopipe_env >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt
  echo step3 >> python_packages.log
  source activate ${RUNROOT}/INSTALL/anaconda2/photopipe_env >> python_packages.log 2>&1
  if [ ! $? -eq 0 ]
  then
      echo "ERROR: activate failed. Do the following:\nClose/Reopen the shell\n'conda activate ${RUNROOT}/INSTALL/anaconda2/photopipe_env'\n and rerun the PHOTOPIPE_MASTER_INSTALL.sh."
      exit 1
  fi
else 
  cd ${RUNROOT}/INSTALL
  export PYTHONPATH=${RUNROOT}/INSTALL/anaconda2/bin/python2:${RUNROOT}/INSTALL/anaconda2/lib/
  export PATH=${RUNROOT}/INSTALL/anaconda2/bin/:${PATH}
  export LD_LIBRARY_PATH=${RUNROOT}/INSTALL/anaconda2/lib/:${LD_LIBRARY_PATH}
  source activate ${RUNROOT}/INSTALL/anaconda2/photopipe_env
fi 

#Install Packages and useful scripts {{{
#If the functions are already installed, skip {{{
if [ ! -d ${RUNROOT}/INSTALL/bpz-1.99.3_expanded ] 
then 
  echo -en "   >\033[0;34m Setting Anaconda configuration \033[0m" 
  echo step4 >> python_packages.log
  echo -e "\033[0;31m - Done! \033[0m" 
  echo -en "   >\033[0;34m Installing Python modules \033[0m" 
  ${RUNROOT}/INSTALL/anaconda2/bin/python -m pip install tdqm numpy astroquery==0.4.0 astropy requests >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt 
  echo -e "\033[0;31m - Done! \033[0m" 
  echo -en "   >\033[0;34m Installing cfitsio, pgplot, gfortran, libxcb, tcsh \033[0m" 
  conda config --set ssl_verify no
  conda install -c conda-forge tcsh screen cfitsio pgplot gfortran_linux-64=9.3.0 \
    libxcb astromatic-swarp imagemagick gawk >> python_packages.log 2>&1 < ${RUNROOT}/INSTALL/yesdoc.txt
  echo -e "\033[0;31m - Done! \033[0m" 
  #}}}
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
      ln -sf ${file} ${file//${THELIPACKSUFFIX}/}
    done
  fi
  cd ${RUNROOT}/INSTALL
  echo -e "\033[0;31m - Done! \033[0m" 
  #}}}
  #Install WCSTools {{{
  echo -en "   >\033[0;34m Installing WCStools \033[0m" 
  cd ${RUNROOT}/INSTALL/
  wget http://tdc-www.harvard.edu/software/wcstools/Old/wcstools-3.9.6.tar.gz > ${RUNROOT}/INSTALL/wcstools_wget.log 2>&1
  tar -xf wcstools-3.9.6.tar.gz 
  cd wcstools-3.9.6
  make all > ${RUNROOT}/INSTALL/wcstools_make.log 2>&1
  echo -e "\033[0;31m - Done! \033[0m"
  #}}}
  #Install BPZ {{{
  echo -en "   >\033[0;34m Installing BPZ \033[0m" 
  cd ${RUNROOT}/INSTALL/
  cp -f ${PACKROOT}/bpz-1.99.3_expanded.tar.gz . 
  tar -xf bpz-1.99.3_expanded.tar.gz
  echo -e "\033[0;31m - Done! \033[0m"
  #}}}
  echo -e "\033[0;31m   ##Script Installations all done!##\033[0m" 
fi 

#Install GAaP {{{
#If the functions are already installed, skip {{{
if [ ! -d ${RUNROOT}/INSTALL/gapphot_TE ] || [ ! -s ${RUNROOT}/INSTALL/gapphot_TE/kk/listfitbeta ] || [ ! -s ${RUNROOT}/INSTALL/gapphot_TE/kk/bigim/pixpsfmap ]
then 
  #Install GAAP {{{
  echo -en "   >\033[0;34m Installing GAAP \033[0m" 
  cd ${RUNROOT}/INSTALL
  cp -r ${PACKROOT}/gapphot_TE . > ${RUNROOT}/INSTALL/gaap_copy.log 2>&1
  #Compile the kk directory 
  cd ${RUNROOT}/INSTALL/gapphot_TE/kk
  make clean  > ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  make >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  sed -i "s@^F77 =@F77 = ${RUNROOT}/INSTALL/anaconda2/photopipe_env/bin/x86_64-conda_cos6-linux-gnu-gfortran \#@" makefile 
  sed -i "s@^libs =@libs = -L ${RUNROOT}/INSTALL/anaconda2/lib/ -L ${RUNROOT}/INSTALL/anaconda2/photopipe_env/lib/ -lXau -lpgplot -lcfitsio -L. -lshape -lutil \#@" makefile 
  #cp ${PACKROOT}/libgfortran.so.5.0.0 ${RUNROOT}/INSTALL/anaconda2/photopipe_env/lib/
  make all >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  #compile the bigim directory 
  cd ${RUNROOT}/INSTALL/gapphot_TE/kk/bigim/
  make clean  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  #This binary isn't removed in the clean
  rm -f kermapm2rot 
  make >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1 || echo cleaned  >> ${RUNROOT}/INSTALL/gaap_make.log 2>&1
  sed -i "s@^F77 =@F77 = ${RUNROOT}/INSTALL/anaconda2/photopipe_env/bin/x86_64-conda_cos6-linux-gnu-gfortran \#@" makefile 
  sed -i "s@^libs =@libs = -L ${RUNROOT}/INSTALL/anaconda2/lib/ -L ${RUNROOT}/INSTALL/anaconda2/photopipe_env/lib/ -lXau -lpgplot -lcfitsio -L. -lshape -lutil \#@" makefile 
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
  #}}}
  echo -e "\033[0;31m   ##GAaP installation done!##\033[0m" 
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
  sed -i "s#\@${OPT}\@#${!OPT}#g" ${RUNROOT}/run_PhotoPipe.sh ${RUNROOT}/${SCRIPTPATH}/QC/*.*
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

