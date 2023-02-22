import astropy.io.fits as fits
import sys, os, string

filename = sys.argv[1]
outname = sys.argv[2]

hdu = fits.open(filename)

i = 1
data = hdu[i].data
header = hdu[i].header
hdu_new=fits.PrimaryHDU(data)
for key in list(header.keys()):
    if key != "COMMENT":
        hdu_new.header[key] = header[key]
if os.path.exists(outname):
    os.remove(outname)
hdu_new.writeto(outname)

