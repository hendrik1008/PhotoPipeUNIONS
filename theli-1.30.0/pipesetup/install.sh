#!/usr/bin/env bash

# calling bash via 'env' does not allow passing an argument
# to bash. Hence we set '-u' here:
set -u

# Main installation script for the THELI pipeline

# IMPORTANT:
#
# The script checks machine type, updates pipeline scripts (progs.ini;
# pipe_env ...)  and compiles pipeline sources the script needs the
# source file 'endian.c' in the directory it is called. Modify the
# paths listed below before running this script.

#
# Modes of the script
#
# ./install.sh -m "TEST"
# tests the machine type and gives information whether you can compile
# sources without problems or whether you have to create new Makefiles
# (see the documentation for more details)
#
# ./install.sh -m "ALL"
# updates paths in all pipeline scripts and compiles pipeline sources
#
# ./install.sh -m "UPDATE"
# only updates paths in pipeline scripts
#
# ./install.sh -m "COMPILE"
# only compiles pipeline sources

#
# HISTORY COMMENTS:
#
# 09.03.2006:
# - I removed the utilities scripts subdirectory from
#   the script updates. It contains no files needed in
#   the official THELI version.
# - I included MAC/Darwin systems to the known architectures.
#
# 14.03.2006:
# I changed the installation scheme for ASTROMETRIX: instead of
# overriding the original script files with modified ones, I keep the
# modified, renamed, ones in the pipeline structure and add the
# corresponding directory the PERL5LIB variable.  This allows us to
# directly call the modified ASTROMETRIX files with the perl
# interpreter.
#
# 27.04.2006:
# I adapted the location of 'global_vers.h' (it is now in
# ../ldactools/library and no longer in ../library)
#
# 01.05.2006:
# The pipeline version for the external version is 'THELI Pipeline
# Version: XXXX'. This allows a distinction to the internal sources.
#
# 15.05.2006:
# - I included a simple check for the presence of necessary, external
#   programs defined in this script.
# - The script now prints some description when being invoked.
#
# 17.05.2006:
# - I changed slightly the welcome message
# - increase of the minor pipeline version number.
# - I improved messages during the installation process.
# - The handling of the test for the availabbility of external
#   UNIX programs is more stable to account for architecture dependent
#   behaviours of the 'which' program.
#
# 14.06.2006:
# - The availability of the external 'sm' program is checked via
#   a 'which' command now.
# - Increase of minor pipeline version number.
#
# 30.08.2006:
# Increase of minor pipeline version number.
#
# 05.11.2006:
# - If the program 'sm' is not present, the variable P_SM
#   is set to an empty string. Without this the value could
#   be quite undefined. This avoids potential problems
#   in the 'sed' command which modifies the progs.ini file.
# - In the 'sed' command which modifies the progs.ini file
#   I removed lines refering to IRAF.
#
# 06.11.2006:
# The version number goes to 0.12.0
#
# 18.09.2007:
# I included code for the possibility to use scamp within THELI. The
# script documentation was updated accordingly.
#
# 03.12.2007:
# I introduced Darwin_PPC and Darwin_INTEL systems; the old 'Darwin'
# was removed. This was necessary as the Intel and PPC MACS come with
# different byte swap for floating point numbers.
#
# 06.07.2008:
# Cosmetic changes to avoid error messages during install
#
# 21.12.2008:
# - The interpreters for python and perl are defined now
#   also via variables. The main problem was that 'alias'
#   deinitions were not taken into account in pipeline
#   script calls leading to problems when the script
#   interpreters were privately installed.
# - I updated the documentation
#
# 09.02.2010:
# Improvements to the verification of installation requirements.
#
# 15.03.2010:
# Bug Fix: Configuration files for WFI were not copied correctly
# to the 'MACHINE' directory
#
# 30.03.2010:
# I included the perl script 'stellar_locus.pl' which needed
# modification in the updatescripts part.
#
# 31.08.2010:
# Version number goes to 1.2.1
#
# 16.09.2010:
# Version number goes to 1.2.2 (internal production version)
#
# 16.09.2010:
# Version number goes to 1.3.0 (new official release)
#
# 08.10.2010:
# Version number goes to 1.3.1 (internal production version)
#
# 26.11.2010:
# - MEGAPRIME files are now included in the distribution
# - Version number goes to 1.4.0 (internal production version);
#   new swarp version and associated changes
#
# 17.01.2011:
# version number goes to 1.4.1 (internal production version)
#
# 24.05.2011:
# version number goes to 1.5.0 (release version; several important bug fixes)
#
# 13.06.2011:
# Version number goes to 1.5.1; I included Makefiles for Darwin_INTEL 64
# architectures.
#
# 27.07.2011:
# Version number goes to 1.5.2 (minor bug fixes)
#
# 05.09.2011:
# Version number goes to 1.5.3 (minor bug fixes / enhancements)
#
# 26.01.2012:
# Version number goes to 1.6.0 (bug fixes / enhancements)
#
# 31.01.2012:
# Version number goes to 1.6.1 (bug fixes / enhancements)
#
# 19.11.2012:
# Version number goes to 1.6.3 (bug fixes / enhancements)
#
# 26.11.2012:
# I implemented photometric calibration on a run basis; version
# number is 1.6.6 now.
#
# 30.12.2012:
# many small updates for a first V0.1 run of KIDS data.
# version number goes to 1.6.7
#
# 10.01.2013:
# introduction of new error reporting and logging scheme
# version number goes to 1.7.0
#
# 18.01.2013:
# lots of small bugs for KIDS processing fixed.
# version number goes to 1.7.1
#
# 11.02.2013:
# again lots of small bugs for KIDS processing fixed.
# version number goes to 1.7.2
#
# 20.02.2013:
# again lots of small bugs for KIDS processing fixed.
# version number goes to 1.7.4
#
# 03.04.2013:
# again lots of small bugs for KIDS processing fixed.
# version number goes to 1.7.5
#
# 16.05.2013:
# and again a lot of small bug fixes; version number goes to 1.7.6.
#
# 24.05.2013:
# upgrade to SExtractor 2.5.0 to use windowed object positions in various
# parts of the pipeline; version number goes to 1.8.0.
#
# 22.09.2013:
# I start to integrate windowed SExtractor quantities; this is done
# within version 1.9.0.
#
# 22.01.2014:
# - The code did not compile on MACOS 10.9 - fixed. Code version goes to 1.9.1
# - We now used windowed position quantities for astrometric calibration.
#
# 03.04.2014:
# - The TEMPDIR var. for progs.ini is no longer modified in this script.
#   We sat it to `pwd` in the include script itself. If the user wants to
#   change it this is done in that script anyway.
# - Version number goes to 1.9.3 - again many small fixes/refinements for
#   KIDS/ATLAS processing.
#
# 15.05.2014:
# Version number goes to 1.9.5 - again many small bug fixes during KIDS data
# processing.
#
# 26.06.2014:
# Version number goes to 1.9.6. Main improvement is a consequent consideration
# of bad CCDs in the preprocessing phase.
#
# 29.09.2014:
# Numerous small refinements with the inclusion of the DECam
# instrument. Version number goes to 1.9.8.
#
# 10.11.2014:
# - cleanup for no-longer used variables in the progs.ini file (they
#   concerned the setiing of astrometric and photometric standardstar
#   catalogues with LDAC calibration).
# - The global pipeline version number now has to be transfered to
#   bash_functions.include to be included in THELI logging messages -
#   version number goes to 1.9.9.
#
# 21.01.2015:
# Numerous refinements in the course of a new KIDS/ATLAS processing run.
# Version number goes to 1.9.10.
#
# 23.02.2015:
# I included treatment of the 'conv_WFI_filter_names.include'. It
# contains an alias definition to perform filter name translation for
# the WFI@MPG/ESO2.2m instrument.
#
# 16.06.2015:
# Inclusion of scripts and modifications to process MEGAPRIME@CFHT
# data with its new 40-chip mode. Version number pushed to 1.9.11.
#
# 05.08.2015:
# - Instead for 'python' I now test for an executable 'python2' to
#   ensure we use python 2 and not python 3.
# - We now test for the number of available CPUs / cores and put this
#   number automatically into the progs.ini file (NPARA variable).
#
# 11.08.2015:
# version number pushed to 1.10.0.
#
# 28.09.2015:
# refinements in the course of KIDS data processing. Version number
# goes to 1.10.1.
#
# 14.12.2015:
# weightwatcher update from version 1.3 to version 1.12. Pipeline version
# goes to 1.11.1
#
# 20.07.2016:
# Refinements in the course of DECam processing. Pipeline version
# goes to 1.12.1
#
# 28.07.2016:
# The variable P_PYTHON was renamed to P_PYTHON3 to reflect the pipeline
# upgrade to python3. Accorfingly the script check_python_modules.sh was
# renamed to check_python3_modules.sh.
#
# 03.08.2016:
# Numerous changes and updates in the framework of DECam processing.
# Version number goes to 1.14.0.
#
# 26.08.2016:
# - Again numerous changes and refinements for DECam processing. Version
#   number goes to 1.15.0.
# - Perl and Python scripts that have the __PIPEVERS__ string somewhere
#   in the code get this replaced by the pipeline version number.
#
# 31.08.2016:
# Fix for a bug introduced with the changes from 26/08: Perl and Python
# scripts were no longer executable after installation.
#
# 14.09.2016:
# numerous fixes/refinements in the course of DECam processing; version
# number goes to 1.16.0.
#
# 11.12.2016:
# numerous fixes/refinements in the course of DECam processing; version
# number goes to 1.17.0.
#
# 06.02.2017:
# numerous fixes/refinements in the course of DECam processing; version
# number goes to 1.18.0.
#
# 20.04.2017:
# refinements/fixes in the course of DECam processing; inclusion of
# brighter/fatter correction for DECam; version number goes to 1.20.0
#
# 19.07.2017:
# refinements/fixes in the course of KiDS-900 processing; version number
# goes to 1.21.0
#
# 23.10.2017:
# refinements/fixes in the course of KiDS-900 processing; version number
# goes to 1.22.0
#
# 18.12.2017:
# refinements in the course of CFIS processing - version goes to 1.23.0
#
# 28.03.2018:
# A number of bug fixes and resolution of python2 -> python3 issues -
# version number goes to 1.24.0
#
# 04.09.2018:
# Again lots of small changes and bug fixes - versdion number goes to 1.30


##
## stuff that has to be modified by the user
##

# needed directories (absolutely necessary)
#
# PD is the main directory with the pipeline sources, i.e. the
# .../theli_xxxx directory. PD has to contain the complete, absolute
# path.

#
# Make a 'good guess' for PD. DO NOT EDIT !!
DIR=`pwd`
cd ..
PD=`pwd`
cd ${DIR}

# needed programs/scripts not included in the pipeline distribution
# (absolutely necessary)
#
# The path for the bash shell (you need version
# 3.0 or higher)
P_BASH=`which bash`
#
# The path to the GNU awk program (awk is NOT sufficient)
P_GAWK=`which gawk`
#
# The path of the UNIX find program (the '-maxdepth' option
# has to be supported; use for instance the GNU find !!)
P_FIND=`which find`
#
# The path of the UNIX sort program (the '-g' option
# has to be supported; use for instance the GNU sort !!)
P_SORT=`which sort`
#
# The path to the perl and python interpreters called
# by pipeline scripts:
P_PYTHON3=`which python3`
P_PERL=`which perl`

#
# the following programs are not absolutely necessary (see the manual
# to learn what restrictions you have in this case)
#
# Path to the Super Mongo (sm) program
P_SM=`which sm`
#
# Path to the scamp program. The current THELI pipeline supports
# V1.7.0 of that program
#
P_SCAMP=`which scamp`
#
# Path to the aclient program. It is mandatory if scamp is used within
# THELI
P_ACLIENT=`which aclient`

# C-compiler used in this script (this affects ONLY this script; not
# the C-compiler used in the compilation of the sources)
CC=gcc

# known architectures for source compilation (see the documentation
# for further information)
KNOWNARCH="AIX Darwin_INTEL Darwin_INTEL_64 Darwin_PPC HP-UX IRIX64 Linux Linux_64 OSF1 SunOS"

##
## here the stuff that should ever be edited by a regular user ends!!!
##


#
# Pipeline version
PIPEVERS="Pipeline Version: 1.30.0"

#
# subprogram definitions
#

#
# by default the machine architecture is not known !!
KNOWN=0
MACHINE=""
NCPUS=1  # defualt for number of available cores / CPUs

function updatescripts() {
  DIR=`pwd`
  cd $1
  #
  # deal with the bash shell scripts where we substitute the first
  # line by the actual location of the bash shell
  SCRIPTFILES=`find . -name \*.sh.raw`

  for SCRIPT in ${SCRIPTFILES}
  do
    BASE=`basename ${SCRIPT} .sh.raw`
    sed -e 's!/bin/bash!'${P_BASH}'!' \
           ${SCRIPT} > ${PD}/scripts/${MACHINE}/${BASE}.sh
    chmod a+x ${PD}/scripts/${MACHINE}/${BASE}.sh
  done

  cd ${DIR}
}

# test the machine type run also sysconf from eclipse check the
# presence of several UNIX tools and optional programs
#
function testmachine() {
  # first test if the endian program is alreay available; if not,
  # compile it!
  if [ ! -f endian.c ]; then
    echo "need the source file 'endian.c'"
    exit 1
  else
    ${CC:?} -o endian endian.c
  fi

  echo "---------------------------------------------------"
  tput bold
  echo "Detect your architecture:"
  tput sgr0
  echo ""

  MACHINE=`uname`

  # test for LINUXALPHA architecture if the machine is Linux
  if [ "${MACHINE}" = "Linux" ]; then
    LONG=`./endian | ${P_GAWK:?} '($1=="Sizes:") {print substr($5,3,1)}'`

    if [ "${LONG}" = "8" ]; then
      MACHINE="Linux_64"
    fi
  fi

  # test for the different MAC/Darwin systems (PPC or Intel)
  if [ "${MACHINE}" = "Darwin" ]; then
    INTEL=`./endian | ${P_GAWK:?} '($2=="machine") {print $5}'`

    MACHINE="Darwin_PPC"
    if [ "${INTEL}" = "Intel" ]; then
      MACHINE="Darwin_INTEL"
    fi

    LONG=`./endian | ${P_GAWK:?} '($1=="Sizes:") {print substr($5,3,1)}'`

    if [ "${LONG}" = "8" ]; then
      MACHINE="${MACHINE}_64"
    fi
  fi

  # test for known architecture
  for arch in ${KNOWNARCH}
  do
    if [ "${MACHINE}" = "${arch}" ]; then
	KNOWN=1
    fi
  done

  if [ "${KNOWN}" = "0" ]; then
    tput bold
    tput setf 4
    echo "Your machinetype \"${MACHINE}\" has not yet been tested !!"
    echo "Please refer to the manual how to proceed"
    echo ""
    ENDIAN=`./endian |  ${P_GAWK:?}'($1 == "this") {print $5}'`

    if [ "${ENDIAN}" = "Intel" ]; then
       echo "You need byte swap !!! (-DBSWAP flag)"
    else
       echo "You do not need byte swap !!!"
    fi

    LONG=`./endian | ${P_GAWK:?}  '($1=="Sizes:") {print substr($5,3,1)}'`

    if [ "${LONG}" = "8" ]; then
       echo "Long is represented by 8 bytes (-DINT64 flag)"
    else
       echo "Long is represented by 4 bytes"
    fi
    tput sgr0

    exit 1
  else
    echo "Your are running a \"${MACHINE}\" machine."
    echo "The compilation of THELI sources should work without problems"
    echo ""
  fi
  #
  # running eclipse's system configuration:
  #

  echo "---------------------------------------------------"
  echo ""
  tput bold
  echo "Run eclipse system configuration tool:"
  tput sgr0
  echo ""

  if [ ! -f sysconf.c ]; then
    echo "need the source file 'sysconf.c'"
    exit 1
  else
    ${CC:?} -o sysconf sysconf.c
  fi
  ./sysconf

  cp ./config.h ${PD}/eclipsetools/eclipse/include
  mv ./config.h ${PD}/eclipsetools/qfits

  # do a simple check whether the necessary UNIX programs defined
  # above are at least present and executable. Passing this test DOES
  # NOT ensure that the programs have the required functionality,
  # though.
  echo ""
  echo "---------------------------------------------------"
  echo ""
  tput bold
  echo "Checking the presence of the absolutely necessary UNIX tools:"
  echo "'bash', 'gawk', 'find', 'sort':"
  tput sgr0
  echo ""

  NEEDED_PROGRAMS="P_BASH P_GAWK P_FIND P_SORT"
  OPTIONAL_PROGRAMS="P_SM P_SCAMP P_ACLIENT"

  for PROGRAM in ${NEEDED_PROGRAMS}
  do
    # if the 'which' command to obtain program paths fails it outputs
    # several words which lead to an error in the following
    # 'if'. Hence the following TESTPROG definition.
    TESTPROG=`echo ${!PROGRAM} | awk '{print $1}'`
    if [ "A_${TESTPROG}" = "A_" ] || [ ! -x ${TESTPROG} ]; then
      tput bold
      tput setf 4
      echo "Program defined in ${PROGRAM} is not in your PATH or not executable"
      echo "Please correct this error before continuing !!"
      echo ""
      tput sgr 0

      exit 1
    fi
  done

  echo "'bash', 'gawk', 'find' and 'sort' seem to be present."
  echo "This test does not ensure that they fulfil the criteria"
  echo "documented in 'install.sh'"
  echo ""
  echo "---------------------------------------------------"
  echo ""
  tput bold
  echo "Checking of optional programs:"
  echo "sm, scamp, aclient:"
  tput sgr0
  echo ""

  for PROGRAM in ${OPTIONAL_PROGRAMS}
  do
    TESTPROG=`echo ${!PROGRAM} | awk '{print $1}'`
    if [ "A_${TESTPROG}" = "A_" ] || [ ! -x ${TESTPROG} ]; then
      tput bold
      tput setf 1
      echo "Program defined in ${PROGRAM} is not your PATH or not executable"
      echo "You can still use the THELI pipeline but with limited functionality !!"
      tput sgr0
      eval ${PROGRAM}=""
    else
      echo "Program defined in ${PROGRAM} is present. The test does not ensure"
      echo "that the executable represents the correct program."
    fi
  done

  echo ""
  echo "---------------------------------------------------"
  echo ""
  tput bold
  echo "Trying to obtain the number of available CPUs / cores of your machine"
  tput sgr0
  echo ""

  if [ -x "${P_PYTHON3}" ]; then
    NCPUS=`${P_PYTHON3} -c 'import multiprocessing; \
                print(multiprocessing.cpu_count())'`

    echo "Your machine has ${NCPUS} CPUs / cores."
  else
    echo "I cannot determine your number of CPUs / cores."
    echo "I will use a default of ${NCPUS}. You can change"
    echo "this manually later in the progs.ini file!"
  fi
}

#
# test version numbers of Python modules
function testpythonmodules() {
  echo ""
  echo "---------------------------------------------------"
  echo ""
  ./check_python3_modules.sh ${P_PYTHON3}
}

#
# create setup script responsible for setting
# the pipeline environment variables:
#
function environment() {
  # just to be sure:
  # first test the machine type if necessary
  if [ "${MACHINE}" = "" ]; then
    testmachine
  fi

  # create the pipe_env scripts

  sed -e 's!PIPESOFT=!PIPESOFT='${PD}'!' \
      -e 's!EXT=!EXT='${MACHINE}'!' \
      ${PD}/pipesetup/pipe_env.bash.raw \
      > ${PD}/pipesetup/pipe_env.bash.${MACHINE}

  sed -e 's! PIPESOFT! PIPESOFT '${PD}'!' \
      -e 's! EXT! EXT '${MACHINE}'!' \
      ${PD}/pipesetup/pipe_env.csh.raw \
      > ${PD}/pipesetup/pipe_env.csh.${MACHINE}
}

#
# print script help message:
function helpmessage() {
  echo "Install script for the THELI core pipeline"
  echo "=========================================="
  echo " "
  echo "PLEASE READ THE FOLLOWING COMPLETELY BEFORE DOING ANYTHING!"
  echo ""
  echo "This script executes necessary steps to compile the THELI sources and"
  echo "to prepare the THELI reduction scripts for your system. Before"
  echo "continuing, make sure the following preconditions:"
  echo ""
  echo "- make sure that you have the GNU C-compiler (gcc) working on your"
  echo "  system. All gcc V3.X and V4.X should work without problem.  "
  echo ""
  echo "- THELI needs the following external programs for full functionality:"
  echo "  - The UNIX/Linux tools: bash shell (V3.X), 'gawk', 'find', 'sort'."
  echo "    Those should be present on a standard UNIX/Linux/MacOS installation"
  echo "    that is suitable for scientific computing."
  echo ""
  echo "  - Emmanuel Bertins 'scamp' program for astrometric calibration (see"
  echo "    http://www.astromatic.net/software/scamp); that program in turn"
  echo "    needs the cdsclient package which contains the 'aclient' program"
  echo "    (see http://cdsweb.u-strasbg.fr/doc/cdsclient.html)"
  echo ""
  echo "  - The SuperMongo (sm) tool for producing control plots. This program"
  echo "    is non-free, but it is typically available at astronomical"
  echo "    institutes (see also http://www.astro.princeton.edu/~rhl/sm/)"
  echo ""
  echo "  - The Python language interpreseter in version V2.Y with Y>=5. In addition,"
  echo "    the python modules 'numpy' (V1.3.0 or higher), 'matplotlib'"
  echo "    (V0.98.1 or higher) and 'pyfits' (V1.1 or higher) are"
  echo "    needed. Python, and those modules are used for essential parts of"
  echo "    astrometric and photometric calibration."
  echo ""
  echo "  This install script has a testing mode (see below) which tries to"
  echo "  verify these preconditions. PLEASE run this mode and CORRECT ALL"
  echo "  potential problems before further installation."
  echo "  "
  echo "- Besids 'scamp' THELI can use Mario Radovichs ASTROMETRIX for astrometric "
  echo "  calibration. A special THELI version of that software is shipped as a "
  echo "  separate package (WIFIX_Distrib_X.X_theli.tgz; the original sources"
  echo "  from Mario can be found at http://www.na.astro.it/~radovich/wifix.htm)"
  echo "  If you want to use it, install it BEFORE continuing with the installation "
  echo "  of this package."
  echo ""
  echo "The actual THELI compilation and script preparation phases are done with the"
  echo "following calls to this script:"
  echo ""
  echo "- ./install.sh -m 'TEST'"
  echo "  This tests your architecture and reports whether THELI can be"
  echo "  compiled without further preparations. Running this mode should"
  echo "  give an output such as:"
  echo ""
  echo "  Your are running a 'MACHINE' machine."
  echo "  The compilation of THELI sources should work without problems"
  echo "  ."
  echo "  ."
  echo "  ."
  echo ""
  echo "  Please note the value of your MACHINE (probably Linux or Linux_64)."
  echo "  Also correct errors reported at the end of the output."
  echo ""
  echo "- ./install.sh -m 'ALL'"
  echo "  This compiles the THELI sources and adaptes the pipeline scripts to"
  echo "  your system. It is a combination of the COMPILE and UPDATE modes"
  echo "  and represents the complete installation task of this script."
  echo ""
  echo "- ./install.sh -m 'COMPILE'"
  echo "  This compiles THELI sources. After compilation, executables can be found"
  echo "  in .../theli-X.XX.XX/bin/MACHINE where MACHINE is your computer"
  echo "  architecture. "
  echo ""
  echo "- ./install.sh -m 'UPDATE'"
  echo "  This adapts the pipeline scripts to your system. The modified and "
  echo "  ready-to-use scripts can be found in .../theli-X.XX.XX/scripts/MACHINE"
  echo ""
  echo "To finish the setup, add the lines in the file"
  echo ".../theli-X.XX.XX/pipesetup/pipe_env.bash.MACHINE (bash shell) or"
  echo ".../theli-X.XX.XX/pipesetup/pipe_env.csh.MACHINE (C-shell) to your"
  echo "shell startup file ('.bashrc' if you use bash and '.cshrc' for the"
  echo "tc-shell respectively). You find the THELI data processing scripts in"
  echo "the directory .../theli-X.XX.XX/scripts/MACHINE."
  echo ""
}


# read command line argument(s)

if [ $# -lt 2 ]; then
  helpmessage
  exit 1
fi

# 4 MODES available:
# - TEST: the the machine type
# - ALL: do script update and compile sources
# - UPDATE: do script update
# - COMPILE: compile sources

MODE=""
while [ $# -gt 0 ]
do
  case $1 in
  -m)
      MODE=${2}
      shift
      shift
      ;;
   *)
      # there might be an 'empty string' argument which we
      # can ignore:
      if [ ! -z "$1" ]; then
        theli_error "Unknown command line option: ${1}"
        exit 1;
      else
        shift
      fi
      ;;
  esac
done

#
# test machine type
#
for mode in ${MODE}
do
  if [ "${mode}" = "TEST" ]; then
    testmachine
    testpythonmodules
  fi
done

#
# update scripts
#
for mode in ${MODE}
do
  if [ "${mode}" = "UPDATE" ] || [ "${mode}" = "ALL" ]; then

    # first test the machine type if necessary
    if [ "${MACHINE}" = "" ]; then
      testmachine
    fi

    # create machine dependent subdirectory in the scripts directory
    if [ ! -d ${PD}/scripts/${MACHINE} ]; then
      mkdir ${PD}/scripts/${MACHINE}
    fi

    # create setup script for pipeline environment if necessary:
    if [ ! -f ${PD}/pipesetup/pipe_env.bash.${MACHINE} ]; then
      environment
    fi

    # now the progs.ini file
    sed -e 's!NPARA=1!NPARA='${NCPUS}'!' \
        -e 's!PIPESOFT=!PIPESOFT='${PD}'!' \
        -e 's!BIN=${PIPESOFT}/bin/!BIN=${PIPESOFT}/bin/'${MACHINE}'/!' \
        -e 's!UTILSCRIPTS=${PIPESOFT}/scripts/utilities!UTILSCRIPTS=${PIPESOFT}/scripts/'${MACHINE}'/!' \
        -e 's!P_PYTHON3=!P_PYTHON3='${P_PYTHON3}'!' \
        -e 's!P_PERL=!P_PERL='${P_PERL}'!' \
        -e 's!P_GAWK=!P_GAWK='${P_GAWK}'!' \
        -e 's!P_FIND=!P_FIND='${P_FIND}'!' \
        -e 's!P_SORT=!P_SORT='${P_SORT}'!' \
        -e 's!P_SM=!P_SM='${P_SM}'!' \
        -e 's!P_SCAMP=!P_SCAMP='${P_SCAMP}'!' \
        -e 's!P_ACLIENT=!P_ACLIENT='${P_ACLIENT}'!' \
        -e 's!S_ASTROMETRIX=!S_ASTROMETRIX='${PD}'/scripts/astrometrix/astrom_theli!' \
        -e 's!S_PHOTOMETRIX=!S_PHOTOMETRIX='${PD}'/scripts/astrometrix/photom_theli!' \
        ${PD}/scripts/reduction/progs.ini.raw \
        > ${PD}/scripts/${MACHINE}/progs.ini

    # include pipeline version for the start logging message in
    # bash_functions.include:
    sed -e 's!__PIPEVERS__!'"${PIPEVERS}"'!' \
       ${PD}/scripts/reduction/bash_functions.include.raw > \
       ${PD}/scripts/${MACHINE}/bash_functions.include

    # now update all the scripts
    SCRIPTDIRS="${PD}/scripts/reduction ${PD}/scripts/WFI_reduction
                ${PD}/scripts/reduction ${PD}/scripts/CFH12K_reduction
                ${PD}/scripts/MEGAPRIME_reduction
                ${PD}/scripts/parallel  ${PD}/scripts/utilities"

    for SCRIPTDIR in ${SCRIPTDIRS}
    do
      updatescripts ${SCRIPTDIR}

      # just copy possible Python scripts (they are called via the env
      # command and hence we do not need to hardcode the location of
      # the Python executable):
      PERLPYFILES=`find ${SCRIPTDIR} \( -name \*.pl -o -name \*.py \)`

      for perlpyfile in ${PERLPYFILES}
      do
        # put pipeline version number into code
	BASE=`basename ${perlpyfile}`
        sed -e 's!__PIPEVERS__!'"${PIPEVERS}"'!' \
          ${perlpyfile} > ${PD}/scripts/${MACHINE}/${BASE}
        chmod a+x ${PD}/scripts/${MACHINE}/${BASE}
      done

      # copy include files
      INCLUDE=`find ${SCRIPTDIR} -name \*.include`

      for include in ${INCLUDE}
      do
        cp ${include} ${PD}/scripts/${MACHINE}
      done
    done

    # copy files from the instrument specific directories to the
    # machine directories
    cp ${PD}/scripts/WFI_reduction/WFI* ${PD}/scripts/${MACHINE}
    cp ${PD}/scripts/WFI_reduction/conv_WFI* ${PD}/scripts/${MACHINE}
    cp ${PD}/scripts/CFH12K_reduction/CFH12K* ${PD}/scripts/${MACHINE}
    cp ${PD}/scripts/CFH12K_reduction/cfh12k* ${PD}/scripts/${MACHINE}
    cp ${PD}/conf/WFI_reduction/WFI* ${PD}/scripts/${MACHINE}
    cp ${PD}/scripts/MEGAPRIME_reduction/MEGAPRIME* ${PD}/scripts/${MACHINE}
    cp ${PD}/conf/MEGAPRIME_reduction/*MEGAPRIME* \
       ${PD}/conf/MEGAPRIME_reduction/megacam.ret ${PD}/scripts/${MACHINE}

    chmod a+x ${PD}/pipesetup/test_dir
    chmod a+x ${PD}/pipesetup/vraag

    # now deal with the perl scripts from ASTROMETRIX:
    sed -e "s!GaBoDSVersion=!GaBoDSVersion=\"${PIPEVERS}\";!" \
      ${PD}/scripts/astrometrix/src/photom_theli.raw >\
      ${PD}/scripts/astrometrix/photom_theli
    chmod +x ${PD}/scripts/astrometrix/photom_theli
    sed -e "s!GaBoDSVersion=!GaBoDSVersion=\"${PIPEVERS}\";!" \
      ${PD}/scripts/astrometrix/src/astrom_theli.raw >\
      ${PD}/scripts/astrometrix/astrom_theli
    chmod +x ${PD}/scripts/astrometrix/astrom_theli
    sed -e "s!GaBoDSVersion=!GaBoDSVersion=\"${PIPEVERS}\";!" \
      ${PD}/scripts/astrometrix/src/nldacsub_theli.pm.raw >\
      ${PD}/scripts/astrometrix/nldacsub_theli.pm
    cp ${PD}/scripts/astrometrix/src/AstSubs_theli.pm.raw \
       ${PD}/scripts/astrometrix/AstSubs_theli.pm
    cp ${PD}/scripts/astrometrix/src/AstConf_theli.pm.raw \
       ${PD}/scripts/astrometrix/AstConf_theli.pm
    cp ${PD}/scripts/astrometrix/src/catsubs_theli.pm.raw \
       ${PD}/scripts/astrometrix/catsubs_theli.pm
    cp ${PD}/scripts/astrometrix/src/PhotConf_theli.pm.raw \
       ${PD}/scripts/astrometrix/PhotConf_theli.pm
  fi
done

# compile sources

for mode in ${MODE}
do
  if [ "${mode}" = "COMPILE" ] || [ "${mode}" = "ALL" ]; then
    # do a test first whether the machine type is already known
    if [ "${MACHINE}" = "" ]; then
      testmachine
    fi

    # create setup script for pipeline environment if necessary:
    if [ ! -f ${PD}/pipesetup/pipe_env.bash.${MACHINE} ]; then
      environment
    fi

    # set environment variables necessary for compilation
    . ${PD}/pipesetup/pipe_env.bash.${MACHINE}

    # write the correct global_vers.h file:
    sed -e "s!#define __PIPEVERS__!#define __PIPEVERS__ \"${PIPEVERS}\"!" \
	  ${PD}/pipesetup/global_vers.h.raw >\
	  ${PD}/ldactools/library/global_vers.h

    cd ${PD}/pipesetup/
    make all
    cd ${PD}/pipesetup/
  fi
done
