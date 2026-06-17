import sys
import string
import ldac
import os
import numpy as np
from astropy.io import fits

catname = sys.argv[1]
outcat = sys.argv[2]
tablename = sys.argv[3]
keystoadd = sys.argv[4:]

#########################################################
#### LDAC version producing strings that are too long ###
#########################################################
#
#### read the input catalogue
#ldac_cat = ldac.LDACCat(catname)
#ldac_table = ldac_cat[tablename]
#nobj = np.shape(ldac_table['SeqNr'])[0]
#
## store the keys in the LDAC table
#ldac_table[keystoadd[0]] = np.full(shape=nobj, fill_value=keystoadd[1])
#ldac_table.set_comment(keystoadd[0], keystoadd[2])
#
#ldac_table[keystoadd[3]] = np.full(shape=nobj, fill_value=keystoadd[4])
#ldac_table.set_comment(keystoadd[3], keystoadd[5])
#
#
#### save the catalogue
#if os.path.exists(outcat):
#    os.remove(outcat)
#ldac_cat.saveas(outcat)

########################################################
### LDAC version producing strings that are too long ###
########################################################
hdu = fits.open(catname)

data = hdu[1].data
cols = data.columns

nobj = np.shape(data['SeqNr'])[0]

new_col  = fits.Column(name=keystoadd[0], format='14A', array=np.full(shape=nobj, fill_value=keystoadd[1]))
new_col2 = fits.Column(name=keystoadd[3], format='10A', array=np.full(shape=nobj, fill_value=keystoadd[4]))

new_cols = fits.ColDefs((new_col, new_col2))

hdu[1] = fits.BinTableHDU.from_columns(cols + new_cols)
hdu[1].name = 'OBJECTS'

# 2. Target the table extension header (typically index 1)
header = hdu[1].header

# 3. Update the comment on the exact TTYPE key for your column (e.g., TTYPE1)
# The format is: header['KEY'] = (value, comment)
header['TTYPE114'] = (keystoadd[0], keystoadd[2])
header['TTYPE115'] = (keystoadd[3], keystoadd[5])

# 4. Flush changes to save them to the file automatically
hdu.flush()

if os.path.exists(outcat):
    os.remove(outcat)
hdu.writeto(outcat)
