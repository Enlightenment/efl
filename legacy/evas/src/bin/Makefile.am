MAINTAINERCLEANFILES = Makefile.in

if EVAS_CSERVE

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

AM_CFLAGS = @WIN32_CFLAGS@

bin_PROGRAMS = evas_cserve evas_cserve_tool

evas_cserve_SOURCES = \
evas_cserve_main.c

evas_cserve_LDADD = \
$(top_builddir)/src/lib/libevas.la

evas_cserve_tool_LDFLAGS =

evas_cserve_tool_SOURCES = \
evas_cserve_tool.c

evas_cserve_tool_LDADD = \
$(top_builddir)/src/lib/libevas.la

evas_cserve_tool_LDFLAGS =

endif
