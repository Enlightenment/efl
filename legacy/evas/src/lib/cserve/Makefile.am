MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@pthread_cflags@

noinst_LTLIBRARIES = libevas_cserve.la
libevas_cserve_la_SOURCES = \
evas_cs.h \
evas_cs_main.c \
evas_cs_server.c \
evas_cs_client.c \
evas_cs_mem.c

libevas_cserve_la_LIBADD = @EINA_LIBS@ -lrt

libevas_cserve_la_DEPENDENCIES = \
$(top_builddir)/config.h
