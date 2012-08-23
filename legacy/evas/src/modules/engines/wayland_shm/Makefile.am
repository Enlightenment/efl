
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve2 \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@evas_engine_wayland_shm_cflags@

if BUILD_ENGINE_WAYLAND_SHM

WAYLAND_SHM_SOURCES = \
evas_engine.c \
evas_outbuf.c

WAYLAND_SHM_LIBADD = @FREETYPE_LIBS@ @PIXMAN_LIBS@ @EVAS_GENERAL_LIBS@ @evas_engine_wayland_shm_libs@

includes_HEADERS = Evas_Engine_Wayland_Shm.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_WAYLAND_SHM

pkgdir = $(libdir)/evas/modules/engines/wayland_shm/$(MODULE_ARCH)
pkg_LTLIBRARIES	= module.la

module_la_SOURCES = $(WAYLAND_SHM_SOURCES)
module_la_LIBADD = $(WAYLAND_SHM_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_wayland_shm.la

libevas_engine_wayland_shm_la_SOURCES = $(WAYLAND_SHM_SOURCES)
libevas_engine_wayland_shm_la_LIBADD = $(WAYLAND_SHM_LIBADD)

endif
endif

EXTRA_DIST = \
evas_engine.h
