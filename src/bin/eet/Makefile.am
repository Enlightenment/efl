MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eet \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eet \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EET_CFLAGS@

if HAVE_WINDOWS
AM_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil
endif

bin_PROGRAMS = eet

eet_SOURCES = eet_main.c
eet_LDADD = \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/eina/libeina.la

if HAVE_WINDOWS
eet_LDADD += $(top_builddir)/src/lib/evil/libdl.la
endif

eet_LDADD += @EET_LIBS@

eet_LDFLAGS = @lt_enable_auto_import@
