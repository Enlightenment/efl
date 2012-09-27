MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-DPACKAGE_BUILD_DIR=\"`pwd`/$(top_builddir)\" \
@CHECK_CFLAGS@ \
@EINA_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = eina_suite cxx_compile_test

eina_suite_SOURCES = \
eina_suite.c \
eina_suite.h \
eina_test_fp.c \
eina_test_stringshare.c \
eina_test_ustringshare.c \
eina_test_ustr.c \
eina_test_binshare.c \
eina_test_binbuf.c \
eina_test_inarray.c \
eina_test_array.c \
eina_test_clist.c \
eina_test_error.c \
eina_test_sched.c \
eina_test_log.c \
eina_test_magic.c \
eina_test_inlist.c \
eina_test_main.c \
eina_test_counter.c \
eina_test_lalloc.c \
eina_test_hash.c \
eina_test_iterator.c \
eina_test_accessor.c \
eina_test_module.c \
eina_test_convert.c \
eina_test_rbtree.c \
eina_test_file.c \
eina_test_benchmark.c \
eina_test_mempool.c \
eina_test_rectangle.c \
eina_test_list.c \
eina_test_matrixsparse.c \
eina_test_tiler.c \
eina_test_strbuf.c \
eina_test_str.c \
eina_test_quadtree.c \
eina_test_simple_xml_parser.c \
eina_test_value.c
# eina_test_model.c

eina_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/eina/libeina.la @EINA_LIBS@ -lm

cxx_compile_test_SOURCES = cxx_compile_test.cxx
cxx_compile_test_LDADD = $(top_builddir)/src/lib/eina/libeina.la @EINA_LIBS@

EXTRA_LTLIBRARIES = module_dummy.la

check-local: module_dummy.la

module_dummy_la_SOURCES = eina_test_module_dummy.c

module_dummy_la_CPPFLAGS = \
-I$(top_srcdir)/src/include/eina \
-I$(top_builddir)/src/include/eina \
@EFL_EINA_BUILD@
module_dummy_la_LIBADD = $(top_builddir)/src/lib/eina/libeina.la @EINA_LIBS@ -lm
module_dummy_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_dummy_la_LIBTOOLFLAGS = --tag=disable-static

endif

clean-local:
	rm -rf *.la *.gcno ..\#..\#src\#*.gcov *.gcda
