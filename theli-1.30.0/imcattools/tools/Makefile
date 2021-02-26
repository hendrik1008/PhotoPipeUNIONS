#
# Generic Makefile with system specific stuff in Makefile.$ARCH
#
all: $(OBJSUB) ic makesubimage album scrunch stats decimaltodms decimaltohms \
     dmstodecimal hmstodecimal imstats imcombflat imhead transformimage install

include $(PIPELIB)/Makefile
#
INCLUDES = -I$(ED_LIB_INC)
LIBS = -lm
#
ICSTDH = ic.h 
SCRUNCHSTDH = scrunch_stuff.h
STATSSTDH = stats_stuff.h
STDH =error.h fits.h iostream.h radecio.h xmemory.h args.h map.h map_private.h

ICSRC=ic.c
IMHEADSRC=imhead.c
SUBSRC=makesubimage.c
ALBUMSRC=album.c
SCRUNCHSRC=scrunch.c s_stuff.c
STATSSRC=stats.c stats_stuff.c
IMSTATSSRC=imstats.c
IMCOMBFLATSRC=imcombflat.c
DMSDECIMALSRC=dmstodecimal.c
HMSDECIMALSRC=hmstodecimal.c
DECIMALDMSSRC=decimaltodms.c
DECIMALHMSSRC=decimaltohms.c
TRANSFORMSRC=transformimage.c

SOURCES = iostream.c \
          fits.c \
          args.c \
          arrays.c \
          convertarray.c \
          radecio.c \
          error.c \
          fastmap.c \
          map.c \
	  t_iso8601.c \
          xmemory.c

OBJECTS = $(OBJSUB)/iostream.o \
          $(OBJSUB)/fits.o \
          $(OBJSUB)/args.o \
          $(OBJSUB)/arrays.o \
          $(OBJSUB)/convertarray.o \
          $(OBJSUB)/radecio.o \
          $(OBJSUB)/error.o \
          $(OBJSUB)/fastmap.o \
          $(OBJSUB)/map.o \
	  $(OBJSUB)/t_iso8601.o \
          $(OBJSUB)/xmemory.o

ALL_OBJECTS = $(OBJECTS) 

$(OBJSUB)/getop.o : getop.c
	$(COMPILE.c) -o $(OBJSUB)/getop.o getop.c

$(OBJSUB)/ic.o : ic.c
	$(COMPILE.c) -o $(OBJSUB)/ic.o ic.c

$(OBJSUB)/ic: $(OBJSUB)/ic.o $(OBJSUB)/getop.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/ic $(OBJSUB)/ic.o $(OBJSUB)/getop.o $(OBJECTS) $(LIBS)

ic: $(OBJSUB) $(OBJSUB)/ic
	@echo

$(OBJSUB)/imhead.o : imhead.c
	$(COMPILE.c) -o $(OBJSUB)/imhead.o imhead.c

$(OBJSUB)/imhead: $(OBJSUB)/imhead.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/imhead $(OBJSUB)/imhead.o $(OBJECTS) $(LIBS)

imhead: $(OBJSUB) $(OBJSUB)/imhead
	@echo

$(OBJSUB)/makesubimage.o : makesubimage.c
	$(COMPILE.c) -o $(OBJSUB)/makesubimage.o makesubimage.c

$(OBJSUB)/makesubimage: $(OBJSUB)/makesubimage.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/makesubimage $(OBJSUB)/makesubimage.o $(OBJECTS) $(LIBS)

makesubimage: $(OBJSUB) $(OBJSUB)/makesubimage
	@echo

$(OBJSUB)/album.o : album.c
	$(COMPILE.c) -o $(OBJSUB)/album.o album.c

$(OBJSUB)/album: $(OBJSUB)/album.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/album $(OBJSUB)/album.o $(OBJECTS) $(LIBS)

album: $(OBJSUB) $(OBJSUB)/album
	@echo

$(OBJSUB)/scrunch.o : scrunch.c
	$(COMPILE.c) -o $(OBJSUB)/scrunch.o scrunch.c

$(OBJSUB)/scrunch_stuff.o : scrunch_stuff.c
	$(COMPILE.c) -o $(OBJSUB)/scrunch_stuff.o scrunch_stuff.c

$(OBJSUB)/scrunch: $(OBJSUB)/scrunch.o $(OBJSUB)/scrunch_stuff.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/scrunch $(OBJSUB)/scrunch.o $(OBJSUB)/scrunch_stuff.o $(OBJECTS) $(LIBS)

scrunch: $(OBJSUB) $(OBJSUB)/scrunch
	@echo

$(OBJSUB)/stats.o : stats.c
	$(COMPILE.c) -o $(OBJSUB)/stats.o stats.c

$(OBJSUB)/stats_stuff.o : stats_stuff.c
	$(COMPILE.c) -o $(OBJSUB)/stats_stuff.o stats_stuff.c

$(OBJSUB)/stats: $(OBJSUB)/stats.o $(OBJSUB)/stats_stuff.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/stats $(OBJSUB)/stats.o $(OBJSUB)/stats_stuff.o $(OBJECTS) $(LIBS)

stats: $(OBJSUB) $(OBJSUB)/stats
	@echo

$(OBJSUB)/imstats.o : imstats.c
	$(COMPILE.c) -o $(OBJSUB)/imstats.o imstats.c

$(OBJSUB)/imstats: $(OBJSUB)/imstats.o $(OBJSUB)/stats_stuff.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/imstats $(OBJSUB)/imstats.o $(OBJSUB)/stats_stuff.o $(OBJECTS) $(LIBS)

imstats: $(OBJSUB) $(OBJSUB)/imstats
	@echo

$(OBJSUB)/imcombflat.o : imcombflat.c
	$(COMPILE.c) -o $(OBJSUB)/imcombflat.o imcombflat.c

$(OBJSUB)/imcombflat: $(OBJSUB)/imcombflat.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/imcombflat $(OBJSUB)/imcombflat.o $(OBJECTS) $(LIBS)

imcombflat: $(OBJSUB) $(OBJSUB)/imcombflat
	@echo

$(OBJSUB)/decimaltodms.o : decimaltodms.c
	$(COMPILE.c) -o $(OBJSUB)/decimaltodms.o decimaltodms.c

$(OBJSUB)/decimaltodms: $(OBJSUB)/decimaltodms.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/decimaltodms $(OBJSUB)/decimaltodms.o $(OBJECTS) $(LIBS)

decimaltodms: $(OBJSUB) $(OBJSUB)/decimaltodms
	@echo

$(OBJSUB)/decimaltohms.o : decimaltohms.c
	$(COMPILE.c) -o $(OBJSUB)/decimaltohms.o decimaltohms.c

$(OBJSUB)/decimaltohms: $(OBJSUB)/decimaltohms.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/decimaltohms $(OBJSUB)/decimaltohms.o $(OBJECTS) $(LIBS)

decimaltohms: $(OBJSUB) $(OBJSUB)/decimaltohms
	@echo

$(OBJSUB)/dmstodecimal.o : dmstodecimal.c
	$(COMPILE.c) -o $(OBJSUB)/dmstodecimal.o dmstodecimal.c

$(OBJSUB)/dmstodecimal: $(OBJSUB)/dmstodecimal.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/dmstodecimal $(OBJSUB)/dmstodecimal.o $(OBJECTS) $(LIBS)

dmstodecimal: $(OBJSUB) $(OBJSUB)/dmstodecimal
	@echo

$(OBJSUB)/hmstodecimal.o : hmstodecimal.c
	$(COMPILE.c) -o $(OBJSUB)/hmstodecimal.o hmstodecimal.c

$(OBJSUB)/hmstodecimal: $(OBJSUB)/hmstodecimal.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/hmstodecimal $(OBJSUB)/hmstodecimal.o $(OBJECTS) $(LIBS)

hmstodecimal: $(OBJSUB) $(OBJSUB)/hmstodecimal
	@echo

$(OBJSUB)/transformimage.o : transformimage.c
	$(COMPILE.c) -o $(OBJSUB)/transformimage.o transformimage.c

$(OBJSUB)/transformimage: $(OBJSUB)/transformimage.o $(OBJECTS)
	$(LINK.c) -o $(OBJSUB)/transformimage $(OBJSUB)/transformimage.o $(OBJECTS) $(LIBS)

transformimage: $(OBJSUB) $(OBJSUB)/transformimage
	@echo

$(ALL_OBJECTS): $(SOURCES)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

install: 
	-cp $(OBJSUB)/ic $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/makesubimage $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/album $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/scrunch $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/stats $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/imstats $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/imhead $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/imcombflat $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/decimaltodms $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/decimaltohms $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/dmstodecimal $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/hmstodecimal $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/transformimage $(PIPEBIN)/$(EXT)

$(OBJSUB):
	-mkdir $(OBJSUB)

clean:
	rm -f $(OBJSUB)/*.o *.bak core
	rm -f *.o

very_clean:
	rm -f $(OBJSUB)/*.o *.bak core
	rm $(OBJSUB)/ic
	rm $(OBJSUB)/imhead
	rm $(OBJSUB)/makesubimage
	rm $(OBJSUB)/album
	rm $(OBJSUB)/scrunch
	rm $(OBJSUB)/stats
	rm $(OBJSUB)/decimaltodms
	rm $(OBJSUB)/decimaltohms
	rm $(OBJSUB)/dmstodecimal
	rm $(OBJSUB)/hmstodecimal
	rm $(OBJSUB)/transformimage

export:
	tar cvf $(TARFILE) $(ICSRC) $(IMHEADSRC) $(SUBSRC)\
                $(ALBUMSRC) $(SOURCES) $(STDH) \
                $(SCRUNCHSRC) $(SCRUNCHSTDH) \
                $(STATSSRC) $(STATSSTDH) $(DECIMALDMSSRC)\
                $(DECIMALHMSSRC) $(DMSDECIMALSRC) \
                $(HMSDECIMALSRC) $(TRANSFORMSRC) Makefile

FORCE:

