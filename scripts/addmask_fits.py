#!/usr/bin/python

import astropy.io.fits as pyfits
import numpy
import sys
import string

inimage = pyfits.open(sys.argv[2]) # axis flipped!
imagedata = inimage[0].data

incat = open(sys.argv[1])
incatlines = incat.readlines()
incat.close

pos=numpy.zeros((2,len(incatlines)), "f")

for k in range(len(incatlines)):
    words = string.split(incatlines[k])
    pos[0,k]=int(string.atof(words[0])-1) # x
    pos[1,k]=int(string.atof(words[1])-1) # y
    print imagedata[pos[1,k],pos[0,k]]
