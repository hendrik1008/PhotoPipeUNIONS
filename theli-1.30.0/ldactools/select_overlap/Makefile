#
# Generic Makefile with system specific stuff in Makefile.$ARCH
#
all: $(OBJSUB) select_overlaps install

include $(PIPELIB)/Makefile
#
INCLUDES = $(LIB_INCS) $(DEFINES)
LIBS = $(LIB_LIBS) -lm 

#
OVERSTDH=select_overlaps.h
STDH = make_perms_globals.h \
       check_overlap_globals.h \
       make_perms_defs.h \
       queue_globals.h \
       queue_types.h

OVERSRC=select_overlaps.c

SOURCES = check_overlap.c \
          largest_perms.c \
          queue.c

OBJECTS = $(OBJSUB)/check_overlap.o \
          $(OBJSUB)/largest_perms.o \
          $(OBJSUB)/queue.o

ALL_OBJECTS = $(OBJECTS) 

select_overlaps: $(OBJSUB) $(OBJSUB)/select_overlaps
	@echo

$(OBJSUB)/select_overlaps.o : select_overlaps.c
	$(COMPILE.c) -o $(OBJSUB)/select_overlaps.o select_overlaps.c

$(OBJSUB)/select_overlaps: $(OBJSUB)/select_overlaps.o $(OBJECTS) $(LIB_LIBS) 
	$(LINK.c) -o $(OBJSUB)/select_overlaps $(OBJSUB)/select_overlaps.o $(OBJECTS) $(LIBS)

$(OBJSUB)/check_overlap.o : check_overlap.c
	$(COMPILE.c) -o $(OBJSUB)/check_overlap.o check_overlap.c

$(OBJSUB)/largest_perms.o : largest_perms.c
	$(COMPILE.c) -o $(OBJSUB)/largest_perms.o largest_perms.c

$(OBJSUB)/queue.o : queue.c
	$(COMPILE.c) -o $(OBJSUB)/queue.o queue.c


install: $(OBJSUB)/select_overlaps
	cp $(OBJSUB)/select_overlaps $(PIPEBIN)/$(EXT)

$(OBJSUB):
	-mkdir $(OBJSUB)

doc:
	@echo

clean:
	rm -f $(OBJSUB)/*.o *.bak core
	rm -f *.o

very_clean:
	rm -f $(OBJSUB)/*.o *.bak core
	-rm $(OBJSUB)/select_overlaps
export:
	tar cvf $(TARFILE) $(OVERSRC) $(OVERSTDH) \
                $(SOURCES) $(STDH) Makefile

FORCE:


