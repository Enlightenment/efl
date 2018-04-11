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

# Eolian_*_Helper.am has this variable as a relative path to src
# Thus we need to override it here in order to make its pattern
# rules work properly.
_EOLIAN_GEN_DEP = ../../bin/eolian/eolian_gen${EXEEXT}
_EOLIAN_CXX_DEP = ../../bin/eolian_cxx/eolian_cxx${EXEEXT}

AM_CXXFLAGS = \
	-I$(srcdir) \
	-I$(builddir) \
	-I$(top_srcdir)/src/lib/efl \
	-I$(top_builddir)/src/lib/efl \
	-I$(top_srcdir)/src/lib/efl/interfaces \
	-I$(top_builddir)/src/lib/efl/interfaces \
	-I$(top_srcdir)/src/lib/eina \
	-I$(top_builddir)/src/lib/eina \
	-I$(top_srcdir)/src/lib/emile \
	-I$(top_builddir)/src/lib/emile \
	-I$(top_srcdir)/src/lib/eo \
	-I$(top_builddir)/src/lib/eo \
	-I$(top_srcdir)/src/bindings/cxx/eo_cxx \
	-I$(top_builddir)/src/bindings/cxx/eo_cxx \
	-I$(top_srcdir)/src/bindings/cxx/eina_cxx \
	-I$(top_builddir)/src/bindings/cxx/eina_cxx \
	-I$(top_srcdir)/src/bindings/cxx/ecore_cxx \
	-I$(top_builddir)/src/bindings/cxx/ecore_cxx \
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

if HAVE_WINDOWS
AM_CXXFLAGS += \
@EFL_WINDOWS_VERSION_CFLAGS@ \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil
endif

AM_CFLAGS = $(AM_CXXFLAGS)

AM_LDFLAGS = \
	-L$(top_builddir)/src/lib/efl \
	-L$(top_builddir)/src/lib/eina \
	-L$(top_builddir)/src/lib/emile \
	-L$(top_builddir)/src/lib/eo \
  -L$(top_builddir)/src/lib/evas \
	-L$(top_builddir)/src/lib/ecore \
	-L$(top_builddir)/src/lib/ecore_evas \
	-lefl -leina -levas -leo -lecore -lecore_evas -pthread

LDADD = \
	$(top_builddir)/src/lib/efl/libefl.la \
	$(top_builddir)/src/lib/eo/libeo.la \
	$(top_builddir)/src/lib/eina/libeina.la \
	$(top_builddir)/src/lib/emile/libemile.la \
	$(top_builddir)/src/lib/evas/libevas.la \
	$(top_builddir)/src/lib/ecore_evas/libecore_evas.la

IMPL = \
	colourable.c \
	colourable_cxx.cc \
	colourablesquare.c \
	colourablesquare_cxx.cc

SRCS = \
	eolian_cxx_simple_01.cc \
	eolian_cxx_callbacks_01.cc \
	ns_colourable.eo \
	ns_colourablesquare.eo \
	$(IMPL)

if HAVE_CXX
GENERATED = \
	ns_colourable.eo.c \
	ns_colourable.eo.h \
	ns_colourable.eo.hh \
	ns_colourable.eo.impl.hh \
	ns_colourablesquare.eo.c \
	ns_colourablesquare.eo.h \
	ns_colourablesquare.eo.hh \
	ns_colourablesquare.eo.impl.hh

BUILT_SOURCES = $(GENERATED)
CLEANFILES += $(BUILT_SOURCES)

EXTRA_PROGRAMS = \
	eolian_cxx_simple_01 \
	eolian_cxx_simple_01_cxx_impl \
	eolian_cxx_callbacks_01

eolian_cxx_simple_01_SOURCES = \
	eolian_cxx_simple_01.cc \
	colourable.c \
	colourablesquare.c
eolian_cxx_simple_01.$(OBJEXT): $(GENERATED)

eolian_cxx_simple_01_cxx_impl_SOURCES = \
	eolian_cxx_simple_01.cc \
	colourable.c \
	colourablesquare.c \
	colourable_cxx.cc \
	colourablesquare_stub.h \
	colourable_stub.h \
	colourablesquare_cxx.cc
eolian_cxx_simple_01_cxx_impl.$(OBJEXT): $(GENERATED)

eolian_cxx_callbacks_01_SOURCES = eolian_cxx_callbacks_01.cc
endif

DATA_FILES = Makefile.examples $(ECXX_EXAMPLE_EOS)
EXTRA_DIST = $(DATA_FILES) \
ns_colourable.eo \
ns_colourablesquare.eo

examples: $(ECXX_EXAMPLE_EOS) $(GENERATED) $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS) $(GENERATED)

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eolian_cxx/examples
	cd $(abs_srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(DESTDIR)$(datadir)/eolian_cxx/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eolian_cxx/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif

