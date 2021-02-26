#
# This is the main export Makefile. It creates the tar files for all
# packages in the LDAC software suite for export use. It makes use of
# the export relation in each of the sub-package makefiles.
#
SEXVER=sextractor-2.5.0
WWVER=weightwatcher-1.12
SWARPVER=swarp2.19.1

very_clean:
	(cd $(SEXVER)/src; make clean)
	(cd $(WWVER)/src; make clean)
	(cd $(SWARPVER)/src; make clean)

export: very_clean
	tar cvf $(TARFILE) Makefile $(SEXVER) $(PSFVER) $(WWVER) $(EYEVER) $(SWARPVER) $(LIMITVER)

all: sex ww swarp install
	@echo

sex: $(SEXVER)/src/sex
	@echo

$(SEXVER)/src/sex:
	(cd $(SEXVER)/src; make)

ww: $(WWVER)/src/ww
	@echo

$(WWVER)/src/ww:
	(cd $(WWVER)/src; make)

swarp: $(SWARPVER)/src/swarp
	@echo

$(SWARPVER)/src/swarp:
	(cd $(SWARPVER)/src; make)

install:
	(cp $(SEXVER)/src/$(OBJSUB)/sex_theli $(PIPEBIN)/$(EXT)/sex_theli)
	(cp $(WWVER)/src/$(OBJSUB)/ww_theli $(PIPEBIN)/$(EXT)/ww_theli)
	(cp $(SWARPVER)/src/$(OBJSUB)/swarp_theli $(PIPEBIN)/$(EXT)/swarp_theli)


