MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
@CHECK_CFLAGS@


if EET_ENABLE_TESTS

check_PROGRAMS = eet_suite

eet_suite_SOURCES = eet_suite.c eet_data_suite.c
eet_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/libeet.la

endif

EXTRA_DIST = eet_suite.h
