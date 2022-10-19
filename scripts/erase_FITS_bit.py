import astropy.io.fits as fits
import numpy as np
import sys,os

fn = sys.argv[1] # assuming an 8-bit FITS image with no extension here
fo = sys.argv[2]
bit_value = int(sys.argv[3])

hdu = fits.open(fn)
data = hdu[0].data
header = hdu[0].header

bit_mask = 255-bit_value

data_new = np.bitwise_and(data,bit_mask)

hdu_new = fits.PrimaryHDU(data_new)
hdu_new.header = header

if os.path.exists(fo):
    os.remove(fo)
hdu_new.writeto(fo)
