import numpy as np
import astropy.io.fits as pyfits
import astropy.stats
import sys
import matplotlib as ml
ml.use('Agg')
import matplotlib.pyplot as plt

plt.rc('font', size=15)

catalogue_file = sys.argv[1]
z_spec_key = sys.argv[2]
z_phot_key = sys.argv[3]
r_mag_key = sys.argv[4]
ZBmax = float(sys.argv[5])
label = sys.argv[6]
outbase = sys.argv[7]

catalogue = pyfits.open(catalogue_file)
catdata_tmp = catalogue[1].data
ZBmask = np.less(catdata_tmp.field(z_phot_key),ZBmax)
catdata = catdata_tmp[ZBmask]
z_spec = catdata.field(z_spec_key)
z_phot = catdata.field(z_phot_key)
r_mag = catdata.field(r_mag_key)

Delta_z = z_spec - z_phot
Delta_z_scaled = Delta_z / (1+z_spec)

bias = "%1.3f" % np.average(Delta_z_scaled)
scatter = "%1.3f" % np.std(Delta_z_scaled)
NMAD = "%1.3f" % astropy.stats.mad_std(Delta_z_scaled)
outlier015 = np.greater(np.abs(Delta_z_scaled),0.15)
outlier_rate015 = "%2.1f" % (float(np.sum(outlier015)) / float(outlier015.shape[0]) * 100.)
outlier025 = np.greater(np.abs(Delta_z_scaled),0.25)
outlier_rate025 = "%2.1f" % (float(np.sum(outlier025)) / float(outlier025.shape[0]) * 100.)

# build a rectangle in axes coords
left, width = .25, .9
bottom, height = .25, .9
right = left + width
top = bottom + height

fig, ax = plt.subplots(1)
plt.title(label)
plt.gca().set_aspect('equal', adjustable='box')
ax.set_xlabel(r"$z_\mathrm{spec}$")
ax.set_ylabel(r"$Z_\mathrm{B}$")
ax.set_xlim(0.,1.95)
ax.set_ylim(0.,1.95)
ax.xaxis.labelpad = -1
ax.scatter(z_spec,z_phot, s=15., alpha=0.01, marker=".", edgecolors="none")
ax.plot([0.,2.],[0.,2.], color="black")
ax.plot([0.,2.],[0.15,2.45], "k:")
ax.plot([0.15,2.],[0.,1.55], "k:")
ax.text(1.2, 0.45, r'bias$ = $'+bias)
ax.text(1.2, 0.35, r'NMAD$ = $'+NMAD)
ax.text(1.2, 0.25, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(outbase+".png")
#plt.savefig(outbase+".pdf")

fig, ax = plt.subplots(1)
plt.title(label)
plt.gca().set_aspect('equal', adjustable='box')
ax.set_xlabel(r"$z_\mathrm{spec}$")
ax.set_ylabel(r"$Z_\mathrm{B}$")
ax.xaxis.labelpad = -1
ax.hist2d(z_spec, z_phot, bins=100, range=((0.,2.),(0.,2.)), norm=ml.colors.LogNorm(), cmap='YlOrRd')
ax.plot([0.,2.],[0.,2.], color="black")
ax.plot([0.,2.],[0.15,2.45], "k:")
ax.plot([0.15,2.],[0.,1.55], "k:")
ax.text(0.95, 0.35, r'bias$ = $'+bias)
ax.text(0.95, 0.25, r'NMAD$ = $'+NMAD)
ax.text(0.95, 0.15, r'$\eta_{0.15} = $'+outlier_rate015+"%")
ax.set_xlim(0.,1.65)
ax.set_ylim(0.,1.65)
plt.savefig(outbase+"_2Dhist.png")
#plt.savefig(outbase+".pdf")

fig, ax = plt.subplots(1)
plt.title(label)
plt.gca().set_aspect('equal', adjustable='box')
ax.set_xlabel(r"$z_\mathrm{spec}$")
ax.set_ylabel(r"$Z_\mathrm{B}$")
ax.set_xlim(0.,6.95)
ax.set_ylim(0.,6.95)
ax.xaxis.labelpad = -1
ax.scatter(z_spec,z_phot, s=15., alpha=0.01, marker=".", edgecolors="none")
ax.plot([0.,7.],[0.,7.], color="black")
ax.plot([0.,7.],[0.15,7.9], "k:")
ax.plot([0.15,7.9],[0.,7], "k:")
ax.text(4.2, 1.45, r'bias$ = $'+bias)
ax.text(4.2, 0.95, r'NMAD$ = $'+NMAD)
ax.text(4.2, 0.45, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(outbase+"_zlt7.png")
#plt.savefig(outbase+"_zlt7.pdf")

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
    stats['bias']['z_spec'][index] = np.average(Delta_z_scaled_filtered)
    stats['NMAD']['z_spec'][index] = astropy.stats.mad_std(Delta_z_scaled_filtered)
    outlier015_filtered = np.greater(np.abs(Delta_z_scaled_filtered),0.15)
    if float(outlier015_filtered.shape[0]) > 0:
        stats['outlier rate']['z_spec'][index] = (float(np.sum(outlier015_filtered)) / float(outlier015_filtered.shape[0]) * 100.)
    else:
        stats['outlier rate']['z_spec'][index] = 0.

    z_phot_low = index / 10.
    z_phot_list.append(z_phot_low+0.05)
    z_phot_high = z_phot_low + 0.1
    z_phot_filter = np.logical_and(np.greater(z_phot,z_phot_low), np.less_equal(z_phot,z_phot_high))
    Delta_z_scaled_filtered = Delta_z_scaled[z_phot_filter]
    stats['bias']['z_phot'][index] = np.average(Delta_z_scaled_filtered)
    stats['NMAD']['z_phot'][index] = astropy.stats.mad_std(Delta_z_scaled_filtered)
    outlier015_filtered = np.greater(np.abs(Delta_z_scaled_filtered),0.15)
    if float(outlier015_filtered.shape[0]) > 0:
        stats['outlier rate']['z_phot'][index] = (float(np.sum(outlier015_filtered)) / float(outlier015_filtered.shape[0]) * 100.)
    else:
        stats['outlier rate']['z_phot'][index] = 0.

    r_mag_low = 17. + index / 2.
    r_mag_list.append(r_mag_low+0.25)
    r_mag_high = r_mag_low + 0.5
    r_mag_filter = np.logical_and(np.greater(r_mag,r_mag_low), np.less_equal(r_mag,r_mag_high))
    Delta_z_scaled_filtered = Delta_z_scaled[r_mag_filter]
    stats['bias']['r_mag'][index] = np.average(Delta_z_scaled_filtered)
    stats['NMAD']['r_mag'][index] = astropy.stats.mad_std(Delta_z_scaled_filtered)
    outlier015_filtered = np.greater(np.abs(Delta_z_scaled_filtered),0.15)
    if float(outlier015_filtered.shape[0]) > 0:
        stats['outlier rate']['r_mag'][index] = (float(np.sum(outlier015_filtered)) / float(outlier015_filtered.shape[0]) * 100.)
    else:
        stats['outlier rate']['r_mag'][index] = 0.

#plt.rc('font', size=5)

fig, ax = plt.subplots(4, 3, sharex='col', sharey='row', figsize=(9,12))
fig.subplots_adjust(hspace=0, wspace=0, bottom=0.2, left=0.2)
#plt.title(label)
#plt.gca().set_aspect('equal', adjustable='box')
ax[3,0].set_xlabel(r"$z_\mathrm{spec}$")
ax[3,1].set_xlabel(r"$z_\mathrm{phot}$")
ax[3,2].set_xlabel(r"$r$")
ax[0,0].set_ylabel(r"$\langle\Delta z\rangle$")
ax[1,0].set_ylabel(r"$\mathrm{NMAD}(\Delta z)$")
ax[2,0].set_ylabel(r"$\eta_{0.15}\: [\%]$")
ax[3,0].set_ylabel(r"rel. freq.")
ax[0,0].set_xlim(0.,1.95)
ax[0,1].set_xlim(0.,1.95)
ax[0,2].set_xlim(17.05,24.95)
#ax.set_ylim(0.,6.95)
#ax.xaxis.labelpad = -1
#ax.scatter(z_spec,z_phot, s=15., alpha=0.1, marker=".", edgecolors="none")
ax[0,0].plot(z_spec_list,stats['bias']['z_spec'])
ax[0,0].plot([0.,2.],[0.,0.], "k:", color="black")
ax[0,1].plot(z_phot_list,stats['bias']['z_phot']) #, color="black")
ax[0,1].plot([0.,2.],[0.,0.], "k:", color="black")
ax[0,2].plot(r_mag_list,stats['bias']['r_mag']) #, color="black")
ax[0,2].plot([16.,25.],[0.,0.], "k:", color="black")

ax[1,0].plot(z_spec_list,stats['NMAD']['z_spec']) #, color="black")
ax[1,1].plot(z_phot_list,stats['NMAD']['z_phot']) #, color="black")
ax[1,2].plot(r_mag_list,stats['NMAD']['r_mag']) #, color="black")

ax[2,0].plot(z_spec_list,stats['outlier rate']['z_spec']) #, color="black")
ax[2,1].plot(z_phot_list,stats['outlier rate']['z_phot']) #, color="black")
ax[2,2].plot(r_mag_list,stats['outlier rate']['r_mag']) #, color="black")

ax[3,0].hist(z_spec, bins=10, range=(0.,1.), normed=True)
ax[3,1].hist(z_phot, bins=10, range=(0.,1.), normed=True)
ax[3,2].hist(r_mag,  bins=10, range=(17.,22.), normed=True)

#ax.plot([0.,7.],[0.15,7.9], "k:")
#ax.plot([0.15,7.9],[0.,7], "k:")
#ax.text(4.2, 1.45, r'bias$ = $'+bias)
#ax.text(4.2, 0.95, r'NMAD$ = $'+NMAD)
#ax.text(4.2, 0.45, r'$\eta_{0.15} = $'+outlier_rate015+"%")
plt.savefig(outbase+"_matrix.png")
