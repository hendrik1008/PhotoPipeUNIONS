# Script to calculate the effective area of KV450
#
# Author: Hendrik Hildebrandt (hendrik@phas.ubc.ca)
#
# Usage: 
# python 
#
# Requirements: AstroPy, Numpy
#

import astropy.io.fits as fits
import numpy as np
import sys, os
import matplotlib.pyplot as plt
from astropy.wcs import WCS
from astropy.utils.data import get_pkg_data_filename
from matplotlib.colors import LinearSegmentedColormap

image_file = sys.argv[1]
#out_file = sys.argv[2]
#RAmin = float(sys.argv[3])
#RAmax = float(sys.argv[4])
#Decmin = float(sys.argv[5])
#Decmax = float(sys.argv[6])
#region = sys.argv[7]
#out_mask = sys.argv[8]

image = fits.open(image_file)
image_data = image[0].data.astype(np.int16)
header = image[0].header
wcs = WCS(image[0].header)

mask4 = np.bitwise_and(image_data, 4)
mask16 = np.bitwise_and(image_data, 16)
mask32 = np.bitwise_and(image_data, 32)
mask1024 = np.bitwise_and(image_data, 1024)
mask2048 = np.bitwise_and(image_data, 2048)
mask4096 = np.bitwise_and(image_data, 4096)
mask8192 = np.bitwise_and(image_data, 8192)
mask16384 = np.bitwise_and(image_data, 16384)

mask_K450 = mask4 + mask16 + mask1024 + mask2048 + mask4096 + mask8192 + mask16384
mask_KV450 = mask4 + mask16 + mask32 + mask1024 + mask2048 + mask4096 + mask8192 + mask16384

print (np.shape(mask_KV450)[0] * np.shape(mask_KV450)[1] - np.count_nonzero(mask_KV450)) / 3600.
print (np.shape(mask_K450)[0] * np.shape(mask_K450)[1] - np.count_nonzero(mask_K450)) / 3600.

#plot_data = np.greater(mask_K450,0.)*5. + np.greater(mask_KV450,0.)*5.
#
#colors = [(27/255.,158/255.,119/255.), (217/255.,95/255.,2/255.), (1, 1, 1)]  # R -> G -> B
#n_bin = 3 # Discretizes the interpolation into bins
#cmap_name = 'my_list'
#cm = LinearSegmentedColormap.from_list(
#    cmap_name, colors, N=n_bin)
#
#fig = plt.figure()
#ax = fig.add_subplot(111, projection=wcs)
#plt.imshow(plot_data, origin='lower', cmap=cm) #plt.cm.viridis)
##plt.xlim(RAmax, RAmin)
##plt.ylim(Decmin, Decmax)
#plt.xlabel('RA')
#plt.ylabel('Dec')
#ax.set_title(region)
#plt.savefig(out_file)
#plt.close()    
#
#hdu=fits.PrimaryHDU(np.equal(mask_KV450,0.)*1)
#hdu.header=header
#
#if os.path.exists(out_mask):
#    os.remove(out_mask)
#hdu.writeto(out_mask)
