
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines/gl_common \
@EINA_CFLAGS@ \
@GL_EET_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@evas_engine_gl_cocoa_cflags@

if BUILD_ENGINE_GL_COCOA

GL_COCOA_SOURCES = evas_engine.c\
evas_gl_cocoa_main.m


GL_COCOA_LIBADD = @evas_engine_gl_cocoa_libs@ $(top_builddir)/src/modules/engines/gl_common/libevas_engine_gl_common.la


include_HEADERS = Evas_Engine_GL_Cocoa.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_GL_COCOA

pkgdir = $(libdir)/evas/modules/engines/gl_cocoa/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = $(GL_COCOA_SOURCES)
module_la_LIBADD = @EINA_LIBS@ @GL_EET_LIBS@ $(GL_COCOA_LIBADD) $(top_builddir)/src/lib/libevas.la @dlopen_libs@
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_gl_cocoa.la

libevas_engine_gl_cocoa_la_SOURCES = $(GL_COCOA_SOURCES)
libevas_engine_gl_cocoa_la_LIBADD = $(GL_COCOA_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
