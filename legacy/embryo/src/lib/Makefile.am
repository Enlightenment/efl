
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@EXOTIC_CFLAGS@ \
@EMBRYO_CPPFLAGS@ \
@EFL_EMBRYO_BUILD@

includes_HEADERS = Embryo.h
includesdir = $(includedir)/embryo-@VMAJ@

lib_LTLIBRARIES = libembryo.la

libembryo_la_SOURCES  = \
embryo_amx.c \
embryo_args.c \
embryo_float.c \
embryo_main.c \
embryo_rand.c \
embryo_str.c \
embryo_time.c

libembryo_la_CFLAGS = @EMBRYO_CFLAGS@
libembryo_la_LIBADD = @EXOTIC_LIBS@ @EVIL_LIBS@ -lm
libembryo_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = embryo_private.h
