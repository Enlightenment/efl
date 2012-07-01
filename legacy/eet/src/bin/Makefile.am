
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EET_CFLAGS@

bin_PROGRAMS = @EET_PRG@
EXTRA_PROGRAMS = eet

eet_SOURCES = eet_main.c
eet_LDADD = $(top_builddir)/src/lib/libeet.la @EET_LIBS@
eet_LDFLAGS = @lt_enable_auto_import@
