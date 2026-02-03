import pyvo as vo
#from astropy.coordinates import SkyCoord
#from astroquery.gaia import Gaia
from math import cos,pi
import sys, os

md =    sys.argv[1]
FIELD = sys.argv[2]
ra  =   float(sys.argv[3])
dec =   float(sys.argv[4])

ramin  = str(ra - (0.3/cos(dec/180*pi)))
ramax  = str(ra + (0.3/cos(dec/180*pi)))
decmin = str(dec - 0.3)
decmax = str(dec + 0.3)

service = vo.dal.TAPService("https://ws-uv.canfar.net/youcat")

query = "SELECT raPS,decPS,gHSC,rMega,iPS,zPS,zHSC FROM pgm.ps_gaia_merged WHERE raPS>"+ramin+" AND raPS<"+ramax+" AND decPS>"+decmin+" and decPS<"+decmax

print(query)

r = service.search(query)
rt = r.to_table()
 
f = md+FIELD+'_PGM.asc'
if os.path.isfile(f):
    os.remove(f)

rt.write(f,format='ascii')
