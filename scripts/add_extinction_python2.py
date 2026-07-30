#!/users/hendrik/anaconda2/bin/python

from astropy.io import fits
import numpy as np
import sys,os
from astropy.wcs import WCS
from astropy.coordinates import SkyCoord
from astropy import wcs

incat = sys.argv[1]
outcat = sys.argv[2]
RA = sys.argv[3]

hemisphere = 'n'
if RA < 90 or RA > 300:
    hemisphere = 's'

inimage = fits.open('/arc/home/hendrik/src/PhotoPipeUNIONS/dustmaps/sfd/SFD_dust_4096_'+hemisphere+'gp.fits') # axis flipped!
imagedata = inimage[0].data

w = WCS('/arc/home/hendrik/src/PhotoPipeUNIONS/dustmaps/sfd/SFD_dust_4096_'+hemisphere+'gp.fits')

hdu = fits.open(incat)

data = hdu[1].data
cols = data.columns

RA = data['ALPHA_J2000']
Dec = data['DELTA_J2000']

coords = SkyCoord(RA, Dec, unit='deg')

pos=wcs.utils.skycoord_to_pixel(coords, w)

ebv = []

for k in range(np.shape(pos)[1]):
    ebv.append(imagedata[int(pos[1][k]),int(pos[0][k])])

ebv_col = fits.Column(name='EXTINCTION', format='E', array=ebv)

new_cols = fits.ColDefs((ebv_col,))

hdu[1] = fits.BinTableHDU.from_columns(cols + new_cols)
hdu[1].name = 'OBJECTS'

if os.path.exists(outcat):
    os.remove(outcat)
hdu.writeto(outcat)
