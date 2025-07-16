import numpy as np
from astropy.stats import mad_std
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt
import sys

md = sys.argv[1]
ending = sys.argv[2]
survey = sys.argv[3]

plt.rc('font', size=15)

bins = np.arange(-5.,5.,0.005)
bins_fine = np.arange(-5.,5.,0.005)

print("# band reference unit mean std median NMAD")

unit="tile"
star=""
for band in ("u", "g", "r", "i", "z", "z2"):
    cat = np.loadtxt(md+band+"_"+survey+star+"_offsets_"+unit+".asc")
    medians = cat[:,3]
    NMADs = cat[:,4]
    means = cat[:,5]
    stds = cat[:,6]
    Ns = cat[:,7]
    
    number_filter = np.greater(Ns, 10)
    unimodal_filter = np.less(np.abs(medians-means),0.02)
    all_filter = number_filter * unimodal_filter
    #if band == "i":
    #    i_filter = np.less(medians,0.1)
    #    all_filter = all_filter * i_filter
    
    mean =  "%.3f" % np.mean(medians[all_filter])
    median = "%.3f" % np.median(medians[all_filter])
    std = "%.3f" % np.std(medians[all_filter])
    NMAD = "%.3f" % mad_std(medians[all_filter])
    N = "%i" % np.sum(all_filter)
    print(band, survey, star, unit, mean, std, median, NMAD, N)
    fig, ax = plt.subplots()
    #ax.set_xlim(-0.49,0.49)
    #ax.xaxis.set_ticks(np.arange(-0.3,0.2,0.1))
    ax.set_xlim(-0.09,0.09)
    ax.xaxis.set_ticks(np.arange(-0.09,0.09,0.03))
    #ax.hist(medians[all_filter]-np.median(medians[all_filter]), bins=bins)
    ax.hist(medians[all_filter], bins=bins)
    ax.xaxis.labelpad = -1
    ax.yaxis.labelpad = -2
    #ax.set_xlabel(r"$\Delta "+band+" - \mathrm{med}(\Delta "+band+")$")
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
    ax.text(0.05, 0.5, '$N='+N+'$',
            horizontalalignment='left',
            verticalalignment='top',
            transform=ax.transAxes)
    ylim = ax.get_ylim()
    plt.plot((0., 0.), (0., ax.get_ylim()[1]), 'k--')
    ax.set_ylim(ylim)
    plt.savefig(md+band+"_"+survey+star+"_offsets_"+unit+'.png')
    plt.close()
