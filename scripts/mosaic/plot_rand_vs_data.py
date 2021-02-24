import numpy as np
import astropy.io.fits as fits
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

rand_file = sys.argv[1]
data_file = sys.argv[2]
out_file  = sys.argv[3]
patch = sys.argv[4]

rand = fits.open(rand_file)
data = fits.open(data_file)

RA_rand = rand[1].data.field('ALPHA_J2000')
Dec_rand = rand[1].data.field('DELTA_J2000')

RA_data = data[1].data.field('ALPHA_J2000')
Dec_data = data[1].data.field('DELTA_J2000')

no_objects_rand = np.shape(RA_rand)[0]
no_objects_data = np.shape(RA_data)[0]

RA_rand_short  = RA_rand[np.arange(1,no_objects_rand,500)]
Dec_rand_short = Dec_rand[np.arange(1,no_objects_rand,500)]

RA_data_short  = RA_data[np.arange(1,no_objects_data,100)]
Dec_data_short = Dec_data[np.arange(1,no_objects_data,100)]

print np.shape(RA_rand_short)
print np.shape(Dec_rand_short)
print np.shape(RA_data_short)
print np.shape(Dec_data_short)

if patch == "G23":
    RA_rand_filter=np.less(RA_rand_short,50.)
    RA_rand_short[RA_rand_filter]=RA_rand_short[RA_rand_filter]+360.
    RA_data_filter=np.less(RA_data_short,50.)
    RA_data_short[RA_data_filter]=RA_data_short[RA_data_filter]+360.

fig, ax = plt.subplots(1)
plt.title(patch)
ax.set_xlabel(r"RA [deg]")
ax.set_ylabel(r"Dec [deg]")
#ax.set_xlim(0.,1.95)
#ax.set_ylim(0.,1.95)
#ax.xaxis.labelpad = -1
#ax.scatter(RA_rand_short,Dec_rand_short, s=1., marker=",", edgecolors="none", alpha=0.7, color="red")
#ax.scatter(RA_data_short,Dec_data_short, s=1., marker=",", edgecolors="none", color="blue")
ax.scatter(RA_rand,Dec_rand, s=1., marker=",", edgecolors="none", alpha=0.7, color="red")
ax.scatter(RA_data,Dec_data, s=1., marker=",", edgecolors="none", color="blue")
plt.savefig(out_file)
