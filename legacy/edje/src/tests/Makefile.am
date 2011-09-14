MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
@EDJE_CFLAGS@ \
@CHECK_CFLAGS@

if EFL_ENABLE_TESTS

noinst_PROGRAMS = edje_suite

check_PROGRAMS = edje_suite

edje_suite_SOURCES = \
edje_suite.c \
edje_test_edje.c \
edje_suite.h

edje_suite_LDADD = \
@EDJE_LIBS@ \
@CHECK_LIBS@ \
$(top_builddir)/src/lib/libedje.la

endif

clean-local:
	rm -rf *.gcno *.gcda
