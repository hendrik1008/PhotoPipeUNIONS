# makefile of SExCat
# HP/UX ANSI C compiler
# EB 30/03/95
# Modified by E.R. Deul  dd 26-03-1996
# Modified by E.R. Deul  dd 09-02-1997
#
# Generic Makefile with system specific stuff in Makefile.$ARCH
#
include $(PIPELIB)/$(EXT)/Makefile
#
INCLUDES = -I. -I$(ED_LIB_INC) 
#
SOURCES=fitscat.c \
        fitsconv.c \
        fitsfield.c \
        fitshead.c \
        fitskey.c \
        fitsmisc.c \
	fitsread.c \
        fitstab.c \
        fitsutil.c \
        fitswrite.c 
OBJECTS=$(OBJSUB)/fitscat.o \
        $(OBJSUB)/fitsconv.o \
        $(OBJSUB)/fitsfield.o \
        $(OBJSUB)/fitshead.o \
        $(OBJSUB)/fitskey.o \
        $(OBJSUB)/fitsmisc.o \
	$(OBJSUB)/fitsread.o \
        $(OBJSUB)/fitstab.o \
        $(OBJSUB)/fitsutil.o \
        $(OBJSUB)/fitswrite.o 

STDH=	fitscat_defs.h fitscat.h tsize.h
LIBRARY=$(OBJSUB)/libfitscat.a

.KEEP_STATE:
.PRECIOUS: $(LIBRARY)

all: $(PIPELIB)/$(EXT)/libfitscat.a 

$(PIPELIB)/$(EXT)/libfitscat.a: $(LIBRARY)
	cp $(LIBRARY) $(PIPELIB)/$(EXT)
	ranlib $(PIPELIB)/$(EXT)/libfitscat.a

$(LIBRARY): $(OBJSUB) $(OBJECTS)
	rm -f $@
	ar rc $@ $(OBJECTS)

$(OBJECTS): $(SOURCES) $(STDH)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

$(OBJSUB):
	-mkdir $(OBJSUB)

doc:
	autodoc -1 -b ldactools -d ../../html/fitscat $(SOURCES)

.IGNORE:
clean:
	rm -f $(OBJSUB)/*.o

very_clean:
	rm -f $(OBJSUB)/*.o *.bak core $(PIPELIB)/$(LIBRARY) $(LIBRARY)

export:
	tar cvf $(TARFILE) $(SOURCES) $(STDH) changes.log Makefile
