## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -I$(top_srcdir)/src/modules/engines \
		       @FREETYPE_CFLAGS@ @qt_cflags@

CXXFLAGS             = $(CFLAGS) -DQT_QWS_EBX -DQT_QWS_CUSTOM -DQWS -DNO_DEBUG 

if BUILD_ENGINE_SOFTWARE_QTOPIA

pkgdir                               = $(libdir)/evas/modules/engines/software_qtopia/$(MODULE_ARCH)

pkg_LTLIBRARIES         = module.la

module_la_SOURCES  = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_qt_main.cpp

module_la_LIBADD       = @qt_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS      = -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs

module_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_qt_main.cpp
