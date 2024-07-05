import astropy.io.fits as fits
import sys, os, string

filename = sys.argv[1]
base = sys.argv[2]
extkey = sys.argv[3]

hdu = fits.open(filename)

for i in range(1,4):
    data = hdu[i].data
    header = hdu[i].header
    name = header[extkey]
    if name == 'IMAGE':
        ending = '.fits'
        data_new = data
    elif name == 'MASK':
        ending = '.flag.fits'
        data_new = data
    elif name == 'VARIANCE':
        ending = '.weight.tmp.fits'
        data_new = 1. / data
    outname = base+ending
    hdu_new=fits.PrimaryHDU(data_new)
    for key in list(header.keys()):
        hdu_new.header[key] = header[key]
    if os.path.exists(outname):
        os.remove(outname)
    hdu_new.writeto(outname)
