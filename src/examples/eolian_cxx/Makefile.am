
AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = \
	-I. \
	-I$(top_builddir)/src/lib/efl \
	-I$(top_srcdir)/src/lib/eina \
	-I$(top_builddir)/src/lib/eina \
	-I$(top_srcdir)/src/lib/eo \
	-I$(top_builddir)/src/lib/eo \
	-I$(top_srcdir)/src/bindings/eo_cxx \
	-I$(top_builddir)/src/bindings/eo_cxx \
	-I$(top_srcdir)/src/bindings/eina_cxx \
	-I$(top_builddir)/src/bindings/eina_cxx \
	-I$(top_srcdir)/src/lib/evas/ \
	-I$(top_builddir)/src/lib/evas/ \
	-I$(top_srcdir)/src/lib/evas/common \
	-I$(top_builddir)/src/lib/evas/common \
	-I$(top_srcdir)/src/lib/evas/canvas \
	-I$(top_builddir)/src/lib/evas/canvas \
	-I$(top_srcdir)/src/lib/ecore \
	-I$(top_builddir)/src/lib/ecore \
	-I$(top_srcdir)/src/lib/ecore_evas \
	-I$(top_builddir)/src/lib/ecore_evas

AM_CFLAGS = $(AM_CXXFLAGS)

AM_LDFLAGS = \
	-L$(top_builddir)/src/lib/efl \
	-L$(top_builddir)/src/lib/eina \
	-L$(top_builddir)/src/lib/eo \
  -L$(top_builddir)/src/lib/evas \
	-L$(top_builddir)/src/lib/ecore \
	-L$(top_builddir)/src/lib/ecore_evas \
	-leina -levas -leo -lecore -lecore_evas

LDADD = \
	$(top_builddir)/src/lib/eo/libeo.la \
	$(top_builddir)/src/lib/eina/libeina.la \
	$(top_builddir)/src/lib/evas/libevas.la \
	$(top_builddir)/src/lib/ecore_evas/libecore_evas.la

GENERATED = \
	colourable.eo.c \
	colourable.eo.h \
	colourable.eo.hh \
	colourablesquare.eo.c \
	colourablesquare.eo.h \
	colourablesquare.eo.hh

EOS = \
	colourable.eo \
	colourablesquare.eo

SRCS = \
	eolian_cxx_simple_01.cc \
	eolian_cxx_inherit_01.cc

EXTRA_PROGRAMS = \
	eolian_cxx_simple_01 \
	eolian_cxx_inherit_01

CLEANFILES =

eolian_cxx_simple_01_SOURCES = \
	eolian_cxx_simple_01.cc \
	colourable.c \
	colourablesquare.c

eolian_cxx_inherit_01_SOURCES = \
	eolian_cxx_inherit_01.cc \
	colourable.c \
	colourablesquare.c

EOLIAN_GEN = $(top_builddir)/src/bin/eolian/eolian_gen${EXEEXT}
EOLIAN_CXX = $(top_builddir)/src/bin/eolian_cxx/eolian_cxx${EXEEXT}
EOLIAN_FLAGS = \
-I$(srcdir) \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas/canvas \
-I$(top_srcdir)/src/lib/edje \
-I$(top_srcdir)/src/lib/ecore_audio

SUFFIXES = .eo .eo.c .eo.h .eo.hh

%.eo.hh: %.eo $(EOLIAN_CXX)
	$(AM_V_EOLCXX)$(EOLIAN_CXX) $(EOLIAN_FLAGS) -I$< -o $@

%.eo.c: %.eo $(EOLIAN_GEN)
	$(AM_V_EOL)$(EOLIAN_GEN) --eo --legacy $(EOLIAN_FLAGS) --gc -o $@ $<

%.eo.h: %.eo $(EOLIAN_GEN)
	$(AM_V_EOL)$(EOLIAN_GEN) --eo $(EOLIAN_FLAGS) --gh -o $@ $<

examples: $(EOS) $(GENERATED) $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS) $(GENERATED)

install-examples:
	mkdir -p $(datadir)/eolian_cxx/examples
	$(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eolian_cxx/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eolian_cxx/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
