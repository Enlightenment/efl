
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = canvas data cache file engines imaging include

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@FREETYPE_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@pthread_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

lib_LTLIBRARIES = libevas.la

include_HEADERS = Evas.h Evas_Data.h

libevas_la_SOURCES = main.c

libevas_la_LIBADD = \
canvas/libevas_canvas.la \
data/libevas_data.la \
file/libevas_file.la \
cache/libevas_cache.la \
imaging/libevas_imaging.la \
engines/common/libevas_engine_common.la \
engines/common_16/libevas_engine_common_16.la \
-lm \
@fnmatch_libs@ \
@dlopen_libs@ \
@FREETYPE_LIBS@ \
@EET_LIBS@ \
@FONTCONFIG_LIBS@ \
@pthread_libs@

libevas_la_DEPENDENCIES = \
$(top_builddir)/config.h \
canvas/libevas_canvas.la \
data/libevas_data.la \
file/libevas_file.la \
cache/libevas_cache.la \
imaging/libevas_imaging.la \
engines/common/libevas_engine_common.la \
engines/common_16/libevas_engine_common_16.la

libevas_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@

