#!/usr/bin/python
# -*- coding: utf-8 -*-
# scipy-0.11, numpy-1.7.1 and astropy-0.2.3 required!

# ----------------------------------------------------------------
# File Name:           get_objects_KiDS_from_SDSS.py
# Author:              Dominik Klaes (dklaes@astro.uni-bonn.de)
# Last modified on:    14.10.2013
# Version:	       V1.2
# Description:         Get objects for KiDS data from SDSS database
# ----------------------------------------------------------------

# $1	main directory of runs
# $2	directory where to save catalogue
# $3	name of the final catalog
# $4	camera
# $5	Reference catalog (at the moment: SDSSDR8, SDSSDR9, SDSSDR10, STRIPE82)

# Importing packages
from __future__ import print_function
import os
import sys
import numpy as np
import astropy
import astropy.io as io
import astropy.io.fits as fits
import gzip
import time

# Importing paths for external programs
PWD=os.getcwd()
PROGS = np.loadtxt(PWD + "/progs_XPSM1530.ini", delimiter="=", dtype={'names': ('variabel', 'path'), 'formats': ('S50', 'S250')})
for i in range(len(PROGS)):
  if PROGS[i][0] == "P_ASCTOLDAC":
    P_ASCTOLDAC = PROGS[i][1]
  if PROGS[i][0] == "P_LDACTOSKYCAT":
    P_LDACTOSKYCAT = PROGS[i][1]
  if PROGS[i][0] == "P_LDACCALC":
    P_LDACCALC = PROGS[i][1]
  if PROGS[i][0] == "P_LDACFILTER":
    P_LDACFILTER = PROGS[i][1]
  if PROGS[i][0] == "P_LDACADDKEY":
    P_LDACADDKEY = PROGS[i][1]
  if PROGS[i][0] == "P_LDACTOASC":
    P_LDACTOASC = PROGS[i][1]

# Reading command line arguments
PATH = sys.argv[1]
TYPES = ['STANDARD', 'SCIENCE', 'SCIENCESHORT']
SAVETOPATH = sys.argv[2]
CATALOG = sys.argv[3]
FILTERS = ['r_SDSS', 'g_SDSS', 'u_SDSS', 'i_SDSS', 'z_SDSS']
CAMERA = sys.argv[4]
#CAMERA = 'OMEGACAM@VST'
REFCAT = sys.argv[5]
#REFCAT = 'SDSSDR10'

# Some configuration
GETSDSS="/vol/science01/scratch/dklaes/data/SDSSR9_query/SDSSR7_objects.py"

CAMERAS = np.loadtxt("cameras.ini", delimiter="\t", dtype={'names': ('THELI_name', 'RA_name', 'DEC_name', 'EXPTIME_name', 'OBJECT_name',  'DARK_name', 'DOMEFLAT_name', 'SKYFLAT_name', 'FOV_x_deg', 'FOV_y_deg'), 'formats': ('S50', 'S10', 'S10', 'S10', 'S10', 'S10', 'S10', 'S10', 'S50', 'S50')})

RA=''
DEC=''
for i in range(len(CAMERAS)):
  if CAMERAS[i][0] == CAMERA:
    RA = CAMERAS[i][1]
    DEC = CAMERAS[i][2]
    EXPTIME = CAMERAS[i][3]
    OBJECT = CAMERAS[i][4]
    DARK = CAMERAS[i][5]
    DOMEFLAT = CAMERAS[i][6]
    SKYFLAT = CAMERAS[i][7]
    FOVX = float(CAMERAS[i][8])
    FOVY = float(CAMERAS[i][9])

if RA == '':
  print("No RA keyword for " + CAMERA + " found!")
elif DEC == '':
  print("No DEC keyword for " + CAMERA + " found!")
elif EXPTIME == '':
  print("No EXPTIME keyword for " + CAMERA + " found!")
elif DARK == '':
  print("No DARK keyword for " + CAMERA + " found!")
elif DOMEFLAT == '':
  print("No DOMEFLAT keyword for " + CAMERA + " found!")
elif SKYFLAT == '':
  print("No SKYFLAT keyword for " + CAMERA + " found!")
elif FOVX == '':
  print("No field of view keyword in x direction for " + CAMERA + " found!")
elif FOVY == '':
  print("No field of view keyword in y direction for " + CAMERA + " found!")

array = []

# Creating list with all images and paths.
FAIL = 0
FAILFILES = ""
for i in range(len(FILTERS)):
  if os.path.isdir(PATH + "/" + FILTERS[i]):
	  os.chdir(PATH + "/" + FILTERS[i])
	  LIST = os.listdir(os.getcwd())
	  
	  for j in range(len(LIST)):
	    print("Getting coordinates from filter " + FILTERS[i] + " in directory " + LIST[j] + "...", end='\r')
	    
	    for k in range(len(TYPES)):
	      if os.path.isdir(os.getcwd() + "/" + LIST[j] + "/" + TYPES[k] + "_" + FILTERS[i] + "/ORIGINALS/"):
		      os.chdir(os.getcwd() + "/" + LIST[j] + "/" + TYPES[k] + "_" + FILTERS[i] + "/ORIGINALS/")
		      FILES = os.listdir(os.getcwd())
		      
		      for l in range(len(FILES)):
			if os.path.exists(os.getcwd() + "/" + FILES[l]):
			  array.append(os.getcwd() + "/" + FILES[l])
			else:
			  FAIL = FAIL + 1
			  FAILFILES = str(FAILFILES + "\n" + os.getcwd() + "/" + FILES[l])
		      os.chdir(PATH + "/" + FILTERS[i])
print(" "*200,end='\r')
if len(array) == 1:
  if FAIL == 1:
    print("File list created. Got " + str(len(array)) + " file in total, " + str(FAIL) + " file missing.")
else:
  if FAIL == 1:
    print("File list created. Got " + str(len(array)) + " files in total, " + str(FAIL) + " file missing.")
  else:
    print("File list created. Got " + str(len(array)) + " files in total, " + str(FAIL) + " files missing.")
if FAIL > 0:
  print("The following files are missing:\n")
  print(FAILFILES)



# Extract RA and DEC from all images.
array2 = np.array([])
NOCOORDS = 0
NOCOORDSFILES = ""
for i in range(len(array)):
  print("Grepping coordinates " + "{:7.0f}".format(i+1) + "/" + str(len(array)) + " ({:2.2f}".format(float(i)/float(len(array))*100) + "%)...", end='\r')
  file = fits.open(array[i])

  # Check if header keywords exist in files.
  if RA in file[0].header:
	if DEC in file[0].header:
		if EXPTIME in file[0].header:
			if float(file[0].header[EXPTIME]) > 0:
				if file[0].header[OBJECT] != DARK:
					if file[0].header[OBJECT] != DOMEFLAT:
						if file[0].header[OBJECT] != SKYFLAT:
							RAVAL = float(file[0].header[RA])
							DECVAL = float(file[0].header[DEC])
							array2 = np.append(array2,(RAVAL,RAVAL,DECVAL,DECVAL))
						else:
							NOCOORDS = NOCOORDS + 1
							NOCOORDSFILES = str(NOCOORDSFILES + "\n" + array[i] + " : Reason: Image is a SKYFLAT!")
					else:
						NOCOORDS = NOCOORDS + 1
						NOCOORDSFILES = str(NOCOORDSFILES + "\n" + array[i] + " : Reason: Image is a DOMEFLAT!")
				else:
					NOCOORDS = NOCOORDS + 1
					NOCOORDSFILES = str(NOCOORDSFILES + "\n" + array[i] + " : Reason: Image is a DARK!")
			else:
				NOCOORDS = NOCOORDS + 1
				NOCOORDSFILES = str(NOCOORDSFILES + "\n" + array[i] + " : Reason: EXPTIME argument is smaller or equal zero!")
		else:
			NOCOORDS = NOCOORDS + 1
			NOCOORDSFILES = str(NOCOORDSFILES + "\n" + array[i] + " : Reason: EXPTIME argument missing in header!")
	else:
		NOCOORDS = NOCOORDS + 1
		NOCOORDSFILES = str(NOCOORDSFILES + "\n" + array[i] + " : Reason: DEC argument missing in header!")
  else:
	NOCOORDS = NOCOORDS + 1
	NOCOORDSFILES = str(NOCOORDSFILES + "\n" + array[i] + " : Reason: RA argument missing in header!")
  file.close()
array2 = array2.reshape((-1,4))
print(" "*200,end='\r')
if len(array2) == 1:
  print("Coordinate list created. Got " + str(len(array2)) + " coordinate, " + str(NOCOORDS) + " missing.\n")
else:
  print("Coordinate list created. Got " + str(len(array2)) + " coordinates, " + str(NOCOORDS) + " missing.\n")

def unique(a):
    order = np.lexsort(a.T)
    a = a[order]
    diff = np.diff(a, axis=0)
    ui = np.ones(len(a), 'bool')
    ui[1:] = (diff != 0).any(axis=1) 
    return a[ui]

array3 = unique(array2)

print(str(len(array3)) + "/" + str(len(array2)) + " coordinates are unique.\n")

if os.path.exists (PWD + "/old_coordinates.csv"):
	OLDPOINTINGS = (np.loadtxt(PWD + "/old_coordinates.csv", delimiter=" ")).reshape((-1,4))
else:
	print("No old coordinate file available!\n")
	OLDPOINTINGS = np.array([])

TODOWNLOAD = np.array([])
for i in range(len(array3)):
	AVAILABLE=0

	# Check if coordinates were already downloaded.
	for j in range(len(OLDPOINTINGS)):
		if np.array_equal(array3[i], OLDPOINTINGS[j]):
			AVAILABLE=1

	# If the coordinates have not been downloaded yet, check if they are already in the to-download-list.
	if AVAILABLE == 0:
		for k in range(len(TODOWNLOAD.reshape((-1,4)))):
			if np.array_equal(array3[i], TODOWNLOAD.reshape((-1,4))[k]):
				AVAILABLE=1

	if AVAILABLE == 0:
		TODOWNLOAD = np.append(TODOWNLOAD, array3[i])


TODOWNLOAD2 = TODOWNLOAD.reshape((-1,4))
print(str(len(array3)-len(TODOWNLOAD2)) + "/" + str(len(array3)) + " unique coordinates are already downloaded.\n")

if len(TODOWNLOAD2) == 0:
	print("No new coordinates found for downloading. Exiting!")
	exit()

print("Downloading " + str(len(TODOWNLOAD2)) + " new coordinates...\n")

# Calculating rectangle
SIZE = np.array(len(TODOWNLOAD2)*[-FOVX/2.0, FOVX/2.0, -FOVY/2.0, FOVY/2.0])
SIZE2 = SIZE.reshape((-1,4))

array4 = TODOWNLOAD2+SIZE2


# This order has to be like this!
for i in range(len(array4)):
  if array4[i][2] < -90.0:
    array4[i][2] = -180.0 - array4[i][2]
    array4[i][0] = array4[i][0] + 180.0
  if array4[i][3] > 90.0:
    array4[i][3] = 180.0-array4[i][3]
    array4[i][1] = array4[i][1] + 180.0
  if array4[i][0] < 0.0:
    array4[i][0] = array4[i][0] + 360.0
  if array4[i][1] > 360.0:
    array4[i][1] = array4[i][1] - 360.0

np.savetxt(PWD + "/old_coordinates.csv", (np.append(OLDPOINTINGS, array4)).reshape((-1,4)), delimiter=" ")

# Downloading catalogs
for i in range(len(array4)):
  print("Downloading area " + "{:7.0f}".format(i+1) + "/" + str(len(array4)) + " ({:2.2f}".format(float(i)/float(len(array4))*100) + "%) (RA: " + "{:10.6f}".format(array4[i][0]) + " -> " + "{:10.6f}".format(array4[i][1]) + "; DEC: " + "{:10.6f}".format(array4[i][2]) + " -> " + "{:10.6f}".format(array4[i][3]) + ")...", end='\r')
  os.popen("python " + PWD + "/SDSS_dataquery.py " + REFCAT + " STARS " + str(array4[i][0]) + " " + str(array4[i][1]) + " " + str(array4[i][2]) + " " + str(array4[i][3]) + " > " + PWD + "/catalog_" + str(array4[i][0]) + "_" + str(array4[i][1])+ "_" + str(array4[i][2]) + "_" + str(array4[i][3]) + ".csv")
  time.sleep(1)
print(" "*200,end='\r')
print("Download complete. Got " + str(len(array4)) + " areas.\n")

# Create asctoldac config file
ENTRIES = np.loadtxt(PWD + "/entries.conf", delimiter="\t", dtype={'names': ('SDSS_name', 'select', 'catalog', 'catalog_name', 'TTYPE', 'HTYPE', 'COMM', 'UNIT', 'DEPTH'), 'formats': ('S50', 'S5', 'S5', 'S10', 'S10', 'S10', 'S50', 'S10', 'int16')})

config = open(PWD + "/asctoldac_tmp.conf","w")
config.write("VERBOSE = DEBUG\n")
config.write("#\n")

for i in range(len(ENTRIES)):
  if ENTRIES[i][2] == "True":
    config.write("COL_NAME = " + ENTRIES[i][3] + "\n")
    config.write("COL_TTYPE = " + ENTRIES[i][4] + "\n")
    config.write("COL_HTYPE = " + ENTRIES[i][5] + "\n")
    config.write("COL_COMM = " + ENTRIES[i][6] + "\n")
    config.write("COL_UNIT = " + ENTRIES[i][7] + "\n")
    config.write("COL_DEPTH = " + str(ENTRIES[i][8]) + "\n")
    config.write("#\n")
config.close()


if os.path.exists (PWD + "/" + CATALOG + "_raw.asc"):
	os.popen("cat " + PWD + "/" + CATALOG + "_raw.asc > " + PWD + "/catalog.tmp")
else:
	print("No old catalog available.")
os.popen("cat " + PWD + "/catalog_*.csv >> " + PWD + "/catalog.tmp")

print("Getting rid of doubled objects...")

os.popen("awk '{if (a[$0]==0) {a[$0]=1; print}}' " + PWD + "/catalog.tmp | sed -ne '/^[[:digit:]]/p' | awk '{print $0, '" + REFCAT + "'}' > " + PWD + "/catalog.tmp2")

print("Converting asc to ldac...")
os.popen(P_ASCTOLDAC + " -i " + PWD + "/catalog.tmp2 -o " + PWD + "/catalog.tmp3 -c " + PWD + "/asctoldac_tmp.conf -t STDTAB -b 1 -n 'sdss ldac cat'")
os.popen(P_LDACCALC + " -i " + PWD + "/catalog.tmp3 -o " + PWD + "/catalog.tmp4 -t STDTAB \
			-c '(umag-gmag);' -n umg '' -k FLOAT \
			-c '(sqrt((uerr*uerr)+(gerr*gerr)));' -n umgerr '' -k FLOAT \
			-c '(umag-rmag);' -n umr '' -k FLOAT \
			-c '(sqrt((uerr*uerr)+(rerr*rerr)));' -n umrerr '' -k FLOAT \
			-c '(umag-imag);' -n umi '' -k FLOAT \
			-c '(sqrt((uerr*uerr)+(ierr*ierr)));' -n umierr '' -k FLOAT \
			-c '(umag-zmag);' -n umz '' -k FLOAT \
			-c '(sqrt((uerr*uerr)+(zerr*zerr)));' -n umzerr '' -k FLOAT \
			-c '(gmag-umag);' -n gmu '' -k FLOAT \
			-c '(sqrt((gerr*gerr)+(uerr*uerr)));' -n gmuerr '' -k FLOAT \
			-c '(gmag-rmag);' -n gmr '' -k FLOAT \
			-c '(sqrt((gerr*gerr)+(rerr*rerr)));' -n gmrerr '' -k FLOAT \
			-c '(gmag-imag);' -n gmi '' -k FLOAT \
			-c '(sqrt((gerr*gerr)+(ierr*ierr)));' -n gmierr '' -k FLOAT \
			-c '(gmag-zmag);' -n gmz '' -k FLOAT \
			-c '(sqrt((gerr*gerr)+(zerr*zerr)));' -n gmzerr '' -k FLOAT \
			-c '(rmag-umag);' -n rmu '' -k FLOAT \
			-c '(sqrt((rerr*rerr)+(uerr*uerr)));' -n rmuerr '' -k FLOAT \
			-c '(rmag-gmag);' -n rmg '' -k FLOAT \
			-c '(sqrt((rerr*rerr)+(gerr*gerr)));' -n rmgerr '' -k FLOAT \
			-c '(rmag-imag);' -n rmi '' -k FLOAT \
			-c '(sqrt((rerr*rerr)+(ierr*ierr)));' -n rmierr '' -k FLOAT \
			-c '(rmag-zmag);' -n rmz '' -k FLOAT \
			-c '(sqrt((rerr*rerr)+(zerr*zerr)));' -n rmzerr '' -k FLOAT \
			-c '(imag-umag);' -n imu '' -k FLOAT \
			-c '(sqrt((ierr*ierr)+(uerr*uerr)));' -n imuerr '' -k FLOAT \
			-c '(imag-gmag);' -n img '' -k FLOAT \
			-c '(sqrt((ierr*ierr)+(gerr*gerr)));' -n imgerr '' -k FLOAT \
			-c '(imag-rmag);' -n imr '' -k FLOAT \
			-c '(sqrt((ierr*ierr)+(rerr*rerr)));' -n imrerr '' -k FLOAT \
			-c '(imag-zmag);' -n imz '' -k FLOAT \
			-c '(sqrt((ierr*ierr)+(zerr*zerr)));' -n imzerr '' -k FLOAT")

os.popen(P_LDACADDKEY + " -i " + PWD + "/catalog.tmp4 -o " + PWD + "/" + CATALOG + ".cat -t STDTAB \
			-k Epoch 2000.0 FLOAT '' n 0 SHORT '' m 0 SHORT '' A_WCS 0.0002 FLOAT '' \
			B_WCS 0.0002 FLOAT '' THETAWCS 0.0 FLOAT '' Flag 0 SHORT ''")

print("Creating skycat file...")
SKYCATCONFIG=open(PWD + "/skycat.conf", "r")
SKYCAT = [i for i in SKYCATCONFIG.readlines()]
SKYCAT = map(lambda s: s.strip(), SKYCAT)
os.popen(P_LDACTOSKYCAT + " -i " + PWD + "/" + CATALOG + ".cat -t STDTAB -k " + SKYCAT[1] + " -l " + SKYCAT[3] + " > " + PWD + "/" + CATALOG + ".skycat")

print("Creating ASCII file...")
ASCIICONFIG=open(PWD + "/ASCII.conf", "r")
ASCII = [i for i in ASCIICONFIG.readlines()]
ASCII = map(lambda s: s.strip(), ASCII)

ASCII2 = str(ASCII[1])
for i in range(len(ASCII)-2):
  ASCII2 = str(ASCII2 + " " + ASCII[i+2])
os.popen(P_LDACTOASC + " -s -i " + PWD + "/" + CATALOG + ".cat -t STDTAB -k " + str(ASCII2[::]) + " > " + PWD + "/" + CATALOG + ".asc")

compressed = gzip.open(PWD + "/" + CATALOG + ".asc.gz", "wb")
os.popen(P_LDACTOASC + " -s -i " + PWD + "/" + CATALOG + ".cat -t STDTAB -k " + str(ASCII2[::]) + " > " + PWD + "/" + CATALOG + ".asc")
data = open(PWD + "/" + CATALOG + ".asc","r")
data1 = [i for i in data.readlines()]
for i in range(len(data1)):
  compressed.write(data1[i])
compressed.close()


# Create DS9 region file with circles with a radius of 50 pixels.
f = open(PWD + "/" + CATALOG + ".reg", "w")
f.write("# Region file format: DS9 version 4.1\n")
f.write("global color=green dashlist=8 3 width=1 font=\"helvetica 10 normal roman\" select=1 highlite=1 dash=0 fixed=0 edit=1 move=1 delete=1 include=1 source=1\n")
f.write("fk5\n")
f.close()
os.popen(P_LDACTOASC + " -b -i " + PWD + "/" + CATALOG + ".cat -t STDTAB -k Ra Dec | awk '{print \"circle(\" $1 \",\" $2 \",50i)\"}' >> " + PWD + "/" + CATALOG + ".reg")

os.rename(PWD + "/catalog.tmp", PWD + "/" + CATALOG + "_raw.asc")

# Remove temp files.
os.remove(PWD + "/catalog.tmp2")
os.remove(PWD + "/catalog.tmp3")
os.remove(PWD + "/catalog.tmp4")
os.remove(PWD + "/asctoldac_tmp.conf")
for i in range(len(TODOWNLOAD2)):
  os.remove(PWD + "/catalog_" + str(array4[i][0]) + "_" + str(array4[i][1]) + "_" + str(array4[i][2]) + "_" + str(array4[i][3]) + ".csv")
