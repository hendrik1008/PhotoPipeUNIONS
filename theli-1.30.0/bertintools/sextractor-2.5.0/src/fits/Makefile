# Custom makefile for the WCS library
# E. Bertin 28/03/2000
#
#
# where to put the result
include $(PIPELIB)/$(EXT)/Macrodefs.$(EXT)
INCLUDEDIR=-I$(PIPESOFT)/ldactools/library
LIBS	= -lm
SOURCES	= fitsbody.c fitscheck.c fitshead.c fitsmisc.c fitstab.c \
          fitswrite.c fitscat.c fitscleanup.c fitskey.c fitsread.c \
          fitsutil.c fitsconv.c t_iso8601.c
OBJECTS	= $(OBJSUB)/fitsbody.o $(OBJSUB)/fitscheck.o \
          $(OBJSUB)/fitshead.o $(OBJSUB)/fitsmisc.o \
          $(OBJSUB)/fitstab.o $(OBJSUB)/fitswrite.o \
          $(OBJSUB)/fitscat.o $(OBJSUB)/fitscleanup.o \
          $(OBJSUB)/fitskey.o $(OBJSUB)/fitsread.o \
          $(OBJSUB)/fitsutil.o $(OBJSUB)/fitsconv.o \
          $(OBJSUB)/t_iso8601.o
INCLUDES= fitscat.h  fitscat_defs.h t_iso8601.h
LIBRARY = $(OBJSUB)/libfits.a

$(LIBRARY): $(OBJSUB) $(OBJECTS)
	$(RM) $@
	ar rc $@ $(OBJECTS)
	-ranlib $@

$(OBJECTS) : $(INCLUDES) $(SOURCES)
	$(CC) -c $(COPTS) $(DEFINES) $(INCLUDEDIR) -o $@ $(@F:.o=.c)

clean:
	$(RM) $(OBJECTS) $(LIBRARY)

$(OBJSUB):
	-mkdir $(OBJSUB)
