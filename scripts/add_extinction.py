from astropy.coordinates import SkyCoord
from dustmaps.sfd import SFDQuery
from dustmaps.planck import PlanckQuery
from astropy.io import fits
import sys

incat = sys.argv[1]
outcat = sys.argv[2]

hdu = fits.open(incat)

data = hdu[1].data
cols = data.columns

RA = data['ALPHA_J2000']
Dec = data['DELTA_J2000']

coords = SkyCoord(RA, Dec, unit='deg')

sfd = SFDQuery()
planck = PlanckQuery()

ebv = sfd(coords)
ebv_p = planck(coords)

ebv_col = fits.Column(name='EXTINCTION', format='E', array=ebv)
ebv_p_col = fits.Column(name='EXTINCTION_PLANCK', format='E', array=ebv_p)

new_cols = fits.ColDefs((ebv_col, ebv_p_col))

print(hdu[1])
print(hdu[1].data)
print(hdu[1].columns)
print

hdu[1] = fits.BinTableHDU.from_columns(cols + new_cols)
hdu[1].name = 'OBJECTS'

print(hdu[1])
print(hdu[1].data)
print(hdu[1].columns)
print

hdu.writeto(outcat)
