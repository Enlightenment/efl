AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include -I$(top_srcdir)/src/modules/engines @FREETYPE_CFLAGS@

if BUILD_ENGINE_SOFTWARE_16

pkgdir = $(libdir)/evas/modules/engines/software_16/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES  = \
evas_engine.c \
evas_soft16.h \
evas_soft16_main.c

module_la_LIBADD = $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
module_la_DEPENDENCIES = $(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.c \
evas_soft16.h \
evas_soft16_main.c
