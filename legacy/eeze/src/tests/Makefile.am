MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-DTESTS_SRC_DIR=\"$(top_srcdir)/src/tests\" \
@CHECK_CFLAGS@ \
@EEZE_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = eeze_suite

eeze_suite_SOURCES = eeze_suite.c
eeze_suite_LDADD = $(top_builddir)/src/lib/libeeze.la @EEZE_LIBS@ @CHECK_LIBS@

endif
