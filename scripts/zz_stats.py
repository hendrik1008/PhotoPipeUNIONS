import numpy as np
import astropy.io.fits as pyfits
import astropy.stats
import sys

catalogue_file = sys.argv[1]
z_spec_key = sys.argv[2]
z_phot_key = sys.argv[3]

catalogue = pyfits.open(catalogue_file)
catdata = catalogue[1].data
z_spec = catdata.field(z_spec_key)
z_phot = catdata.field(z_phot_key)

n_obj = np.shape(z_spec)[0]

Delta_z = z_spec - z_phot
Delta_z_scaled = Delta_z / (1+z_spec)

bias = np.average(Delta_z_scaled)
scatter = np.std(Delta_z_scaled)
NMAD = astropy.stats.mad_std(Delta_z_scaled)
outlier015 = np.greater(np.abs(Delta_z_scaled),0.15)
outlier_rate015 = float(np.sum(outlier015)) / float(outlier015.shape[0])
outlier025 = np.greater(np.abs(Delta_z_scaled),0.25)
outlier_rate025 = float(np.sum(outlier025)) / float(outlier025.shape[0])

print "%d % 1.3f %1.3f %1.3f %1.3f %1.3f" % (n_obj, bias, scatter, NMAD, outlier_rate015, outlier_rate025)
