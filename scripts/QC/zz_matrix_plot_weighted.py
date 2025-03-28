import numpy as np
import astropy.io.fits as pyfits
import astropy.stats
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt
import math

plt.rc('font', size=15)

def weighted_std(values, weights):
    """
    Return the weighted average and standard deviation.

    They weights are in effect first normalized so that they 
    sum to 1 (and so they must not all be 0).

    values, weights -- NumPy ndarrays with the same shape.
    """
    average = np.average(values, weights=weights)
    # Fast and numerically precise:
    variance = np.average((values-average)**2, weights=weights)
    return math.sqrt(variance)

def weighted_median(values, weights):
    i = np.argsort(values)
    c = np.cumsum(weights[i])
    return values[i[np.searchsorted(c, 0.5 * c[-1])]]

def weighted_NMAD(values, weights):
    values_abs = np.abs(values)
    i = np.argsort(values_abs)
    c = np.cumsum(weights[i])
    return 1.4826*values_abs[i[np.searchsorted(c, 0.5 * c[-1])]]

catalogue_file = sys.argv[1]
catalogue_file2 = sys.argv[2]
z_spec_key = sys.argv[3]
z_phot_key = sys.argv[4]
r_mag_key = sys.argv[5]
ZBmax = float(sys.argv[6])
label = sys.argv[7]
outbase = sys.argv[8]
r_hist_file = sys.argv[9]

### catalogue 1 ###

catalogue = pyfits.open(catalogue_file)
catdata_tmp = catalogue[1].data
ZBmask = np.less(catdata_tmp.field(z_phot_key),ZBmax)
catdata = catdata_tmp[ZBmask]
z_spec = catdata.field(z_spec_key)
z_phot = catdata.field(z_phot_key)
r_mag = catdata.field(r_mag_key)

r_hist = np.loadtxt(r_hist_file)
r_hist_spec = np.histogram(r_mag, bins=20, range=(15.,25.))
weight_hist = np.nan_to_num(r_hist[:,2]/r_hist_spec[0])
weight_hist[np.greater(weight_hist,1E6)] = 0.
weight = np.interp(r_mag, (r_hist[:,0]+r_hist[:,1])/2., weight_hist)

Delta_z = z_spec - z_phot
Delta_z_scaled = Delta_z / (1+z_spec)

### bias, scatter, outlier rate as fct. of ZB, z_spec, mag
stats = {}
for statistic in ('bias', 'NMAD', 'outlier rate'):
    stats[statistic] = {}
    for variable in ('z_spec', 'z_phot', 'r_mag'):
        stats[statistic][variable] = np.zeros(20)

z_spec_list = []
z_phot_list = []
r_mag_list = []

for index in range(20):
    z_spec_low = index / 10.
    z_spec_list.append(z_spec_low+0.05)
    z_spec_high = z_spec_low + 0.1
    z_spec_filter = np.logical_and(np.greater(z_spec,z_spec_low), np.less_equal(z_spec,z_spec_high))
    Delta_z_scaled_filtered = Delta_z_scaled[z_spec_filter]
    weight_filtered = weight[z_spec_filter]
    if np.sum(weight_filtered>0):
        stats['bias']['z_spec'][index] = weighted_median(Delta_z_scaled_filtered,weight_filtered)
        stats['NMAD']['z_spec'][index] = weighted_NMAD(Delta_z_scaled_filtered,weight_filtered)
    outlier015_filtered = np.greater(np.abs(Delta_z_scaled_filtered),0.15)
    if float(outlier015_filtered.shape[0]) > 0:
        stats['outlier rate']['z_spec'][index] = (float(np.sum(weight_filtered[outlier015_filtered])) / float(np.sum(weight_filtered)) * 100.)
    else:
        stats['outlier rate']['z_spec'][index] = 0.

    z_phot_low = index / 10.
    z_phot_list.append(z_phot_low+0.05)
    z_phot_high = z_phot_low + 0.1
    z_phot_filter = np.logical_and(np.greater(z_phot,z_phot_low), np.less_equal(z_phot,z_phot_high))
    Delta_z_scaled_filtered = Delta_z_scaled[z_phot_filter]
    weight_filtered = weight[z_phot_filter]
    if np.sum(weight_filtered>0):
        stats['bias']['z_phot'][index] = weighted_median(Delta_z_scaled_filtered,weight_filtered)
        stats['NMAD']['z_phot'][index] = weighted_NMAD(Delta_z_scaled_filtered,weight_filtered)
    outlier015_filtered = np.greater(np.abs(Delta_z_scaled_filtered),0.15)
    if float(outlier015_filtered.shape[0]) > 0:
        stats['outlier rate']['z_phot'][index] = (float(np.sum(weight_filtered[outlier015_filtered])) / float(np.sum(weight_filtered)) * 100.)
    else:
        stats['outlier rate']['z_phot'][index] = 0.

    r_mag_low = 17. + index / 2.
    r_mag_list.append(r_mag_low+0.25)
    r_mag_high = r_mag_low + 0.5
    r_mag_filter = np.logical_and(np.greater(r_mag,r_mag_low), np.less_equal(r_mag,r_mag_high))
    Delta_z_scaled_filtered = Delta_z_scaled[r_mag_filter]
    weight_filtered = weight[r_mag_filter]
    if np.sum(weight_filtered>0):
        stats['bias']['r_mag'][index] = weighted_median(Delta_z_scaled_filtered,weight_filtered)
        stats['NMAD']['r_mag'][index] = weighted_NMAD(Delta_z_scaled_filtered,weight_filtered)
    outlier015_filtered = np.greater(np.abs(Delta_z_scaled_filtered),0.15)
    if float(outlier015_filtered.shape[0]) > 0:
        stats['outlier rate']['r_mag'][index] = (float(np.sum(weight_filtered[outlier015_filtered])) / float(np.sum(weight_filtered)) * 100.)
    else:
        stats['outlier rate']['r_mag'][index] = 0.

### catalogue 2 ###

catalogue2 = pyfits.open(catalogue_file2)
catdata_tmp2 = catalogue2[1].data
ZBmask2 = np.less(catdata_tmp2.field(z_phot_key),ZBmax)
catdata2 = catdata_tmp2[ZBmask2]
z_spec2 = catdata2.field(z_spec_key)
z_phot2 = catdata2.field(z_phot_key)
r_mag2  = catdata2.field(r_mag_key)

r_hist_spec2 = np.histogram(r_mag2, bins=20, range=(15.,25.))
weight_hist2 = np.nan_to_num(r_hist[:,2]/r_hist_spec2[0])
weight_hist2[np.greater(weight_hist2,1E6)] = 0.
weight2 = np.interp(r_mag2, (r_hist[:,0]+r_hist[:,1])/2., weight_hist2)

Delta_z2 = z_spec2 - z_phot2
Delta_z_scaled2 = Delta_z2 / (1+z_spec2)

### bias, scatter, outlier rate as fct. of ZB, z_spec, mag
stats2 = {}
for statistic in ('bias', 'NMAD', 'outlier rate'):
    stats2[statistic] = {}
    for variable in ('z_spec', 'z_phot', 'r_mag'):
        stats2[statistic][variable] = np.zeros(20)

for index in range(20):
    z_spec_low = index / 10.
    z_spec_high = z_spec_low + 0.1
    z_spec_filter2 = np.logical_and(np.greater(z_spec2,z_spec_low), np.less_equal(z_spec2,z_spec_high))
    Delta_z_scaled_filtered2 = Delta_z_scaled2[z_spec_filter2]
    weight_filtered2 = weight2[z_spec_filter2]
    if np.sum(weight_filtered2>0):
        stats2['bias']['z_spec'][index] = weighted_median(Delta_z_scaled_filtered2,weight_filtered2)
        stats2['NMAD']['z_spec'][index] = weighted_NMAD(Delta_z_scaled_filtered2,weight_filtered2)
    outlier015_filtered2 = np.greater(np.abs(Delta_z_scaled_filtered2),0.15)
    if float(outlier015_filtered2.shape[0]) > 0:
        stats2['outlier rate']['z_spec'][index] = (float(np.sum(weight_filtered2[outlier015_filtered2])) / float(np.sum(weight_filtered2)) * 100.)
    else:
        stats2['outlier rate']['z_spec'][index] = 0.

    z_phot_low = index / 10.
    z_phot_high = z_phot_low + 0.1
    z_phot_filter2 = np.logical_and(np.greater(z_phot2,z_phot_low), np.less_equal(z_phot2,z_phot_high))
    Delta_z_scaled_filtered2 = Delta_z_scaled2[z_phot_filter2]
    weight_filtered2 = weight2[z_phot_filter2]
    if np.sum(weight_filtered2>0):
        stats2['bias']['z_phot'][index] = weighted_median(Delta_z_scaled_filtered2,weight_filtered2)
        stats2['NMAD']['z_phot'][index] = weighted_NMAD(Delta_z_scaled_filtered2,weight_filtered2)
    outlier015_filtered2 = np.greater(np.abs(Delta_z_scaled_filtered2),0.15)
    if float(outlier015_filtered2.shape[0]) > 0:
        stats2['outlier rate']['z_phot'][index] = (float(np.sum(weight_filtered2[outlier015_filtered2])) / float(np.sum(weight_filtered2)) * 100.)
    else:
        stats2['outlier rate']['z_phot'][index] = 0.

    r_mag_low = 17. + index / 2.
    r_mag_high = r_mag_low + 0.5
    r_mag_filter2 = np.logical_and(np.greater(r_mag2,r_mag_low), np.less_equal(r_mag2,r_mag_high))
    Delta_z_scaled_filtered2 = Delta_z_scaled2[r_mag_filter2]
    weight_filtered2 = weight2[r_mag_filter2]
    if np.sum(weight_filtered2>0):
        stats2['bias']['r_mag'][index] = weighted_median(Delta_z_scaled_filtered2,weight_filtered2)
        stats2['NMAD']['r_mag'][index] = weighted_NMAD(Delta_z_scaled_filtered2,weight_filtered2)
    outlier015_filtered2 = np.greater(np.abs(Delta_z_scaled_filtered2),0.15)
    if float(outlier015_filtered2.shape[0]) > 0:
        stats2['outlier rate']['r_mag'][index] = (float(np.sum(weight_filtered2[outlier015_filtered2])) / float(np.sum(weight_filtered2)) * 100.)
    else:
        stats2['outlier rate']['r_mag'][index] = 0.

#plt.rc('font', size=5)

fig, ax = plt.subplots(4, 3, sharex='col', sharey='row', figsize=(9,10))
fig.subplots_adjust(hspace=0, wspace=0, bottom=0.2, left=0.2)
plt.suptitle(label)
#plt.gca().set_aspect('equal', adjustable='box')
ax[3,0].set_xlabel(r"$z_\mathrm{spec}$")
ax[3,1].set_xlabel(r"$z_\mathrm{phot}$")
ax[3,2].set_xlabel(r"$r$")
#ax[0,0].set_ylabel(r"$\langle\Delta z\rangle$")
ax[0,0].set_ylabel(r"$\mathrm{median}(\Delta z/(1+z))$")
ax[1,0].set_ylabel(r"$\mathrm{NMAD}(\Delta z/(1+z))$")
ax[2,0].set_ylabel(r"$\eta_{0.15}\: [\%]$")
#ax[3,0].set_ylabel(r"rel. freq.")
ax[3,0].set_ylabel(r"$N$")
ax[0,0].set_xlim(0.,1.95)
ax[0,0].set_ylim(-0.19,0.19)
ax[0,1].set_xlim(0.,1.95)
ax[1,0].set_ylim(0.,0.19)
ax[0,2].set_xlim(17.05,24.95)
ax[2,0].set_ylim(0.,49.)
#ax.set_ylim(0.,6.95)
#ax.xaxis.labelpad = -1
#ax.scatter(z_spec,z_phot, s=15., alpha=0.1, marker=".", edgecolors="none")
ax[0,0].plot(z_spec_list,stats['bias']['z_spec'])
ax[0,0].plot(z_spec_list,stats2['bias']['z_spec'])
ax[0,0].plot([0.,2.],[0.,0.], "k:", color="black")
ax[0,1].plot(z_phot_list,stats['bias']['z_phot']) #, color="black")
ax[0,1].plot(z_phot_list,stats2['bias']['z_phot']) #, color="black")
ax[0,1].plot([0.,2.],[0.,0.], "k:", color="black")
ax[0,2].plot(r_mag_list,stats['bias']['r_mag']) #, color="black")
ax[0,2].plot(r_mag_list,stats2['bias']['r_mag']) #, color="black")
ax[0,2].plot([16.,28.],[0.,0.], "k:", color="black")

ax[1,0].plot(z_spec_list,stats['NMAD']['z_spec']) #, color="black")
ax[1,0].plot(z_spec_list,stats2['NMAD']['z_spec']) #, color="black")
ax[1,1].plot(z_phot_list,stats['NMAD']['z_phot']) #, color="black")
ax[1,1].plot(z_phot_list,stats2['NMAD']['z_phot']) #, color="black")
ax[1,2].plot(r_mag_list,stats['NMAD']['r_mag']) #, color="black")
ax[1,2].plot(r_mag_list,stats2['NMAD']['r_mag']) #, color="black")

ax[2,0].plot(z_spec_list,stats['outlier rate']['z_spec']) #, color="black")
ax[2,0].plot(z_spec_list,stats2['outlier rate']['z_spec']) #, color="black")
ax[2,1].plot(z_phot_list,stats['outlier rate']['z_phot']) #, color="black")
ax[2,1].plot(z_phot_list,stats2['outlier rate']['z_phot']) #, color="black")
ax[2,2].plot(r_mag_list,stats['outlier rate']['r_mag']) #, color="black")
ax[2,2].plot(r_mag_list,stats2['outlier rate']['r_mag']) #, color="black")

ax[3,0].hist(z_spec,  bins=20, range=(0.,2.),   normed=False, log=True, histtype="step")
ax[3,0].hist(z_spec2, bins=20, range=(0.,2.),   normed=False, log=True, histtype="step")
ax[3,1].hist(z_phot,  bins=20, range=(0.,2.),   normed=False, log=True, histtype="step")
ax[3,1].hist(z_phot2, bins=20, range=(0.,2.),   normed=False, log=True, histtype="step")
ax[3,2].hist(r_mag,   bins=20, range=(15.,27.), normed=False, log=True, histtype="step")
ax[3,2].hist(r_mag2,  bins=20, range=(15.,27.), normed=False, log=True, histtype="step")

#ax.plot([0.,7.],[0.15,7.9], "k:")
#ax.plot([0.15,7.9],[0.,7], "k:")
#ax.text(4.2, 1.45, r'bias$ = $'+bias)
#ax.text(4.2, 0.95, r'NMAD$ = $'+NMAD)
#ax.text(4.2, 0.45, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(outbase+"_matrix.png")
