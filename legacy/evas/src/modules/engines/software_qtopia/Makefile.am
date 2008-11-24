
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_software_qtopia_cflags@

AM_CXXFLAGS = $(CFLAGS) -DQT_QWS_EBX -DQT_QWS_CUSTOM -DQWS -DNO_DEBUG

if BUILD_ENGINE_SOFTWARE_QTOPIA

pkgdir = $(libdir)/evas/modules/engines/software_qtopia/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_qt_main.cpp

module_la_LIBADD = @EINA_LIBS@ @evas_engine_software_qtopia_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

include_HEADERS = Evas_Engine_Software_Qtopia.h

endif

EXTRA_DIST = evas_engine.h
