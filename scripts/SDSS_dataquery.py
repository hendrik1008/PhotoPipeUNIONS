#!/usr/bin/env python

# script to retrieve SDSS sources directly from the SDSS
# databases.

# program docstring:
"""
SCRIPT NAME:
    SDSS_dataquery.py - retrieve stars/galaxies from the SLOAN database

SYNOPSIS:
    SDSS_dataquery.py CATALOG STARS|GALZ|GALPHOT ramin ramax decmin decmax

DESCRIPTION:
    The script retrieves SLOAN objects within the rectangle defined
    by the ramin/ramax and decmin/decmax coordinate (to be provided
    in decimal degrees) pairs.
    The first argument determines which objects to retrieve from where:

    - CATALOG: Choosing where to get the data from. Up to now SDSSDR8,
               SDSSDR9, SDSSDR10 and STRIPE82 are supported.
    - STARS: stellar sources; retrieved are objectID, Ra, Dec, 
             Ra Error, Dec Error, PSF Magnitudes and errors in u, g, r, i, z
    - GALZ: galaxies with spectroscopic redshifts; retrieved are:
            objectID, Ra, Dec, Ra Error, Dec Error, z, z error, z warning, 
            model magnitudes and errors in u, g, r, i, z
    - GALPHOT galaxies with photometric measurements; retreived are:
            objectID, Ra, Dec, Ra Error, Dec Error, model magnitudes and 
            errors in u, g, r, i, z

    The script prints the retrieved objects to STDOUT. The first line
    of the output is a comment identifying the catalogues contents.

REMARKS:
    - The script depends on Tamas Budavaris 'sqlcl' module 
      (budavari@jhu.edu)
    - The script is based on program skeletons from Patrick Kelly
      (pkelly3@stanford.edu)
    - If the area is too large (several square degrees), the retrieval
      fails; especially for the STARS and GALPHOT modes.
    - For the 'STARS' and 'GALPHOT' samples, objects marked as blended
      are rejected.
    - The script is for Python 2.x!

EXAMPLES:
    ./SDSS_dataquery.py DR9 STARS 212.0 213.0 51.0 52.0

    This command retrieves stellar sources in the one square degree
    area around ra=212.5, dec=51.5 from SDSS-DR9.
    
AUTHOR:
    Thomas Erben         (terben@astro.uni-bonn.de)
    Dominik Klaes        (dklaes@astro.uni-bonn.de)

"""

import string
import math
import sys
import sqlcl

# main program
# ============

# sanity check on the number of command line arguments:
if len(sys.argv) != 7:
    print __doc__
    sys.exit(1)

catalog = sys.argv[1]
objects_mode = sys.argv[2]
ramin = float(sys.argv[3])
ramax = float(sys.argv[4])
decmin = float(sys.argv[5])
decmax = float(sys.argv[6])

if catalog == "SDSSDR8":
    public_url='http://skyserver.sdss3.org/dr8/en/tools/search/x_sql.asp'
elif catalog == "SDSSDR9":
    public_url='http://skyserver.sdss3.org/dr9/en/tools/search/x_sql.asp'
elif catalog == "SDSSDR10":
    public_url='http://skyserver.sdss3.org/dr10/en/tools/search/x_sql.aspx'
elif catalog == "STRIPE82":
    public_url='http://cas.sdss.org/public/en/tools/search/x_sql.asp'


# define the SQL query string for the SDSS database; dependent
# on the required object types:
if objects_mode == "STARS" :
    query = "select objID, ra, dec, raErr, decErr, psfMag_u, psfMagErr_u, \
          psfMag_g, psfMagErr_g, psfMag_r, psfMagErr_r, \
          psfMag_i, psfMagErr_i, psfMag_z, psfMagErr_z, \
          flags, flags_u, flags_g, flags_r, flags_i, flags_z, \
          clean from star where ra between " + str(ramin) +\
          " and  " + str(ramax) + " and dec between " + str(decmin) +\
          " and " + str(decmax) + \
          " AND flags & dbo.fPhotoFlags('BLENDED') = 0" + \
          " and clean = 1 and mode = 1" + \
          "AND ((flags_r & 0x10000000) != 0)" + \
          "AND ((flags_r & 0x8100000c00a4) = 0)" + \
          "AND (((flags_r & 0x400000000000) = 0))" + \
          "AND (((flags_r & 0x100000000000) = 0) or (flags_r & 0x1000) = 0)" + \
          "AND ((flags_i & 0x10000000) != 0)" + \
          "AND ((flags_i & 0x8100000c00a4) = 0)" + \
          "AND (((flags_i & 0x400000000000) = 0))" + \
          "AND (((flags_i & 0x100000000000) = 0) or (flags_i & 0x1000) = 0)" + \
          "AND (psfMagErr_u < 0.05)" + \
          "AND (psfMagErr_g < 0.05)" + \
          "AND (psfMagErr_r < 0.05)" + \
          "AND (psfMagErr_i < 0.05)" + \
          "AND (psfMagErr_z < 0.05)"

if objects_mode == "GALZ" :
    query = "select s.ra, s.dec, s.z, s.zErr, s.zWarning, \
             s.BestObjID, \
             g.modelMag_u, g.modelMag_g, g.modelMag_r, g.modelMag_i, \
             g.modelMag_z, g.modelMagErr_u, g.modelMagErr_g, \
             g.modelMagErr_r, g.modelMagErr_i, g.modelMagErr_z, \
             g.objID from SpecObj as s \
             join galaxy as g on s.bestObjID = g.objID where s.ra between " +\
             str(ramin) + " and " + str(ramax) + " and s.dec between " +\
             str(decmin) + " and " + str(decmax)

if objects_mode == "GALPHOT" :
    query = "select modelMag_u, modelMag_g, modelMag_r, modelMag_i, \
             modelMag_z, modelMagErr_u, modelMagErr_g, \
             modelMagErr_r, modelMagErr_i, modelMagErr_z, \
             objID, ra, dec, raErr, decErr, flags \
             from galaxy where ra between " + \
             str(ramin) + " and " + str(ramax) + " and dec between " +\
             str(decmin) + " and " + str(decmax) + \
             " AND flags & dbo.fPhotoFlags('BLENDED') = 0 "

# query the SDSS database:
lines = sqlcl.query(query,public_url).readlines()
if len(lines) == 8:
    print "An error occured during your request; probably"
    print "the selected area is too large"
    sys.exit(1)

# This became necessary due to a change in SDSS!
if catalog == "SDSSDR10":
	START=2
	columns = lines[1][:-1].split(',')
else:
	START=1
	columns = lines[0][:-1].split(',')

data = []

# print query results:
print "# script call: %s %s %s %s %s %s" % (sys.argv[0], sys.argv[1],
                                            sys.argv[2], sys.argv[3],
                                            sys.argv[4], sys.argv[5])
print "#"



for line in range(START,len(lines[1:])+1):
    dt0 = {}
    for j in range(len(lines[line][:-1].split(','))):
        dt0[columns[j]] = lines[line][:-1].split(',')[j]
    if string.find(lines[line][:-1],'font') == -1:
        data.append(dt0)

for i in range(0, len(data)):
    if objects_mode == "STARS" :
        if i == 0:
            print "# catalogue contents: SDSS_ID Ra Dec RaErr DecErr "+\
                  "psfMag_u psfMagErr_u " +\
                  "psfMag_g psfMagErr_g " +\
                  "psfMag_r psfMagErr_r " +\
                  "psfMag_i psfMagErr_i " +\
                  "psfMag_z psfMagErr_z"            
        print data[i]['objID'], data[i]['ra'], data[i]['dec'], \
              data[i]['raErr'], data[i]['decErr'], \
              data[i]['psfMag_u'], data[i]['psfMagErr_u'], \
              data[i]['psfMag_g'], data[i]['psfMagErr_g'], \
              data[i]['psfMag_r'], data[i]['psfMagErr_r'], \
              data[i]['psfMag_i'], data[i]['psfMagErr_i'], \
              data[i]['psfMag_z'], data[i]['psfMagErr_z']

    if objects_mode == "GALPHOT" :
        if i == 0:
            print "# catalogue contents: SDSS_ID Ra Dec RaErr DecErr "+\
                  "modelMag_u modelMagErr_u " +\
                  "modelMag_g modelMagErr_g " +\
                  "modelMag_r modelMagErr_r " +\
                  "modelMag_i modelMagErr_i " +\
                  "modelMag_z modelMagErr_z"        
        print data[i]['objID'], data[i]['ra'], data[i]['dec'], \
              data[i]['raErr'], data[i]['decErr'], \
              data[i]['modelMag_u'], data[i]['modelMagErr_u'], \
              data[i]['modelMag_g'], data[i]['modelMagErr_g'], \
              data[i]['modelMag_r'], data[i]['modelMagErr_r'], \
              data[i]['modelMag_i'], data[i]['modelMagErr_i'], \
              data[i]['modelMag_z'], data[i]['modelMagErr_z']

    if objects_mode == "GALZ" :
        if i == 0:
            print "# catalogue contents: SDSS_ID Ra Dec " +\
                  "modelMag_u modelMagErr_u " +\
                  "modelMag_g modelMagErr_g " +\
                  "modelMag_r modelMagErr_r " +\
                  "modelMag_i modelMagErr_i " +\
                  "modelMag_z modelMagErr_z, z, zErr, zWarning"            
        print data[i]['objID'], data[i]['ra'], data[i]['dec'], \
              data[i]['modelMag_u'], data[i]['modelMagErr_u'], \
              data[i]['modelMag_g'], data[i]['modelMagErr_g'], \
              data[i]['modelMag_r'], data[i]['modelMagErr_r'], \
              data[i]['modelMag_i'], data[i]['modelMagErr_i'], \
              data[i]['modelMag_z'], data[i]['modelMagErr_z'], \
              data[i]['z'], data[i]['zErr'], data[i]['zWarning']
