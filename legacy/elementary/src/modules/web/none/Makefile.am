AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

CLEANFILES =

EOLIAN_FLAGS = @DEPS_EOLIAN_FLAGS@ \
	       -I$(top_srcdir)/src/lib

include $(top_srcdir)/Makefile_Eolian_Helper.am

AM_CPPFLAGS = \
-DELM_INTERNAL_API_ARGESFSDFEFC=1 \
-DMODULES_PATH=\"$(pkglibdir)/modules\" \
-I. \
-I$(top_builddir) \
-I$(top_srcdir) \
-I$(top_srcdir)/src/imported \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" \
-DLOCALE_DIR=\"$(localedir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DICON_DIR=\"$(datadir)/icons\" \
-DELM_TOP_BUILD_DIR=\"$(top_builddir)\" \
-DELEMENTARY_BUILD \
@ELEMENTARY_CFLAGS@

pkgdir = $(pkglibdir)/modules/web/none/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

BUILT_SOURCES = elm_web_none.eo.h elm_web_none.eo.c

module_la_SOURCES = elm_web_none.c elm_web_none.eo.h
module_la_LIBADD = @ELEMENTARY_LIBS@ $(top_builddir)/src/lib/libelementary.la
module_la_LDFLAGS = -module @EFL_LTMODULE_FLAGS@
module_la_LIBTOOLFLAGS = --tag=disable-static

EXTRA_DIST = elm_web_none.eo


