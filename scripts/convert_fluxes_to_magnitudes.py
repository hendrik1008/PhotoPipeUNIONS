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
u_offset = float(sys.argv[3])

### read the input catalogue

ldac_cat = ldac.LDACCat(catname)
ldac_table = ldac_cat['OBJECTS']
nobj = np.shape(ldac_table['SeqNr'])[0]

### convert fluxes to magnitudes

for aperture in ('0p7', '1p0'):
    aperture2 = aperture.replace("p", ".")
    for band in ('u', 'g', 'r', 'i1', 'i2'):
        band_cap = band.capitalize()[0]
        # read the SLR+Gaia calibration from the header
        if aperture == '0p7':
            if band_cap == "I":
                band_num = band.capitalize()[1]
                SLR_Gaia_offset = ldac_cat.header['DMAG_'+band_cap+"_07_i"+band_num]
                if SLR_Gaia_offset < -98.:
                    SLR_Gaia_offset = 0.
            else:
                if ldac_cat.header['DMAG_'+band_cap+"_07_i1"] > -99. and ldac_cat.header['DMAG_'+band_cap+"_07_i2"] > -99.:
                    SLR_Gaia_offset = (ldac_cat.header['DMAG_'+band_cap+"_07_i1"]+ldac_cat.header['DMAG_'+band_cap+"_07_i2"])/2.0
                elif ldac_cat.header['DMAG_'+band_cap+"_07_i1"] > -99.:
                    SLR_Gaia_offset = ldac_cat.header['DMAG_'+band_cap+"_07_i1"]
                elif ldac_cat.header['DMAG_'+band_cap+"_07_i2"] > -99.:
                    SLR_Gaia_offset = ldac_cat.header['DMAG_'+band_cap+"_07_i2"]
                else:
                    SLR_Gaia_offset = 0.                
        if aperture == '1p0':
            if band_cap == "I":
                band_num = band.capitalize()[1]
                SLR_Gaia_offset = ldac_cat.header['DMAG_'+band_cap+'_10_i'+band_num]
                if SLR_Gaia_offset < -98.:
                    SLR_Gaia_offset = 0.
            else: 
                if ldac_cat.header['DMAG_'+band_cap+"_10_i1"] > -99. and ldac_cat.header['DMAG_'+band_cap+"_10_i2"] > -99.:
                    SLR_Gaia_offset = (ldac_cat.header['DMAG_'+band_cap+'_10_i1']+ldac_cat.header['DMAG_'+band_cap+"_10_i2"])/2.0
                elif ldac_cat.header['DMAG_'+band_cap+"_10_i1"] > -99.:
                    SLR_Gaia_offset = ldac_cat.header['DMAG_'+band_cap+"_10_i1"]
                elif ldac_cat.header['DMAG_'+band_cap+"_10_i2"] > -99.:
                    SLR_Gaia_offset = ldac_cat.header['DMAG_'+band_cap+"_10_i2"]
                else:
                    SLR_Gaia_offset = 0.                

        if band == "u":
            SLR_Gaia_offset -= u_offset
            
        flag = ldac_table['FLAG_GAAP_'+aperture+'_'+band]
        flux = ldac_table['FLUX_GAAP_'+aperture+'_'+band]
        fluxerr = ldac_table['FLUXERR_GAAP_'+aperture+'_'+band]
        
        # convert flux -> mag adding the calibration
        mag = -2.5 * np.log10(flux) + SLR_Gaia_offset

        # convert fluxerr -> magerr
        magerr = 2.5 / np.log(10.) * np.sqrt((fluxerr / flux)**2)

        # non detections
        mag[np.logical_and(flux < fluxerr, fluxerr>0.)] = 99.0
        
        # check for failures
        flag[flux==0.] = 1
        magerr[flux==0.] = -99.0
        mag[flux==0.] = -99.0
        
        flag[fluxerr<0.] = 1
        mag[fluxerr<0.] = -99.0
        magerr[fluxerr<0.] = -99.0

        flag[np.isinf(magerr)] = 1
        mag[np.isinf(magerr)] = -99.0
        magerr[np.isinf(magerr)] = -99.0
        
        flag[np.isinf(mag)] = 1
        magerr[np.isinf(mag)] = -99.0
        mag[np.isinf(mag)] = -99.0

        flag[np.isnan(mag)] = 1
        magerr[np.isnan(mag)] = -99.0
        mag[np.isnan(mag)] = -99.0

        flag[np.isnan(magerr)] = 1
        mag[np.isnan(magerr)] = -99.0
        magerr[np.isnan(magerr)] = -99.0

        # store the mag and magerr in the LDAC table
        ldac_table['FLAG_GAAP_'+aperture+'_'+band] = flag
        ldac_table.set_comment('FLAG_GAAP_'+aperture+'_'+band, band+'-band GAaP flag min_aper='+aperture2+'arcsec')
        ldac_table['MAG_GAAP_'+aperture+'_'+band] = mag
        ldac_table.set_comment('MAG_GAAP_'+aperture+'_'+band, band+'-band GAaP magnitude min_aper='+aperture2+'arcsec')
        ldac_table.set_unit('MAG_GAAP_'+aperture+'_'+band, 'mag')
        ldac_table['MAGERR_GAAP_'+aperture+'_'+band] = magerr
        ldac_table.set_comment('MAGERR_GAAP_'+aperture+'_'+band, band+'-band GAaP magnitude error min_aper='+aperture2+'arcsec')
        ldac_table.set_unit('MAGERR_GAAP_'+aperture+'_'+band, 'mag')

### take a decision, which apertures to use

R = np.zeros((nobj,10))
i=0
for band in ('u', 'g', 'r', 'i1', 'i2', 'Z', 'Y', 'J', 'H', 'Ks'):
    # first assign the smaller apertures throughout
    ldac_table['MAG_GAAP_'+band] = ldac_table['MAG_GAAP_0p7_'+band]
    ldac_table.set_comment('MAG_GAAP_'+band, band+'-band GAaP magnitude optimal min_aper')
    ldac_table.set_unit('MAG_GAAP_'+band, 'mag')
    ldac_table['MAGERR_GAAP_'+band] = ldac_table['MAGERR_GAAP_0p7_'+band]
    ldac_table.set_comment('MAGERR_GAAP_'+band, band+'-band GAaP magnitude error optimal min_aper')
    ldac_table.set_unit('MAGERR_GAAP_'+band, 'mag')
    ldac_table['FLUX_GAAP_'+band] = ldac_table['FLUX_GAAP_0p7_'+band]
    ldac_table.set_comment('FLUX_GAAP_'+band, band+'-band GAaP flux optimal min_aper')
    ldac_table.set_unit('FLUX_GAAP_'+band, 'count')
    ldac_table['FLUXERR_GAAP_'+band] = ldac_table['FLUXERR_GAAP_0p7_'+band]
    ldac_table.set_comment('FLUXERR_GAAP_'+band, band+'-band GAaP flux error optimal min_aper')
    ldac_table.set_unit('FLUXERR_GAAP_'+band, 'count')
    ldac_table['FLAG_GAAP_'+band] = ldac_table['FLAG_GAAP_0p7_'+band]
    ldac_table.set_comment('FLAG_GAAP_'+band, 'GAaP Flag for MAG_GAAP_'+band+' optimal min_aper')
    if band == 'Z' or band == 'Y' or band == 'J' or band == 'H' or band == 'Ks':
        ldac_table['GAAP_nexp_'+band] = ldac_table['GAAP_nexp_0p7_'+band]
        ldac_table.set_comment('GAAP_nexp_'+band, 'GAaP number of exposures '+band+'-band optimal min_aper')
        ldac_table['GAAP_chi_sq_dof_'+band] = ldac_table['GAAP_chi_sq_dof_0p7_'+band]
        ldac_table.set_comment('GAAP_chi_sq_dof_'+band, 'GAaP chi^2/dof '+band+'-band optimal min_aper')

    # Calculate R
    fluxerr1 = ldac_table['FLUXERR_GAAP_1p0_'+band]
    fluxerr2 = ldac_table['FLUXERR_GAAP_0p7_'+band]
    R[:,i] = fluxerr1 / fluxerr2
    R[:,i][fluxerr1==-1] = 1.
    i=i+1

# Same for the semi-major and -minor axes
ldac_table['Agaper'] = ldac_table['Agaper_0p7']
ldac_table.set_comment('Agaper', 'Major axis of GAaP aperture optimal min_aper (arcsec)')
ldac_table.set_unit('Agaper', 'arcsec')
ldac_table['Bgaper'] = ldac_table['Bgaper_0p7']
ldac_table.set_comment('Bgaper', 'Minor axis of GAaP aperture optimal min_aper (arcsec)')
ldac_table.set_unit('Bgaper', 'arcsec')
    
# decide whether the larger aperture should be used
large_aper = ( np.min(R, axis=1) < 1./np.max(R, axis=1)   )     |     ( np.max(R,axis=1) < 0  )

# assign the larger aperture to the objects where large_aper==True
for band in ('u', 'g', 'r', 'i1', 'i2', 'Z', 'Y', 'J', 'H', 'Ks'):
    ldac_table['MAG_GAAP_'+band][large_aper] = ldac_table['MAG_GAAP_1p0_'+band][large_aper]
    ldac_table['MAGERR_GAAP_'+band][large_aper] = ldac_table['MAGERR_GAAP_1p0_'+band][large_aper]
    ldac_table['FLUX_GAAP_'+band][large_aper] = ldac_table['FLUX_GAAP_1p0_'+band][large_aper]
    ldac_table['FLUXERR_GAAP_'+band][large_aper] = ldac_table['FLUXERR_GAAP_1p0_'+band][large_aper]
    ldac_table['FLAG_GAAP_'+band][large_aper] = ldac_table['FLAG_GAAP_1p0_'+band][large_aper]
    if band == 'Z' or band == 'Y' or band == 'J' or band == 'H' or band == 'Ks':
        ldac_table['GAAP_nexp_'+band][large_aper] = ldac_table['GAAP_nexp_1p0_'+band][large_aper]
        ldac_table['GAAP_chi_sq_dof_'+band][large_aper] = ldac_table['GAAP_chi_sq_dof_1p0_'+band][large_aper]

ldac_table['Agaper'][large_aper] = ldac_table['Agaper_1p0'][large_aper]
ldac_table['Bgaper'][large_aper] = ldac_table['Bgaper_1p0'][large_aper]

### save the catalogue

if os.path.exists(outcat):
    os.remove(outcat)
ldac_cat.saveas(outcat)
