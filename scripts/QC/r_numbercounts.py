import numpy as np
import astropy.io.fits as pyfits
import astropy.stats
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

catalogue_file = sys.argv[1]
z_phot_key = sys.argv[2]
r_mag_key = sys.argv[3]
ZBmax = float(sys.argv[4])
rmax = float(sys.argv[5])
label = sys.argv[6]
outbase = sys.argv[7]

catalogue = pyfits.open(catalogue_file)
catdata_tmp = catalogue[1].data
ZBmask = np.less(catdata_tmp.field(z_phot_key),ZBmax)
rmask = np.less(catdata_tmp.field(r_mag_key),rmax)
catdata = catdata_tmp[ZBmask*rmask]
z_phot = catdata.field(z_phot_key)
r_mag = catdata.field(r_mag_key)

r_hist = np.histogram(r_mag, bins=20, range=(15.,25.), normed=True)
np.savetxt(outbase+".txt",np.transpose(np.stack((r_hist[1][:-1],r_hist[1][1:],r_hist[0]))))

fig, ax = plt.subplots(1)
plt.title(label)
ax.set_xlabel(r"$r$")
ax.set_ylabel(r"$N(z)$")
#ax.set_xlim(0.,1.95)
#ax.set_ylim(0.,6.95)
#ax.xaxis.labelpad = -1
ax.hist(r_mag, bins=20, range=(15.,25.), log=True)
#ax.text(4.2, 0.45, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(outbase+".png")
