import numpy as np
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

base = sys.argv[1]
band = sys.argv[2]

data = np.loadtxt(base + ".asc", usecols=(3,))

fig, ax = plt.subplots(1)
#plt.xscale('log')
#plt.yscale('log')
plt.title(r"tile-wise fraction of stars used, $"+band+"$-band")
ax.set_xlabel(r"fraction")
ax.set_ylabel("#tiles")
ax.xaxis.labelpad = -1
ax.yaxis.labelpad = -2
ax.hist(data, bins=100) #, log=True)
plt.savefig(base+".png")

fig, ax = plt.subplots(1)
#plt.xscale('log')
#plt.yscale('log')
plt.title(r"tile-wise fraction of stars used, $"+band+"$-band")
ax.set_xlabel(r"fraction")
ax.set_ylabel("#tiles")
ax.xaxis.labelpad = -1
ax.yaxis.labelpad = -2
ax.hist(data, bins=100, log=True)
plt.savefig(base+"_log.png")
