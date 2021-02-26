#
# Generic Makefile 
#
all: 
	$(MAKE) everything

everything: $(OBJSUB) preprocess mefsplit mefcreate swarpfilter \
	fitscollapse install

include $(PIPELIB)/Makefile
#
INCLUDES = -I$(ED_LIB_INC) -I$(QFITS_LIB_INC) -I$(ECLIPSE_LIB_INC) $(DEFINES)
LIBS = $(ECLIPSE_LIB) $(QFITS_LIB) $(ECLIPSE_LIB) -lm
#
PROCESSSRC=preprocess.c prefs_imred.c
PROCESSOBJ=$(OBJSUB)/preprocess.o $(OBJSUB)/prefs_imred.o
PROCESSSTDH=preflist_imred.h prefs_imred.h 
#
SPLITSRC=mefsplit.c prefs_split.c
SPLITOBJ=$(OBJSUB)/mefsplit.o $(OBJSUB)/prefs_split.o
SPLITSTDH=prefs_split.h preflist_split.h
#
CREATESRC=mefcreate.c prefs_create.c
CREATEOBJ=$(OBJSUB)/mefcreate.o $(OBJSUB)/prefs_create.o
CREATESTDH=prefs_create.h preflist_create.h
#
FIXCRSRC=fixcr.c
FIXCROBJ=$(OBJSUB)/fixcr.o
#
SWARPFILTSRC=swarpfilter.c
SWARPFILTOBJ=$(OBJSUB)/swarpfilter.o
#
FITSCOLLSRC=fitscollapse.c
FITSCOLLOBJ=$(OBJSUB)/fitscollapse.o
FITSCOLLSTDH=fitstools.h

ALL_EXECS=$(OBJSUB)/preprocess $(OBJSUB)/mefsplit $(OBJSUB)/mefcreate \
          $(OBJSUB)/fixcr $(OBJSUB)/swarpfilter $(OBJSUB)/fitscollapse
#
SOURCES=wfip_lib.c
#
OBJECTS=$(OBJSUB)/wfip_lib.o
#
STDH=wfip_lib.h
#
ALL_SOURCES = $(PROCESSSRC) $(SPLITSRC) $(CREATESRC) $(FIXCRSCR) \
              $(SWARPFILTSRC) $(FITSCOLLSRC) $(SOURCES)
#
ALL_OBJECTS = $(PROCESSOBJ) $(SPLITOBJ) $(CREATEOBJ) $(FIXCROBJ) \
              $(SWARPFILTOBJ) $(FITSCOLLOBJ) $(OBJECTS)
#
ALL_STDHS = $(PROCESSSTDH) $(SPLITSTDH) $(CREATESTDH) $(STDH) \
	$(FITSCOLLSTDH)

preprocess: $(OBJSUB)/preprocess
	@echo

mefsplit: $(OBJSUB)/mefsplit
	@echo

mefcreate: $(OBJSUB)/mefcreate
	@echo

fixcr: $(OBJSUB)/fixcr
	@echo

swarpfilter: $(OBJSUB)/swarpfilter
	@echo

fitscollapse: $(OBJSUB)/fitscollapse
	@echo

$(OBJSUB)/preprocess: $(PROCESSOBJ) $(OBJECTS) $(QFITS_LIB) $(ECLIPSE_LIB)
	$(LINK.c) -o $(OBJSUB)/preprocess $(PROCESSOBJ) $(OBJECTS) $(LIBS)

$(OBJSUB)/mefsplit: $(SPLITOBJ) $(OBJECTS) $(QFITS_LIB) $(ECLIPSE_LIB)
	$(LINK.c) -o $(OBJSUB)/mefsplit $(SPLITOBJ) $(OBJECTS) $(LIBS)

$(OBJSUB)/mefcreate: $(CREATEOBJ) $(QFITS_LIB) $(ECLIPSE_LIB)
	$(LINK.c) -o $(OBJSUB)/mefcreate $(CREATEOBJ) $(LIBS)

$(OBJSUB)/fixcr: $(FIXCROBJ) $(QFITS_LIB) $(ECLIPSE_LIB)
	$(LINK.c) -o $(OBJSUB)/fixcr $(FIXCROBJ) $(LIBS)

$(OBJSUB)/swarpfilter: $(SWARPFILTOBJ) $(QFITS_LIB) $(ECLIPSE_LIB)
	$(LINK.c) -o $(OBJSUB)/swarpfilter $(SWARPFILTOBJ) $(LIBS)

$(OBJSUB)/fitscollapse: $(FITSCOLLOBJ) $(QFITS_LIB) $(ECLIPSE_LIB)
	$(LINK.c) -o $(OBJSUB)/fitscollapse $(FITSCOLLOBJ) $(LIBS)

$(OBJSUB): 
	-mkdir $(OBJSUB)

install: $(ALL_EXECS)
	-cp $(OBJSUB)/preprocess   $(PIPEBIN)/$(EXT)/preprocess
	-cp $(OBJSUB)/mefsplit     $(PIPEBIN)/$(EXT)/mefsplit
	-cp $(OBJSUB)/mefcreate    $(PIPEBIN)/$(EXT)/mefcreate
	-cp $(OBJSUB)/fixcr        $(PIPEBIN)/$(EXT)/fixcr
	-cp $(OBJSUB)/swarpfilter  $(PIPEBIN)/$(EXT)/swarpfilter
	-cp $(OBJSUB)/fitscollapse $(PIPEBIN)/$(EXT)/fitscollapse

$(ALL_OBJECTS): $(OBJSUB) $(ALL_SOURCES)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

clean:
	rm -f $(OBJSUB)/*.o

very_clean: 
	rm -f $(OBJSUB)/*.o *.bak core

export:
	tar cvf $(TARFILE) $(ALL_SOURCES) $(ALL_STDHS) Makefile
