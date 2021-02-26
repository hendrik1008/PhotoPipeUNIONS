#
# Generic Makefile 
#
all: 
	$(MAKE) everything
everything: $(OBJSUB) airmass dfits_theli hierarch28_theli replacekey_theli \
            fitsort_theli readlink caldate install

include $(PIPELIB)/Makefile
#
INCLUDES =
LIBS = -lm
#
AIRMASSSRC=airmass.c 
AIRMASSOBJ=$(OBJSUB)/airmass.o 
DFITSSRC=dfits_theli.c 
DFITSOBJ=$(OBJSUB)/dfits_theli.o
IOFITSSRC=iofits_theli.c 
IOFITSOBJ=$(OBJSUB)/iofits_theli.o
HIERARCHSRC=hierarch28_theli.c
HIERARCHOBJ=$(OBJSUB)/hierarch28_theli.o
REPLACESRC=replacekey_theli.c 
REPLACEOBJ=$(OBJSUB)/replacekey_theli.o
FITSORTSRC=fitsort_theli.c 
FITSORTOBJ=$(OBJSUB)/fitsort_theli.o
READLINKSRC=readlink.c 
READLINKOBJ=$(OBJSUB)/readlink.o
NIGHTSRC=nightid.c 
NIGHTOBJ=$(OBJSUB)/nightid.o
MJDSRC=mjd.c 
MJDOBJ=$(OBJSUB)/mjd.o
LSTSRC=lst.c 
LSTOBJ=$(OBJSUB)/lst.o
CALSRC=caldate.c 
CALOBJ=$(OBJSUB)/caldate.o

ALL_EXECS=$(OBJSUB)/airmass   $(OBJSUB)/hierarch28_theli \
          $(OBJSUB)/replacekey_theli \
          $(OBJSUB)/fitsort_theli $(OBJSUB)/dfits_theli $(OBJSUB)/readlink \
          $(OBJSUB)/nightid   $(OBJSUB)/mjd $(OBJSUB)/lst \
          $(OBJSUB)/caldate   $(OBJSUB)/iofits_theli

ALL_SOURCES=$(AIRMASSSRC) $(DFITSSRC) $(HIERARCHSRC) $(REPLACESRC) \
            $(FITSORTSRC) $(READLINKSRC) $(NIGHTSRC) $(LSTSRC) \
            $(MJDSRC) $(CALSRC) $(IOFITSSRC)

ALL_OBJECTS=$(AIRMASSOBJ) $(DFITSOBJ) $(HIERARCHOBJ) $(REPLACEOBJ)\
            $(FITSORTOBJ) $(READLINKOBJ) $(NIGHTOBJ) $(LSTOBJ) \
            $(MJDOBJ) $(CALOBJ) $(IOFITSOBJ) 

airmass: $(OBJSUB)/airmass
	@echo

dfits_theli: $(OBJSUB)/dfits_theli
	@echo

iofits_theli: $(OBJSUB)/iofits_theli
	@echo

hierarch28_theli: $(OBJSUB)/hierarch28_theli
	@echo

replacekey_theli: $(OBJSUB)/replacekey_theli
	@echo

fitsort_theli: $(OBJSUB)/fitsort_theli
	@echo

readlink: $(OBJSUB)/readlink
	@echo

nightid: $(OBJSUB)/nightid
	@echo

lst: $(OBJSUB)/lst
	@echo

mjd: $(OBJSUB)/mjd
	@echo

caldate: $(OBJSUB)/caldate
	@echo

$(OBJSUB)/airmass: $(AIRMASSOBJ)
	$(LINK.c) -o $(OBJSUB)/airmass $(AIRMASSOBJ) $(LIBS)

$(OBJSUB)/dfits_theli: $(DFITSOBJ)
	$(LINK.c) -o $(OBJSUB)/dfits_theli $(DFITSOBJ) $(LIBS)

$(OBJSUB)/iofits_theli: $(IOFITSOBJ)
	$(LINK.c) -o $(OBJSUB)/iofits_theli $(IOFITSOBJ) $(LIBS)

$(OBJSUB)/hierarch28_theli: $(HIERARCHOBJ)
	$(LINK.c) -o $(OBJSUB)/hierarch28_theli $(HIERARCHOBJ) $(LIBS)

$(OBJSUB)/replacekey_theli: $(REPLACEOBJ)
	$(LINK.c) -o $(OBJSUB)/replacekey_theli $(REPLACEOBJ) $(LIBS)

$(OBJSUB)/fitsort_theli: $(FITSORTOBJ)
	$(LINK.c) -o $(OBJSUB)/fitsort_theli $(FITSORTOBJ) $(LIBS)

$(OBJSUB)/readlink: $(READLINKOBJ)
	$(LINK.c) -o $(OBJSUB)/readlink $(READLINKOBJ) $(LIBS)

$(OBJSUB)/nightid: $(NIGHTOBJ)
	$(LINK.c) -o $(OBJSUB)/nightid $(NIGHTOBJ) $(LIBS)

$(OBJSUB)/mjd: $(MJDOBJ)
	$(LINK.c) -o $(OBJSUB)/mjd $(MJDOBJ) $(LIBS)

$(OBJSUB)/lst: $(LSTOBJ)
	$(LINK.c) -o $(OBJSUB)/lst $(LSTOBJ) $(LIBS)

$(OBJSUB)/caldate: $(CALOBJ)
	$(LINK.c) -o $(OBJSUB)/caldate $(CALOBJ) $(LIBS)

$(OBJSUB): 
	-mkdir $(OBJSUB)

install: $(ALL_EXECS)
	-cp $(OBJSUB)/airmass            $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/dfits_theli        $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/iofits_theli       $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/hierarch28_theli   $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/replacekey_theli   $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/fitsort_theli      $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/readlink           $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/nightid            $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/mjd                $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/lst                $(PIPEBIN)/$(EXT)
	-cp $(OBJSUB)/caldate            $(PIPEBIN)/$(EXT)


$(ALL_OBJECTS): $(OBJSUB) $(ALL_SOURCES)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

clean:
	rm -f $(OBJSUB)/*.o

very_clean: 
	rm -f $(OBJSUB)/*.o *.bak core

export:
	tar cvf $(TARFILE) $(ALL_SOURCES) Makefile






