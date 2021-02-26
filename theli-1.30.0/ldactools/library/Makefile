#
# Generic Makefile for the ED routines
#
include $(PIPELIB)/$(EXT)/Makefile
INCLUDES = $(LIB_INCS) $(DEFINES)
#
#
SOURCES=bin_search.c \
	casts.c \
        chain.c \
        field_desc.c \
        graph.c \
        LU.c \
        moments.c \
        obj_obj_util.c \
        options.c \
        sort.c \
	t_iso8601.c \
        tabutil.c \
        utils.c \
        util_lists.c

OBJECTS=$(OBJSUB)/bin_search.o \
        $(OBJSUB)/casts.o \
        $(OBJSUB)/chain.o \
        $(OBJSUB)/field_desc.o \
        $(OBJSUB)/graph.o \
        $(OBJSUB)/LU.o \
        $(OBJSUB)/moments.o \
        $(OBJSUB)/obj_obj_util.o \
        $(OBJSUB)/options.o \
        $(OBJSUB)/sort.o \
        $(OBJSUB)/t_iso8601.o \
        $(OBJSUB)/tabutil.o \
        $(OBJSUB)/utils.o \
        $(OBJSUB)/util_lists.o

STDH=bin_search_globals.h \
     casts_globals.h \
     chain_defs.h \
     chain_globals.h \
     chain_types.h \
     datatypes.h \
     field_desc.h \
     graph_defs.h \
     graph_globals.h \
     graph_types.h \
     image_io_defs.h \
     image_io_globals.h \
     image_io_types.h \
     lists_globals.h \
     LU_globals.h \
     moments_globals.h \
     obj_obj_util.h \
     options_defs.h \
     options_globals.h \
     options_types.h \
     sort_globals.h \
     t_iso8601.h \
     tabutil_globals.h \
     utils_defs.h \
     utils_globals.h \
     utils_macros.h \
     utils_types.h 

LIBRARY=$(OBJSUB)/libedfuncs.a

.KEEP_STATE:
.PRECIOUS: $(LIBRARY)

all: $(PIPELIB)/$(EXT)/libedfuncs.a

$(PIPELIB)/$(EXT)/libedfuncs.a: $(LIBRARY)
	cp $(LIBRARY) $(PIPELIB)/$(EXT)/libedfuncs.a
	ranlib $(PIPELIB)/$(EXT)/libedfuncs.a

$(LIBRARY): $(OBJSUB) $(OBJECTS)
	rm -f $@
	ar rc $@ $(OBJECTS)

$(OBJECTS): $(SOURCES) $(STDH)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

$(OBJSUB):
	-mkdir $(OBJSUB)/

.IGNORE:

clean:
	rm -f *.o

very_clean:
	rm -f *.o *.bak core $(PIPELIB)/$(LIBRARY) $(LIBRARY)

export:
	tar cvf $(TARFILE) $(SOURCES) $(STDH) Makefile
