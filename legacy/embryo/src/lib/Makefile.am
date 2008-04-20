
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\"

include_HEADERS = Embryo.h

lib_LTLIBRARIES = libembryo.la

libembryo_la_SOURCES  = \
embryo_amx.c \
embryo_args.c \
embryo_float.c \
embryo_main.c \
embryo_rand.c \
embryo_str.c \
embryo_time.c

libembryo_la_CFLAGS = @WIN32_CFLAGS@
libembryo_la_LIBADD = -lm @fnmatch_libs@
libembryo_la_DEPENDENCIES = $(top_builddir)/config.h
libembryo_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@

EXTRA_DIST = embryo_private.h
