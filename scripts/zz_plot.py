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
label = sys.argv[4]
outfile = sys.argv[5]
outfile2 = sys.argv[6]

catalogue = pyfits.open(catalogue_file)
catdata = catalogue[1].data
z_spec = catdata.field(z_spec_key)
z_phot = catdata.field(z_phot_key)

Delta_z = z_spec - z_phot
Delta_z_scaled = Delta_z / (1+z_spec)

bias = "%1.3f" % np.average(Delta_z_scaled)
scatter = "%1.3f" % np.std(Delta_z_scaled)
NMAD = "%1.3f" % astropy.stats.mad_std(Delta_z_scaled)
outlier015 = np.greater(np.abs(Delta_z_scaled),0.15)
outlier_rate015 = "%2.1f" % (float(np.sum(outlier015)) / float(outlier015.shape[0]) * 100.)
outlier025 = np.greater(np.abs(Delta_z_scaled),0.25)
outlier_rate025 = "%2.1f" % (float(np.sum(outlier025)) / float(outlier025.shape[0]) * 100.)

Z_B_gt1_filter = np.greater(z_phot, 0.9)
Delta_z_scaled_zgt1 = Delta_z_scaled[Z_B_gt1_filter]

bias_zgt1 = "%1.3f" % np.average(Delta_z_scaled_zgt1)
scatter_zgt1 = "%1.3f" % np.std(Delta_z_scaled_zgt1)
NMAD_zgt1 = "%1.3f" % astropy.stats.mad_std(Delta_z_scaled_zgt1)
outlier015_zgt1 = np.greater(np.abs(Delta_z_scaled_zgt1),0.15)
if float(outlier015_zgt1.shape[0]) > 0:
    outlier_rate015_zgt1 = "%2.1f" % (float(np.sum(outlier015_zgt1)) / float(outlier015_zgt1.shape[0]) * 100.)
else:
    outlier_rate015_zgt1 = 0.
outlier025_zgt1 = np.greater(np.abs(Delta_z_scaled_zgt1),0.25)
if float(outlier025_zgt1.shape[0]) > 0:
    outlier_rate025_zgt1 = "%2.1f" % (float(np.sum(outlier025_zgt1)) / float(outlier025_zgt1.shape[0]) * 100.)
else:
    outlier_rate025_zgt1 = 0.

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
ax.scatter(z_spec,z_phot, s=15., alpha=0.9, marker=".", edgecolors="none")
ax.plot([0.,2.],[0.,2.], color="black")
ax.plot([0.,2.],[0.15,2.45], "k:")
ax.plot([0.15,2.],[0.,1.55], "k:")
#ax.plot([0.,2.],[0.9,0.9], "k--")
#ax.text(1.2, 0.5, r'all objects:')
#ax.text(1.2, 1.65, r'$<\Delta z / (1+z)> = $'+bias)
#ax.text(1.2, 1.7, r'$\sigma = $'+scatter)
ax.text(1.2, 0.45, r'bias$ = $'+bias)
ax.text(1.2, 0.35, r'NMAD$ = $'+NMAD)
ax.text(1.2, 0.25, r'$\eta_{0.15} = $'+outlier_rate015+"%")
#ax.text(1.2, 0.25, r'$\eta_{0.25} = $'+outlier_rate025+"%")
#ax.text(0.1, 1.8, r'objects with $Z_\mathrm{B}>0.9$:')
#ax.text(0.1, 0.35, r'$<\Delta z / (1+z)> = $'+bias_zgt1)
#ax.text(0.1, 0.7, r'$\sigma = $'+scatter_zgt1)
#ax.text(0.1, 1.65, r'NMAD$ = $'+NMAD_zgt1)
#ax.text(0.1, 1.55, r'$\eta_{0.15} = $'+outlier_rate015_zgt1+"%")
#ax.text(0.1, 1.55, r'$\eta_{0.25} = $'+outlier_rate025_zgt1+"%")
#ax.arrow(0.3, 0.9, 0., 0.1)
#ax.arrow(1.0, 0.9, 0., 0.1)
#ax.arrow(1.7, 0.9, 0., 0.1)
#if label == "8band" or label == "9band" or label == "KiDS+VIKING":
#    ax.text(1.15, 0.725, 'usable with',
#        horizontalalignment='center',
#        verticalalignment='center',
#        rotation='vertical',
#        transform=ax.transAxes)
#    ax.text(1.2, 0.725, 'optical+NIR data',
#        horizontalalignment='center',
#        verticalalignment='center',
#        rotation='vertical',
#        transform=ax.transAxes)
#    ax.plot([1.,1.1],[1.,0.725], "k:", transform=ax.transAxes, clip_on=False)
#    ax.plot([1.,1.1],[0.45,0.725], "k:", transform=ax.transAxes, clip_on=False)
#if label == "4band" or label == "AW 4band" or label == "AWsetup 4band" or label == "KiDS":
#    ax.text(1.15, 0.725, 'unusable with',
#        horizontalalignment='center',
#        verticalalignment='center',
#        rotation='vertical',
#        transform=ax.transAxes)
#    ax.text(1.2, 0.725, 'optical data only',
#        horizontalalignment='center',
#        verticalalignment='center',
#        rotation='vertical',
#        transform=ax.transAxes)
#    ax.plot([1.,1.1],[1.,0.725], "k:", transform=ax.transAxes, clip_on=False)
#    ax.plot([1.,1.1],[0.45,0.725], "k:", transform=ax.transAxes, clip_on=False)
plt.savefig(outfile)
plt.savefig(outfile2)
