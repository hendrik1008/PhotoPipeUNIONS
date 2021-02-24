import numpy as np
import astropy.io.fits as fits
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

rand_file = sys.argv[1]
rand_file2 = sys.argv[2]
data_file = sys.argv[3]
out_file  = sys.argv[4]
patch = sys.argv[5]

rand = fits.open(rand_file)
rand2 = fits.open(rand_file2)
data = fits.open(data_file)

RA_rand = rand[1].data.field('ALPHA_J2000')
Dec_rand = rand[1].data.field('DELTA_J2000')

RA_rand2 = rand2[1].data.field('ALPHA_J2000')
Dec_rand2 = rand2[1].data.field('DELTA_J2000')

RA_data = data[1].data.field('ALPHA_J2000')
Dec_data = data[1].data.field('DELTA_J2000')

fig, ax = plt.subplots(1)
plt.title(patch)
ax.set_xlabel(r"RA [deg]")
ax.set_ylabel(r"Dec [deg]")
ax.scatter(RA_rand,Dec_rand, s=1., marker=",", edgecolors="none", alpha=0.7, color="red")
ax.scatter(RA_rand2,Dec_rand2, s=1., marker=",", edgecolors="none", alpha=0.7, color="blue")
ax.scatter(RA_data,Dec_data, s=1., marker=",", edgecolors="none", alpha=0.7, color="green")
plt.savefig(out_file)
