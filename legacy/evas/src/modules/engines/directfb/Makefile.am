## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@ @DIRECTFB_CFLAGS@

if BUILD_ENGINE_DIRECTFB

pkgdir			= $(libdir)/evas/modules/engines/directfb/$(MODULE_ARCH)
pkg_LTLIBRARIES      = module.la

module_la_SOURCES  = \
evas_engine_dfb.c \
evas_engine_dfb_image_objects.c \
evas_engine_dfb_image_objects.h \
evas_engine_dfb.h

module_la_LIBADD       = @DIRECTFB_LIBS@
module_la_LDFLAGS      = -module -avoid-version \
				-L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs

module_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine_dfb.c \
evas_engine_dfb_image_objects.c \
evas_engine_dfb_image_objects.h \
evas_engine_dfb.h
