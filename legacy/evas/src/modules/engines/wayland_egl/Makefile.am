
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines/gl_common \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@GL_EET_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@evas_engine_wayland_egl_cflags@

if BUILD_ENGINE_WAYLAND_EGL

WAYLAND_EGL_SOURCES  = \
evas_engine.c \
evas_wl_main.c

WAYLAND_EGL_LIBADD = @FREETYPE_LIBS@ @GL_EET_LIBS@ @EVAS_GENERAL_LIBS@ @evas_engine_wayland_egl_libs@ @dlopen_libs@ $(top_builddir)/src/modules/engines/gl_common/libevas_engine_gl_common.la

includes_HEADERS = Evas_Engine_Wayland_Egl.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_WAYLAND_EGL

pkgdir = $(libdir)/evas/modules/engines/wayland_egl/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = $(WAYLAND_EGL_SOURCES)
module_la_LIBADD = $(WAYLAND_EGL_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static


else

noinst_LTLIBRARIES = libevas_engine_wayland_egl.la

libevas_engine_wayland_egl_la_SOURCES = $(WAYLAND_EGL_SOURCES)
libevas_engine_wayland_egl_la_LIBADD = $(WAYLAND_EGL_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
