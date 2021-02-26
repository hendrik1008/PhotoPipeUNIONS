#
#
all: 
	$(MAKE) everything

everything: $(OBJSUB) $(OBJSUB)/associate install
#
#
# Generic Makefile for the ED routines
# with system specific stuff in Makefile.$ARCH
#
include $(PIPELIB)/Makefile
#
#DEBUG = -g 
INCLUDES = $(LIB_INCS)

STDH = assoc_defs.h assoc_globals.h assoc_types.h assoc_vers.h assoc_globals.h

ALL_OBJECTS = $(OBJSUB)/associate.o

associate: $(OBJSUB)/associate
	@echo

$(OBJSUB)/associate: $(OBJSUB)/associate.o $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/associate $(OBJSUB)/associate.o $(LIB_LIBS) -lm

ALL_OBJECTS = $(OBJSUB)/associate.o

$(ALL_OBJECTS): associate.c $(OBJSUB) $(STDH)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

$(OBJSUB):
	-mkdir $(OBJSUB)/

install: $(OBJSUB)/associate
	-cp $(OBJSUB)/associate $(PIPEBIN)/$(EXT)/associate

.c.o:
	$(COMPILE.c) -o $*.o $*.c

clean:
	rm -f *.o

very_clean:
	rm -f *.o *.bak core
	-rm $(OBJSUB)/associate 

export:
	tar cvf $(TARFILE) associate.c $(STDH) Makefile
