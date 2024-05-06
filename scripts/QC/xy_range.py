import numpy as np
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

base = sys.argv[1]
band = sys.argv[2]

data = np.loadtxt(base + ".asc", usecols=(1,2,3,4))

Delta_x = data[:,0] - data[:,2]

fig, ax = plt.subplots(1)
#plt.xscale('log')
#plt.yscale('log')
plt.title(r"tile-wise $\Delta x$-range $"+band+"$-band")
ax.set_xlabel(r"$\Delta x$")
ax.set_ylabel("#tiles")
ax.xaxis.labelpad = -1
ax.yaxis.labelpad = -2
ax.hist(Delta_x, bins=100) #, log=True)
plt.savefig(base+"_x.png")

fig, ax = plt.subplots(1)
#plt.xscale('log')
#plt.yscale('log')
plt.title(r"tile-wise $\Delta x$-range $"+band+"$-band")
ax.set_xlabel(r"$\Delta x$")
ax.set_ylabel("#tiles")
ax.xaxis.labelpad = -1
ax.yaxis.labelpad = -2
ax.hist(Delta_x, bins=100, log=True)
plt.savefig(base+"_x_log.png")

Delta_y = data[:,1] - data[:,3]

fig, ax = plt.subplots(1)
#plt.xscale('log')
#plt.yscale('log')
plt.title(r"tile-wise $\Delta y$-range $"+band+"$-band")
ax.set_xlabel(r"$\Delta y$")
ax.set_ylabel("#tiles")
ax.xaxis.labelpad = -1
ax.yaxis.labelpad = -2
ax.hist(Delta_y, bins=100) #, log=True)
plt.savefig(base+"_y.png")

fig, ax = plt.subplots(1)
#plt.xscale('log')
#plt.yscale('log')
plt.title(r"tile-wise $\Delta y$-range $"+band+"$-band")
ax.set_xlabel(r"$\Delta y$")
ax.set_ylabel("#tiles")
ax.xaxis.labelpad = -1
ax.yaxis.labelpad = -2
ax.hist(Delta_y, bins=100, log=True)
plt.savefig(base+"_y_log.png")
