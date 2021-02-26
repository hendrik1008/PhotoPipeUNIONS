# Custom makefile for the WCS library
# E. Bertin 28/03/2000
#
include $(PIPELIB)/$(EXT)/Macrodefs.$(EXT)
LIBS	= -lm
SOURCES	= cel.c lin.c poly.c proj.c sph.c tnx.c wcs.c wcstrig.c
OBJECTS	= $(OBJSUB)/cel.o $(OBJSUB)/lin.o $(OBJSUB)/poly.o \
          $(OBJSUB)/proj.o $(OBJSUB)/sph.o $(OBJSUB)/tnx.o \
          $(OBJSUB)/wcs.o $(OBJSUB)/wcstrig.o
INCLUDES= cel.h lin.h poly.h proj.h tnx.h wcs.h wcsmath.h wcstrig.h
LIBRARY = $(OBJSUB)/libwcs_c.a

$(LIBRARY): $(OBJSUB) $(OBJECTS)
	$(RM) $@
	ar rc $@ $(OBJECTS)
	-ranlib $@

$(OBJECTS) : $(INCLUDES) $(SOURCES)
	$(CC) -c $(COPTS) $(DEFINES) -o $@ $(@F:.o=.c)

clean:
	$(RM) $(OBJECTS) $(LIBRARY)

$(OBJSUB):
	-mkdir $(OBJSUB)

