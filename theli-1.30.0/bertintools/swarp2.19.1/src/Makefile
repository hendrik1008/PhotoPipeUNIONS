# SWarp V2.19.1
#
# General makefile
#
# E.BERTIN 18/08/2000
#
include $(PIPELIB)/$(EXT)/Macrodefs.$(EXT)
INCLUDES=-I$(PIPESOFT)/ldactools/library
DEFINES=-DUSE_THREADS
LIBS  = -L./wcs/$(OBJSUB) -L./fits/$(OBJSUB) -lwcs_c -lfits -lpthread -lm
SOURCES = back.c coadd.c data.c field.c fitswcs.c header.c interpolate.c \
        main.c makeit.c prefs.c projapprox.c resample.c threads.c weight.c \
	xml.c
OBJ   = $(OBJSUB)/back.o \
        $(OBJSUB)/coadd.o \
        $(OBJSUB)/data.o \
        $(OBJSUB)/field.o \
        $(OBJSUB)/fitswcs.o \
        $(OBJSUB)/header.o \
        $(OBJSUB)/interpolate.o \
        $(OBJSUB)/main.o \
        $(OBJSUB)/makeit.o \
        $(OBJSUB)/prefs.o \
        $(OBJSUB)/projapprox.o \
        $(OBJSUB)/resample.o \
        $(OBJSUB)/threads.o \
        $(OBJSUB)/weight.o \
        $(OBJSUB)/xml.o 
H     =	*.h
EXEC  = $(OBJSUB)/swarp_theli

$(EXEC): $(OBJSUB) $(OBJ) ./wcs/$(OBJSUB)/libwcs_c.a ./fits/$(OBJSUB)/libfits.a
	$(CC) $(OBJ) ./wcs/$(OBJSUB)/libwcs_c.a ./fits/$(OBJSUB)/libfits.a \
              -o $(EXEC) $(LIBS) $(COPTS)

$(OBJ):	$(H) $(SOURCES)
	$(CC) -c $(COPTS) $(DEFINES) $(INCLUDES) -o $@ $(@F:.o=.c)

./wcs/$(OBJSUB)/libwcs_c.a: wcs/*.c
	($(CD) wcs && $(MAKE))

./fits/$(OBJSUB)/libfits.a: fits/*.c
	($(CD) fits && $(MAKE))

clean:
	$(RM) $(OBJ) $(EXEC) 
	($(CD) wcs && $(MAKE) clean)
	($(CD) fits && $(MAKE) clean)

$(OBJSUB):
	-mkdir $(OBJSUB)
