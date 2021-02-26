# SExtractor V2.5
#
# General makefile
#
# E.BERTIN 18/08/2000
#
include $(PIPELIB)/$(EXT)/Macrodefs.$(EXT)
INCLUDES=-I$(PIPESOFT)/ldactools/library
DEFINES=-DUSE_THREADS
LIBS  = -L./wcs/$(OBJSUB) -L./fits/$(OBJSUB) -lwcs_c -lfits -lpthread -lm
SOURCES = analyse.c catout.c filter.c interpolate.c neurro.c prefs.c \
	  scan.c assoc.c check.c flag.c main.c pc.c psf.c som.c \
          astrom.c clean.c graph.c makeit.c photom.c readimage.c  \
          weight.c back.c extract.c growth.c manobjlist.c plist.c refine.c \
          winpos.c bpro.c field.c image.c misc.c poly.c retina.c xml.c

OBJ   = ${OBJSUB}/analyse.o \
        ${OBJSUB}/assoc.o \
        ${OBJSUB}/astrom.o \
        ${OBJSUB}/back.o \
        ${OBJSUB}/bpro.o \
        ${OBJSUB}/catout.o \
        ${OBJSUB}/check.o \
        ${OBJSUB}/clean.o \
        ${OBJSUB}/extract.o \
        ${OBJSUB}/field.o \
        ${OBJSUB}/filter.o \
        ${OBJSUB}/flag.o \
        ${OBJSUB}/graph.o \
        ${OBJSUB}/growth.o \
        ${OBJSUB}/image.o \
        ${OBJSUB}/interpolate.o \
        ${OBJSUB}/main.o \
        ${OBJSUB}/makeit.o \
        ${OBJSUB}/manobjlist.o \
        ${OBJSUB}/misc.o \
        ${OBJSUB}/neurro.o \
        ${OBJSUB}/pc.o \
        ${OBJSUB}/photom.o \
        ${OBJSUB}/plist.o \
        ${OBJSUB}/poly.o \
        ${OBJSUB}/prefs.o \
        ${OBJSUB}/psf.o \
        ${OBJSUB}/readimage.o \
        ${OBJSUB}/refine.o \
        ${OBJSUB}/retina.o \
        ${OBJSUB}/scan.o \
        ${OBJSUB}/som.o \
        ${OBJSUB}/weight.o \
        ${OBJSUB}/winpos.o \
        ${OBJSUB}/xml.o

H     =	*.h
EXEC  = $(OBJSUB)/sex_theli

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
