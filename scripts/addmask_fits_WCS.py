#!/users/hendrik/anaconda2/bin/python

import astropy.io.fits as pyfits
import numpy as np
import sys
import string
from astropy.wcs import WCS
from astropy.coordinates import SkyCoord
import astropy.wcs as pywcs

inimage = pyfits.open(sys.argv[2]) # axis flipped!
imagedata = inimage[0].data

w = WCS(sys.argv[2])

radec = np.loadtxt(sys.argv[1])

c = SkyCoord(radec[:,0], radec[:,1], unit="deg")

pos=pywcs.utils.skycoord_to_pixel(c, w)

for k in range(np.shape(pos)[1]):
    if int(pos[1][k])>=0 and int(pos[1][k])<np.shape(imagedata)[0] and int(pos[0][k])>=0 and int(pos[0][k])<np.shape(imagedata)[1]:
        print imagedata[int(pos[1][k]),int(pos[0][k])]
    else:
        print 1
