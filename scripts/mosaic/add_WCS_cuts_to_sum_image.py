# Script to modify a 16Bit FITS mask by cutting out a
# rectangular RA-Dec region.
#
# Author: Hendrik Hildebrandt (hendrik@phas.ubc.ca)
#
# Usage: 
# python modify_mask.py image mask output_mask RA_min RA_max Dec_min Dec_max
#
# Requirements: AstroPy, Numpy
#
# It takes the astrometric header information from the image and
# converts the WCS cuts into cuts in pixel space. All pixels outside
# this region are set to zero.

import astropy.io.fits as fits
import numpy as np
import sys, os
import astropy.wcs as wcs

image=sys.argv[1]
outimage=sys.argv[2]
ralow=float(sys.argv[3])
rahigh=float(sys.argv[4])
declow=float(sys.argv[5])
dechigh=float(sys.argv[6])

hdulist = fits.open(image)
data = hdulist[0].data
header = fits.getheader(image)
wcs_header = wcs.WCS(header)

NAXIS1 = header['NAXIS1']
NAXIS2 = header['NAXIS2']

n=np.arange(NAXIS1*NAXIS2)
p=np.ones(NAXIS1*NAXIS2,dtype=np.int16)*NAXIS1
x=np.mod(n,p)
y=np.divide(n,p)
# delete n and p here
sky = wcs_header.all_pix2world(x,y,0)
# delete x and y here
ra = sky[0]
dec = sky[1]
# delete sky here
deccut1 = np.greater(dec, declow)
deccut2 = np.less_equal(dec, dechigh)
# delete dec here
racut1 = np.greater(ra, ralow)
racut2 = np.less_equal(ra, rahigh)
# delete ra here
if ralow < rahigh:
    wcscut = racut1 * racut2 * deccut1 * deccut2
else:
    wcscut = np.logical_or(racut1, racut2) * deccut1 * deccut2
# delete racut1/2 and deccut1/2 here

wcscut = np.reshape(wcscut, (NAXIS2, NAXIS1))

output_data = wcscut*data

hdu=fits.PrimaryHDU(output_data)
hdu.header=header

if os.path.exists(outimage):
    os.remove(outimage)
hdu.writeto(outimage)
