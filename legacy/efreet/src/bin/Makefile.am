
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EFREET_CFLAGS@

internal_bindir=$(libdir)/efreet
internal_bin_PROGRAMS = \
efreet_desktop_cache_create

if ICON_CACHE
internal_bin_PROGRAMS += \
efreet_icon_cache_create
endif

efreet_desktop_cache_create_LDADD = \
$(top_builddir)/src/lib/libefreet.la \
@EFREET_LIBS@

efreet_desktop_cache_create_SOURCES = \
efreet_desktop_cache_create.c

if ICON_CACHE

efreet_icon_cache_create_LDADD = \
$(top_builddir)/src/lib/libefreet.la \
@EFREET_LIBS@

efreet_icon_cache_create_SOURCES = \
efreet_icon_cache_create.c

endif
