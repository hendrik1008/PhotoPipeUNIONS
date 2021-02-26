# makefile for flips library files within the LDAC pipeline
# 
# TE: 10.03.01
#
# Generic Makefile with system specific stuff in Makefile.$ARCH
#
include $(PIPELIB)/$(EXT)/Makefile
#
INCLUDES = -I. 
#
LIBS  =	
SOURCES=byteswap.c \
	cache.c \
        expkey.c \
        fits_h.c \
        fits_md5.c \
        fits_p.c \
        fits_rw.c \
	ieeefp-compat.c \
        md5.c \
	pafs.c \
        pixio.c \
        qerror.c \
        simple.c \
        t_iso8601.c \
        tfits.c \
	version.c \
	xmemory.c

OBJECTS=$(OBJSUB)/byteswap.o \
	$(OBJSUB)/cache.o \
        $(OBJSUB)/expkey.o \
        $(OBJSUB)/fits_h.o \
        $(OBJSUB)/fits_md5.o \
        $(OBJSUB)/fits_p.o \
        $(OBJSUB)/fits_rw.o \
	$(OBJSUB)/ieeefp-compat.o \
        $(OBJSUB)/md5.o \
	$(OBJSUB)/pafs.o \
        $(OBJSUB)/pixio.o \
        $(OBJSUB)/qerror.o \
        $(OBJSUB)/simple.o \
        $(OBJSUB)/t_iso8601.o \
        $(OBJSUB)/tfits.o \
	$(OBJSUB)/version.o \
	$(OBJSUB)/xmemory.o


STDH=	
LIBRARY=$(OBJSUB)/libqfits.a

.KEEP_STATE:
.PRECIOUS: $(LIBRARY)

all: $(PIPELIB)/$(EXT)/libqfits.a 

$(PIPELIB)/$(EXT)/libqfits.a: $(LIBRARY)
	cp $(LIBRARY) $(PIPELIB)/$(EXT)
	ranlib $(PIPELIB)/$(EXT)/libqfits.a

$(LIBRARY): $(OBJSUB) $(OBJECTS)
	rm -f $@
	ar rc $@ $(OBJECTS)

$(OBJECTS): $(SOURCES) $(STDH)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

$(OBJSUB):
	-mkdir $(OBJSUB)

.IGNORE:
clean:
	rm -f $(OBJSUB)/*.o

very_clean:
	rm -f $(OBJSUB)/*.o *.bak core $(PIPELIB)/$(LIBRARY) $(LIBRARY)

export:
	tar cvf $(TARFILE) $(SOURCES) $(STDH) Makefile
