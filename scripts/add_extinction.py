from astropy.coordinates import SkyCoord
from dustmaps.sfd import SFDQuery
from astropy.io import fits
import sys

incat = sys.argv[1]
outcat = sys.argv[2]

hdu = fits.open(incat)

data = hdu[1].data

RA = data['ALPHA_J2000']
Dec = data['DELTA_J2000']

coords = SkyCoord(RA, Dec, unit='deg')
