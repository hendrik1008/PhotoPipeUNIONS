import numpy as np
import sys

tilename=sys.argv[1]

def mega2theli(tilename):
    xxx=tilename.split('.')[0]
    yyy=tilename.split('.')[1]
    dec = float(yyy)/2-90
    ra = float(xxx)/2/np.cos(dec*np.pi/180.)
    tiletheli = str(round(ra,1)).zfill(5)+'_'+str(round(dec,1)).zfill(4)
    return tiletheli

print(mega2theli(tilename))
