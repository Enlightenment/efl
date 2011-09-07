MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ESCAPE_CPPFLAGS@

base_sources = \
escape_mman.c \
escape_libgen.c \
escape_unistd.c

EXTRA_DIST = \
sys/mman.h \
Escape.h

nobase_stdheadersdir = $(includedir)/escape-@VMAJ@
dist_nobase_stdheaders_DATA = Escape.h escape_libgen.h escape_unistd.h
stdheadersdir = $(includedir)/escape-@VMAJ@/sys
dist_stdheaders_DATA = sys/mman.h


lib_LTLIBRARIES = libescape.la

libescape_la_SOURCES = $(base_sources)

libescape_la_LIBADD = @iconv_libs@ @rt_libs@ @ESCAPE_LIBS@
libescape_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@
libescape_la_CFLAGS = @ESCAPE_CFLAGS@
