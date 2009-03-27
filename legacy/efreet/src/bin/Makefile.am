
SUBDIRS = data compare

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EFREET_CFLAGS@

bin_PROGRAMS = efreet_test efreet_spec_test efreet_cache_test

efreet_test_DEPENDENCIES = $(top_builddir)/src/lib/libefreet.la \
                           $(top_builddir)/src/lib/libefreet_mime.la
efreet_test_LDADD = $(top_builddir)/src/lib/libefreet.la \
                    $(top_builddir)/src/lib/libefreet_mime.la \
                    @EFREET_LIBS@
efreet_test_SOURCES = \
ef_test.h \
ef_data_dirs.c \
ef_icon_theme.c \
ef_ini.c \
ef_utils.c \
ef_desktop.c \
ef_menu.c \
ef_mime.c \
main.c

if DEFAULT_VISIBILITY
efreet_test_SOURCES += \
ef_locale.c
endif

efreet_spec_test_LDADD = $(top_builddir)/src/lib/libefreet.la @EINA_LIBS@
efreet_spec_test_SOURCES = \
efreet_spec_test.c

efreet_cache_test_LDADD = $(top_builddir)/src/lib/libefreet.la @EFREET_LIBS@
efreet_cache_test_SOURCES = \
ef_cache.c
