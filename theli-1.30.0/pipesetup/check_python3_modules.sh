#!/bin/sh

# HISTORY INFORMATION
# ===================

# 06.08.2010:
# I modified the check for the Python version so that also
# versions such as '2.6.5+' are treated correctly. The old test
# had problems with the '+' sign.
#
# 26.01.2012:
# I refined error messages in case the version number of an
# (existing) module cannot be determined. Before it was just
# reported that the module is not present at all.
#
# 25.07.2015:
# I substitute pyfits dependencies with astropy. pyfits functionality
# is completely contained in astropy and the former will not
# be supported anymore in the future. I therefore change the THELI pyfits
# dependency to astropy.

#$1: absolute path to the Python Interpreter

"""":
if [ $# -ne 1 ]; then
  echo "$0 python_interpreter_with_absolute_path"
  exit 1
fi

if [ -x $1 ]; then
  exec $1 "$0" "$@"
else
  tput bold
  tput setf 1
  echo "It seems that you do not have Python installed!" >&2
  echo "THELI needs Python 3.X with X >= 5" >& 2
  tput sgr0
  exit 1
fi
"""

__doc__ = """
Check the existence of necessary Python modules for THELI
"""

import sys
import string

def testmodule(modulename, moduleversion):
    """
    test if a Python module is installed, and
    if yes, if its version is equal or higher than
    a reference version
    """

    bold = "\033[1m"
    probbold = "\033[1;34m"
    reset = "\033[0;0m"

    print(bold + \
          "Testing Python module installation for module '%s':" % \
          (modulename) + reset)
    print("THELI needs at least version %s" % (moduleversion))

    try:
        mod = __import__(modulename)
        refversion = moduleversion.split(sep=".")
        currversion = mod.__version__.split(sep=".")

        if list(map(int, currversion)) < list(map(int, refversion)):
            print(probbold + "PROBLEM: You have it with V%s\n" % \
                  (mod.__version__) + reset)
        else:
            print("Your version %s of '%s' is fine!\n" % \
                  (mod.__version__, modulename))

    # error because a module cannot be imported:
    except ImportError:
        print(probbold)
        print(probbold + \
              "PROBLEM: You do not have the Python module '%s' installed!\n" % \
             (modulename) + reset)

    # errors occuring from the 'map' command if the version string cannot
    # be split:
    except ValueError:
        print(probbold)
        print(probbold + \
              "PROBLEM: It seems that Python module '%s' is installed!" % \
              (modulename) + reset)
        print(probbold + \
              "         But I cannot verify the version number!" + reset)
        print(probbold + \
              "         Please do this manually\n" + reset)

# define the Python modules, and the versions we need:
THELImodules = { 'astropy' : '1.0', 'numpy' : '1.1', 'matplotlib' : '0.98.1' }

bold = "\033[1m"         # print bold
probbold = "\033[1;34m"  # print bold blue
reset = "\033[0;0m"      # reset special print settings

print(bold + "THELI Python checking tool" + reset)
print
print(bold + "Checking Python Version:" + reset)
print("THELI needs Python Version 3.X with X>=5")
pyversion = sys.version.split()[0].split(sep=".")

if list(map(int, pyversion)) < [3, 5, 0]:
    print(probbold + "PROBLEM: You have Python V%s.%s.%s\n" \
                      % (pyversion[0], pyversion[1], pyversion[2]) + reset)
    print()
else:
    print("Your Python version %s is fine!" % (sys.version.split()[0]))
    print()

for modulename in THELImodules.keys():
    testmodule(modulename, THELImodules[modulename])

