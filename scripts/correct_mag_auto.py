#!/users/hendrik/anaconda2/bin/python

import astropy.io.fits as fits
import numpy as np
import os
import sys
import datetime
import ldac

catname = sys.argv[1]
outcat = sys.argv[2]
mag_corr = float(sys.argv[3])

ldac_cat = ldac.LDACCat(catname)
ldac_table = ldac_cat['OBJECTS']

#Save the raw MAG_AUTO 
ldac_table['MAG_AUTO_raw'] = ldac_table['MAG_AUTO']
ldac_table['FLUX_AUTO_raw'] = ldac_table['FLUX_AUTO']
#Apply the MAG_GAAP_r correction to MAG_AUTO
ldac_table['MAG_AUTO'] += mag_corr
ldac_table['FLUX_AUTO'] *= 10**(-0.4*mag_corr)

if os.path.exists(outcat):
    os.remove(outcat)
ldac_cat.saveas(outcat)
