#!/users/hendrik/anaconda2/bin/python

import astropy.io.fits as fits
import numpy as np
import os
import sys
import datetime
import ldac

catname = sys.argv[1]
outcat = sys.argv[2]

ldac_cat = ldac.LDACCat(catname)
ldac_table = ldac_cat['OBJECTS']
#nobj = np.shape(ldac_table['SeqNr'])[0]

mags = {}
extinctions = {}

for band in ("u","g","r","i","z","z2"):
    mags[band] = ldac_table['MAG_GAAP_'+band]
    extinctions[band] = ldac_table['EXTINCTION_'+band]
    mags[band][np.logical_and(mags[band]!=99., mags[band]!=-99.)] = mags[band][np.logical_and(mags[band]!=99., mags[band]!=-99.)] - extinctions[band][np.logical_and(mags[band]!=99., mags[band]!=-99.)]
    ldac_table['MAG_GAAP_'+band] = mags[band]

if os.path.exists(outcat):
    os.remove(outcat)
ldac_cat.saveas(outcat)
