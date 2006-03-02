AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include -I$(top_srcdir)/src/modules/engines @FREETYPE_CFLAGS@

if BUILD_ENGINE_BUFFER

pkgdir = $(libdir)/evas/modules/engines/buffer/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS =  -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
module_la_DEPENDENCIES = $(top_builddir)/config.h

include_HEADERS = Evas_Engine_Buffer.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
Evas_Engine_Buffer.h
