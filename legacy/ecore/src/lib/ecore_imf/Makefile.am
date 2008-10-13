MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_IMF
AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@EINA_CFLAGS@

AM_CFLAGS = @WIN32_CFLAGS@

lib_LTLIBRARIES = libecore_imf.la
include_HEADERS = \
Ecore_IMF.h

libecore_imf_la_SOURCES = \
ecore_imf.c \
ecore_imf_context.c \
ecore_imf_module.c \
ecore_imf_private.h

libecore_imf_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@EINA_LIBS@

libecore_imf_la_LDFLAGS = @lt_no_undefined@ @lt_enable_auto_import@ -version-info @version_info@
endif

EXTRA_DIST = \
Ecore_IMF.h \
ecore_imf.c \
ecore_imf_context.c \
ecore_imf_module.c \
ecore_imf_private.h
