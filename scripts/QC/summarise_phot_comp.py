import numpy as np
from astropy.stats import mad_std
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt
import sys
import astropy.coordinates as coord
import astropy.units as u

md = sys.argv[1]
MERGE = sys.argv[2]
survey = sys.argv[3]

plt.rc('font', size=15)

bins = np.arange(-5.,5.,0.005)
bins_fine = np.arange(-5.,5.,0.005)

print("# band reference unit mean std median NMAD")

unit="tile"
star=""

bands = ("u", "g", "r", "i", "z", "z2")

if survey == "PGM":
    bands = ("g", "r", "i", "z", "z2")

if survey == "SDSS":
    bands = ("u",)

for band in bands:
    cat = np.loadtxt(md+band+"_"+survey+star+"_offsets_"+unit+"_"+MERGE+".asc")
    medians = cat[:,3]
    NMADs = cat[:,4]
    means = cat[:,5]
    stds = cat[:,6]
    Ns = cat[:,7]
    RA = cat[:,1]
    Dec = cat[:,2]
    
    number_filter = np.greater(Ns, 10)
    unimodal_filter = np.less(np.abs(medians-means),0.02)
    all_filter = number_filter #* unimodal_filter
    #if band == "i":
    #    i_filter = np.less(medians,0.1)
    #    all_filter = all_filter * i_filter

    RA = coord.Angle(RA[all_filter] * u.degree)
    Dec = coord.Angle(Dec[all_filter] * u.degree)
    
    for plot_type in "Delta",: # "DeltaMinusMedian":
        for stat in "median",: # "mean":
            if stat == "median":
                mean =  "%.3f" % np.mean(medians[all_filter])
                median = "%.3f" % np.median(medians[all_filter])
                std = "%.3f" % np.std(medians[all_filter])
                NMAD = "%.3f" % mad_std(medians[all_filter])
            elif stat == "mean":
                mean =  "%.3f" % np.mean(means[all_filter])
                median = "%.3f" % np.median(means[all_filter])
                std = "%.3f" % np.std(means[all_filter])
                NMAD = "%.3f" % mad_std(means[all_filter])
            N = "%i" % np.sum(all_filter)
            if plot_type == "Delta" and stat == "median":
                print(band, survey, stat, unit, mean, std, median, NMAD, N)
            fig, ax = plt.subplots()
            ax.set_xlim(-0.49,0.49)
            #ax.xaxis.set_ticks(np.arange(-0.3,0.2,0.1))
            #ax.set_xlim(-0.09,0.09)
            #ax.xaxis.set_ticks(np.arange(-0.09,0.09,0.03))
            if stat == 'median':
                if plot_type == "DeltaMinusMedian":
                    ax.hist(medians[all_filter]-np.median(medians[all_filter]), bins=bins)
                    ax.set_xlabel(r"$\Delta "+band+" - \mathrm{med}(\Delta "+band+")$")
                elif plot_type == "Delta":
                    ax.hist(medians[all_filter], bins=bins)
                    ax.set_xlabel(r"$\Delta "+band+"$")
            elif stat == 'mean':
                if plot_type == "DeltaMinusMedian":
                    ax.hist(means[all_filter]-np.median(means[all_filter]), bins=bins)
                    ax.set_xlabel(r"$\Delta "+band+" - \mathrm{med}(\Delta "+band+")$")
                elif plot_type == "Delta":
                    ax.hist(means[all_filter], bins=bins)
                    ax.set_xlabel(r"$\Delta "+band+"$")
            ax.text(0.05, 0.9, '$<\Delta '+band+'>='+mean+'$',
                    horizontalalignment='left',
                    verticalalignment='top',
                    transform=ax.transAxes)
            ax.text(0.05, 0.8, 'med$(\Delta '+band+')='+median+'$',
                    horizontalalignment='left',
                    verticalalignment='top',
                    transform=ax.transAxes)
            ax.xaxis.labelpad = -1
            ax.yaxis.labelpad = -2
            ax.set_ylabel(r"$N$")
            ax.text(0.05, 0.7, '$\sigma_{\Delta '+band+'}='+std+'$',
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
            if stat == 'mean':
                plt.title('$\Delta '+band+'=<'+band+'_{GAaP}-'+band+'_{'+survey+'}>$')
            elif stat == 'median':
                plt.title('$\Delta '+band+'=med('+band+'_{GAaP}-'+band+'_{'+survey+'})$')
            plt.savefig(md+band+"_"+survey+star+"_"+plot_type+"_"+stat+"_"+unit+"_"+MERGE+'.png')
            plt.close()

            #### Sky distribution plot
            if stat == 'median' and plot_type == "Delta":
                c = medians[all_filter]
                print(c.shape, RA.shape, Dec.shape)
                fig = plt.figure()
                ax = fig.add_subplot(111)
                ax.set_xlabel('RA')
                ax.set_ylabel('Dec')
                ax.grid(True)
                ax.set_xlim(360.,0.)
                ax.set_ylim(-9.,89.)
                size=5
                cs = ax.scatter(RA.degree, Dec.degree, c=c, alpha=0.8, edgecolor='none', s=size, cmap='coolwarm', vmin=np.percentile(c,5), vmax=np.percentile(c,95), marker='s')
                fig.colorbar(cs, label=r"$\Delta "+band+"=\mathrm{med}("+band+"_{GAaP}-"+band+"_{"+survey+"})$")
                plt.savefig(md+band+"_"+survey+star+"_"+plot_type+"_"+stat+"_"+unit+"_"+MERGE+'_sky.png')
                plt.close()

