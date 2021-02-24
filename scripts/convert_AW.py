from astropy.table import Table
import numpy as np
import astropy.io.fits as fits
import astropy
import sys
import string

catname = sys.argv[1]
outcatname = sys.argv[2]
asctoldac_file = sys.argv[3]

cat = Table.read(catname)

formats = {}

f = open(asctoldac_file, "w")

t = Table()

col_file = open("@CATALOGUEKEYSFILE@")

for line in col_file:
    if len(string.split(line))>=3 and string.split(line)[0]!="#":
        #print line
        COL_NAME_old = string.split(line, ',')[0]
        COL_NAME_old = COL_NAME_old.replace('\t','')
        COL_NAME_old = COL_NAME_old.replace(' ','')
        COL_NAME = string.split(line, ',')[1]
        COL_NAME = COL_NAME.replace('\t','')
        COL_NAME = COL_NAME.replace(' ','')
        COL_COMM = string.split(line, ',')[2]
        COL_COMM = COL_COMM.replace('"','')
        COL_COMM = COL_COMM.rstrip()
        COL_UNIT = string.split(line, ',')[3]
        COL_UNIT = COL_UNIT.replace('"','')
        COL_UNIT = COL_UNIT.rstrip()
        if COL_UNIT == "":
            COL_UNIT = " "
        column_format = str(cat[COL_NAME_old].dtype)
        t[COL_NAME] = cat[COL_NAME_old]
        if column_format == ">f4":
            COL_TTYPE = "FLOAT"
            COL_HTYPE = "FLOAT"
            COL_DEPTH = "1"
            formats[COL_NAME]="%e"
        elif COL_NAME == "FLAG" or COL_NAME == "Flag" or COL_NAME == "FLAG_GAAP_u" or \
                COL_NAME == "FLAG_GAAP_g" or COL_NAME == "FLAG_GAAP_r" or COL_NAME == "FLAG_GAAP_i":
            COL_TTYPE = "SHORT"
            COL_HTYPE = "INT"
            COL_DEPTH = "1"
            formats[COL_NAME]="%i"
        elif COL_NAME == "SeqNr":
            COL_TTYPE = "LONG"
            COL_HTYPE = "INT"
            COL_DEPTH = "1"
            formats[COL_NAME]="%i"
        elif column_format == ">f8":
            COL_TTYPE = "DOUBLE"
            COL_HTYPE = "FLOAT"
            COL_DEPTH = "1"
            formats[COL_NAME]="%3.6f"
        elif column_format == ">i8":
            COL_TTYPE = "LONG"
            COL_HTYPE = "INT"
            COL_DEPTH = "1"
            formats[COL_NAME]="%i"
        elif column_format == ">i4":
            COL_TTYPE = "SHORT"
            COL_HTYPE = "INT"
            COL_DEPTH = "1"
            formats[COL_NAME]="%i"
        elif column_format == "|S25":
            COL_TTYPE = "STRING"
            COL_HTYPE = "STRING"
            COL_DEPTH = "25"
            formats[COL_NAME]="%s"
        f.write('COL_NAME = '+COL_NAME+'\n')
        f.write('COL_TTYPE = '+COL_TTYPE+'\n')
        f.write('COL_HTYPE = '+COL_HTYPE+'\n')
        f.write('COL_COMM = "'+COL_COMM+'"'+'\n')
        f.write('COL_UNIT = "'+COL_UNIT+'"'+'\n')
        f.write('COL_DEPTH = '+COL_DEPTH+'\n')
        f.write('#'+'\n')

f.close()

col_file.close()

t.write(outcatname, format='ascii', formats=formats)
