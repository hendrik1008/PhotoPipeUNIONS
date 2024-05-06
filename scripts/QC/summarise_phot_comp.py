import numpy as np
from astropy.stats import mad_std
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt
import sys

md = sys.argv[1] #"/net/home/fohlen13/hendrik/PhotoPipe/RUNDIR_CLEAN/work_clean_KiDS-Legacy/"
ending = sys.argv[2]

plt.rc('font', size=15)

bins = np.arange(-5.,5.,0.01)
bins_fine = np.arange(-5.,5.,0.005)

print "# band reference unit mean std median NMAD"

unit="tile"
star=""
for band in ("u", "g", "r", "i", "z"):
    survey="SDSS"
    cat = np.loadtxt(md+"phot_comp/"+band+"_"+survey+star+"_offsets_"+unit+".asc")
    medians = cat[:,0]
    Ns = cat[:,4]
    number_filter = np.greater(Ns, 10)
    mean =  "%.3f" % np.mean(medians[number_filter])
    median = "%.3f" % np.median(medians[number_filter])
    std = "%.3f" % np.std(medians[number_filter])
    NMAD = "%.3f" % mad_std(medians[number_filter])
    percentage = "%3.1f" % (np.float(np.sum(number_filter))/np.float(np.shape(medians)[0])*100.,)
    print band, survey, star, unit, mean, std, median, NMAD, percentage
    fig, ax = plt.subplots()
    #ax.set_xlim(-0.49,0.49)
    #ax.xaxis.set_ticks(np.arange(-0.3,0.2,0.1))
    ax.set_xlim(-0.19,0.19)
    ax.xaxis.set_ticks(np.arange(-0.15,0.15,0.05))
    ax.hist(medians[number_filter], bins=bins)
    ax.xaxis.labelpad = -1
    ax.set_xlabel(r"$\Delta "+band+"$")
    ax.set_ylabel(r"$N$")
    ax.text(0.05, 0.9, '$<\Delta '+band+'>='+mean+'$',
            horizontalalignment='left',
            verticalalignment='top',
            transform=ax.transAxes)
    ax.text(0.05, 0.8, '$\sigma_{\Delta '+band+'}='+std+'$',
            horizontalalignment='left',
            verticalalignment='top',
            transform=ax.transAxes)
    ax.text(0.05, 0.7, 'med$(\Delta '+band+')='+median+'$',
            horizontalalignment='left',
            verticalalignment='top',
            transform=ax.transAxes)
    ax.text(0.05, 0.6, 'NMAD$(\Delta '+band+')='+NMAD+'$',
            horizontalalignment='left',
            verticalalignment='top',
            transform=ax.transAxes)
    ax.text(0.05, 0.5, 'compl.$='+percentage+'$%',
            horizontalalignment='left',
            verticalalignment='top',
            transform=ax.transAxes)
    plt.plot((0., 0.), (0., ax.get_ylim()[1]), 'k--')
    plt.savefig(md+"phot_comp/"+band+"_"+survey+star+"_offsets_"+unit+'.png')
    plt.close()
