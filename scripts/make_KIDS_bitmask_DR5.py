#!/usr/bin/env python

import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as P

import subprocess as S
import numpy as np
from astropy.io import fits
from astropy import wcs
import sys
import os
import warnings
import pylab
import shutil
import bashreader as bash

from string import digits, ascii_uppercase, ascii_lowercase
from random import sample

def rand_char_string(length=6):
    chars = ascii_lowercase + ascii_uppercase + digits
    s = ''.join(sample(chars, length))
    return s
rand_str = rand_char_string()


"""
make_KIDS_bitmask.py
  for a given KIDS coadd, convert various region files to FITS flag ("bitmask") files


MASK value design:
=================

KiDS:
- AUTOMASK dir = /vol/braid1/vol3/thomas/KIDSCOLLAB_V0.5.5/KIDS_xxxx_xxxx/r_SDSS/masks_V0.5.5A
- MANUALMASK dir = ???  move everything to AUTOMASK dir, if possible
- SATURATION PIX: 
- 
- r-band auto stellar mask: use UCAC4 re-run  [KIDS_131p0_0p5_r_SDSS_stars.reg]
- r-band manual mask from Reiko (magenta)
- r-band asteroid mask (red)  [KIDS_xxxx_xxxx_r_SDSS_asteroids.reg]
- r-band saturated mask ((bit mask))
- ugri-band void masks  [KIDS_131p0_0p5_r_SDSS_voids.reg]
- ugri-band flag images (**already in FITS)

KIDS MASK BITS  [CURRENT_VERSION]
     1(0): manual_mask_aggressive (THELI det.band)
     2(1): star_halo_large_faint
           (THELI det.band; cyan, 10.5<m_r<11.5 of UCAC4 and GSC1 stellar catalog)
     4(2): star_halo_large (THELI det.band; magenta, m_r<10.5 of UCAC4 and GSC1) 
           + stellar_mask (THELI det.band; green, m_r<14.0 of UCAC4 and GSC1)
     8(3): manual_mask (THELI det.band; magenta)
    16(4): weight==0 mask (THELI det.band; masking due to saturation, chip gap, etc)
           + void mask (THELI det.band; green) and asteroids (THELI det.band; red)
    32(5): (Reserved/Unused)
    64(6): AW manual mask (AW u mask regions)
   128(7): AW manual mask (AW g mask regions)
   256(8): AW manual mask (AW r mask regions)
   512(9): AW manual mask (AW i mask regions)
 1024(10): AW halo+stellar mask (AW u mask regions)
           + weight==0 mask (u masking due to saturation, chip gap, etc)
 2048(11): AW halo+stellar mask (AW g mask regions)
           + weight==0 mask (g masking due to saturation, chip gap, etc)
 4096(12): AW halo+stellar mask (AW u mask regions)
           + weight==0 mask (r masking due to saturation, chip gap, etc)
 8192(13): AW halo+stellar mask (AW i mask regions)
           + weight==0 mask (i masking due to saturation, chip gap, etc)
16384(14): outside the WCS RA/DEC cut (trimming)
32768(15): (Reserved)  [mask bits FITS in signed 2-byte integer; #15 is reserved for negative sign]


Procedure:
 1) split star auto-mask into regular and conservative (different bits are set) => ds9 region file
 2) [manual mask bits are left untouched; include in (A) if available]
 3) combine void mask with weight mask from coadd directory => FITS file
    ... combine the weight=0 FITS file with the void and stellar region files (A)
 4) swarp each AW mask into THELI frame (requires astrometry header from both) => FITS files
    ... then combine into single FITS flag file (B)
 5) Combine the two FITS files (A+B) in an appropriate manner

"""

#
# Debug flags
#
remove_temp_files = True     # remove intermediate files?
run_aw_swarp = True          # run SWARP (to match AW to THELI coordinates)
run_aw_ww = True             # run weight watcher (to combine AW masks) (B)
run_make_wcs_mask = True     # generate wcs mask
run_theli_combo_ww = True    # run weight watcher (to combine THELI weight and reg files) (A)
run_ic = True                # run ic (to combine existing bitmasks) and apply WCS cut
aw_mask_exists = True

#
# flag bit info (information to be included in the final flagfile FITS header)
#
final_BITPIX = 16
#flagbit_dict = {'F_TH_MAN_CON' : (0x0001,'Theli flag (det.band): manual mask, conservative'),
#                'F_TH_STAR_CON': (0x0002,'Theli flag (det.band): starhalo mask, conservative'),
#                'F_TH_STAR'    : (0x0004,'Theli flag (det.band): stellar and starhalo mask'),
#                'F_TH_MANUAL'  : (0x0008,'Theli flag (det.band): manual mask'),
#                'F_TH_VOID'    : (0x0010,'Theli flag (det.band): void and weight==0 mask'),
#                'F_RESERVED_1' : (0x0020,'Reserved, unused'),
#                'F_AW_U_MAN1'  : (0x0040,'AW manual masks (u)'),
#                'F_AW_G_MAN1'  : (0x0080,'AW manual masks (g)'),
#                'F_AW_R_MAN1'  : (0x0100,'AW manual masks (r)'),
#                'F_AW_I_MAN1'  : (0x0200,'AW manual masks (i)'),
#                'F_AW_U_AUTO'  : (0x0400,'AW flag (u): auto masks'),
#                'F_AW_G_AUTO'  : (0x0800,'AW flag (g): auto masks'),
#                'F_AW_R_AUTO'  : (0x1000,'AW flag (r): auto masks'),
#                'F_AW_I_AUTO'  : (0x2000,'AW flag (i): auto masks'),
#                'F_KIDS_WCS'   : (0x4000,'KIDS WCS tiling cuts'),
#                'F_RESERVED_2' : (0x8000,'Reserved, unused'),
#                }

flagbit_dict = {'F_TH_STAR_CON': (0x0001,'Theli flag (det.band): starhalo mask, conservative'),
                'F_TH_STAR'    : (0x0002,'Theli flag (det.band): stellar and starhalo mask'),
                'F_TH_MANUAL'  : (0x0004,'Theli flag (det.band): manual mask'),
                'F_TH_VOID'    : (0x0008,'Theli flag (det.band): void and weight==0 mask'),
                'F_AW_U_AUTO'  : (0x0010,'AW flag (u): auto masks'),
                'F_AW_G_AUTO'  : (0x0020,'AW flag (g): auto masks'),
                'F_AW_R_AUTO'  : (0x0040,'AW flag (r): auto masks'),
                'F_AW_I_AUTO'  : (0x0080,'AW flag (i): auto masks'),
                'F_AW_I2_AUTO' : (0x0100,'AW flag (i2): auto masks'),
                'F_VISTA_Z'    : (0x0200,'VISTA footprint mask (Z)'),
                'F_VISTA_Y'    : (0x0400,'VISTA footprint mask (Y)'),
                'F_VISTA_J'    : (0x0800,'VISTA footprint mask (J)'),
                'F_VISTA_H'    : (0x1000,'VISTA footprint mask (H)'),
                'F_VISTA_Ks'   : (0x2000,'VISTA footprint mask (Ks)'),
                'F_KIDS_WCS'   : (0x4000,'KIDS WCS tiling cuts'),
                'F_RESERVED_2' : (0x8000,'Reserved, unused'),
                }

#
# main code
#
try:

    #
    # System specific directories and files
    #

    ## data specific info
    field = sys.argv[1]           # $FIELD
    Ver = sys.argv[2]             # $COADDIDENT
    filters = sys.argv[3].split() # $FILTERS   # Note: two filters (r/i) have the same center

    ## temporary directory
    temp_dir = sys.argv[4]        # $TEMPDIR

    ## theli weight and mask dir
    base_dir = sys.argv[5]        # $MD      : base of data directory structure
    mask_dir = sys.argv[6]        # $MASKDIR : mask dir of detection band, contains AW masks as well

    ## WCS cuts for each field
    kids_wcscut_data = sys.argv[7] # $WCSCUTPATH
    if not os.path.isfile(kids_wcscut_data):
        print 'ERROR: WCS cut info file', kids_wcscut_data, 'not found, exiting'
        sys.exit(9)
    KIDS_wcscut_dict = dict()
    KIDS_field_list = []
    with open(kids_wcscut_data) as infile:
        for line in infile:
            (field_name, ra_min, ra_max, dec_min, dec_max) = line.split()
            KIDS_wcscut_dict[field_name] = (float(ra_min), float(ra_max), float(dec_min), float(dec_max))
            KIDS_field_list += [field_name,]
    try:
        (ra_min, ra_max, dec_min, dec_max) = KIDS_wcscut_dict[field]
    except KeyError:
        print 'ERROR: WCS cut info file', kids_wcscut_data, 'does not contain the field', field
        print 'exiting'
        sys.exit(9)

    ## final output base filename (will be contained under $MASKDIR)
    bitmask_fname = sys.argv[8]   # $FLAGFITS


    #
    # working parameters
    #

    ## General Info
    detection_band = filters[0]
    theli_long_filter_list = filters
    theli_filter_list = [filt[0] for filt in theli_long_filter_list]  # i.e., ['r', 'i',]

    aw_filter_list = ['u','g','r','i', 'i2']  # from AstroWise.  Filter order = bitmask orders for each filter

    ## various version numbers to keep track of
    # TODO: update AW mask versions (w/ spell correction), then update masking script
    unknown_str = 'unknown'
    aw_star_mask_version_tag = 'AW_STARMASK_VER'
    aw_star_mask_software_name = 'Pullecenella' # misspelled in AW header (should be Pulecenella)
    aw_star_mask_version_str = 'AstroWise %s mask version' % (aw_star_mask_software_name)
    aw_maskver_dict = {'AW_BADPIXMASK_VER'      : (unknown_str, 'AstroWise badpix mask version'),
                       'AW_MANMASK_VER'         : (unknown_str, 'AstroWise manual mask version'),
                       aw_star_mask_version_tag : (unknown_str, aw_star_mask_version_str),
                       }
    # Theli starmask (DS9 region file) header string contains the line
    #   (USERNAME=dklaes, PIPEVERS=1.9.7-98-g078cc93, STARHALO_MAGLIMIT=10.5,
    #    STARHALO_MAGLIMIT_FAINT=11.5, STARMASK_MAGLIMIT=14.0)
    # Theli voidmask (DS9 region file) header string contains the line
    #   (USERNAME=dklaes, PIPEVERS=1.9.7-145-gbc84f38)
    theli_mask_version_str = 'PIPEVERS'
    theli_star_mask_version_tag = 'TH_STARMASK_VER'
    theli_void_mask_version_tag = 'TH_VOIDMASK_VER'
    theli_maskver_dict = {theli_star_mask_version_tag : (unknown_str, 'Theli star mask version'),
                          theli_void_mask_version_tag : (unknown_str, 'Theli void mask version'),
                          }
    theli_starmask_maglimit_dict = {
        'STARHALO_MAGLIMIT'       : (unknown_str, 'Theli star refl. halo mag limit (r)'),
        'STARHALO_MAGLIMIT_FAINT' : (unknown_str, 'Theli starhalo conserv.lim. (r)'),
        'STARMASK_MAGLIMIT'       : (unknown_str, 'Theli stellar mag limit (r)'),
        }
    ## save the mask version info in a temporary file
    ver_file = os.path.join(temp_dir, '%s_mask_version%s.txt' % (field, rand_str))

    #
    # start making FITS bitmask files
    #
    print field

    ## THELI coadd weight file prep
    theli_coadd_dir = os.path.join(base_dir, field, detection_band, 'coadd_'+Ver)
    wtmask_fname = '%s_%s.%s.swarp.cut.flag.fits' % (field, detection_band, Ver)
    coadd_mask_file = os.path.join(theli_coadd_dir, wtmask_fname)
    coadd_mask_gz = coadd_mask_file + '.gz'
    new_coadd_mask_file = os.path.join(temp_dir, wtmask_fname.replace('.fits', '%s.fits'%(rand_str)))

    if not os.path.isfile(coadd_mask_gz):
        print 'ERROR: weight file', coadd_mask_gz, 'missing from', theli_coadd_dir
        print 'exiting'
        sys.exit(9)
    if not os.path.isfile(new_coadd_mask_file):
        shcommand = 'gunzip -c %s > %s' % (coadd_mask_gz, new_coadd_mask_file)
        print shcommand
        S.call(shcommand, shell=True)

    ## collect THELI header astrometry info
    with fits.open(new_coadd_mask_file) as hdulist:
        hdr = hdulist[0].header
        try:
            CRVAL1 = hdr['CRVAL1']   # coordinates of the reference pixel (RA)
            CRVAL2 = hdr['CRVAL2']   # coordinates of the reference pixel (Dec)
            CRPIX1 = hdr['CRPIX1']   # reference pixel (x)
            CRPIX2 = hdr['CRPIX2']   # reference pixel (y)
            NAXIS1 = hdr['NAXIS1']   # output pixel dimensions (x_size)
            NAXIS2 = hdr['NAXIS2']   # output pixel dimensions (y_size)
            CD1_1 = hdr['CD1_1']     # linear scaling coefficient
            CD1_2 = hdr['CD1_2']     # linear scaling coefficient
            CD2_1 = hdr['CD2_1']     # linear scaling coefficient
            CD2_2 = hdr['CD2_2']     # linear scaling coefficient
            theli_pixscale_as = np.sqrt(np.abs(CD1_1*CD2_2 - CD1_2*CD2_1)) * 3600.
            #print 'THELI pixel scale is', theli_pixscale_as, 'arcsec/pixel'
            coadd_wcs = wcs.WCS(hdr)
        except KeyError:
            print 'ERROR: astrometry info missing in file', coadd_mask_file
            print 'exiting'
            sys.exit()


    #
    # take AW flags and swarp into THELI coordinates
    #
    aw_outflag_list = ''   # store the SWARPed AW flag maps for future use
    aw_outflag_vals = ''   # store the bit value for future use
    mmask_bit = []
    aw_shift_bit = flagbit_dict['F_AW_U_AUTO'][0]
    aw_man1_outflag_dict = {
        'u':flagbit_dict['F_AW_U_AUTO'][0]/aw_shift_bit,  # bits to be assigned per filter,
        'g':flagbit_dict['F_AW_G_AUTO'][0]/aw_shift_bit,  # shifted by aw_shift_bit (0x40)
        'r':flagbit_dict['F_AW_R_AUTO'][0]/aw_shift_bit,
        'i':flagbit_dict['F_AW_I_AUTO'][0]/aw_shift_bit,
        'i2':flagbit_dict['F_AW_I2_AUTO'][0]/aw_shift_bit,
        }
    aw_auto_outflag_dict = {
        'u':flagbit_dict['F_AW_U_AUTO'][0]/aw_shift_bit,
        'g':flagbit_dict['F_AW_G_AUTO'][0]/aw_shift_bit,
        'r':flagbit_dict['F_AW_R_AUTO'][0]/aw_shift_bit,
        'i':flagbit_dict['F_AW_I_AUTO'][0]/aw_shift_bit,
        'i2':flagbit_dict['F_AW_I2_AUTO'][0]/aw_shift_bit,
        }
    aw_flag_orig_file_list = []
    for i_filt, filt in enumerate(aw_filter_list):

        ## keep track of sub-mask filenames
        base_fname = '%s_%s_mask_AW.fits' % (field, filt)
        aw_orig_flag_file = os.path.join(mask_dir, base_fname)
        aw_flag_file = os.path.join(temp_dir, base_fname.replace('.fits', rand_str+'.fits'))
        aw_flag_orig_file_list += [aw_flag_file,]

        resamp_suffix = '.resamp.fits'
        aw_outflag_fname = os.path.basename(aw_flag_file).replace('.fits', resamp_suffix)
        aw_outflag_fname = os.path.join(temp_dir, aw_outflag_fname)

        aw_outflag_swarped_centered = aw_outflag_fname.replace('.fits', '.temp.fits')
        aw_outflag_auto_fname = \
            os.path.join(temp_dir, '%s_%s_mask_AW_auto_THELI%s.fits'%(field, filt, rand_str))

        ## make available the original AW mask file
        gz_file = aw_orig_flag_file+'.gz'
        if not os.path.isfile(gz_file):
            # if the file does not exist, log to error file and continue
            print 'file', base_fname, 'missing from', mask_dir,
            print 'WARNING: mask bits not set for', base_fname
            continue
        elif not os.path.isfile(aw_flag_file):
            shcommand = 'gunzip -c %s > %s' % (gz_file, aw_flag_file)
            print shcommand
            S.call(shcommand, shell=True)

        ## collect manual mask bit info
        with fits.open(aw_flag_file) as hdulist:
            hdr = hdulist[0].header
            # save manual mask bit info
            try:
                F_MAN1 = hdr['F_MAN1']  # AW manual mask bit integer (should be 128)
            except KeyError:
                F_MAN1 = 128            # some number above 128, for use in ww
        mmask_bit += [F_MAN1]

        ## prepare for ww in the next step
        aw_outflag_list += aw_outflag_auto_fname + ','
        aw_outflag_list += aw_outflag_swarped_centered
        aw_outflag_vals += str(aw_auto_outflag_dict[filt]) + ','
        aw_outflag_vals += str(aw_man1_outflag_dict[filt])
        if i_filt < len(aw_filter_list)-1:
            aw_outflag_list += ','
            aw_outflag_vals += ','

        if run_aw_swarp:

            if i_filt == 0:   # print it only once
                print
                print 'running SWARP for AstroWise'
                print

            ## collect header info
            with fits.open(aw_flag_file) as hdulist:
                hdr = hdulist[0].header
                # (1) collect flag file BITPIX info
                BITPIX = hdr['BITPIX']      # data size of a pixel (should be 16)
                # (2) get AW mask version  TODO: improve, when AW header is fixed
                cat_header_lines = hdr.__str__().split('\n')
                version_info_line = filter(lambda ln: aw_star_mask_software_name in ln,
                                           cat_header_lines)
                aw_star_mask_version = version_info_line[0].split(aw_star_mask_software_name)[1]
                aw_star_mask_version = aw_star_mask_version.split(')')[0].strip()
                if len(aw_star_mask_version) > 0:
                    shcommand = 'echo %s %s >> %s' % (aw_star_mask_version_tag,
                                                      aw_star_mask_version, ver_file)
                    print shcommand
                    S.call(shcommand, shell=True)
                # (3) save manual mask bit info
                try:
                    F_MAN1 = hdr['F_MAN1']  # AW manual mask bit integer (should be 128)
                except KeyError:
                    F_MAN1 = 0              # to signify that this bit doesn't exist


            ## SWARP the original AW mask file and generate a resampled file
            shcommand = '@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/swarp_theli %s' % (aw_flag_file)
            shcommand += ' -IMAGEOUT_NAME   xxx'    # unused (see RESAMPLE_SUFFIX below)
            shcommand += ' -HEADER_ONLY     N'      # want output image
            shcommand += ' -HEADER_SUFFIX   xxx'    # don't use external header; run manually
            shcommand += ' -WEIGHT_TYPE     NONE'
            shcommand += ' -COMBINE         N'      # no combining, only resampling
            shcommand += ' -CELESTIAL_TYPE  NATIVE' # same as first input file
            shcommand += ' -CENTER_TYPE     MANUAL' # center assigned manually
            shcommand += ' -CENTER          %f,%f' % (CRVAL1, CRVAL2)  # ref_pix coords in degrees
            shcommand += ' -PIXELSCALE_TYPE MANUAL' # manually set the pixelscale
            shcommand += ' -PIXEL_SCALE     %f' % (theli_pixscale_as)  # pixel scale in arcseconds
            shcommand += ' -RESAMPLE        Y'      # input must be resampled
            shcommand += ' -RESAMPLE_DIR    %s' % (temp_dir) # resampled file output directory
            shcommand += ' -RESAMPLE_SUFFIX %s' % (resamp_suffix)  # SWARP output
            shcommand += ' -RESAMPLING_TYPE NEAREST'
            shcommand += ' -OVERSAMPLING    0'      # automatic oversamping
            shcommand += ' -INTERPOLATE     N'
            shcommand += ' -FSCALASTRO_TYPE NONE'   # pixel value does not scale with pixel size
            shcommand += ' -SUBTRACT_BACK   N'      # no background subtraction
            shcommand += ' -COPY_KEYWORDS   ""'
            shcommand += ' -VMEM_DIR %s' % (temp_dir)

            ## run SWARP
            if not os.path.isfile(aw_outflag_fname):
                print shcommand
                S.call(shcommand, shell=True)

            ## now resize the image with the correct center
            shcommand =  '@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/makesubimage'
            shcommand += " -%d -%d %d %d -o 1 -c < %s" % (CRPIX1, CRPIX2, NAXIS1, NAXIS2,
                                                          aw_outflag_fname,)
            shcommand += " | %s -p %d '%%1' - > %s" % ('@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic', BITPIX,
                                                       aw_outflag_swarped_centered)
            if not os.path.isfile(aw_outflag_swarped_centered):
                print shcommand
                S.call(shcommand, shell=True)

            ## split file into one without manual mask bits, and one with only manual mask bits
            with fits.open(aw_outflag_swarped_centered) as hdulist:
                # (3) remove manual mask bit info
                if F_MAN1 > 0:
                    print 'separating manual mask bit', F_MAN1, 'from the AW mask'
                    hdulist[0].data = np.bitwise_and(hdulist[0].data, np.invert(F_MAN1))
                    hdulist[0].writeto(aw_outflag_auto_fname, clobber=True)
                else:
                    shcommand = 'cp %s %s' % (aw_outflag_swarped_centered, aw_outflag_auto_fname)
                    S.call(shcommand, shell=True)

            ## clean-up
            if remove_temp_files:
                os.remove(aw_outflag_fname)
                os.remove(aw_outflag_fname.replace('.fits', '.weight.fits'))


    aw_ww_outflag_fname = os.path.join(temp_dir, '%s_mask_AW.combo%s.fits'%(field, rand_str))
    if aw_outflag_list == '':
        aw_mask_exists = False
        run_aw_ww = False
    if run_aw_ww:

        print
        print 'running WW for AstroWise'
        print

        ## collect flag file BITPIX info
        aw_flag_file_list = aw_outflag_list.split(',')
        with fits.open(aw_flag_file_list[0]) as hdulist:
            hdr = hdulist[0].header
            BITPIX = hdr['BITPIX']   # data size of each pixel (typically 16 for flags)

        ## combine THELI-scaled AW flag files using WeightWatchers
        temp_fname = aw_ww_outflag_fname.replace('.fits', '.temp.fits')
        shcommand =  '@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ww_theli '
        shcommand += ' -c @RUNROOT@/@CONFIGPATH@/MAKEFLAGMASK.default.ww'  # TODO: fix ww, remove config file
        shcommand += ' -WEIGHT_NAMES    ' + aw_outflag_list
        shcommand += ' -WEIGHT_MIN      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1' # pixel value below -1 is masked
        shcommand += ' -WEIGHT_MAX      0.5,%d,0.5,%d,0.5,%d,0.5,%d,0.5,%d' %  \
            (mmask_bit[0]-0.5, mmask_bit[1]-0.5,
             mmask_bit[2]-0.5, mmask_bit[3]-0.5, mmask_bit[4]-0.5)  # value above this masked
        shcommand += ' -WEIGHT_OUTFLAGS ' + aw_outflag_vals # set pixelmask flags to these values
        shcommand += ' -FLAG_NAMES      ""'
        shcommand += ' -POLY_NAMES      ""'
        shcommand += ' -VERBOSE_TYPE    FULL'
        shcommand += ' -OUTFLAG_NAME    ' + temp_fname
        shcommand += ' -OUTWEIGHT_NAME  ""'
        if not os.path.isfile(temp_fname):
            print shcommand
            S.call(shcommand, shell=True,)

        ## convert output to BITPIX (= int16)
        shcommand =   '@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic'
        shcommand += " -p %d '%%1' %s > %s" % (BITPIX, temp_fname, aw_ww_outflag_fname)
        if not os.path.isfile(aw_ww_outflag_fname):
            print shcommand
            S.call(shcommand, shell=True,)

        ## remove temporary files
        if remove_temp_files:
            os.remove(temp_fname)
            for i in range(len(aw_flag_file_list)):
                os.remove(aw_flag_file_list[i])
            for i in range(len(aw_flag_orig_file_list)):
                os.remove(aw_flag_orig_file_list[i])

    #
    # generate WCS polygon mask file
    #
    wcs_polygon_mask = os.path.join(temp_dir, '%s_wcs_mask%s.reg'%(field, rand_str))
    wcscut_mask_fname = os.path.join(temp_dir, '%s_wcs_mask%s.fits'%(field, rand_str))
    if run_make_wcs_mask:

        print
        print 'creating WCS mask file with %.2f %.2f %.2f %.2f' % (ra_min, ra_max, dec_min, dec_max)
        print

        if ra_min > ra_max:   # this is the format expected from the WCS cut input file
            ra_min -= 360.0

        ## get the polygon verticies
        grid_points = 50  # number of gridpoints
        delta_ra = (ra_max - ra_min) / (grid_points - 1)
        delta_dec = (dec_max - dec_min) / (grid_points - 1)
        dra  = [delta_ra,     0.0,   -(delta_ra),     0.0    ]
        ddec = [    0.0,   delta_dec,     0.0,   -(delta_dec)]
        ra = [ra_min,]
        dec = [dec_min,]
        for i in range(len(dra)):
            for n in range(grid_points):
                ra += [ (ra[-1] + dra[i]), ]
                dec += [ (dec[-1] + ddec[i]), ]
            # undo the last (excess) delta addition
            ra.pop()
            dec.pop()

        ra = np.array(ra)
        dec = np.array(dec)

        ## convert the ra/dec into
        world_coords = np.column_stack((ra, dec))
        pix_coords = coadd_wcs.wcs_world2pix(world_coords, 1)

        ## create region file
        reg_str = '#\n#\nphysical\nPOLYGON('   # ds9 region file POLYGON region, pixel coord
        for (x_pix, y_pix) in pix_coords:
            reg_str += '%.2f,%.2f,' % (x_pix, y_pix)
        reg_str = reg_str.strip(',')   # remove the last unnecessary comma
        reg_str += ')'

        ## write to file
        with open(wcs_polygon_mask, 'w') as outf:
            print >> outf, reg_str

        ## convert region file to FITS file
        shcommand =  '@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ww_theli '
        shcommand += ' -c @RUNROOT@/@CONFIGPATH@/MAKEFLAGMASK.default.ww'  # TODO: fix ww, remove config file
        shcommand += ' -WEIGHT_NAMES ' + new_coadd_mask_file
        shcommand += ' -WEIGHT_MIN -1e9 -WEIGHT_MAX 1e9'  # don't mask based on new_coadd_mask_file
        shcommand += ' -WEIGHT_OUTFLAGS 0'
        shcommand += ' -FLAG_NAMES ""'
        shcommand += ' -POLY_NAMES ' + wcs_polygon_mask
        shcommand += ' -POLY_OUTFLAGS 1'
        shcommand += ' -VERBOSE_TYPE FULL'
        shcommand += ' -OUTFLAG_NAME ' + wcscut_mask_fname # (masked region == 1, shift later)
        shcommand += ' -OUTWEIGHT_NAME ""'
        print shcommand
        S.call(shcommand, shell=True,)

        ## cleanup
        if remove_temp_files:
            os.remove(wcs_polygon_mask)
            os.remove(new_coadd_mask_file)

    #
    # combine THELI void masks, weight==0 mask and the masked regions, name it to "theli_combo"
    #

    ## keep track of sub-mask filename
    theli_ww_theli_combo_fname = \
        os.path.join(temp_dir, '%s_%s_mask_THELI.combo%s.fits'%(field, detection_band, rand_str))

    if run_theli_combo_ww:

        print
        print 'running WW for THELI masking flags in the detection band'
        print

        theli_mask_dir = os.path.join(base_dir, field, detection_band, 'masks_'+Ver)

        ## prepare THELI void mask
        voidmask_exists = True
        voidmask_fname = '%s_%s_voids.reg' % (field, detection_band,)
        voidmask_reg = os.path.join(theli_mask_dir, voidmask_fname)
        if not os.path.isfile(voidmask_reg):
            print 'WARNING: void mask file', voidmask_fname, 'missing from', theli_mask_dir
            print 'WARNING: mask bits not set for voids'
            voidmask_exists = False

        ## prepare manual mask
        manualmask_fname = '%s_%s_manual.reg' % (field, detection_band,)
        manualmask_reg = os.path.join(theli_mask_dir, manualmask_fname)
        if not os.path.isfile(manualmask_reg):
            print 'No manual mask file', manualmask_fname, ' in ', theli_mask_dir, '.'
            print 'Not applying any manual masks.'

        """
        ## prepare THELI asteroid mask  [[unused]]
        asteroids_fname = '%s_%s_asteroids.reg' % (field, detection_band)
        asteroids_reg = os.path.join(theli_mask_dir, asteroids_fname)
        new_asteroids_reg = os.path.join(temp_dir, asteroids_fname)
        if not os.path.isfile(new_asteroids_reg):
            shcommand = "sed 's/POLYGON/polygon/g' %s > %s" % (asteroids_reg,
                                                               new_asteroids_reg)
            S.call(shcommand, shell=True)
        """

        ## prepare THELI wt==0 mask
        ## note: in flag.fits, the masked bits == 1; non-masked bits == 0.
        wtmask_fname = '%s_%s.%s.swarp.cut.flag.fits' % (field, detection_band, Ver)
        weight_dir = os.path.join(base_dir, field, detection_band, 'coadd_'+Ver)
        wtmask_fits = os.path.join(weight_dir, wtmask_fname)
        wtmask_fits_gz = wtmask_fits + '.gz'
        new_wtmask_fits = \
            os.path.join(temp_dir, wtmask_fname.replace('.fits', '%s.fits'%(rand_str)))
        if not os.path.isfile(wtmask_fits_gz):
            print 'ERROR: weight file', wtmask_fname, 'missing from', weight_dir
            print 'exiting'
            sys.exit(9)
        shcommand = 'gunzip -c %s > %s' % (wtmask_fits_gz, new_wtmask_fits)
        print shcommand
        S.call(shcommand, shell=True)

        ## prepare THELI stellar mask
        starmask_fname = '%s_%s_stars.reg' % (field, detection_band,)
        starmask_reg = os.path.join(theli_mask_dir, starmask_fname)  # the united star mask
        if not os.path.isfile(starmask_reg):
            print 'ERROR: star mask file', starmask_fname, 'missing from', theli_mask_dir
            print 'exiting'
            sys.exit(9)
        # bright haloes and stars
        star_brighthalo_fname = starmask_fname.replace('.reg', '.star_brighthalo.reg')
        star_brighthalo_reg = \
            os.path.join(temp_dir, star_brighthalo_fname.replace('.reg', '%s.reg'%(rand_str)))
        if not os.path.isfile(star_brighthalo_reg):
            shcommand = "grep 'green\|magenta' %s > %s" % (starmask_reg, star_brighthalo_reg)
            S.call(shcommand, shell=True)
        # faint haloes
        fainthalo_fname = starmask_fname.replace('.reg', '.fainthalo.reg')
        fainthalo_reg = \
            os.path.join(temp_dir, fainthalo_fname.replace('.reg', '%s.reg'%(rand_str)))
        if not os.path.isfile(fainthalo_reg):
            shcommand = "grep 'global\|cyan' %s > %s" % (starmask_reg, fainthalo_reg)
            S.call(shcommand, shell=True)

        ## get star/void mask versions from headers
        reg_tag_pair = [(starmask_reg, theli_star_mask_version_tag),]
        if voidmask_exists:
            reg_tag_pair += [(voidmask_reg, theli_void_mask_version_tag),]
        for (reg, tag) in reg_tag_pair:
            shcommand = 'head -3 %s | grep %s' % (reg, theli_mask_version_str)
            ver_line = S.check_output(shcommand, shell=True)
            mask_version = ver_line.split(theli_mask_version_str+'=')[1]
            mask_version = mask_version.split(')')[0].strip()
            if reg == starmask_reg:
                # split to further get the magnitude limits
                for maglim_key in theli_starmask_maglimit_dict.keys():
                    maglim = mask_version.split(maglim_key+'=')[1]
                    maglim = maglim.split(',')[0].strip()
                    if len(maglim) > 0:
                        shcommand = 'echo %s %s >> %s' % (maglim_key, maglim, ver_file)
                        print shcommand
                        S.call(shcommand, shell=True)
                # and make sure the mask version is correct
                mask_version = mask_version.split(',')[0].strip()
            if len(mask_version) > 0:
                shcommand = 'echo %s %s >> %s' % (tag, mask_version, ver_file)
                print shcommand
                S.call(shcommand, shell=True)

        ## prepare strings for WeightWatcher
        poly_names = ''
        poly_outflags = ''
        if voidmask_exists:
            poly_names += voidmask_reg + ',' ## + new_asteroids_reg + ','
            poly_outflags += '%d,' % (flagbit_dict['F_TH_VOID'][0],)      # void
        poly_names += star_brighthalo_reg + ',' + fainthalo_reg
        poly_outflags += '%d,%d' % (flagbit_dict['F_TH_STAR'][0],      # star_brighthalo
                                    flagbit_dict['F_TH_STAR_CON'][0])  # fainthalo
        if os.path.isfile(manualmask_reg):
            poly_names += ',' + manualmask_reg
            poly_outflags += ',%d' % (flagbit_dict['F_TH_MANUAL'][0])
        weight_outflag_val = str(flagbit_dict['F_TH_VOID'][0])

        ## combine void[, asteroid,] and wt==0 masks
        shcommand =  '@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ww_theli '
        shcommand += ' -c @RUNROOT@/@CONFIGPATH@/MAKEFLAGMASK.default.ww'  # TODO: fix ww, remove config file
        shcommand += ' -WEIGHT_NAMES ' + new_wtmask_fits        # this is a 0/1 file
        shcommand += ' -WEIGHT_MIN -1 -WEIGHT_MAX 0.5'          # 1==masked
        shcommand += ' -WEIGHT_OUTFLAGS ' + weight_outflag_val
        shcommand += ' -FLAG_NAMES ""'
        shcommand += ' -POLY_NAMES ' + poly_names
        shcommand += ' -POLY_OUTFLAGS ' + poly_outflags
        shcommand += ' -VERBOSE_TYPE FULL'
        shcommand += ' -OUTFLAG_NAME ' + theli_ww_theli_combo_fname # (masked region == 1)
        shcommand += ' -OUTWEIGHT_NAME ""'
        print shcommand
        S.call(shcommand, shell=True,)

        # cleanup
        if remove_temp_files:
            os.remove(new_wtmask_fits)
            os.remove(fainthalo_reg)
            os.remove(star_brighthalo_reg)


        """
        # [[unused]]
        manual_mask_dir = 'KIDS_manual'
        # manual masks (base_flag * 2**3)
        manual_fname = '%s.%s.manualmask.reg' % (field, Ver,)
        manual_reg = os.path.join(manual_mask_dir, manual_fname)
        new_manual_reg = os.path.join(temp_dir, manual_fname)
        if not os.path.isfile(manual_reg):  # os.path.isfile() works for symbolic links too
            shcommand = 'echo "polygon(0,0,0,0,0,0,0,0,0,0) # magenta" > %s' % \
                (new_manual_reg)
        else:
            shcommand = 'cp %s %s' % (manual_reg, new_manual_reg)
        S.call(shcommand, shell=True)

        # aggressive manual masks (base_flag * 2**0)
        manual_agg_fname = '%s.%s.manualmask.aggressive.reg' % (field, Ver,)
        manual_agg_reg = os.path.join(theli_mask_dir, manual_agg_fname)
        new_manual_agg_reg = os.path.join(temp_dir, manual_agg_fname)
        if not os.path.isfile(manual_agg_reg):
            shcommand = 'echo "polygon(0,0,0,0,0,0,0,0,0,0) # cyan" > %s' % \
                (new_manual_agg_reg)
        else:
            shcommand = 'cp %s %s' % (manual_agg_reg, new_manual_agg_reg)
        S.call(shcommand, shell=True)
        """

    #
    #  Combine AW_combo, THELI_combo, and WCS cut bitmask
    #
    # CHECK: run image calculator (ic) and combine the above bitmasks
    bitmask_temp = os.path.join(temp_dir, bitmask_fname.replace('.fits', '%s.fits'%(rand_str)))
    bitmask_fits = os.path.join(mask_dir, bitmask_fname)

    if run_ic:

        print
        print 'running ic to combine various flag files'
        print

        ## first, run ic on the void images, which has the correct WCS header info
        flag_images = ''
        flag_commands = ''
        file_counter = 0
        ## the THELI_combo flags (shift bits if necessary)
        shift_flag = flagbit_dict['F_TH_STAR_CON'][0]  # the smallest Theli r-band mask bit
        file_counter += 1
        flag_commands += ' %%%d' % (file_counter,)
        flag_commands += ' %d mult' % (shift_flag)
        flag_images += ' ' + theli_ww_theli_combo_fname

        ## the AW flags
        if aw_mask_exists:
            file_counter += 1
            flag_commands += ' %%%d %d mult +' % (file_counter, aw_shift_bit)
            flag_images += ' ' + aw_ww_outflag_fname

        ## the WCS cut mask
        if run_make_wcs_mask:
            file_counter += 1
            flag_commands += ' %%%d -1 mult 1 +' % (file_counter,)   # invert the mask
            flag_commands += ' %d mult +' % (flagbit_dict['F_KIDS_WCS'][0],)
            flag_images += ' ' + wcscut_mask_fname

        ## run ic
        shcommand =  '@RUNROOT@/INSTALL/theli-@THELIPACKVERS@/bin/@MACHINE@/ic'
        shcommand += " -p %d '%s' %s" % (final_BITPIX, flag_commands, flag_images)
        shcommand += " > %s" % (bitmask_temp,)
        print shcommand
        S.call(shcommand, shell=True,)

        ## add version number and flag bit info in header
        with open(ver_file, 'r') as versionf:
            for line in versionf:
                ls = line.split()
                if len(ls) < 2:
                    print 'ERROR: version file', ver_file, 'has the wrong format'
                    print 'exiting'
                    sys.exit(9)
                for d in (aw_maskver_dict, theli_maskver_dict, theli_starmask_maglimit_dict):
                    if ls[0] in d.keys():
                        comment = d[ls[0]][1]
                        d[ls[0]] = (ls[1].strip(), comment)
        with fits.open(bitmask_temp) as hdulist:
            hdr = hdulist[0].header
            for d in (aw_maskver_dict, theli_maskver_dict, theli_starmask_maglimit_dict):
                for key in sorted(d.keys()):   # sort keys alphabetically
                    hdr[key] = d[key][0]
                    hdr.comments[key] = d[key][1]
            for key in sorted(flagbit_dict, key=flagbit_dict.get):  # sort by flag bit value
                hdr[key] = flagbit_dict[key][0]
                hdr.comments[key] = flagbit_dict[key][1]
            hdulist[0].writeto(bitmask_temp, clobber=True)

        ### zip the resulting output
        #shcommand = 'gzip -c %s > %s' % (bitmask_temp, bitmask_fits+'.gz',)
        #print shcommand
        #S.call(shcommand, shell=True,)
        ## move the resulting output to the final file name
        shcommand = 'mv %s %s' % (bitmask_temp, bitmask_fits,)
        print shcommand
        S.call(shcommand, shell=True,)

        ## cleanup
        if remove_temp_files:
            #os.remove(bitmask_temp)
            if aw_mask_exists:
                os.remove(aw_ww_outflag_fname)
            os.remove(theli_ww_theli_combo_fname)
            os.remove(ver_file)
            if run_make_wcs_mask:
                os.remove(wcscut_mask_fname)

except KeyboardInterrupt:

    print 'Interrupt'
    if remove_temp_files:
        print 'Cleaning temporary files for script', sys.argv[0]
        for f in temp_file_list_all:
            if os.path.isfile(f):
                os.remove(f)
    else:
        print 'Exiting with working directory', temp_dir, 'intact.'
        sys.exit(0)

except SystemExit as e:
    # retain contents of the working directory
    sys.exit(e)
