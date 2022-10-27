# Python script to examine individual images for suitability in
# superflat creation. We simply check whethe ran image has a certain
# percentage of pixels below a given threshold (In THELI we feed object
# subtracted images to superflat creation; object position get a large,
# negative pixel value).

# HISTORY INFORMATION:
#
# 05.01.2012
# Project started
#
# 17.01.2012
# The number of CPUs to be used is now given as command line argument
# If '0' is given the number of available cores/CPUs is determined
# automatically.
#
# 23.01.2012
# I corrected an error in the synchronization of test jobs.
#
# 24.07.2015:
# The pyfits module was replaced by the equivalent 'astropy.io.fits'.
# pyfits will not be supported anymore at some point in the future.
#
# 28.07.2016:
# I converted the script to python3
#
# 25.09.2016:
# I updated the documentation (it still stated that the script runs
# under python2).

import os
import sys
import multiprocessing
import astropy.io.fits as aif
import numpy

def PrintUsage():
  """Just print the scripts usage message"""

  print("SCRIPT NAME:")
  print("    superflexclusion.py - list images to be excluded from superflats")
  print("")
  print("SYNOPSIS:")
  print("    superflexclusion.py filelist pixthresh exclpercentage ncpus")
  print("")
  print("DESCRIPTION:")
  print("")
  print("    The script lists science images that should be excluded from a")
  print("    superflat.  Within THELI superflat creation we determine, for each")
  print("    science image, pixels to be excluded from the superflat. If the")
  print("    number of excluded pixels is higher than a certain thresshold")
  print("    (e.g. 10% of all image pixels) we would like to completely reject")
  print("    the image.")
  print("    ")
  print("    The script expects an ASCII file with all the science images to be")
  print("    expected. The images need to flag bad (bad for a superflat) pixels")
  print("    with a large, negative value. Within THELI such pixels have the")
  print("    value -70000. Each line of the ASCII file contains the absolute")
  print("    path to one image.  All pixels below 'pixthresh' are considered as")
  print("    bad and counted.  If the number of these pixels exceeds")
  print("    'exclpercentage' the image is output to stdout. 'exclpercentage'")
  print("    is given as percentage of the total number of input pixels")
  print("    (e.g. 0.1 for 10%).")
  print("    ")
  print("    The script can perform the task in parallel. If 'ncpus' is zero")
  print("    the script automatically determines the number of available cores")
  print("    (CPUs) and uses them all.")
  print("")
  print("REMARK:")
  print("    - The script is for Python 3.x (x >= 5)!")
  print("      It uses the 'astropy' and 'numpy' Python modules.")
  print("")
  print("EXAMPLE:")
  print("    - python ./superflatexclusion.py imagelist.txt -5.0 0.1 0")
  print("      Test all images isted in the file 'imagelist.txt'. Files are")
  print("      positively tested if 10% of all image pixels have a value below")
  print("      -5.0. The process is done on as many CPUs/cores as are available")
  print("      on the machine.    ")
  print("")
  print("AUTHOR:")
  print("    Thomas Erben         (terben@astro.uni-bonn.de)")
  print("")
  print("")

  return

def testImage(image):
    """
    The function tests whether an image has a certain percentage
    of pixels below a given threshhold.
    Image name (absolute path), the pixel value threshhold and the
    thresshold percentage are given as list elements in the argument
    'image'.
    If the image is tested 'positively' it's name is returned as a
    string.
    """

    result = ""

    try:
      hdulist = aif.open(image[0])
      scidata = hdulist[0].data

      # total number of image pixels:
      npix = scidata.shape[0] * scidata.shape[1]

      # number of pixels with a value smaller than
      # the pixel threshhold:
      pix_threshhold = image[1]
      pix_rejectpercen = image[2]

      npix_thresh = numpy.where(scidata.ravel() < pix_threshhold)[0].size

      # if more than 10% of the pixels (hardwired value) are masked
      # we want to exclude the image from the superflat:
      if (float)(npix_thresh) / (float)(npix) > pix_rejectpercen:
         result = image[0]

    except IOError as e:
      errno, strerror = e.args
      print("I/O error: {0}".format(strerror))

    finally:
      # close files
      hdulist.close()

    return result

if __name__ == '__main__':
    # sanity check for command line arguments:
    if (len(sys.argv) != 5):
      PrintUsage()
      sys.exit(1)

    pix_threshhold = (float)(sys.argv[2])
    pix_rejectpercen = (float)(sys.argv[3])

    try:
      catalogfile = open(sys.argv[1], "r")
    except IOError:
      print("Error: cannot open file: ", sys.argv[1])
      sys.exit(1)

    imagelist = []
    for line in catalogfile:
      imagelist.append([line.rstrip(),  pix_threshhold, pix_rejectpercen])

    catalogfile.close()

    # perform the task to convert the catalogues in multiprocessor
    # mode.

    # get the number of available CPUs / cores if a '0' was given on the
    # command line:
    ncpus = (int)(sys.argv[4])

    if ncpus == 0:
      ncpus = multiprocessing.cpu_count()

    # Initialise process pool:
    pool = multiprocessing.Pool(processes=ncpus)

    # execute the conversion with a 'pool-map' command:
    images = pool.map(testImage, imagelist)

    # print images to be excluded from superflat creation:
    for image in images:
      if image != '':
        print(image)
