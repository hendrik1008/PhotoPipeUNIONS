# script to average the fluxes from multiple measurements
# of the same source

# program docstring:
"""
SCRIPT NAME:
    average_fluxes_list.py - average fluxes from multiple measurements

SYNOPSIS:
    averahe_fluxes_list.py no_obj_in outfile infiles

DESCRIPTION:
    The script takes several ASCII input files with flux measurements of 
    sources and computes a weighted average of the fluxes and flux errors
    if multiple measurements of the same source, identified by its sequence
    number, occur:

    - no_obj_in: Number of objects in the input and later output catalogue.

    - outfile: Name of the ASCII file with the output fluxes.

    - infiles: Sequence of input file names with individual flux 
               measurements.

REMARKS:
    - The script assumes the following columns:
      SeqNr - column 10
      flux - column 6
      flux_err - column 7
      GAaP flag - column 9
    - The output contains the following columns:
      column 1 - SeqNr
      column 2 - flux (weighted average)
      column 3 - flux error (weighted average)
      column 4 - flags (0: OK, 1: flux measurement corrupted)
      column 5 - number of flux measurements that were averaged
      column 6 - chi^2 of the flux measurements that were averaged

EXAMPLES:
    ./average_fluxes_list.py 1000 outfile infile1.gaap infile2.gaap

    This command averages the fluxes of infile1.gaap and infile2.gaap
    and writes the averaged fluxes of objects with 0<SeqNr<=1000 to
    the output file.
    
AUTHOR:
    Hendrik Hildebrandt (hendrik@astro.uni-bonn.de)

"""

import numpy as np
import sys

no_obj_input_cat = int(sys.argv[1])

flux_list = [[] for i in range(no_obj_input_cat)]
fluxerr_list = [[] for i in range(no_obj_input_cat)]
flag_list = [[] for i in range(no_obj_input_cat)]

outfile = sys.argv[2]

for infile in sys.argv[3:]:
    indata=np.loadtxt(infile)
    if np.ndim(indata) > 1:
        for i in range(indata.shape[0]):
            flux_list[int(indata[i,9])-1].append(indata[i,5])
            fluxerr_list[int(indata[i,9])-1].append(indata[i,6])
            flag_list[int(indata[i,9])-1].append(indata[i,8])

length = len(sorted(flux_list,key=len, reverse=True)[0])
fluxes = np.array([xi+[np.nan]*(length-len(xi)) for xi in flux_list])
fluxerrors = np.array([xi+[np.nan]*(length-len(xi)) for xi in fluxerr_list])
flags = np.array([xi+[np.nan]*(length-len(xi)) for xi in flag_list])

fluxweights = 1. / fluxerrors**2

flux_mask = np.logical_or(np.logical_or(np.isnan(fluxes),np.less_equal(fluxerrors,0.)), np.equal(fluxes,0.))

flag_mask = np.logical_or(np.equal(flags,100),np.greater(flags,600))

all_masks = np.logical_or(flux_mask, flag_mask)

nexp = np.sum(np.logical_not(all_masks).astype(np.int32), axis=1)

fluxes_ma = np.ma.MaskedArray(fluxes,           mask=all_masks)
fluxerrors_ma = np.ma.MaskedArray(fluxerrors,   mask=all_masks)
fluxweights_ma = np.ma.MaskedArray(fluxweights, mask=all_masks)

wtot_ma = np.ma.sum(fluxweights_ma, axis=1)

flux_average_ma = np.ma.average(fluxes_ma, weights=fluxweights_ma, axis=1)

fluxflags = np.logical_not(np.greater(nexp,0)).astype(np.int32)

werr_ma = 1.0/np.sqrt(wtot_ma)
werr_ma[np.equal(wtot_ma,0.0)] = -1.0 

seqnr = np.arange(no_obj_input_cat).astype(np.int32) + 1

chi_square = np.ma.sum(
    (
        fluxes_ma
        - np.transpose(np.array([flux_average_ma,]*length))
    )**2
    / fluxerrors_ma**2,
    axis=1)
chi_square_red = chi_square / (nexp-1.)

np.savetxt(outfile,
           np.transpose(
               np.vstack(
                   (seqnr, flux_average_ma, werr_ma, fluxflags, nexp, chi_square_red)
               )
           ),
           fmt='%d %f %f %d %d %f')
