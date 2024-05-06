import numpy as np
import astropy.io.fits as fits
from astropy.wcs import WCS
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm

#plt.rc('font', size=15)

wd = sys.argv[1]
tile = sys.argv[2]
band = sys.argv[3]

image_fname = wd + "/" + tile + "_" + band + ".fits"
out_fname = wd + "/" + tile + "_" + band + ".png"
weight_fname = wd + "/" + tile + "_" + band + ".weight.fits"
weightout_fname = wd + "/" + tile + "_" + band + ".weight.png"

image = fits.open(image_fname)[0]
wcs = WCS(image.header)

ax = plt.subplot(projection=wcs)
plt.title(r" "+tile+"    $"+band+"$")
ax.imshow(image.data, norm=LogNorm(vmin=0.01,vmax=1.))
#plt.gca().set_aspect('equal', adjustable='box')
ax.set_xlabel(r"RA")
ax.set_ylabel(r"Dec")
#ax.set_xlim(0.,6.5)
#ax.set_ylim(24.,14.)
#ax.xaxis.labelpad = -1
#ax.scatter(cat[:,4][flags],-2.5*np.log10(cat[:,2][flags])+MAGZP, marker=".", s=15.) #, alpha=0.01, edgecolors="none")
#ax.scatter(starcat[:,4][starflags],-2.5*np.log10(starcat[:,2][starflags])+MAGZP, marker=".", s=15.) #, alpha=0.01, s=15., edgecolors="none")
#ax.plot([0.,2.],[0.,2.], color="black")
#ax.plot([0.,2.],[0.15,2.45], "k:")
#ax.plot([0.15,2.],[0.,1.55], "k:")
#ax.text(1.2, 0.45, r'bias$ = $'+bias)
#ax.text(1.2, 0.35, r'NMAD$ = $'+NMAD)
#ax.text(1.2, 0.25, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(out_fname)
