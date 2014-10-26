
if EFL_HAVE_TESTS

check_PROGRAMS = elm_code_suite

elm_code_suite_SOURCES = \
elm_code_file_test_load.c \
elm_code_test_basic.c \
elm_code_suite.c

elm_code_suite_CPPFLAGS = -I$(top_builddir)/elm_code/lib/ \
-DPACKAGE_TESTS_DIR=\"$(top_srcdir)/elm_code/tests/\" \
-DPACKAGE_BUILD_DIR=\"`pwd`/$(top_builddir)/elm_code/tests/\" \
@EFL_CFLAGS@ \
@CHECK_CFLAGS@

elm_code_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/elm_code/lib/libelm_code.la
elm_code_suite_DEPENDENCIES = $(top_builddir)/elm_code/lib/libelm_code.la

endif

EXTRA_DIST = elm_code_suite.h
