
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
-I$(top_srcdir)/src/modules/engines/software_8 \
@FREETYPE_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_software_8_x11_cflags@ \
-I/usr/include/pixman-1

if BUILD_ENGINE_SOFTWARE_8_X11

SOFTWARE_8_X11_SOURCES =  \
evas_engine.c \
evas_x_buffer.c

SOFTWARE_8_X11_LIBADD = @evas_engine_software_8_x11_libs@

pkgdir = $(libdir)/evas/modules/engines/software_8_x11/$(MODULE_ARCH)

include_HEADERS = Evas_Engine_Software_8_X11.h

if !EVAS_STATIC_BUILD_SOFTWARE_8_X11

pkg_LTLIBRARIES	= module.la

module_la_SOURCES = $(SOFTWARE_8_X11_SOURCES)
module_la_LIBADD = @EINA_LIBS@ $(SOFTWARE_8_X11_LIBADD) $(top_builddir)/src/lib/libevas.la
#-lxcb-image -lxcb-shm -lxcb -lpixman-1
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_software_8_x11.la

libevas_engine_software_8_x11_la_SOURCES = $(SOFTWARE_8_X11_SOURCES)
libevas_engine_software_8_x11_la_LIBADD = $(SOFTWARE_8_X11_LIBADD)

endif
endif

EXTRA_DIST = evas_engine.h
