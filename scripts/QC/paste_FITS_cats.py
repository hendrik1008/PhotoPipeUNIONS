import astropy.io.fits as pyfits
import numpy as np
import os, sys

outfile = sys.argv[1]
table_name = sys.argv[2]
infiles = sys.argv[3:]

nr_infiles = 0

data_list = []
columns_list = []

for infile in infiles:
    if os.path.exists(infile):
        catalogue  = pyfits.open(infile)
        catdata    = catalogue[1].data
        data_list.append(catdata)
        catcolumns = catalogue[1].columns
        columns_list.append(catcolumns)
        nr_infiles = nr_infiles + 1
        catalogue.close()
print(nr_infiles)
nr_columns = len(columns_list[0].names)

new_cols_list = []

for i in range(nr_columns):
    array_list = []
    for j in range(nr_infiles):
        array_list.append(data_list[j].field(columns_list[0].names[i]))
    array_data = np.concatenate(array_list)
    new_col = pyfits.Column(name=columns_list[0].names[i], format=columns_list[0].formats[i], array=array_data)
    new_cols_list.append(new_col)

new_cols = pyfits.ColDefs(new_cols_list)

hdu = pyfits.BinTableHDU.from_columns(new_cols)
hdu.name=table_name

if os.path.exists(outfile):
    os.remove(outfile)
hdu.writeto(outfile)
