import numpy as np
import sys
from astropy.stats import mad_std

infile = sys.argv[1]
mag_min = float(sys.argv[2])
mag_max = float(sys.argv[3])

cat = np.loadtxt(infile)
mag = cat[:,2]
magref = cat[:,4]

filter1 = np.greater(magref,mag_min)
filter2 = np.less(magref,mag_max)
filter = filter1 * filter2

delta = mag[filter]-magref[filter]

print np.median(delta), mad_std(delta), np.mean(delta), np.std(delta), np.shape(delta)[0]
