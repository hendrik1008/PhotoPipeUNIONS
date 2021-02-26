#
# Generic Makefile with system specific stuff in Makefile.$ARCH
#
all:
	$(MAKE) everything
#
everything: $(OBJSUB) $(OBJSUB)/make_ssc install
#
lint:
	$(LINT) obj_sets.c
	$(LINT) ssc_utils.c
	$(LINT) ssc_table.c
	$(LINT) ssc_merge.c
	$(LINT) make_ssc.c
#
include $(PIPELIB)/Makefile
#
INCLUDES = $(LIB_INCS) $(DEFINES)
LIBS = $(LIB_LIBS) -lm 
#
SOURCES = obj_sets.c \
          ssc_utils.c \
          ssc_table.c \
          ssc_merge.c \
          make_ssc.c 
OBJECTS = $(OBJSUB)/obj_sets.o \
          $(OBJSUB)/ssc_utils.o \
          $(OBJSUB)/ssc_table.o \
          $(OBJSUB)/ssc_merge.o \
          $(OBJSUB)/make_ssc.o 
STDH = ssc_defs.h ssc_globals.h ssc_types.h obj_sets_globals.h \
       obj_sets_types.h ssc_utils_globals.h ssc_table_globals.h

$(OBJSUB):
	-mkdir $(OBJSUB)

doc:
	autodoc ssc_utils.c
	autodoc ssc_table.c
	autodoc obj_sets.c
	autodoc make_ssc.c

make_ssc: $(OBJSUB)/make_ssc
	@echo

$(OBJSUB)/make_ssc: $(OBJECTS) $(LIB_LIBS)
	$(LINK.c) -o $(OBJSUB)/make_ssc $(OBJECTS) $(LIBS)

$(OBJECTS): $(OBJSUB) $(SOURCES) $(STDH)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

install: $(OBJSUB)/make_ssc
	cp $(OBJSUB)/make_ssc $(PIPEBIN)/$(EXT)

clean:
	rm -f *.o

very_clean:
	rm -f *.o *.bak core
	-rm ${OBJSUB}/make_ssc

export:
	tar cvf $(TARFILE) $(SOURCES) $(STDH) Makefile

FORCE:
	
