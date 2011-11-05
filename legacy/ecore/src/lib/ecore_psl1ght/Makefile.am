MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_input \
@EFL_ECORE_PSL1GHT_BUILD@ \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

lib_LTLIBRARIES = libecore_psl1ght.la
includes_HEADERS = Ecore_Psl1ght.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_psl1ght_la_SOURCES = \
ecore_psl1ght.c \
moveutil.c \
spursutil.c

libecore_psl1ght_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
@EINA_LIBS@

libecore_psl1ght_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

EXTRA_DIST = Ecore_Psl1ght_Keys.h ecore_psl1ght_private.h
