include $(PIPELIB)/$(EXT)/Makefile

$(CAT_LIB): FORCE
	(cd $(CAT_LIB_DIR); make all DEBUG="$(DEBUG)")

$(ED_LIB): FORCE
	(cd $(ED_LIB_DIR); make all DEBUG="$(DEBUG)")

$(QFITS_LIB): FORCE
	(cd $(QFITS_LIB_DIR); make all DEBUG="$(DEBUG)")

$(ECLIPSE_LIB): FORCE
	(cd $(ECLIPSE_LIB_DIR); make all DEBUG="$(DEBUG)")

LIB_LIBS=$(TR_LIB) $(ED_LIB) $(CAT_LIB)

FORCE:
