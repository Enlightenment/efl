
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
-DEFL_EMBRYO_BUILD \
@EVIL_CFLAGS@ \
@EXOTIC_CFLAGS@

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
embryo_time.c \
embryo_private.h

libembryo_la_LIBADD = @EXOTIC_LIBS@ @EVIL_LIBS@ -lm
libembryo_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@
