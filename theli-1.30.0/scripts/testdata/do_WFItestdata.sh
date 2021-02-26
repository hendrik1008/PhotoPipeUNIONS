#!/bin/bash

# This script illustrates the basic usage of the THELI pipeline.  The
# commands below process a WFI test data set from raw image to a final
# co-added image.
# At the end of this script we list the individual images which are
# part of this test data set. We give also a very short introduction
# into THELI there.

# This script also serves as an example for possible 'superscripts'
# collecting various pipeline tasks.

# Note that the script (as most of the individual THELI subtasks)
# does NOT react intelligently on errors during image reduction!!!!

#
# Edit the following two lines to point to the MD of your data and
# your reduction directory (The directory where all your reduction
# scripts are located and where you will execute individual processing
# steps):

. ./progs.ini

export MD=/vol/aibn234/data1/thomas/thomas.aibn202_2/THELI_WFI_TESTDATA
export REDDIR=/vol/aibn234/data1/thomas/thomas.aibn202_2/reduce
export PHOTCAT=/vol/aibn234/data1/thomas/thomas.aibn202_2/reduce/Stetson_std_2.0.cat

# setup THELI logging:
: ${THELI_LOGGING:="Y"}
: ${THELI_LOGLEVEL="2"} # Log level 2 means 'extensive output logging from
                        # all the scripts
export THELI_LOGGING
export THELI_LOGLEVEL

#
# THELI uses the INSTRUMENT environment variable to
# identify the instrument it is working on. Configuration
# files named ${INSTRUMENT}.ini are usually loaded by
# processing scripts to load necessary, instrument specific
# information:
export INSTRUMENT=WFI
#
# The following is just used in this script to identify
# data directories.
export FILTER=R

# Do astrometric calibration with SCAMP or ASTROMETRIX?
# We exclusively use SCMAP now!
ASTROMMETHOD=SCAMP

ASTROMADD=""
if [ "${ASTROMMETHOD}" = "SCAMP" ]
then
  ASTROMADD="_scamp_2MASS"
fi

#
# Here, RUN processing is performed:
# ==================================
#

# We log run and set processing to different directories:
export THELI_LOGDIR=`pwd`/logs_TESTDATA_RUN

# R1: Split the MEF data into individual chips
# ============================================
#
# split data and update headers. This steps takes data
# from ${MD}/BIAS/ORIGINALS (accordingly for other species),
# splits the MEF files into single chips, updates the
# headers to the THELI format and puts resulting files
# to  ${MD}/BIAS etc.
./process_split_WFI_eclipse.sh -md ${MD} -sd BIAS
./process_split_WFI_eclipse.sh -md ${MD} -sd DARK
./process_split_WFI_eclipse.sh -md ${MD} -sd SKYFLAT_${FILTER}
./process_split_WFI_eclipse.sh -md ${MD} -sd SCIENCE_${FILTER}
./process_split_WFI_eclipse.sh -md ${MD} -sd STANDARD_${FILTER}
#
# Here you could do a first data check and verify that the modes of
# raw images are within predefined values. A typical script call to
# only accept for further processing only skyflats with a mode
# between 5000 and 40000 would look like:
# ./check_files.sh ${MD}/ SKYFLAT_${FILTER} 5000 40000
#
# we omit this step as the data were chosen to be good !!

#
# R2: process BIAS frames
# =======================
#
# This step processed BIAS images (overscan correction and
# stacking). Final results can be found in ${MD}/BIAS and
# the produced final BIAS frames are called BIAS_i.fits (i being
# chip number)
./parallel_manager.sh ./process_bias_eclipse_para.sh ${MD}/ BIAS
#
# R3: process DARK frames
# =======================
#
# The same as the previous step for DARK frames
./parallel_manager.sh ./process_bias_eclipse_para.sh ${MD}/ DARK
#
# R4: process FLAT frames
# ======================
#
# This step processes FLAT frames (overscan correction, BIAS subtraction,
# stacking). Final results can be found in ${MD}/SKYFLAT_R and the produced
# final SKYFLAT frames are called SKYFLAT_${FILTER}_i.fits
./parallel_manager.sh ./process_flat_eclipse_para.sh ${MD}/ BIAS \
                      SKYFLAT_${FILTER}
#
# R5: process SCIENCE frames
# ==========================
#
# (overscan correction, bias subtraction,
# flatfielding, superflatfielding, defringing)
./parallel_manager.sh ./process_science_eclipse_para.sh ${MD}/ BIAS \
                      SKYFLAT_${FILTER} \
                      SCIENCE_${FILTER} NORESCALE
./parallel_manager.sh ./process_superflat_eclipse_para.sh ${MD}/ \
                      SCIENCE_${FILTER} OFC FRINGE NOSUPERTEST
./parallel_manager.sh ./create_illumfringe_para.sh ${MD}/ SCIENCE_${FILTER}
./parallel_manager.sh ./process_science_illum_fringe_eclipse_para.sh ${MD}/ \
                      SCIENCE_${FILTER} OFC RESCALE
#
# R6: create postage stamps of SCIENCE images
# ===========================================
#
# we create 8x8 binned mosaic images showing the prereduced images
# The result goes to ${MD}/SCIENCE_${FILTER}/BINNED
./create_binnedmosaics_exposurepara.sh ${MD} SCIENCE_${FILTER} WFI OFCSF 8 -32

#
# you should take a look at the created mosaics at this stage. Usually
# you would use them to identify whether preprocessing steps should
# be iterated or whether masks should be created (for instance to mark
# satellite tracks). For the example set the only necessary masks
# are WFI.2000-12-26T07:30:37.238_[5-8].reg. Make sure they are in the
# ${MD}/SCIENCE_${FILTER}/reg directory before proceeding.
#
# R7: create weight frames
# ========================
#
# first rescale SKYFLATs and SUPER FLATS to a mode of '1'.
# Outputs are 8 files in ${MD}/ SKYFLAT_R_norm  and
# ${MD}/SCIENCE_R_norm/:
./parallel_manager.sh ./create_norm_para.sh ${MD} SKYFLAT_${FILTER}
./parallel_manager.sh ./create_norm_para.sh ${MD} SCIENCE_${FILTER}

# produce global weights and flags:
# outputs: 8 globalflags and globalweights in ${MD}/WEIGHTS
./parallel_manager.sh ./create_global_weights_flags_para.sh ${MD} NOLINK\
                      SKYFLAT_${FILTER}_norm 0.7 1.3 DARK -9 9 \
                      SCIENCE_${FILTER}_norm 0.9 1.1

# finally create weight images for all individual images:
./parallel_manager.sh ./create_weights_flags_para.sh ${MD} \
                      SCIENCE_${FILTER} OFCSF WEIGHTS HIGHSN

#
# R8: Photometric calibration
# ===========================
#
# first process standardstar frames in exactly the same way as the
# science observations.
./parallel_manager.sh ./process_science_eclipse_para.sh ${MD} BIAS \
                        SKYFLAT_${FILTER} STANDARD_${FILTER} NORESCALE
./parallel_manager.sh ./create_illumfringe_para.sh ${MD} STANDARD_${FILTER} \
                        SCIENCE_${FILTER}
./parallel_manager.sh ./process_science_illum_fringe_eclipse_para.sh ${MD} \
                        STANDARD_${FILTER} OFC RESCALE

# R8.1: create weights and flags for the standard star fields:
# ------------------------------------------------------------
./parallel_manager.sh ./create_weights_flags_para.sh ${MD} \
                      STANDARD_${FILTER} OFCSF WEIGHTS_FLAGS HIGHSN

# R8.2 source extraction from the standardstar fields
# ---------------------------------------------------
./parallel_manager.sh ./create_astromcats_weights_para.sh \
                      ${MD} STANDARD_${FILTER} OFCSF cat \
                      WEIGHTS OFCSF.weight OFCSF.flag 5

# R8.3 astrometric calibration for the standard star fields
# ---------------------------------------------------------
#
# we use the 2MASS catalogue because it is the most accurate
# all-sky catalogue.
# Note that we depend on SCAMP as astrometric tool for this step!
if [ "${ASTROMMETHOD}" = "SCAMP" ]; then
  ./create_scamp_astrom_photom_multiinst.sh \
         -i WFI -d ${MD} STANDARD_${FILTER} OFCSF -p 2.0 \
         -k SHORT -c 2MASS

  # merge the extracted sources with those from a standard star catalogue:
  ./parallel_manager.sh ./create_stdphotom_prepare_para.sh ${MD} \
                        STANDARD_${FILTER} OFCSF 2MASS

  ./create_stdphotom_merge_exposurepara.sh ${MD} STANDARD_${FILTER} \
         OFCSF ${PHOTCAT}

  # determine an absolute photometric solution and update headers
  # of science frames:
  #
  # change AUTOMATIC to INTERACTIVE if you want to select
  # the photometric solution interactively:
  ./create_abs_photo_info.sh ${MD} STANDARD_${FILTER} SCIENCE_${FILTER} \
         OFCSF R R VmR -0.07 0.0 NIGHTCALIB AUTOMATIC 24.4 24.55 1
else
  echo "Absolute photometric calibration requires scamp as"
  echo "astrometry tool."
fi

# include the following calls if you checked your results up to now
# and if you want to clean up disk space
## ./cleanfiles.sh ${MD}/BIAS WFI "."
## ./cleanfiles.sh ${MD}/BIAS WFI "OC."
## ./cleanfiles.sh ${MD}/DARK WFI "."
## ./cleanfiles.sh ${MD}/DARK WFI "OC."
## ./cleanfiles.sh ${MD}/SKYFLAT_${FILTER} WFI "."
## ./cleanfiles.sh ${MD}/SKYFLAT_${FILTER} WFI "OC."
## ./cleanfiles.sh ${MD}/SCIENCE_${FILTER}/SPLIT_IMAGES WFI "."
## rmdir ${MD}/SCIENCE_${FILTER}/SPLIT_IMAGES
## ./cleanfiles.sh ${MD}/SCIENCE_${FILTER}/OFC_IMAGES WFI "OFC."
## rmdir ${MD}/SCIENCE_${FILTER}/OFC_IMAGES
## ./clesnfiles.sh ${MD}/SCIENCE_${FILTER}/SUB_IMAGES WFI "OFC_sub."
## rmdir ${MD}/SCIENCE_${FILTER}/SUB_IMAGES
## ./cleanfiles.sh ${MD}/STANDARD_${FILTER}/SPLIT_IMAGES WFI "."
## rmdir ${MD}/STANDARD_${FILTER}/SPLIT_IMAGES
## ./cleanfiles.sh ${MD}/STANDARD_${FILTER}/OFC_IMAGES WFI "OFC."
## rmdir ${MD}/STANDARD_${FILTER}/OFC_IMAGES
##

# R9: RUN -> SET data structure
# =============================
#
# set new logging directory for set processing:
export THELI_LOGDIR=`pwd`/logs_TESTDATA_SET

# sort the SCIENCE frames in different sets; here only a single set is present
# and all images are moved to ${MD}/set_1. The distribute_sets script
# pts the set directories below the same main directory as the RUNS.
./distribute_sets.sh ${MD} SCIENCE_${FILTER} OFCSF 1000

#
# Here, SET processing is performed:
# ==================================
#

# S1: Perform astrometric calibration of science data
# ===================================================


# S1.1: create catalogues for astrometric and photometric calibration
# -------------------------------------------------------------------
./parallel_manager.sh ./create_astromcats_weights_para.sh ${MD} set_1 \
                        OFCSF cat WEIGHTS OFCSF.weight NONE 5

# S1.2: perform astrometric calibration
# -------------------------------------
if [ ${ASTROMMETHOD} != "SCAMP" ]
then
  #
  # run ASTROMETRIX and PHOTOMETRIX
  ./create_astrometrix_astrom.sh ${MD} set_1 OFCSF
  ./create_photometrix.sh ${MD} set_1 OFCSF
else
  # alternatively to the previous two scripts you
  # can perform astrometry/relative photometry with scamp.
  # In this case the following script call is necessary:
  ./create_scamp_astrom_photom_multiinst.sh \
          -i WFI -d ${MD} set_1 OFCSF -p 2.0 -k LONG -c 2MASS
fi

# S2: estimate some statistics on images:
# =======================================
./create_stats_table.sh ${MD} set_1 OFCSF headers${ASTROMADD}

# S3: estimate final photometric zeropoint for a co-added stack
# (This is based on quantities from the previous statistics step)
# ===============================================================
./create_absphotom_photometrix.sh ${MD} set_1

#
# S4: create disgnostic plots with SuperMongo (SM)
# ================================================
./make_checkplot_stats.sh ${MD} set_1 chips.cat5 STATS set_1

# S5: subtract the sky from science images
# ========================================
./parallel_manager.sh ./create_skysub_para.sh \
                      ${MD} set_1 OFCSF ".sub" TWOPASS

# S6: Image Co-addition
# =====================
#
# perform final image co-addition; if you want to use the experimental
# header update feature after co-addition (see below) you ALWAYS
# have to provide a coaddition condition (-l option) to the following
# script.

# S6.1: some preparation, file creation for co-addition
# -----------------------------------------------------
./prepare_coadd_swarp.sh -m ${MD} -s set_1 -e "OFCSF.sub" -n TEST \
                         -w ".sub" -eh headers${ASTROMADD} \
                         -r 170.661597 -d -21.70969 \
                         -l ${MD}/set_1/cat/chips.cat5 STATS\
                            "(SEEING<2.0);" \
                            ${MD}/set_1/cat/TEST.cat

# S6.2: Resample single images
# ----------------------------
./parallel_manager.sh ./resample_coadd_swarp_para.sh ${MD} set_1 \
                      "OFCSF.sub" TEST ${REDDIR}

# we noticed that resampled images have a slight bias in the
# overall sky-background value. We again estimate a background
# level for the whole image and subtract it:
./parallel_manager.sh ./create_modesub_para.sh ${MD}/set_1 \
                      coadd_TEST ? \
                      OFCSF.sub.TEST.resamp.

# S6.3:  Final image co-addition
# ------------------------------
./perform_coadd_swarp.sh ${MD} set_1 TEST OFCSF.sub.TEST.resamp. \
                     "" ${REDDIR} MAP_WEIGHT FLAG WEIGHTED

#
# HERE THE FORMAL RESPONSIBILITY OF THE PIPELINE ENDS
# THE STEPS DESCRIBED IN THE FOLLOWING ARE EXPERIMENTAL
# AND NOT YET CLEANLY IMPLEMENTED IN THE PIPELINE FLOW.

#
# update the header of the co-added image with information on
# total exposure time, effective gain, magnitude zeropoint information

#
# first get the magnitude zeropoint for the co-added image:
if [ -f ${MD}/set_1/cat/chips_phot.cat5 ]; then
  MAGZP=$(${P_LDACTOASC} -i ${MD}/set_1/cat/chips_phot.cat5 -t ABSPHOTOM \
                   -k COADDZP | awk 'END {print $0}')
else
  MAGZP=-1.0
fi
#
# now do the actual header update
./update_coadd_header.sh ${MD} set_1 TEST STATS coadd ${MAGZP} Vega \
                         "(SEEING<2.0);" 2MASS WFI "MPG/ESO-2.2"
##
##
## END OF PROCESSING SCRIPT
##


# Appendix:
#
# 1. (Very) brief introduction to THELI
# =====================================
#
# For details on reduction algorithms and an overview on THELI
# internals see Erben et al. (AN....)
#
# THELI data-processing, which aims at the production of co-added images from
# dithered observations of different targets happens in two major steps: (1)
# The RUN processing which treats all images of a given observing run for
# removing the instrumental signature; i.e. a RUN represnts a period for which
# the same calibration (BIAS, FLAT, FRINGE correction etc.)  can be applied to
# science images. Obviously observations in different passbands represent
# different RUNS. (2) The SET processing. After RUN processing images are
# rearranged into set which represnt the different targets (possibly in
# different filters) which should result as co-added stacks from THELI
# processing. On SETS the operations astrometric calibration, 'final'
# photometric calibration and co-addition are performed.  Note that a target
# can have been observed in different RUNS!

# THELI expects the data within fixed directory structures for RUNS and SETS.
# The directory structures head is often called 'MAIN DIRECTORY' (MD) for RUN
# processing and 'SET DIRECTORY' (SD) for work on SETs. Often the same head is
# used for both structures (as is the case for this script). Unter the head
# directories for each species of files (BIAS, DARK FLATs, SCIENCE exposures,
# STANDARD stars are created and the original images put therin (see below)
#
# During processing THELI usually creates new images, subdirectories and
# files therin. Unfortunately documentation of what is created is very poor
# at this stage!
#
# I advise you to put the data in the directory structure below and to run
# the above scripts step by step to see what is happening. I tried to document
# above what each script produces so that you can check step by step whether
# necessary files have been produced (very preliminary and uncomplete).

# 2. The WFI test data set
# ========================
#
# The test data set being processed by this script consists of R-band images
# from the ESO Imaging Survey Deep Field Deep3a (see
# http://www.eso.org/eis). They should be sorted in a starting directory
# structure as indicated below.
#
# NOTE: If you get original images with the ending '.fz' they are compressed
# with the so-called Rice algorithm, an optimised compression technique for
# FITS images. To uncompress you can use a tool called 'imcopy' which is based
# on the cfitsio library
# (see http://heasarc.gsfc.nasa.gov/docs/software/fitsio/cexamples.html)
#
# The WFI test data set contains in total 61 files:

# BIAS images
#
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-25T20:30:58.176.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-25T20:32:10.666.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-25T20:33:22.205.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-25T20:35:53.923.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-27T09:43:53.731.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-27T09:46:22.080.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-27T09:47:36.384.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-25T20:34:36.250.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-27T09:42:41.414.fits.fz
# ${MD}/BIAS/ORIGINALS/WFI.2000-12-27T09:45:10.195.fits.fz

# DARK images
#
# ${MD}/DARK/ORIGINALS/WFI.2000-12-25T21:50:22.445.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-25T22:22:47.914.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T11:13:59.606.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T21:20:00.442.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T21:37:51.888.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T22:13:38.669.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T22:49:21.043.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-25T22:06:35.827.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T10:57:45.965.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T11:30:12.038.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T21:55:46.099.fits.fz
# ${MD}/DARK/ORIGINALS/WFI.2000-12-27T22:31:29.510.fits.fz

# SCIENCE images
#
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-26T07:44:06.202.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-27T07:36:55.930.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-27T07:43:46.502.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-27T07:57:29.376.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-26T06:28:27.782.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-26T07:17:06.374.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-26T07:23:55.824.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-26T07:30:37.238.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-26T07:37:23.578.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-27T07:30:06.048.fits.fz
# ${MD}/SCIENCE_R/ORIGINALS/WFI.2000-12-27T07:50:38.198.fits.fz

# SKYFLAT images
#
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-24T00:25:37.574.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-24T09:15:43.920.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T00:12:56.045.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T00:16:27.725.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T00:18:27.389.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T09:01:19.834.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T09:07:10.618.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-24T09:14:08.448.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-24T09:17:15.245.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T00:14:31.085.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T08:55:00.710.fits.fz
# ${MD}/SKYFLAT_R/ORIGINALS/WFI.2000-12-25T09:04:38.467.fits.fz

# STANDARD stars (NOT USED FOR PHOTOMETRIC CALIBRATION
# WITHIN THIS SCRIPT)
#
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-26T01:02:40.733.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-26T05:42:32.486.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-26T05:44:02.602.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-26T06:08:16.973.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T01:04:03.245.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T06:04:06.067.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T07:00:51.955.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T08:10:54.192.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T08:44:44.419.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T08:46:26.285.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-26T01:04:13.958.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-26T06:06:46.253.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T01:02:33.821.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T06:05:44.131.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T07:02:30.538.fits.fz
# ${MD}/STANDARD_R/ORIGINALS/WFI.2000-12-27T08:12:30.355.fits.fz

# 3. Produced files by pipeline calls:
# ====================================
#
# in ${MD}/BIAS/
#
# final, co-added bias images:
# BIAS_1.fits
# BIAS_2.fits
# .
# BIAS_8.fits
#
# splitted, individual original bias images
#
# WFI.2000-12-25T20:30:58.176_1.fits
# WFI.2000-12-25T20:30:58.176_2.fits
# .
# overscan corrected and cutted individual
# bias images:
# WFI.2000-12-25T20:30:58.176_1OC.fits
# WFI.2000-12-25T20:30:58.176_2OC.fits
# .
############################################
# in ${MD}/DARK
# final, co-added dark images:
# DARK_1.fits
# DARK_2.fits
# .
# splitted, individual original bias images
#
# WFI.2000-12-25T21:50:22.445_1.fits
# WFI.2000-12-25T21:50:22.445_2.fits
# .
# WFI.2000-12-25T21:50:22.445_1OC.fits
# WFI.2000-12-25T21:50:22.445_2OC.fits
# .
############################################
# in ${MD}/SKYFLAT_R
# final, co-added skyflat images:
# SKYFLAT_1.fits
# SKYFLAT_2.fits
# .
# splitted, individual original skyflat images
#
# WFI.2000-12-24T00:25:37.574_1.fits
# WFI.2000-12-24T00:25:37.574_2.fits
# .
# overscan corrected and cutted individual
# skyflat images:
# WFI.2000-12-24T00:25:37.574_1OC.fits
# WFI.2000-12-24T00:25:37.574_2OC.fits
# .
############################################
# in ${MD}/SCIENCE_R
#
# final overscan corrected, bias subtracted, sky-
# flatfielded, superflat-fielded and fringe corrected
# individual science images:
#
# WFI.2000-12-26T06:28:27.782_1OFCSF.fits
# WFI.2000-12-26T06:28:27.782_2OFCSF.fits
# .
# median co-added science images - basis for
# illumination and fringe-frame correction:
# SCIENCE_R_1.fits
# SCIENCE_R_2.fits
# .
#
# large-scale illumination pattern data:
# in subdirectory SPLIT_IMAGES
# SCIENCE_R_1_illum.fits
# SCIENCE_R_2_illum.fits
# .
#
# Fringe-pattern data:
# SCIENCE_R_1_fringe.fits
# SCIENCE_R_2_fringe.fits
# .
#
# splitted, individual original science images
# WFI.2000-12-26T06:28:27.782_1.fits
# WFI.2000-12-26T06:28:27.782_2.fits
# .
# in subdirectory SUB_IMAGES
# object subtracted images which are stacked to form
# the superflats:
# WFI.2000-12-26T06:28:27.782_1OFC_sub.fits
# WFI.2000-12-26T06:28:27.782_2OFC_sub.fits
# .
######### in subdirectory ${MD}/SCIENCE_R/BINNED/
# binned mosaics for visual inspection:
# WFI.2000-12-26T06:28:27.782_mos.fits
# .
#
###########################################
# in ${MD}/STANDARD_R
#
# overscan corrected, bias subtracted, sky-
# flatfielded, superflat-fielded and fringe corrected
# individual standardstar images:
#
# WFI.2000-12-26T01:02:40.733_1OFCSF.fits
# WFI.2000-12-26T01:02:40.733_2OFCSF.fits
# .
