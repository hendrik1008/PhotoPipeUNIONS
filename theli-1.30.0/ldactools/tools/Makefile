#
# Generic Makefile
#
all:
	$(MAKE) everything
everything: $(OBJSUB) ldacaddkey ldacaddtab   ldacconv   ldacdelkey ldacdesc \
                      ldacpaste  ldacputxy    ldacrenkey ldacrentab ldacsplit \
                      ldactoasc  ldactoskycat ldaccalc   ldacsort  \
                      asctoldac  ldacaddkey   ldacdeltab ldacminmax \
                      ldacstats  ldaccomment  ldacaddmask install

include $(PIPELIB)/Makefile
#
INCLUDES = $(LIB_INCS) $(DEFINES)
LIBS = $(LIB_LIBS) -lm
#
ADDMASKSRC=ldacaddmask.c
ADDMASKOBJ=$(OBJSUB)/ldacaddmask.o
JOINKEYSRC=ldacjoinkey.c
JOINKEYOBJ=$(OBJSUB)/ldacjoinkey.o
ADDKEYSRC=ldacaddkey.c
ADDKEYOBJ=$(OBJSUB)/ldacaddkey.o
ADDTABSRC=ldacaddtab.c
ADDTABOBJ=$(OBJSUB)/ldacaddtab.o
CONVSRC=ldacconv.c
CONVOBJ=$(OBJSUB)/ldacconv.o
CONVSTDH=ldacconv.h
COMMSRC=ldaccomment.c
COMMOBJ=$(OBJSUB)/ldaccomment.o
DELKEYSRC=ldacdelkey.c
DELKEYOBJ=$(OBJSUB)/ldacdelkey.o
DELTABSRC=ldacdeltab.c
DELTABOBJ=$(OBJSUB)/ldacdeltab.o
DESCSRC=ldacdesc.c
DESCOBJ=$(OBJSUB)/ldacdesc.o
TESTEXISTSRC=ldactestexist.c
TESTEXISTOBJ=$(OBJSUB)/ldactestexist.o
FILTERSRC=ldacfilter.c filter.c condition.c
FILTEROBJ=$(OBJSUB)/ldacfilter.o  $(OBJSUB)/filter.o $(OBJSUB)/condition.o
FILTERSTDH=filter_globals.h tpar.h parse_globals.h
CALCSRC=ldaccalc.c  condition.c
CALCOBJ=$(OBJSUB)/ldaccalc.o
PASTESRC=ldacpaste.c
PASTEOBJ=$(OBJSUB)/ldacpaste.o
PUTXYSRC=ldacputxy.c
PUTXYOBJ=$(OBJSUB)/ldacputxy.o
RENKEYSRC=ldacrenkey.c
RENKEYOBJ=$(OBJSUB)/ldacrenkey.o
RENTABSRC=ldacrentab.c
RENTABOBJ=$(OBJSUB)/ldacrentab.o
SPLITSRC=ldacsplit.c
SPLITOBJ=$(OBJSUB)/ldacsplit.o
MINMAXSRC=ldacminmax.c
MINMAXOBJ=$(OBJSUB)/ldacminmax.o
CHECKSRC=ldaccheck.c
CHECKOBJ=$(OBJSUB)/ldaccheck.o
TOASCSRC=ldactoasc.c
TOASCOBJ=$(OBJSUB)/ldactoasc.o
TOSKYCATSRC=ldactoskycat.c
TOSKYCATOBJ=$(OBJSUB)/ldactoskycat.o
STATSSRC=ldacstats.c
STATSOBJ=$(OBJSUB)/ldacstats.o
TOLDACSRC=asctoldac.c
TOLDACOBJ=$(OBJSUB)/asctoldac.o
TOLDACSTDH=asctoldac_defs.h
JOINSRC=make_join.c
JOINOBJ=$(OBJSUB)/make_join.o
JOINSTDH=make_join_globals.h make_join_types.h make_join_vers.h
SORTSRC=ldacsort.c
SORTOBJ=$(OBJSUB)/ldacsort.o
SORTSTDH=ldacsort_globals.h ldacsort_types.h ldacsort_defs.h

ALL_EXECS=$(OBJSUB)/ldacaddkey   $(OBJSUB)/ldacaddtab \
          $(OBJSUB)/ldacdelkey   $(OBJSUB)/ldacdesc   $(OBJSUB)/ldacfilter \
          $(OBJSUB)/ldacpaste    $(OBJSUB)/ldacputxy  $(OBJSUB)/ldacrenkey \
          $(OBJSUB)/ldacsplit    $(OBJSUB)/ldactoasc  $(OBJSUB)/ldacsort \
          $(OBJSUB)/ldactoskycat $(OBJSUB)/asctoldac  $(OBJSUB)/make_join \
          $(OBJSUB)/ldaccheck    $(OBJSUB)/ldacrentab $(OBJSUB)/ldaccalc \
          $(OBJSUB)/ldacjoinkey  $(OBJSUB)/ldacdeltab\
          $(OBJSUB)/ldacminmax   $(OBJSUB)/ldacstats $(OBJSUB)/ldactestexist \
          $(OBJSUB)/ldacaddmask

ALL_SOURCES=$(ADDKEYSRC) $(ADDTABSRC) $(CONVSRC)     $(DELKEYSRC) \
 	    $(DESCSRC)   $(FILTERSRC) $(PASTESRC)  $(PUTXYSRC)    $(RENKEYSRC) \
            $(SPLITSRC)  $(TOASCSRC)  $(TOSKYCATSRC) $(TOLDACSRC) $(SORTSRC) \
            $(JOINSRC)   $(CHECKSRC)  $(RENTABSRC) $(CALCSRC)      \
            $(JOINKEYSRC) $(PLOTSRC)  $(DELTABSRC) $(STATSSRC)    $(MINMAXSRC) \
            $(COMMSRC)   $(TESTEXISTSRC) $(ADDMASKSRC)

ALL_OBJECTS=$(ADDKEYOBJ) $(ADDTABOBJ) $(CONVOBJ)   $(DELKEYOBJ) \
 	    $(DESCOBJ)   $(FILTEROBJ) $(PASTEOBJ)  $(PUTXYOBJ)    $(RENKEYOBJ) \
            $(SPLITOBJ)  $(TOASCOBJ)  $(TOSKYCATOBJ) $(TOLDACOBJ) $(SORTOBJ) \
            $(JOINOBJ)   $(CHECKOBJ)  $(RENTABOBJ) $(CALCOBJ)     \
            $(JOINKEYOBJ) $(PLOTOBJ)  $(DELTABOBJ) $(STATSOBJ)    $(MINMAXOBJ) \
            $(COMMOBJ) $(TESTEXISTOBJ) $(ADDMASKOBJ)

ALL_STDHS=$(CONVSTDH) $(FILTERSTDH) $(TOLDACSTDH) $(JOINSTDH) \
          $(SORTSTDH)

ldacaddmask: $(OBJSUB)/ldacaddmask
	@echo

ldacjoinkey: $(OBJSUB)/ldacjoinkey
	@echo

ldacaddkey: $(OBJSUB)/ldacaddkey
	@echo

ldacaddtab: $(OBJSUB)/ldacaddtab
	@echo

ldacconv: $(OBJSUB)/ldacconv
	@echo

ldaccomment: $(OBJSUB)/ldaccomment
	@echo

ldacdelkey: $(OBJSUB)/ldacdelkey
	@echo

ldacdeltab: $(OBJSUB)/ldacdeltab
	@echo

ldacdesc: $(OBJSUB)/ldacdesc
	@echo

ldactestexist: $(OBJSUB)/ldactestexist
	@echo

ldacfilter: $(OBJSUB)/ldacfilter
	@echo

ldaccalc: $(OBJSUB)/ldaccalc
	@echo

ldacpaste: $(OBJSUB)/ldacpaste
	@echo

ldacputxy: $(OBJSUB)/ldacputxy
	@echo

ldacrenkey: $(OBJSUB)/ldacrenkey
	@echo

ldacrentab: $(OBJSUB)/ldacrentab
	@echo

ldacshow: $(OBJSUB)/ldacshow
	@echo

ldacsplit: $(OBJSUB)/ldacsplit
	@echo

ldactoasc: $(OBJSUB)/ldactoasc
	@echo

ldactoskycat: $(OBJSUB)/ldactoskycat
	@echo

asctoldac: $(OBJSUB)/asctoldac
	@echo

make_join: $(OBJSUB)/make_join
	@echo

ldaccheck: $(OBJSUB)/ldaccheck
	@echo

ldacsort: $(OBJSUB)/ldacsort
	@echo

ldacstats: $(OBJSUB)/ldacstats
	@echo

ldacminmax: $(OBJSUB)/ldacminmax
	@echo

$(OBJSUB)/ldacaddmask: $(ADDMASKOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacaddmask $(ADDMASKOBJ) $(LIBS)

$(OBJSUB)/ldacjoinkey: $(JOINKEYOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacjoinkey $(JOINKEYOBJ) $(LIBS)

$(OBJSUB)/ldacaddkey: $(ADDKEYOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacaddkey $(ADDKEYOBJ) $(LIBS)

$(OBJSUB)/ldacaddtab: $(ADDTABOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacaddtab $(ADDTABOBJ) $(LIBS)

$(OBJSUB)/ldacconv: $(CONVOBJ) $(CONVSTDH) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacconv $(CONVOBJ) $(LIBS)

$(OBJSUB)/ldaccomment: $(COMMOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldaccomment $(COMMOBJ) $(LIBS)

$(OBJSUB)/ldacdelkey: $(DELKEYOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacdelkey $(DELKEYOBJ) $(LIBS)

$(OBJSUB)/ldacdeltab: $(DELTABOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacdeltab $(DELTABOBJ) $(LIBS)

$(OBJSUB)/ldacdesc: $(DESCOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacdesc $(DESCOBJ) $(LIBS)

$(OBJSUB)/ldactestexist: $(TESTEXISTOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldactestexist $(TESTEXISTOBJ) $(LIBS)

$(OBJSUB)/ldacfilter: $(FILTEROBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacfilter $(FILTEROBJ) $(LIBS)

$(OBJSUB)/ldaccalc: $(CALCOBJ) $(OBJSUB)/condition.o $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldaccalc $(CALCOBJ) $(OBJSUB)/condition.o $(LIBS)

$(OBJSUB)/ldacpaste: $(PASTEOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacpaste $(PASTEOBJ) $(LIBS)

$(OBJSUB)/ldacputxy: $(PUTXYOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacputxy $(PUTXYOBJ) $(LIBS)

$(OBJSUB)/ldacrenkey: $(RENKEYOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacrenkey $(RENKEYOBJ) $(LIBS)

$(OBJSUB)/ldacrentab: $(RENTABOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacrentab $(RENTABOBJ) $(LIBS)

$(OBJSUB)/ldacsplit: $(SPLITOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacsplit $(SPLITOBJ) $(LIBS)

$(OBJSUB)/ldactoasc: $(TOASCOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldactoasc $(TOASCOBJ) $(LIBS)

$(OBJSUB)/ldactoskycat: $(TOSKYCATOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldactoskycat $(TOSKYCATOBJ) $(LIBS)

$(OBJSUB)/asctoldac: $(TOLDACOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/asctoldac $(TOLDACOBJ) $(LIBS)

$(OBJSUB)/make_join: $(JOINOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/make_join $(JOINOBJ) $(LIBS)

$(OBJSUB)/ldaccheck: $(CHECKOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldaccheck $(CHECKOBJ) $(LIBS)

$(OBJSUB)/ldacsort: $(SORTOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacsort $(SORTOBJ) $(LIBS)

$(OBJSUB)/bileave: $(BILOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/bileave $(BILOBJ) $(LIBS)

$(OBJSUB)/ldacstats: $(STATSOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacstats $(STATSOBJ) $(LIBS)

$(OBJSUB)/ldacminmax: $(MINMAXOBJ) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/ldacminmax $(MINMAXOBJ) $(LIBS)

$(OBJSUB):
	-mkdir $(OBJSUB)

install: $(ALL_EXECS)
	-cp $(OBJSUB)/ldacaddmask  $(PIPEBIN)/$(EXT)/ldacaddmask_theli
	-cp $(OBJSUB)/ldacjoinkey  $(PIPEBIN)/$(EXT)/ldacjoinkey_theli
	-cp $(OBJSUB)/ldacaddkey   $(PIPEBIN)/$(EXT)/ldacaddkey_theli
	-cp $(OBJSUB)/ldacaddtab   $(PIPEBIN)/$(EXT)/ldacaddtab_theli
	-cp $(OBJSUB)/ldacconv     $(PIPEBIN)/$(EXT)/ldacconv_theli
	-cp $(OBJSUB)/ldacdelkey   $(PIPEBIN)/$(EXT)/ldacdelkey_theli
	-cp $(OBJSUB)/ldacdesc     $(PIPEBIN)/$(EXT)/ldacdesc_theli
	-cp $(OBJSUB)/ldactestexist  $(PIPEBIN)/$(EXT)/ldactestexist_theli
	-cp $(OBJSUB)/ldacfilter   $(PIPEBIN)/$(EXT)/ldacfilter_theli
	-cp $(OBJSUB)/ldacpaste    $(PIPEBIN)/$(EXT)/ldacpaste_theli
	-cp $(OBJSUB)/ldacputxy    $(PIPEBIN)/$(EXT)/ldacputxy_theli
	-cp $(OBJSUB)/ldacrenkey   $(PIPEBIN)/$(EXT)/ldacrenkey_theli
	-cp $(OBJSUB)/ldacrentab   $(PIPEBIN)/$(EXT)/ldacrentab_theli
	-cp $(OBJSUB)/ldacsplit    $(PIPEBIN)/$(EXT)/ldacsplit_theli
	-cp $(OBJSUB)/ldactoasc    $(PIPEBIN)/$(EXT)/ldactoasc_theli
	-cp $(OBJSUB)/ldactoskycat $(PIPEBIN)/$(EXT)/ldactoskycat_theli
	-cp $(OBJSUB)/asctoldac    $(PIPEBIN)/$(EXT)/asctoldac_theli
	-cp $(OBJSUB)/make_join    $(PIPEBIN)/$(EXT)/make_join_theli
	-cp $(OBJSUB)/ldaccheck    $(PIPEBIN)/$(EXT)/ldaccheck_theli
	-cp $(OBJSUB)/ldaccalc     $(PIPEBIN)/$(EXT)/ldaccalc_theli
	-cp $(OBJSUB)/ldacsort     $(PIPEBIN)/$(EXT)/ldacsort_theli
	-cp $(OBJSUB)/ldacdeltab   $(PIPEBIN)/$(EXT)/ldacdeltab_theli
	-cp $(OBJSUB)/ldacstats    $(PIPEBIN)/$(EXT)/ldacstats_theli
	-cp $(OBJSUB)/ldacminmax   $(PIPEBIN)/$(EXT)/ldacminmax_theli
	-cp $(OBJSUB)/ldaccomment  $(PIPEBIN)/$(EXT)/ldaccomment_theli


$(ALL_OBJECTS): $(OBJSUB) $(ALL_SOURCES)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

clean:
	rm -f $(OBJSUB)/*.o

very_clean:
	rm -f $(OBJSUB)/*.o *.bak core

export:
	tar cvf $(TARFILE) $(ALL_SOURCES) $(ALL_STDHS) Makefile
