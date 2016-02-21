MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = check-results.xml

if EFL_HAVE_TESTS

check_PROGRAMS = elm_code_suite

elm_code_suite_SOURCES = \
elm_code_file_test_load.c \
elm_code_file_test_memory.c \
elm_code_test_basic.c \
elm_code_test_line.c \
elm_code_test_parse.c \
elm_code_test_text.c \
widget/elm_code_test_widget.c \
widget/elm_code_test_widget_text.c \
widget/elm_code_test_widget_selection.c \
widget/elm_code_test_widget_undo.c \
elm_code_suite.c

elm_code_suite_CPPFLAGS = \
-DEFL_BETA_API_SUPPORT \
-DEFL_EO_API_SUPPORT \
-I$(top_srcdir)/elm_code/src/lib \
-I$(top_builddir)/elm_code/src/lib \
-I$(top_srcdir)/elm_code/src/lib/widget \
-I$(top_builddir)/elm_code/src/lib/widget \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DPACKAGE_TESTS_DIR=\"$(top_srcdir)/elm_code/src/tests/\" \
-DPACKAGE_BUILD_DIR=\"`pwd`/$(top_builddir)/elm_code/src/tests/\" \
-DTESTS_DIR=\"$(abspath $(srcdir))/\" \
-DEFL_BETA_API_SUPPORT \
@EFL_CFLAGS@ \
-DEFL_ELM_CODE_BUILD \
@CHECK_CFLAGS@

elm_code_suite_LDADD = @EFL_LIBS@ @CHECK_LIBS@ $(top_builddir)/elm_code/src/lib/libelm_code.la
elm_code_suite_DEPENDENCIES = $(top_builddir)/elm_code/src/lib/libelm_code.la

testdir = $(PACKAGE_TESTS_DIR)
test_DATA = \
testfile.txt \
testfile-windows.txt \
testfile-withblanks.txt \
testdiff.diff

EXTRA_DIST = elm_code_suite.h $(test_DATA)

endif
