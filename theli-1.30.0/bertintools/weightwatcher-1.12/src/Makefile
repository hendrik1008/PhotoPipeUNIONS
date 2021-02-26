# General makefile
#
include $(PIPELIB)/$(EXT)/Macrodefs.$(EXT)
INCLUDES=-I$(PIPESOFT)/ldactools/library
DEFINES=-DUSE_THREADS
LIBS  = -L./wcs/$(OBJSUB) -L./fits/$(OBJSUB) -lwcs_c -lfits -lpthread -lm
SOURCES = field.c  fitswcs.c  main.c  makeit.c  prefs.c  readimage.c \
	vector.c  xml.c
OBJ   = ${OBJSUB}/field.o \
        ${OBJSUB}/fitswcs.o \
        ${OBJSUB}/main.o \
        ${OBJSUB}/makeit.o \
        ${OBJSUB}/prefs.o \
        ${OBJSUB}/readimage.o \
        ${OBJSUB}/vector.o \
        ${OBJSUB}/xml.o

H     =	*.h
EXEC  = $(OBJSUB)/ww_theli

$(EXEC): $(OBJSUB) $(OBJ) ./wcs/$(OBJSUB)/libwcs_c.a ./fits/$(OBJSUB)/libfits.a
	$(CC) $(OBJ) ./wcs/$(OBJSUB)/libwcs_c.a ./fits/$(OBJSUB)/libfits.a \
              -o $(EXEC) $(LIBS) $(COPTS)

$(OBJ):	$(H) $(SOURCES)
	$(CC) -c $(COPTS) $(DEFINES) $(INCLUDES) -o $@ $(@F:.o=.c)

./wcs/$(OBJSUB)/libwcs_c.a: wcs/*.c
	($(CD) wcs && $(MAKE))

./fits/$(OBJSUB)/libfits.a: fits/*.c
	($(CD) fits && $(MAKE))

clean:
	$(RM) $(OBJ) $(EXEC) 
	($(CD) wcs && $(MAKE) clean)
	($(CD) fits && $(MAKE) clean)

$(OBJSUB):
	-mkdir $(OBJSUB)
