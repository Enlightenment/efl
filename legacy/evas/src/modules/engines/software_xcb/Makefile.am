AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/include -I$(top_srcdir)/src/modules/engines @FREETYPE_CFLAGS@ @xcb_cflags@

if BUILD_ENGINE_SOFTWARE_XCB

pkgdir = $(libdir)/evas/modules/engines/software_xcb/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la
module_la_SOURCES = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_xcb_buffer.c \
evas_xcb_color.c \
evas_xcb_main.c

module_la_LIBADD = @xcb_libs@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
module_la_DEPENDENCIES = $(top_builddir)/config.h

include_HEADERS = Evas_Engine_Software_Xcb.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_xcb_buffer.c \
evas_xcb_color.c \
evas_xcb_main.c \
Evas_Engine_Software_Xcb.h
