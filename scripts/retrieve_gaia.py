import astropy.units as u
from astropy.coordinates import SkyCoord
from astroquery.gaia import Gaia
from math import cos,pi
import sys, os

Gaia.MAIN_GAIA_TABLE = "gaiadr3.gaia_source"

md =    sys.argv[1]
FIELD = sys.argv[2]
ra  =   float(sys.argv[3])
dec =   float(sys.argv[4])

coord = SkyCoord(ra=ra, dec=dec, unit=(u.degree, u.degree), frame='icrs')
width = u.Quantity(0.6/cos(dec/180.*pi), u.deg)
height = u.Quantity(0.6, u.deg)

r = Gaia.query_object_async(coordinate=coord, width=width, height=height, columns=['ra','dec','phot_g_mean_mag','bp_rp','classprob_dsc_combmod_star'])

#print(r.colnames)
#r.pprint(max_lines=12, max_width=130)
#print(r['ra'])
#print(r)

f = md+FIELD+'_Gaia.asc'
if os.path.isfile(f):
    os.remove(f)

r.write(f,format='ascii')
