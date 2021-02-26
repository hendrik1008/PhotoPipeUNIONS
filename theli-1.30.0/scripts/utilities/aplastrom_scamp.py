#!/usr/bin/env python3

# HISTORY INFORMATION
# ===================
#
# 06.08.2010:
# I modified the check for the Python version so that also
# versions such as '2.6.5+' are treated correctly. The old test
# had problems with the '+' sign.
#
# 09.03.2014:
# I did not treat correctly the Ra=0.0 pole - fixed.
#
# 27.04.2015:
# I significantly rewrote the script to be able to process multiple
# catalogues in parallel. This is realised with a file that contains info
# on parameters that change with each input catalogue (new command line
# option).
#
# 24.07.2015:
# - The pyfits module was replaced by the equivalent 'astrpy.io.fits'.
#   pyfits will not be supported anymore at some point in the future.
# - We substituted the depreceated astropy function 'new_table' with
#   the new BinTableHDU.from_columns. The former will no longer
#   be supported in a future version of astropy.
#
# 28.09.2015:
# The script now explicitely invokes python2 instead of python.
# This is to avoid a call to python3 where this is the standard now.
#
# 13.07.2016:
# If neither an input catalogue nor a file listing input catalogues was
# provided, the script stops with a help message.
#
# 28.07.2016:
# I converted the script to python3 and I changed the invocation to python3
# accordingly. There is no more need to check for a certain python2-version
# and I removed that part of the code!
#
# 29.07.2016:
# I substituted calls to 'string.split' by calls to methods of the
# string objects. The former is no longer supported by python3.
#
# 31.10.2016:
# I made many operationsmore numpy-array like.
#
# 16.12.2016:
# I added the possibility to specify the number of processors to be used
# in case multiple catalogues are processed. Before, the program used all
# available processors/cores on the machine.

# script to calculate 'correct' Ra and Dec values taking into account
# the contents of a scamp header.

# module 're' is for regular expressions:
import re
import string
import optparse
import sys
import os
import multiprocessing

import astropy.io.fits as aif
import numpy as np
from functools import reduce

def aplastrom_cat(data):
    """
    calculates correct sky-positions (Ra, Dec).

    Argument is a list with the following entries:

    - input catalogue
    - scamp header file
    - name of output catalogue
    - LDAC table name containing the objects data
    - tuple with pixel position
    - tuple with output Ra, Dec names

    We put all the arguments into a list so that we
    function can be parallelised with multiprocessing
    tools.

    The function writes an output catalogue and has
    no return value!
    """

    inputcat = data[0]
    scamphead = data[1]
    output = data[2]
    table = data[3]
    poskeys = data[4]
    resultposkeys = data[5]

    try:
        hdu = aif.open(inputcat)
    except:
        sys.stderr.write("Warning: Could not read input"
                         " catalogue '%s'\n" % (inputcat))
        return

    # see how many relevant OBJECT tables we have in the input
    # catalogue For later iteration we need the table indices within
    # the catalogue:
    tableindices = []
    for i in range(len(hdu)):
        if hdu[i].name == table:
            tableindices.append(i)

    print("catalogue %s has %d %s extensions" \
          % (inputcat, len(tableindices), table))

    try:
        hf = open(scamphead, 'r').readlines()
    except IOError:
        sys.stderr.write("Could not read scamp header '%s'\n" % \
                             (scamphead))
        return

    # How many extensions do we have in the scamp header file? We get
    # them by counting the 'END' lines. We need the indices of the END
    # statements within the file so that we can easily iterate over
    # individual extensions below.  We precede the index list for the
    # position of the END lines with a zero; see the 'for line ...'
    # loop below
    endindices = [ 0 ] + [ i for i, v in enumerate(hf) if v.strip() == 'END' ]

    # sanity check for consistency of extensions in catalogue and scamp file:
    if len(tableindices) != len(endindices) - 1:
        sys.stderr.write("The scamp catalogue has %d extensions\n" \
                        % (len(endindices) - 1))
        sys.stderr.write("This is inconsistent with the %d extensions"
                         " of %s\n" % (len(tableindices), inputcat))
        return None

    # now go through the OBJECT extensions:
    for i in range(len(tableindices)):
        print("Calculating Ra and Dec for Object Extension %d" % (i + 1))
        # We save the headerkeywords within the scamp file in a
        # dictionary; the keys are the FITS keys and the dictionary
        # values are the corresponding header key values; we only need
        # astrometry relevant quantities:
        headerkeys = {}
        for line in hf[endindices[i]:endindices[i + 1]]:
            if line.find('=') != -1:
                res = re.split('=', line)
                name = res[0].strip()
                res = re.split('/', res[1])
                value = res[0].strip()
                if name.find('CD') != -1 or \
                   name.find('PV') != -1 or \
                   name.find('CR') != -1:
                    headerkeys[name] = float(value)

        table = hdu[tableindices[i]].data
        try:
            x0 = table.field(poskeys[0]) - headerkeys['CRPIX1']
            y0 = table.field(poskeys[1]) - headerkeys['CRPIX2']
        except KeyError:
            print(poskeys[0], poskeys[1], i + 1, inputcat)
            sys.stderr.write("Warning: Keys %s and/or %s not present in"
                             " object ext. %d of catalogue %s\n" % \
                             (poskeys[0], poskeys[1], i + 1, inputcat))
            sys.stderr.write("I skip that catalogue!\n")
            return None

        x = x0 * headerkeys['CD1_1'] + y0 * headerkeys['CD1_2']
        y = x0 * headerkeys['CD2_1'] + y0 * headerkeys['CD2_2']
        r = (x**2. + y**2.)**0.5

        # the following lines evaluate polynomials of the form:
        # xi = PV1_0 + PV1_1 * x + PV1_2 * y + ...
        # VERY elegant solution with dictionaries, filter and reduce
        xi_terms = {'PV1_0' : np.ones(len(x)), \
                    'PV1_1' : x, 'PV1_2' : y, 'PV1_3' : r, 'PV1_4' : x**2., \
                    'PV1_5' : x * y, 'PV1_6' : y**2., 'PV1_7' : x**3., \
                    'PV1_8' : x**2. * y, 'PV1_9' : x * y**2., \
                    'PV1_10' : y**3., 'PV1_11' : r**3, 'PV1_12' : x**4, \
                    'PV1_13' : x**3 * y, 'PV1_14' : x**2 * y**2, \
                    'PV1_15' : x * y**3, 'PV1_16' : y**4, \
                    'PV1_17' : x**5, 'PV1_18' : x**4 * y, \
                    'PV1_19' : x**3 * y**2, 'PV1_20' : x**2 * y**3, \
                    'PV1_21' : x * y**4, 'PV1_22' : y**5, 'PV1_23' : r**5}

        pv1_keys = [x for x in list(headerkeys.keys()) if x.find('PV1') != -1]
        xi = reduce(lambda x, y : x + y, \
                    [xi_terms[k] * headerkeys[k] for k in pv1_keys])

        # dito for eta:
        eta_terms = {'PV2_0' : np.ones(len(x)), \
                     'PV2_1' : y, 'PV2_2' : x, 'PV2_3' : r, 'PV2_4' : y**2., \
                     'PV2_5' : y * x, 'PV2_6' : x**2., 'PV2_7' : y**3., \
                     'PV2_8' : y**2. * x, 'PV2_9' : y * x**2., 'PV2_10' : x**3.,
                     'PV2_11' : r**3, 'PV2_12' : y**4, 'PV2_13' : y**3 * x, \
                     'PV2_14' : y**2 * x**2, 'PV2_15' : y * x**3, \
                     'PV2_16' : x**4, 'PV2_17' : y**5, 'PV2_18' : y**4 * x, \
                     'PV2_19' : y**3 * x**2, 'PV2_20' : y**2 * x**3, \
                     'PV2_21' : y * x**4, 'PV2_22' : x**5, 'PV2_23' : r**5}

        pv2_keys = [x for x in list(headerkeys.keys()) if x.find('PV2') != -1]
        eta = reduce(lambda x, y : x + y, \
                     [eta_terms[k] * headerkeys[k] for k in pv2_keys])

        # calculate 'real' Ra and Dec values:
        CRVAL1 = headerkeys['CRVAL1'] / 180.0 * np.pi
        CRVAL2 = headerkeys['CRVAL2'] / 180.0 * np.pi
        XI = xi / 180.0 * np.pi
        ETA = eta / 180.0 * np.pi

        p = np.sqrt(XI**2. + ETA**2.)
        c = np.arctan(p)
        a = CRVAL1 + np.arctan((XI * np.sin(c)) / \
                               (p * np.cos(CRVAL2) * np.cos(c) - \
                                ETA * np.sin(CRVAL2) * np.sin(c)))
        d = np.arcsin(np.cos(c) * np.sin(CRVAL2) + \
                      ETA * np.sin(c) * np.cos(CRVAL2) / p)

        # For Ra we need to deal with the Ra=0 pole:
        RaCorr = a * 180.0 / np.pi
        RaCorr[RaCorr < 0.0] = RaCorr[RaCorr < 0.0] + 360.0
        RaCorr[RaCorr > 360.0] = RaCorr[RaCorr > 360.0] - 360.0

        DecCorr = d * 180.0 / np.pi

        # create new table columns and append them to the old
        # objects table:
        RaCorrColumn = aif.Column(name = resultposkeys[0],
                                  format = '1D', unit = 'deg',
                                  array = RaCorr)

        DecCorrColumn = aif.Column(name = resultposkeys[1],
                                   format = '1D', unit = 'deg',
                                   array = DecCorr)

        newObjectColumns = hdu[tableindices[i]].columns + \
                           RaCorrColumn + DecCorrColumn

        newObjectTableHDU = aif.BinTableHDU.from_columns(newObjectColumns,
                                hdu[tableindices[i]].header)

        # just replace the old table by the new one:
        hdu[tableindices[i]] = newObjectTableHDU

    # write result catalogue:
    if os.path.isfile(output):
        print("WARNING: File %s already exists. I delete it!" % \
            (output))
        os.remove(output)

    print("Writing output catalogue %s" % (output))
    try:
        hdu.writeto(output)
    except:
        sys.stderr.write("Warning: Could not write to '%s' "
                         "(file exists?)\n" % (output))

    return None

# The following class allows us to define a usage message epilogue
# which does not skip newline characters:
class MyParser(optparse.OptionParser):
    def format_epilog(self, formatter):
        return self.epilog

usage = "%prog [Options] - apply scamp astrometry to objects"
parser = MyParser(usage=usage, epilog=
"""
Description:
Emmanuel Bertins 'scamp' program allows us to obtain astrometric
calibration of optical imaging data. It provides the resulting astrometric
information as ASCII header keywords which can be used by 'SExtractor'
or 'swarp'. Unfortunately, 'scamp' does not give us directly the
'correct' astrometric Ra and Dec positions of the sources it used
for its calibration. This script performs this task.
It calculates correct Ra, Dec values from a SExtractor catalogue
and an astrometric scamp header. Typically, the provided catalogue is
the same that 'scamp' used for its calibrations. The script takes
the image pixel positions from sources (by default the SExtractor
parameters X_IMAGE and Y_IMAGE) and outputs a new catalogue where
the objects binary LDAC table (default LDAC_OBJECTS) contains two new
entries for the 'correct' Ra and Dec object positions. If a file with
the provided output name already exists, the program overwrites the
old file.

The script can also be used to process multiple catalogues in parallel.
To do so you need to provide a textfile containing in each line three
columns specifying input catalogue, scamp header and output catalogue.

Example:
- aplastrom_scamp.py -i 956233p_4C.cat -s 956233p_4.head -o 956233p_4C_corr.cat

- aplastrom_scamp.py -i 956233p_4C_ldac.cat -s 956233p_4.head
                     -t OBJECTS -p Xpos Ypos -o 956233p_4C_corr.cat

Known Bugs/Shortcomings:
The script only deals with astrometric solutions up to a polynomial degree
of five!

Authors:
  Patrick Kelly   (pkelly3@stanford.edu)
  Thomas Erben    (terben@astro.uni-bonn.de)
""")

parser.add_option("-f", "--file", dest="inputfile",
                  help="file listing input catalogues, scamp headers and "
                       "output files (one record per line)")
parser.add_option("--processes", dest="processes",
                  help="number of processes to use if '-f' option is used."
                       " The default is to use all available kernels",
                  default=0)
parser.add_option("-i", "--input", dest="input",
                  help="name of input LDAC_FITS catalogue (default: %default)")
parser.add_option("-o", "--output", dest="output",
                  help="name of output LDAC_FITS catalogue (default: %default)",
                  default="output.cat")
parser.add_option("-s", "--scamphead", dest="scamphead",
                  help="name of ASCII scamp header (default: %default)",
                  default="scamp.head")
parser.add_option("-t", "--table", dest="table",
                  help="name of OBJECTS table (default: %default)",
                  default="LDAC_OBJECTS")
parser.add_option("-p", "--position-keys", dest="poskeys", nargs=2,
                  help="names of pixel position keys (default: %default)",
                  default=('X_IMAGE', 'Y_IMAGE'))
parser.add_option("-r", "--result-position-keys", dest="resultposkeys",
                  nargs=2,
                  help="names of result position keys (default: %default)",
                  default=('ALPHA_J2000_corr', 'DELTA_J2000_corr'))

(options, args) = parser.parse_args()

# we either must provide a single catalogue to process or a file that
# lists catalogues to be processed:
if options.inputfile == None and options.input == None:
    parser.print_help(file=sys.stderr)
    sys.exit(1)

# test whether we only need to process a single catalogue or multiple
# catalogues in parallel:
if options.inputfile == None:
    aplastrom_cat([options.input, options.scamphead, options.output,
                   options.table, options.poskeys, options.resultposkeys])
else:
    processlist = []
    # parameters except those varying for each input catalogue:
    baselist = [ options.table, options.poskeys, options.resultposkeys ]

    try:
        catalogfile = open(options.inputfile, "r")
    except IOError:
        print("Error: cannot open file:", options.inputfile)
        sys.exit(1)

    for line in catalogfile:
        processlist.append(line.strip().split() + baselist)

    catalogfile.close()

    # perform the conversion in parallel; we use as many processors as
    # your machne has available:

    # Initialise process pool:
    if options.processes == 0:
        processes = None
    else:
        processes = options.processes

    pool = multiprocessing.Pool(processes=1)

    # execute the conversion with a 'pool-map' command:
    pool.map(aplastrom_cat, processlist)

# and bye:
sys.exit(0)
