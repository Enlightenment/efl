
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-DELM_INTERNAL_API_ARGESFSDFEFC=1 \
-I. \
-I$(top_builddir) \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DELEMENTARY_BUILD \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_FB_CFLAGS@ \
@ELEMENTARY_SDL_CFLAGS@ \
@ELEMENTARY_WIN32_CFLAGS@ \
@ELEMENTARY_ELOCATION_CFLAGS@ \
@ELEMENTARY_EWEATHER_CFLAGS@ \
@ELEMENTARY_WEB_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_WAYLAND_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_DRM_CFLAGS@

pkgdir = $(libdir)/elementary/modules/datetime_input_ctxpopup/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = datetime_input_ctxpopup.c

module_la_LIBADD = @ELEMENTARY_LIBS@ $(top_builddir)/src/lib/libelementary.la
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
