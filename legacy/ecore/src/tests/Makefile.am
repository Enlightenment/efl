MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_x \
@EINA_CFLAGS@ \
@CHECK_CFLAGS@

if EFL_ENABLE_TESTS

noinst_PROGRAMS = ecore_suite

check_PROGRAMS = ecore_suite

ecore_suite_SOURCES = \
ecore_suite.c \
ecore_test_ecore.c \
ecore_test_ecore_con.c \
ecore_test_ecore_x.c \
ecore_suite.h

ecore_suite_LDADD = \
@CHECK_LIBS@ \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_con/libecore_con.la

if BUILD_ECORE_X
ecore_suite_LDADD += \
$(top_builddir)/src/lib/ecore_x/libecore_x.la
endif

endif

clean-local:
	rm -rf *.gcno *.gcda
