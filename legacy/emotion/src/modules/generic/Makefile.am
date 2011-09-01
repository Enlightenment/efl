MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/modules \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EMOTION_CFLAGS@ \
@EMOTION_CPPFLAGS@ \
@EFL_EMOTION_BUILD@

if EMOTION_BUILD_GENERIC
if !EMOTION_STATIC_BUILD_GENERIC

pkgdir = $(libdir)/emotion

pkg_LTLIBRARIES = em_generic.la
includes_HEADERS = Emotion_Generic_Plugin.h
includesdir = $(includedir)/emotion-@VMAJ@
noinst_HEADERS = emotion_generic.h

em_generic_la_SOURCES = emotion_generic.c
em_generic_la_LIBADD = $(top_builddir)/src/lib/libemotion.la
em_generic_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
em_generic_la_LIBTOOLFLAGS = --tag=disable-static
em_generic_la_DEPENDENCIES = emotion_generic.h Emotion_Generic_Plugin.h

endif
endif
