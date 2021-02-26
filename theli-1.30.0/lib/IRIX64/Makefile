DEBUG=-O3 -funroll-loops
CFLAGS=$(DEBUG) $(INCLUDES) -Wall
LFLAGS=
CC=gcc 
CPP=g++
CPPFLAGS=$(DEBUG) $(INCLUDES) -I. -Wall -DPERF
COMPILE.c=$(CC) $(CFLAGS) -c
COMPILE.cc=$(CPP) $(CPPFLAGS) -c
LINK.c=$(CC) $(CFLAGS)
LINK.cc=$(CPP) $(CPPFLAGS)
#
LIB_INCS=-I$(ED_LIB_INC) -I$(CAT_LIB_INC) 
#
CAT_LIB_DIR=$(PIPESOFT)/ldactools/common
CAT_LIB_INC=$(CAT_LIB_DIR)
CAT_LIB=$(PIPELIB)/$(EXT)/libfitscat.a
#
ED_LIB_DIR=$(PIPESOFT)/ldactools/library
ED_LIB_INC=$(ED_LIB_DIR)
ED_LIB=$(PIPELIB)/$(EXT)/libedfuncs.a
#
#
#
QFITS_LIB_DIR=$(PIPESOFT)/eclipsetools/qfits
QFITS_LIB_INC=$(QFITS_LIB_DIR)
QFITS_LIB=$(PIPELIB)/$(EXT)/libqfits.a
#
ECLIPSE_LIB_DIR=$(PIPESOFT)/eclipsetools/eclipse
ECLIPSE_LIB_INC=$(ECLIPSE_LIB_DIR)/include
ECLIPSE_LIB=$(PIPELIB)/$(EXT)/libeclipse.a
