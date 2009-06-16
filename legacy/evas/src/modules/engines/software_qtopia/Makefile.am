
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

SOFTWARE_QTOPIA_SOURCES = \
evas_engine.c \
evas_outbuf.c \
evas_qt_main.cpp

SOFTWARE_QTOPIA_LIBADD = @evas_engine_software_qtopia_libs@

pkgdir = $(libdir)/evas/modules/engines/software_qtopia/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Software_Qtopia.h

if !EVAS_STATIC_BUILD_SOFTWARE_QTOPIA

pkg_LTLIBRARIES = module.la
module_la_SOURCES = $(SOFTWARE_QTOPIA_SOURCES)
module_la_LIBADD = @EINA_LIBS@ $(SOFTWARE_QTOPIA_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_qtopia.la

libevas_engine_software_qtopia_la_SOURCES = $(SOFTWARE_QTOPIA_SOURCES)
libevas_engine_software_qtopia_la_LIBADD = $(SOFTWARE_QTOPIA_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
