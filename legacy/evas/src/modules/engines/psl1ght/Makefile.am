
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EINA_CFLAGS@ \
@evas_engine_psl1ght_cflags@

if BUILD_ENGINE_PSL1GHT

PSL1GHT_SOURCES = \
evas_engine.c \
rsxutil.c

PSL1GHT_LIBADD = @evas_engine_psl1ght_libs@


includes_HEADERS = Evas_Engine_PSL1GHT.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_PSL1GHT

pkgdir = $(libdir)/evas/modules/engines/psl1ght/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = $(PSL1GHT_SOURCES)
module_la_LIBADD = @EINA_LIBS@ $(PSL1GHT_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_psl1ght.la

libevas_engine_psl1ght_la_SOURCES = $(PSL1GHT_SOURCES)
libevas_engine_psl1ght_la_LIBADD = $(PSL1GHT_LIBADD)

endif
endif

EXTRA_DIST = \
evas_engine.h \
rsxutil.h
