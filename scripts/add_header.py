#!/users/hendrik/anaconda2/bin/python

import astropy.io.fits as fits
import numpy as np
import os
import sys
import datetime
import ldac

incat = fits.open(sys.argv[1])
outcat = sys.argv[2]
phot_cat = fits.open(sys.argv[3])
header = phot_cat[0].header

incat[0].header = header

if os.path.exists(outcat):
    os.remove(outcat)
incat.writeto(outcat)
