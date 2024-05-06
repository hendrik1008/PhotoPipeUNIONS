import numpy as np
import sys

base = sys.argv[1]

data = np.loadtxt(base+".asc")

f = open(base+"_SL.txt", "w")
f.write(str(np.mean(data[:,5]))+" "+str(np.std(data[:,5]))+"\n")
f.close()
