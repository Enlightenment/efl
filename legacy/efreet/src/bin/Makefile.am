
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)\" \
@EFREET_CFLAGS@

internal_bindir=$(libdir)/efreet
internal_bin_PROGRAMS = \
efreet_desktop_cache_create \
efreet_icon_cache_create

efreet_desktop_cache_create_LDADD = \
$(top_builddir)/src/lib/libefreet.la \
@EFREET_LIBS@

efreet_desktop_cache_create_SOURCES = \
efreet_desktop_cache_create.c

efreet_icon_cache_create_LDADD = \
$(top_builddir)/src/lib/libefreet.la \
@EFREET_LIBS@

efreet_icon_cache_create_SOURCES = \
efreet_icon_cache_create.c
