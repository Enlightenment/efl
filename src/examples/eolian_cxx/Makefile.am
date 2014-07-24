
AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES =

EOLIAN_FLAGS = \
  -I$(srcdir) \
  -I$(top_srcdir)/src/lib/eo \
  -I$(top_srcdir)/src/lib/evas/canvas \
  -I$(top_srcdir)/src/lib/edje \
  -I$(top_srcdir)/src/lib/efl \
  -I$(top_srcdir)/src/lib/ecore_audio

include $(top_srcdir)/src/Makefile_Eolian_Helper.am
include $(top_srcdir)/src/Makefile_Eolian_Cxx_Helper.am

AM_CXXFLAGS = \
	-I$(srcdir) \
	-I$(builddir) \
	-I$(top_builddir)/src/lib/efl \
	-I$(top_srcdir)/src/lib/eina \
	-I$(top_builddir)/src/lib/eina \
	-I$(top_srcdir)/src/lib/eo \
	-I$(top_builddir)/src/lib/eo \
	-I$(top_srcdir)/src/bindings/eo_cxx \
	-I$(top_builddir)/src/bindings/eo_cxx \
	-I$(top_srcdir)/src/bindings/eina_cxx \
	-I$(top_builddir)/src/bindings/eina_cxx \
	-I$(top_srcdir)/src/bindings/ecore_cxx \
	-I$(top_builddir)/src/bindings/ecore_cxx \
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
	-leina -levas -leo -lecore -lecore_evas -pthread

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

BUILT_SOURCES = $(GENERATED)
CLEANFILES += $(BUILT_SOURCES)

EOS = \
	colourable.eo \
	colourablesquare.eo

IMPL = \
	colourable.c \
	colourable_cxx.cc \
	colourablesquare.c \
	colourablesquare_cxx.cc \
	colourable_stub.c \
	colourablesquare_stub.c

SRCS = \
	eolian_cxx_simple_01.cc \
	eolian_cxx_inherit_01.cc \
	eolian_cxx_callbacks_01.cc \
	eolian_cxx_eo_events_01.cc \
	eolian_cxx_complex_types_01.cc \
	$(IMPL)

EXTRA_PROGRAMS = \
	eolian_cxx_simple_01 \
	eolian_cxx_simple_01_cxx_impl \
	eolian_cxx_inherit_01 \
	eolian_cxx_callbacks_01 \
	eolian_cxx_eo_events_01 \
	eolian_cxx_complex_types_01

DATA_FILES = Makefile.examples $(EOS)
EXTRA_DIST = $(DATA_FILES)

eolian_cxx_simple_01_SOURCES = \
	eolian_cxx_simple_01.cc \
	colourable.c \
	colourablesquare.c
eolian_cxx_simple_01.$(OBJEXT): $(GENERATED)

eolian_cxx_simple_01_cxx_impl_SOURCES = \
	eolian_cxx_simple_01.cc \
	colourable_stub.c \
	colourable_stub.h \
	colourablesquare_stub.c \
	colourablesquare_stub.h \
	colourable_cxx.cc \
	colourablesquare_cxx.cc
eolian_cxx_simple_01_cxx_impl.$(OBJEXT): $(GENERATED)

eolian_cxx_inherit_01_SOURCES = \
	eolian_cxx_inherit_01.cc \
	colourable.c \
	colourablesquare.c
eolian_cxx_inherit_01.$(OBJEXT): $(GENERATED)

eolian_cxx_callbacks_01_SOURCES = eolian_cxx_callbacks_01.cc

eolian_cxx_eo_events_01_SOURCES = eolian_cxx_eo_events_01.cc

eolian_cxx_complex_types_01_SOURCES = eolian_cxx_complex_types_01.cc

%.eo.hh: %.eo
	$(AM_V_EOLCXX)$(EOLIAN_CXX) $(EOLIAN_FLAGS) -I${abs_srcdir} -o $@ $<

%.eo.c: %.eo
	$(AM_V_EOL)$(EOLIAN_GEN) --eo --legacy $(EOLIAN_FLAGS) --gc -o $@ $<

%.eo.h: %.eo
	$(AM_V_EOL)$(EOLIAN_GEN) --eo $(EOLIAN_FLAGS) --gh -o $@ $<

examples: $(EOS) $(GENERATED) $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS) $(GENERATED)

install-examples:
	mkdir -p $(datadir)/eolian_cxx/examples
	cd $(abs_srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eolian_cxx/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eolian_cxx/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

