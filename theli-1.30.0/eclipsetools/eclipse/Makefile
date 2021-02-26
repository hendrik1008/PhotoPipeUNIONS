# makefile for flips library files within the LDAC pipeline
# 
# TE: 10.03.01
#
# Generic Makefile with system specific stuff in Makefile.$ARCH
#
include $(PIPELIB)/$(EXT)/Makefile
#
INCLUDES = -I./include -I../qfits -D_ECLIPSE_
#
SOURCES = corner.c \
	  cube2image.c \
	  cube_arith.c \
	  cube_filters.c \
	  cube_handling.c \
	  cube_load.c \
	  cube_save.c \
	  dead_pixels.c \
	  detect.c \
	  detect_ks.c \
	  detect_sq.c \
	  detector.c \
	  extraction.c \
	  fourier.c \
	  framelist.c \
	  generate.c \
	  histogram.c \
	  history.c \
	  image_arith.c \
	  image_filters.c \
	  image_handling.c \
	  image_intops.c \
	  image_io.c \
	  image_rtd.c \
	  image_stats.c \
	  intimage.c \
	  irstd.c \
	  matchpoint.c \
	  photometry.c \
	  pixel_handling.c \
	  pixelmaps.c \
	  resampling.c \
	  shift.c \
	  xcorrelation.c \
	  chebyshev.c \
	  doubles.c \
	  dstats.c \
	  fft_base.c \
	  fit_curve.c \
	  function_1d.c \
	  ipow.c \
	  legendre.c \
	  matrix.c \
	  median.c \
	  optimization.c \
	  poly2d.c \
	  polygon.c \
	  random.c \
	  calendar.c \
	  charmatrix.c \
	  chronos.c \
	  cmdline.c \
	  comm.c \
	  dictionary.c \
	  e_config.c \
	  e_version.c \
	  file_handling.c \
	  filelock.c \
	  filename.c \
	  filesys.c \
	  getopt.c \
	  gnuplot_i.c \
	  iniparser.c \
	  manpage.c \
	  memstr.c \
	  parse_tok.c \
	  pid_i.c \
	  ptrace.c \
	  rtd_i.c \
	  show.c \
	  strlib.c \
	  t_iso8601.c \
	  t_stamp.c \
	  userid.c


OBJECTS=$(OBJSUB)/corner.o \
	$(OBJSUB)/cube2image.o \
	$(OBJSUB)/cube_arith.o \
	$(OBJSUB)/cube_filters.o \
	$(OBJSUB)/cube_handling.o \
	$(OBJSUB)/cube_load.o \
	$(OBJSUB)/cube_save.o \
	$(OBJSUB)/dead_pixels.o \
	$(OBJSUB)/detect.o \
	$(OBJSUB)/detect_ks.o \
	$(OBJSUB)/detect_sq.o \
	$(OBJSUB)/detector.o \
	$(OBJSUB)/extraction.o \
	$(OBJSUB)/fourier.o \
	$(OBJSUB)/framelist.o \
	$(OBJSUB)/generate.o \
	$(OBJSUB)/histogram.o \
	$(OBJSUB)/history.o \
	$(OBJSUB)/image_arith.o \
	$(OBJSUB)/image_filters.o \
	$(OBJSUB)/image_handling.o \
	$(OBJSUB)/image_intops.o \
	$(OBJSUB)/image_io.o \
	$(OBJSUB)/image_rtd.o \
	$(OBJSUB)/image_stats.o \
	$(OBJSUB)/intimage.o \
	$(OBJSUB)/irstd.o \
	$(OBJSUB)/matchpoint.o \
	$(OBJSUB)/photometry.o \
	$(OBJSUB)/pixel_handling.o \
	$(OBJSUB)/pixelmaps.o \
	$(OBJSUB)/resampling.o \
	$(OBJSUB)/shift.o \
	$(OBJSUB)/xcorrelation.o \
	$(OBJSUB)/chebyshev.o \
	$(OBJSUB)/doubles.o \
	$(OBJSUB)/dstats.o \
	$(OBJSUB)/fft_base.o \
	$(OBJSUB)/fit_curve.o \
	$(OBJSUB)/function_1d.o \
	$(OBJSUB)/ipow.o \
	$(OBJSUB)/legendre.o \
	$(OBJSUB)/matrix.o \
	$(OBJSUB)/median.o \
	$(OBJSUB)/optimization.o \
	$(OBJSUB)/poly2d.o \
	$(OBJSUB)/polygon.o \
	$(OBJSUB)/random.o \
	$(OBJSUB)/calendar.o \
	$(OBJSUB)/charmatrix.o \
	$(OBJSUB)/chronos.o \
	$(OBJSUB)/cmdline.o \
	$(OBJSUB)/comm.o \
	$(OBJSUB)/dictionary.o \
	$(OBJSUB)/e_config.o \
	$(OBJSUB)/e_version.o \
	$(OBJSUB)/file_handling.o \
	$(OBJSUB)/filelock.o \
	$(OBJSUB)/filename.o \
	$(OBJSUB)/filesys.o \
	$(OBJSUB)/getopt.o \
	$(OBJSUB)/gnuplot_i.o \
	$(OBJSUB)/iniparser.o \
	$(OBJSUB)/manpage.o \
	$(OBJSUB)/memstr.o \
	$(OBJSUB)/parse_tok.o \
	$(OBJSUB)/pid_i.o \
	$(OBJSUB)/ptrace.o \
	$(OBJSUB)/rtd_i.o \
	$(OBJSUB)/show.o \
	$(OBJSUB)/strlib.o \
	$(OBJSUB)/t_iso8601.o \
	$(OBJSUB)/t_stamp.o \
	$(OBJSUB)/userid.o


STDH=include/ansiterm.h \
     include/calendar.h \
     include/charmatrix.h \
     include/chebyshev.h \
     include/chronos.h \
     include/cmdline.h \
     include/comm.h \
     include/config.h \
     include/corner.h \
     include/cube2image.h \
     include/cube_arith.h \
     include/cube_defs.h \
     include/cube_filters.h \
     include/cube_handling.h \
     include/cube_load.h \
     include/cube_save.h \
     include/dead_pixels.h \
     include/detect.h \
     include/detect_ks.h \
     include/detect_sq.h \
     include/detector.h \
     include/dictionary.h \
     include/doubles.h \
     include/dstats.h \
     include/e_config.h \
     include/e_version.h \
     include/eclipse.h \
     include/emission_lines.h \
     include/extraction.h \
     include/fft_base.h \
     include/file_handling.h \
     include/filelock.h \
     include/filename.h \
     include/filesys.h \
     include/fit_curve.h \
     include/fourier.h \
     include/framelist.h \
     include/function_1d.h \
     include/generate.h \
     include/getopt.h \
     include/gnuplot_i.h \
     include/histogram.h \
     include/history.h \
     include/image_arith.h \
     include/image_filters.h \
     include/image_handling.h \
     include/image_intops.h \
     include/image_io.h \
     include/image_rtd.h \
     include/image_stats.h \
     include/iniparser.h \
     include/intimage.h \
     include/ipow.h \
     include/irlist.h \
     include/irstd.h \
     include/irtemp.h \
     include/legendre.h \
     include/local_types.h \
     include/manpage.h \
     include/matchpoint.h \
     include/matrix.h \
     include/median.h \
     include/memstr.h \
     include/optimization.h \
     include/parse_tok.h \
     include/photometry.h \
     include/pi.h \
     include/pid_i.h \
     include/pixel_handling.h \
     include/pixelmaps.h \
     include/poly2d.h \
     include/polygon.h \
     include/random.h \
     include/resampling.h \
     include/shift.h \
     include/show.h \
     include/static_sz.h \
     include/strlib.h \
     include/t_iso8601.h \
     include/t_stamp.h \
     include/unmem.h \
     include/userid.h \
     include/version.h \
     include/xcorrelation.h

LIBRARY=$(OBJSUB)/libeclipse.a

.KEEP_STATE:
.PRECIOUS: $(LIBRARY)

all: $(PIPELIB)/$(EXT)/libeclipse.a 

$(PIPELIB)/$(EXT)/libeclipse.a: $(LIBRARY)
	cp $(LIBRARY) $(PIPELIB)/$(EXT)
	ranlib $(PIPELIB)/$(EXT)/libeclipse.a

$(LIBRARY): $(OBJSUB) $(OBJECTS)
	rm -f $@
	ar rcvs $@ $(OBJECTS)

$(OBJECTS): $(SOURCES) $(STDH)
	$(COMPILE.c) -o $@ $(@F:.o=.c)

$(OBJSUB):
	-mkdir $(OBJSUB)

.IGNORE:
clean:
	rm -f $(OBJSUB)/*.o

very_clean:
	rm -f $(OBJSUB)/*.o *.bak core $(PIPELIB)/$(LIBRARY) $(LIBRARY)

export:
	tar cvf $(TARFILE) $(SOURCES) $(STDH) Makefile
