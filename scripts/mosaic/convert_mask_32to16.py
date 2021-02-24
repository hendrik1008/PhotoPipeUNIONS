import astropy.io.fits as pyfits
import numpy as np
import sys, os

image=sys.argv[1]
outimage=sys.argv[2]

hdulist = pyfits.open(image)
header = pyfits.getheader(image)
hdudata=hdulist[0].data
x=hdudata.astype(np.int16)
hdu=pyfits.PrimaryHDU(x)
hdu.header=header
if os.path.exists(outimage):
    os.remove(outimage)
hdu.writeto(outimage)
