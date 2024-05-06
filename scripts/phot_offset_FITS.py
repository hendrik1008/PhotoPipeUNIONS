import numpy as np
import sys, os
from astropy.stats import mad_std
import astropy.io.fits as fits
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

base = sys.argv[1]
infile = sys.argv[2]
band = sys.argv[3]
field = sys.argv[4]

# The following relations are based on
# https://www.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/en/megapipe/docs/filt.html (u and r)
# https://hsc.mtk.nao.ac.jp/pipedoc/pipedoc_8_e/colorterms.html (g and z)
# https://arxiv.org/abs/1203.0297 (for i)

if band == "u":
    band1 = "u"
    band2 = "g"
    A0 = -0.165
    A1 =  0.036
    mag_min = 16.
    mag_max = 19.
elif band == "g":
    band1 = "g"
    band2 = "r"
    A0 = -0.009777
    A1 = -0.077235
    mag_min = 17.
    mag_max = 19.
elif band == "r":
    band1 = "g"
    band2 = "r"
    A0 =  0.
    A1 = -0.087
    mag_min = 18.
    mag_max = 20.
elif band == "i":
    band1 = "g"
    band2 = "r"
    A0 =  0.004
    A1 = -0.014
    mag_min = 16.
    mag_max = 19.
elif band == "z":
    band1 = "z"
    band2 = "i"
    A0 = -0.005761
    A1 = 0.001317
    mag_min = 16.
    mag_max = 19.

catalogue = fits.open(infile)
catdata = catalogue[1].data
mag = catdata.field("MAG_GAAP_"+band)
magref = catdata.field(band+"_SDSS")
mag1 = catdata.field(band1+"_SDSS")
mag2 = catdata.field(band2+"_SDSS")

filter1 = np.greater(magref,0.)
filter2 = np.less(magref,99.)
filter3 = np.greater(mag1,0.)
filter4 = np.less(mag1,99.)
filter5 = np.greater(mag2,0.)
filter6 = np.less(mag2,99.)
filter7 = np.greater(mag,mag_min)
filter8 = np.less(mag,mag_max)
filter = filter1 * filter2 * filter3 * filter4 * filter5 * filter6 * filter7 * filter8

delta = mag[filter]-magref[filter] - (A0 + A1 * (mag1[filter]-mag2[filter]) )

print np.median(delta), mad_std(delta), np.mean(delta), np.std(delta), np.shape(delta)[0]

filter7 = np.greater(mag,0.)
filter8 = np.less(mag,99.)
filterall = filter1 * filter2 * filter3 * filter4 * filter5 * filter6 * filter7 * filter8

delta2 = mag[filterall]-magref[filterall] - (A0 + A1 * (mag1[filterall]-mag2[filterall]) )

fig, ax = plt.subplots(1)
plt.title(field)
ax.set_xlabel(r"$"+band+"$")
ax.set_ylabel(r"$"+band+"-"+band+"_{\mathrm{SDSS}}$")
ax.set_xlim(mag_min-1.,mag_max+1.)
ax.set_ylim(-0.7,0.7)
#ax.xaxis.labelpad = -1
ax.yaxis.labelpad = -2
ax.scatter(mag[filterall],delta2) #, s=15., alpha=0.9, marker=".", edgecolors="none")
ax.scatter(mag[filter],delta) #, s=15., alpha=0.9, marker=".", edgecolors="none")
ax.plot([mag_min-1.,mag_max+1.], [0.,0.], "k:")
#ax.plot([0.,2.],[0.15,2.45], "k:")
#ax.plot([0.15,2.],[0.,1.55], "k:")
ax.text(mag_max-2., 0.45, r'median$ = $'+str(np.round(np.median(delta),decimals=3)))
ax.text(mag_max-2., 0.35, r'NMAD$ = $'  +str(np.round(mad_std(delta),decimals=3)))
#ax.text(1.2, 0.25, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(base+"_"+band+"_offset.png")
