## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     = -L/usr/local/lib
INCLUDES	     = \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -I$(includedir) \
                       -I/usr/local/include \
		       @freetype_cflags@ @qt_cflags@

CXXFLAGS             = $(CFLAGS) -DQT_QWS_EBX -DQT_QWS_CUSTOM -DQWS -DNO_DEBUG 

if BUILD_ENGINE_SOFTWARE_QTOPIA

noinst_LTLIBRARIES      = libevas_engine_software_qtopia.la
libevas_engine_software_qtopia_la_SOURCES  = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_qt_main.cpp

libevas_engine_software_qtopia_la_LIBADD       = \
@qt_libs@ $(LDFLAGS)

libevas_engine_software_qtopia_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_qt_main.cpp
