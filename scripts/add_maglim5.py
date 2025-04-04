#from astropy.table import Table
import numpy as np
#import astropy.io.fits as fits
#import astropy
import sys
import string
import ldac
import os

### command line arguments

catname = sys.argv[1]
outcat = sys.argv[2]

### read the input catalogue

ldac_cat = ldac.LDACCat(catname)
ldac_table = ldac_cat['OBJECTS']
nobj = np.shape(ldac_table['SeqNr'])[0]

### calculate limiting magnitudes

for aperture in ('0p7', '1p0'):
    aperture2 = aperture.replace("p", ".")
    for band in ('u', 'g', 'r', 'i', 'z'):
        SLR_Gaia_offset = 0.0
        ZP = 30.
        if band == "g" or band == 'z':
            ZP = 27.
        
        flux = ldac_table['FLUX_GAAP_'+aperture+'_'+band]
        fluxerr = ldac_table['FLUXERR_GAAP_'+aperture+'_'+band]
        
        # convert fluxerr -> maglim adding the calibration
        maglim = ZP - 2.5 * np.log10(fluxerr) + SLR_Gaia_offset

        # check for failures
        maglim[flux==0.] = -99.0
        
        maglim[fluxerr<=0.] = -99.0

        # store the maglim in the LDAC table
        ldac_table['MAG_LIM_'+aperture+'_'+band] = maglim
        ldac_table.set_comment('MAG_LIM_'+aperture+'_'+band, band+'-band limiting magnitude min_aper='+aperture2+'arcsec')
        ldac_table.set_unit('MAG_LIM_'+aperture+'_'+band, 'mag')

### take a decision, which apertures to use

R = np.zeros((nobj,5))
i=0
for band in ('u', 'g', 'r', 'i', 'z'):
    # first assign the smaller apertures throughout
    ldac_table['MAG_LIM_'+band] = ldac_table['MAG_LIM_0p7_'+band]
    ldac_table.set_comment('MAG_LIM_'+band, band+'-band limiting magnitude optimal min_aper')
    ldac_table.set_unit('MAG_LIM_'+band, 'mag')

    # Calculate R
    fluxerr1 = ldac_table['FLUXERR_GAAP_1p0_'+band]
    fluxerr2 = ldac_table['FLUXERR_GAAP_0p7_'+band]
    R[:,i] = fluxerr1 / fluxerr2
    R[:,i][fluxerr1==-1] = 1.
    i=i+1

# decide whether the larger aperture should be used
large_aper = ( np.min(R, axis=1) < 1./np.max(R, axis=1)   )     |     ( np.max(R,axis=1) < 0  )

# assign the larger aperture to the objects where large_aper==True
for band in ('u', 'g', 'r', 'i', 'z'):
    ldac_table['MAG_LIM_'+band][large_aper] = ldac_table['MAG_LIM_1p0_'+band][large_aper]

### save the catalogue

if os.path.exists(outcat):
    os.remove(outcat)
ldac_cat.saveas(outcat)
