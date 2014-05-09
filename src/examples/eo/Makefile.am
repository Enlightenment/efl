AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I. \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
@EO_CFLAGS@

EXTRA_PROGRAMS = eo_isa eo_simple

if EO_BUILD_EXAMPLE_EVAS

EXTRA_PROGRAMS += eo_evas

endif

eo_isa_SOURCES = \
isa/eo_isa_complex.c \
isa/eo_isa_complex.h \
isa/eo_isa_interface.c \
isa/eo_isa_interface.h \
isa/eo_isa_main.c \
isa/eo_isa_mixin.c \
isa/eo_isa_mixin.h \
isa/eo_isa_simple.c \
isa/eo_isa_simple.h

eo_isa_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LDFLAGS@ @EFL_PTHREAD_LIBS@

if EO_BUILD_EXAMPLE_EVAS

AM_CPPFLAGS += @ELM_CFLAGS@

eo_evas_SOURCES = \
evas/evas_elw_box.c \
evas/evas_elw_box.h \
evas/evas_elw_boxedbutton.c \
evas/evas_elw_boxedbutton.h  \
evas/evas_elw_button.c \
evas/evas_elw_button.h \
evas/evas_elw_win.h \
evas/evas_elw_win.c \
evas/evas_evas_obj.c \
evas/evas_evas_obj.h \
evas/evas_test.c

eo_evas_LDADD = $(top_builddir)/src/lib/eo/libeo.la @ELM_LIBS@ @EO_LDFLAGS@ @EFL_PTHREAD_LIBS@

endif

eo_simple_SOURCES = \
simple/simple_interface.c \
simple/simple_interface.h \
simple/simple_main.c \
simple/simple_mixin.c \
simple/simple_mixin.h \
simple/simple_simple.c \
simple/simple_simple.h

eo_simple_LDADD = $(top_builddir)/src/lib/eo/libeo.la @EO_LDFLAGS@ @EFL_PTHREAD_LIBS@

DATA_FILES = Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/eo/examples
	$(install_sh_DATA) -c $(DATA_FILES) $(datadir)/eo/examples
	mkdir -p $(datadir)/eo/examples/isa
	$(install_sh_DATA) -c $(eo_isa_SOURCES) $(datadir)/eo/examples/isa
	mkdir -p $(datadir)/eo/examples/simple
	$(install_sh_DATA) -c $(eo_simple_SOURCES) $(datadir)/eo/examples/simple
if EO_BUILD_EXAMPLE_EVAS
	mkdir -p $(datadir)/eo/examples/evas
	$(install_sh_DATA) -c $(eo_evas_SOURCES) $(datadir)/eo/examples/evas
endif

uninstall-local:
	for f in $(DATA_FILES) ; do \
	  rm -f $(datadir)/eo/examples/$$f ; \
	done
	for f in $(eo_isa_SOURCES) ; do \
	  rm -f $(datadir)/eo/examples/isa/$$f ; \
	done
	for f in $(eo_simple_SOURCES) ; do \
	  rm -f $(datadir)/eo/examples/simple/$$f ; \
	done
	for f in $(eo_evas_SOURCES) ; do \
	  rm -f $(datadir)/eo/examples/evas/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
