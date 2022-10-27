#
# Generic Makefile with system specific stuff in Makefile.$ARCH
#
all: $(OBJSUB) preanisotropy anisotropy analyseldac install

include $(PIPELIB)/Makefile
#
INCLUDES = $(LIB_INCS) $(DEFINES)
LIBS = $(LIB_LIBS) -lm
CPPLIBS=$(LIB_LIBS) $(CPP_LIB) -lm
#
STDH = stats_stuff.h \
       error.h \
       fits.h \
       arrays.h \
       object_stuff.h \
       magic.h \
       nrutil.h \
       status.h \
       interpol.h \
       analyseldac.h
#
SOURCES = stats_stuff.c \
          error.c \
          fits.c \
          arrays.c \
          nrutil.c \
          interpol.c

OBJECTS = $(OBJSUB)/stats_stuff.o \
          $(OBJSUB)/error.o \
          $(OBJSUB)/fits.o \
          $(OBJSUB)/arrays.o \
          $(OBJSUB)/nrutil.o \
          $(OBJSUB)/interpol.o
#
PREANISOSRC=preanisotropy.c
PREANISOOBJ=$(OBJSUB)/preanisotropy.o
ANALYSELDACSRC=analyseldac.c
ANALYSELDACOBJ=$(OBJSUB)/analyseldac.o 
ANISOSRC=anisotropy.c
ANISOOBJ=$(OBJSUB)/anisotropy.o

preanisotropy: $(OBJSUB) $(OBJSUB)/preanisotropy
	@echo

anisotropy: $(OBJSUB) $(OBJSUB)/anisotropy
	@echo

analyseldac: $(OBJSUB) $(OBJSUB)/analyseldac
	@echo

$(OBJSUB)/preanisotropy.o : preanisotropy.c
	$(COMPILE.c) -o $(OBJSUB)/preanisotropy.o preanisotropy.c

$(OBJSUB)/preanisotropy: $(OBJSUB)/preanisotropy.o $(OBJECTS) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/preanisotropy $(OBJSUB)/preanisotropy.o $(OBJECTS) $(LIBS)

$(OBJSUB)/anisotropy.o : anisotropy.c
	$(COMPILE.c) -o $(OBJSUB)/anisotropy.o anisotropy.c

$(OBJSUB)/anisotropy: $(OBJSUB)/anisotropy.o $(OBJECTS) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/anisotropy $(OBJSUB)/anisotropy.o $(OBJECTS) $(LIBS)

$(OBJSUB)/analyseldac.o : analyseldac.c
	$(COMPILE.c) -o $(OBJSUB)/analyseldac.o analyseldac.c

$(OBJSUB)/analyseldac: $(OBJSUB)/analyseldac.o $(OBJECTS) $(LIB_LIBS) 
	$(LINK.c) -o $(OBJSUB)/analyseldac $(OBJSUB)/analyseldac.o $(OBJECTS) $(LIBS)

install: $(OBJSUB)/preanisotropy $(OBJSUB)/anisotropy
	 cp $(OBJSUB)/preanisotropy $(PIPEBIN)/$(EXT)
	 cp $(OBJSUB)/anisotropy $(PIPEBIN)/$(EXT)
	 cp $(OBJSUB)/analyseldac $(PIPEBIN)/$(EXT)

$(OBJECTS) : $(SOURCES)
	 $(COMPILE.c) -o $@ $(@F:.o=.c)

$(OBJSUB):
	-mkdir $(OBJSUB)

doc:
	@echo

clean:
	rm -f $(OBJSUB)/*.o *.bak core
	rm -f *.o

very_clean:
	rm -f $(OBJSUB)/*.o *.bak core
	-rm $(OBJSUB)/preanisotropy
	-rm $(OBJSUB)/anisotropy
	-rm $(OBJSUB)/analyseldac

export:
	tar cvf $(TARFILE) $(PREANISOSRC) $(ANISOSRC) $(SOURCES) $(STDH)\
                $(ANALYSELDACSRC) Makefile

FORCE:
