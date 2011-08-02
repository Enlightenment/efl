MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

#put here all EDCs one needs to the examples
EDCS = basic.edc \
	swallow.edc \
	text.edc \
	table.edc \
	box.edc \
	signals-messages.edc

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
AM_CPPFLAGS += @EDJE_CFLAGS@

pkglib_PROGRAMS += \
	edje-basic \
	edje-swallow \
	edje-text \
	edje-table \
	edje-box \
	edje-box2 \
	edje-signals-messages

LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_LIBS@

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
	$(srcdir)/edje-basic.c \
	$(srcdir)/edje-swallow.c \
	$(srcdir)/edje-text.c \
	$(srcdir)/edje-table.c \
	$(srcdir)/edje-box.c \
	$(srcdir)/edje-box2.c \
	$(srcdir)/edje-signals-messages.c
endif

EXTRA_DIST = $(EDCS) \
	$(srcdir)/red.png \
	$(srcdir)/edje-basic.c \
	$(srcdir)/edje-swallow.c \
	$(srcdir)/edje-text.c \
	$(srcdir)/edje-table.c \
	$(srcdir)/edje-box.c \
	$(srcdir)/edje-box2.c \
	$(srcdir)/edje-signals-messages.c
