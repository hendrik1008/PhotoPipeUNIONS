import numpy as np
import astropy.io.fits as pyfits
import astropy.stats
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

wd = sys.argv[1]
tile = sys.argv[2]
band = sys.argv[3]

MAGZP=30.
if band == "g" or band == "z":
    MAGZP=27.

cat_fname = wd + "/" + tile + "_" + band + "_cat.asc"
starcat_fname = wd + "/" + tile + "_" + band + "_star_cat_GAaP.asc"
out_fname = wd + "/" + tile + "_" + band + "_chimney.png"

cat = np.loadtxt(cat_fname)
starcat = np.loadtxt(starcat_fname)

flags = (cat[:,9] <= 4)
starflags = (starcat[:,9] <= 4)

fig, ax = plt.subplots(1)
plt.title(r" "+tile+"    $"+band+"$")
#plt.gca().set_aspect('equal', adjustable='box')
ax.set_xlabel(r"FLUX_RADIUS")
ax.set_ylabel(r"$"+band+"$")
ax.set_xlim(0.,6.5)
ax.set_ylim(24.,14.)
#ax.xaxis.labelpad = -1
ax.scatter(cat[:,4][flags],-2.5*np.log10(cat[:,2][flags])+MAGZP, marker=".", s=15.) #, alpha=0.01, edgecolors="none")
ax.scatter(starcat[:,4][starflags],-2.5*np.log10(starcat[:,2][starflags])+MAGZP, marker=".", s=15.) #, alpha=0.01, s=15., edgecolors="none")
#ax.plot([0.,2.],[0.,2.], color="black")
#ax.plot([0.,2.],[0.15,2.45], "k:")
#ax.plot([0.15,2.],[0.,1.55], "k:")
#ax.text(1.2, 0.45, r'bias$ = $'+bias)
#ax.text(1.2, 0.35, r'NMAD$ = $'+NMAD)
#ax.text(1.2, 0.25, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(out_fname)
