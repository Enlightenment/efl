MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

#put here all EDCs one needs to the examples
EDCS = basic.edc

filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

if BUILD_EXAMPLES

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DPACKAGE_EXAMPLES_DIR=\"$(datadir)/$(PACKAGE)/examples\"

pkglib_PROGRAMS =

#the ones using ecore_evas follow
AM_CPPFLAGS += @ECORE_EVAS_CFLAGS@

pkglib_PROGRAMS += edje_basic
edje_basic_DEPS = $(srcdir)/basic.edc
edje_basic_SOURCES = edje-basic.c
edje_basic_LDADD = $(top_builddir)/src/lib/libedje.la @ECORE_EVAS_LIBS@

basic.edj: ${edje_basic_DEPS}

.edc.edj:
	$(edje_cc) -v -id $(srcdir) $< $(builddir)/$(@F)

EDJS = $(EDCS:%.edc=%.edj)

files_DATA += $(EDJS)

endif # if BUILD_EXAMPLES

if INSTALL_EXAMPLES

#put here additional data when installing examples
files_DATA += $(srcdir)/red.png

files_DATA += \
	$(EDCS) \
	$(srcdir)/edje-basic.c
endif

EXTRA_DIST = $(EDCS) \
	$(srcdir)/edje-basic.c