
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@DIRECTFB_CFLAGS@

if BUILD_ENGINE_DIRECTFB

pkgdir = $(libdir)/evas/modules/engines/directfb/$(MODULE_ARCH)

pkg_LTLIBRARIES = module.la

module_la_SOURCES  = \
evas_engine_dfb.c \
evas_engine_dfb_image_objects.c \
evas_engine_dfb_image_objects.h \
evas_engine_dfb.h

module_la_LIBADD = @DIRECTFB_LIBS@ $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_DEPENDENCIES = $(top_builddir)/config.h

include_HEADERS = Evas_Engine_DirectFB.h

endif

EXTRA_DIST = \
evas_engine_dfb.c \
evas_engine_dfb_image_objects.c \
evas_engine_dfb_image_objects.h \
evas_engine_dfb.h \
Evas_Engine_DirectFB.h
