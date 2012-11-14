MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
@CHECK_CFLAGS@ \
@EET_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = evas_suite

evas_suite_SOURCES = \
evas_suite.c \
evas_test_init.c \
evas_test_textblock.c \
evas_test_text.c \
evas_test_callbacks.c \
evas_tests_helpers.h \
evas_suite.h
evas_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/evas/libevas.la $(top_builddir)/src/lib/eina/libeina.la

endif
