#!/usr/bin/env python3

# simple Python script to paste LDAC MEF catalogues.
#
# The script takes one command line argument. It is a textfile
# containing catalogues to be converted from the output FITS-LDAC
# SExtractor format to a MEF format which is used by scamp.

# HISTORY INFORMATION:
#
# 23.02.2009:
# complete rewrite of the script to treat multiple images at the same
# time. This makes it much more efficient for a large number of images
# as the Python interpreter only has to be loaded once!
#
# 31.03.2011:
# rewrite of script to make use of multi-processor and multicore machines
# (To be optimised: Currently the scrpt uses as many processors / cores
# as are physically available. As the main task is I/O the load is low and we
# could use more processes).
#
# 28.09.2015:
# The script now explicitely invokes python2 instead of python.
# This is to avoid a call to python3 where this is the standard now.
#
# 28.07.2016:
# I converted the script to python3 and I changed the invocation
# accordingly.
#
# 29.07.2016:
# The pyfits-module pyfits was substitutes by astropy. The former is no longer
# supported by python3.

import os
import sys
import multiprocessing
import astropy.io.fits as aif

def PrintUsage():
  """Just print the scripts usage message"""

  print("SCRIPT NAME:")
  print("    scampcat.py - merge single frame THELI files to a scamp MEF catalogue")
  print("")
  print("SYNOPSIS:")
  print("    python scampcat.py filelist")
  print("")
  print("DESCRIPTION:")
  print("    The script transforms single frame SExtractor FITS-LDAC catalogues")
  print("    from the THELI pipeline to a Multi-Extension-Fitsfile format which")
  print("    is suitable for the 'scamp' program. The conversion is necessary if")
  print("    single-frame THELI catalogues from a multi-chip camera need to be")
  print("    run through that program. The input catalogues have to")
  print("    be in the FITS-LDAC format which is directly output by SExtractor,")
  print("    NOT the format after the 'ldacconv' program has been used!  ")
  print("")
  print("    The filelist argument is an ASCII file containing, on each line,")
  print("    the catalogues to be merged to the last catalogue name in the")
  print("    corresponding line, e.g.:")
  print("")
  print("    If the filelist contains the two lines:")
  print("")
  print("    781_single.cat 782_single.cat 78_mef.cat")
  print("    581_single.cat 582_single.cat 583_single.cat 58_mef.cat")
  print("")
  print("    catalogues 781_single.cat and 782_single.cat would be merged")
  print("    into 78_mef.cat and catalogues 581_single.cat 582_single.cat ")
  print("    and 583_single.cat into 58_mef.cat.")
  print("    ")
  print("REMARKS:")
  print("    - The script is for Python 2.x!")
  print("    - The script automatically detects and uses the number of")
  print("      available processors / cores on your machine.")
  print("    - The script overwrites result catalogues if they already existed.")
  print("    ")
  print("AUTHOR:")
  print("    Thomas Erben         (terben@astro.uni-bonn.de)")
  print("")
  print("")

  return

def scampcat_create(catlist):
    """
    The function transforms single-image THELI catalogues to
    a Multi-Extension FITS file suitable for the scamp program.

    Input:
    - catlist: A list of 'n' catalogue names. The first 'n-1'
               catalogues are merged to the catalogue in the
               last argument.
    Return:
    - A boolean value indicating whether the operation succeeded
      (True = successful, False = Some Error)
    """

    result = True
    outputfile = catlist[len(catlist) - 1]

    # create structures for new MEF catalogue:
    try:
        hdu = aif.PrimaryHDU()
        hdulist = aif.HDUList([hdu])
        hdulisttable = []

        print("creating", outputfile)

        # now go through the catalogues and append the
        # first two tables to the new catalogue.
        for i in range(0, len(catlist) - 1):
            hdulisttable.append(aif.open(catlist[i]))

            hdulist.append(hdulisttable[i][1])
            hdulist.append(hdulisttable[i][2])

        if (os.path.isfile(outputfile)):
            print("File %s already exists. I overwrite!" % (outputfile))
            os.remove(outputfile)

        print("writing", outputfile, "to disk")
        hdulist.writeto(outputfile)
    except IOError as e:
        errno, strerror = e.args
        print("I/O error: {0}".format(strerror))
        result = False

    finally:
        # close files
        hdulist.close()

        for i in range(len(hdulisttable) - 1):
            hdulisttable[i].close()

    return result

if __name__ == '__main__':
    # sanity check for command line arguments:
    if (len(sys.argv) != 2):
        PrintUsage()
        sys.exit(1)

    try:
        catalogfile = open(sys.argv[1], "r")
    except IOError:
        print("Error: cannot open file:", sys.argv[1])
        sys.exit(1)

    scampcatlist = []
    for line in catalogfile:
        scampcatlist.append(line.split())

    catalogfile.close()

    # perform the task to convert the catalogues in multiprocessor
    # mode. We automatically use as many processors / cores as are
    # available on your machine. Note that most of the time is I/O
    # and hence the CPU load is not high. We probably could just
    # use even more processes than physical processors and cores:

    # get the number of CPUs / cores
    ncpus = multiprocessing.cpu_count()

    # Initialise process pool:
    pool = multiprocessing.Pool()

    # execute the conversion with a 'pool-map' command:
    pool.map(scampcat_create, scampcatlist)

