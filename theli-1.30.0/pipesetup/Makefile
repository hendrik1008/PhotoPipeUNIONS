#
# This is the main export Makefile. It creates the tar files for all
# packages in the LDAC software suite for export use. It makes use of
# the export relation in each of the sub-package makefiles.
#
PIPEEXP=$(PIPESOFT)/export
UNTAR=tar xvf 
PACKAGES = bertin.pack \
           imcat.pack \
           small.pack \
           psf.pack \
           eclipse.pack

LDACTOOLS= ldactools.pack
#
#           doc.pack \
#
LIBRARIES = library.pack \
            libs.pack
#
#           affine.pack
#
OTHER = conf.pack \
        scripts.pack
#
PACKAGES_CLR=$(PACKAGES:.pack=.clean)
LDACTOOLS_CLR=$(LDACTOOLS:.pack=.clean)
LIBRARIES_CLR=$(LIBRARIES:.pack=.clean)
OTHER_CLR=$(OTHER:.pack=.clean)
#
PACKAGES_MAKE=$(PACKAGES:.pack=.make)
LDACTOOLS_MAKE=$(LDACTOOLS:.pack=.make)
#
#
all: bin_dirs $(PACKAGES_MAKE) $(LDACTOOLS_MAKE)

very_clean: $(PACKAGES_CLR) $(LIBRARIES_CLR) $(OTHER_CLR)
	rm -f *.tar test_dir vraag Makefile 
	rm -rf ../[a-c]* ../[f-z]*

clean: $(PACKAGES_CLR) $(LIBRARIES_CLR) $(OTHER_CLR)
	rm -f *.tar ../[a-c]* ../[f-z]*

ldactools.clean:
	(cd $(PIPESOFT)/ldactools/common; $(MAKE) very_clean)
	(cd $(PIPESOFT)/ldactools/tools; $(MAKE) very_clean)
	(cd $(PIPESOFT)/ldactools/associate; $(MAKE) very_clean)
	(cd $(PIPESOFT)/ldactools/make_ssc; $(MAKE) very_clean)
	(cd $(PIPESOFT)/ldactools/select_overlap; $(MAKE) very_clean)

ldactools.make:
	(cd $(PIPESOFT)/ldactools/tools; $(MAKE) all)
	(cd $(PIPESOFT)/ldactools/associate; $(MAKE) all)
	(cd $(PIPESOFT)/ldactools/make_ssc; $(MAKE) all)
	(cd $(PIPESOFT)/ldactools/select_overlap; $(MAKE) all)

bertin.clean:
	(cd $(PIPESOFT)/bertintools; $(MAKE) very_clean)

bertin.make:
	(cd $(PIPESOFT)/bertintools; $(MAKE) all);

library.clean:
	(cd $(PIPESOFT)/library; $(MAKE) very_clean)

conf.clean:
	(cd $(PIPESOFT)/conf; $(MAKE) very_clean)

scripts.clean:
	(cd $(PIPESOFT)/scripts; $(MAKE) very_clean)

libs.clean:

small.clean:
	(cd $(PIPESOFT)/smalltools; $(MAKE) very_clean)

small.make:
	(cd $(PIPESOFT)/smalltools; $(MAKE) all)


imcat.clean:
	(cd $(PIPESOFT)/imcattools/tools; $(MAKE) very_clean)

imcat.make:
	(cd $(PIPESOFT)/imcattools/tools; $(MAKE) all)

psf.clean:
	(cd $(PIPESOFT)/psftools; $(MAKE) very_clean)

psf.make:
	(cd $(PIPESOFT)/psftools; $(MAKE) all)

eclipse.clean:
	(cd $(PIPESOFT)/eclipsetools/qfits; $(MAKE) very_clean)
	(cd $(PIPESOFT)/eclipsetools/exlipse; $(MAKE) very_clean)
	(cd $(PIPESOFT)/eclipsetools/tools; $(MAKE) very_clean)

eclipse.make:
	(cd $(PIPESOFT)/eclipsetools/tools; $(MAKE) all)


bin_dirs:
	 ./test_dir $(PIPEBIN) RETAIN
	 ./test_dir $(PIPEBIN)/$(EXT) OVERWRITE

question:
	 ./vraag 


