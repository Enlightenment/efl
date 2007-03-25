SUBDIRS = data compare
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I. -I$(top_srcdir)/src/lib @ECORE_CFLAGS@

bin_PROGRAMS = efreet_test efreet_spec_test efreet_cache_test

efreet_test_DEPENDENCIES = $(top_builddir)/src/lib/libefreet.la
efreet_test_LDADD = $(top_builddir)/src/lib/libefreet.la @ECORE_LIBS@
efreet_test_SOURCES = \
ef_data_dirs.c \
ef_icon_theme.c \
ef_ini.c \
ef_locale.c \
ef_desktop.c \
ef_menu.c \
ef_utils.c \
main.c


efreet_spec_test_DEPENDENCIES = $(top_builddir)/src/lib/libefreet.la
efreet_spec_test_LDADD = $(top_builddir)/src/lib/libefreet.la
efreet_spec_test_SOURCES = \
efreet_spec_test.c

efreet_cache_test_DEPENDENCIES = $(top_builddir)/src/lib/libefreet.la
efreet_cache_test_LDADD = $(top_builddir)/src/lib/libefreet.la @ECORE_LIBS@
efreet_cache_test_SOURCES = \
ef_cache.c
