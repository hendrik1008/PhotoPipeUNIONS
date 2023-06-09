#!/users/hendrik/anaconda2/bin/python

import math
import sys
import string

def eq2gal(theta, phi):
    jgal = [[-0.0548755604, +0.4941094279, -0.8676661490],
            [-0.8734370902, -0.4448296300, -0.1980763734],
            [-0.4838350155, +0.7469822445, +0.4559837762]]
    rra = degrad(theta)
    rdec = degrad(phi)
    
    # Convert to geocentric, equatorial rectangular
    # coordinates
    pos = (math.cos(rra) * math.cos(rdec),
           math.sin(rra) * math.cos(rdec),
           math.sin(rdec))
    
    # Rotate to galactic
    pos1 = []
    for i in range(3):
        pos1.append(pos[0]*jgal[0][i] + pos[1]*jgal[1][i] + pos[2]*jgal[2][i])
        
    # Back from Cartesian to spherical
    x = pos1[0]
    y = pos1[1]
    z = pos1[2]
    rl = math.atan2(y, x)
    if (rl<0):
        rl=rl+2.*math.pi
    rxy2 = x*x + y*y
    rxy = math.sqrt(rxy2)
    rb = math.atan2(z, rxy)
    
    dl = raddeg(rl)
    db = raddeg(rb)
    return dl, db

def degrad(deg):
    rad = deg*math.pi/180.
    return rad

def raddeg(rad):
    deg = rad*180./math.pi
    return deg



if __name__ == "__main__":

    if len(sys.argv) != 2:
        print "Usage:"
        print "\n\n\teq2ga list"

    coord_list  = open(sys.argv[1], 'r')
    lines = coord_list.readlines()
    coord_list.close()
    for i in range(len(lines)):
        words=string.split(lines[i])
        l, b = eq2gal(string.atof(words[0]), string.atof(words[1]))
        print l, b
