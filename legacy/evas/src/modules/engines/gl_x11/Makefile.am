
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines/gl_common \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EINA_CFLAGS@ \
@GL_EET_CFLAGS@ \
@evas_engine_gl_xlib_cflags@

if BUILD_ENGINE_GL_X11

GL_X11_SOURCES = \
evas_engine.c \
evas_x_main.c

GL_X11_LIBADD = @evas_engine_gl_xlib_libs@ $(top_builddir)/src/modules/engines/gl_common/libevas_engine_gl_common.la 


includes_HEADERS = Evas_Engine_GL_X11.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_GL_X11

pkgdir = $(libdir)/evas/modules/engines/gl_x11/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = $(GL_X11_SOURCES)
module_la_LIBADD = @EINA_LIBS@ @GL_EET_LIBS@ $(GL_X11_LIBADD) $(top_builddir)/src/lib/libevas.la @dlopen_libs@
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_gl_x11.la

libevas_engine_gl_x11_la_SOURCES = $(GL_X11_SOURCES)
libevas_engine_gl_x11_la_LIBADD = $(GL_X11_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
