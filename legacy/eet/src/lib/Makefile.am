
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@COVERAGE_CFLAGS@

include_HEADERS = Eet.h

lib_LTLIBRARIES = libeet.la

libeet_la_SOURCES  = \
eet_lib.c \
eet_data.c \
eet_image.c \
eet_memfile.c \
eet_dictionary.c \
eet_utils.c

libeet_la_CFLAGS = @WIN32_CFLAGS@
libeet_la_LIBADD = @COVERAGE_LIBS@ @EVIL_LIBS@ @WIN32_LIBS@ -lz -ljpeg @fnmatch_libs@ -lm
libeet_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@
libeet_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST = Eet_private.h

clean-local:
	@rm -rf *.gcno
