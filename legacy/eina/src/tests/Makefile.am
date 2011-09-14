MAINTAINERCLEANFILES = Makefile.in

benchdir = $(bindir)

AM_CPPFLAGS = 					\
-I$(top_srcdir)/src/lib 			\
-I$(top_srcdir)/src/include 			\
-I$(top_builddir)/src/include 			\
-I$(top_builddir)/src/lib 			\
-DPACKAGE_BIN_DIR=\"$(bindir)\" 		\
-DPACKAGE_LIB_DIR=\"$(libdir)\" 		\
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" 	\
-DPACKAGE_BUILD_DIR=\"`pwd`/$(top_builddir)\" 	\
@CHECK_CFLAGS@					\
@GLIB_CFLAGS@

if EINA_HAVE_GLIB

AM_CPPFLAGS += -DEINA_BENCH_HAVE_GLIB

endif

if EINA_ENABLE_BENCHMARK_E17

AM_CPPFLAGS += -DEINA_ENABLE_BENCH_E17

endif

if EFL_ENABLE_TESTS

check_PROGRAMS = eina_suite

eina_suite_SOURCES = 	\
eina_suite.c 		\
eina_test_fp.c		\
eina_test_stringshare.c \
eina_test_ustringshare.c\
eina_test_ustr.c	\
eina_test_binshare.c    \
eina_test_binbuf.c	\
eina_test_array.c 	\
eina_test_error.c	\
eina_test_sched.c       \
eina_test_log.c 	\
eina_test_magic.c 	\
eina_test_inlist.c 	\
eina_test_main.c 	\
eina_test_counter.c 	\
eina_test_lalloc.c 	\
eina_test_hash.c 	\
eina_test_iterator.c	\
eina_test_accessor.c	\
eina_test_module.c	\
eina_test_convert.c	\
eina_test_rbtree.c	\
eina_test_file.c	\
eina_test_benchmark.c	\
eina_test_mempool.c	\
eina_test_rectangle.c	\
eina_test_list.c	\
eina_test_matrixsparse.c \
eina_test_tiler.c       \
eina_test_strbuf.c	\
eina_test_str.c		\
eina_test_quadtree.c	\
eina_test_simple_xml_parser.c

eina_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/libeina.la -lm

module_dummydir = $(libdir)/eina/test
module_dummy_LTLIBRARIES = module_dummy.la

module_dummy_la_SOURCES = \
eina_test_module_dummy.c

module_dummy_la_CPPFLAGS = 			\
-I$(top_srcdir)/src/lib 			\
-I$(top_srcdir)/src/include 			\
-I$(top_builddir)/src/include 			\
-I$(top_builddir)/src/lib 			\
@EFL_EINA_BUILD@
module_dummy_la_LIBADD       = $(top_builddir)/src/lib/libeina.la @EINA_LIBS@
module_dummy_la_LDFLAGS      = -no-undefined @lt_enable_auto_import@ -module -avoid-version
module_dummy_la_LIBTOOLFLAGS = --tag=disable-static

endif

if EFL_ENABLE_BENCHMARK

bench_PROGRAMS = eina_bench

eina_bench_SOURCES = 	\
eina_bench.c 		\
eina_bench_sort.c	\
eina_bench_hash.c	\
eina_bench_stringshare.c \
eina_bench_convert.c	\
eina_bench_mempool.c	\
eina_bench_stringshare_e17.c \
eina_bench_array.c	\
eina_bench_rectangle_pool.c \
ecore_list.c \
ecore_strings.c \
ecore_hash.c \
ecore_sheap.c \
evas_hash.c \
evas_list.c \
evas_mempool.c \
evas_object_list.c \
evas_stringshare.c \
eina_bench_quad.c

if CITYHASH_BENCH
noinst_LTLIBRARIES = libcity.la
libcity_la_SOURCES = city.cc

nodist_EXTRA_eina_bench_SOURCES = dummy.cc
eina_bench_LDADD = @GLIB_LIBS@ $(top_builddir)/src/lib/libeina.la libcity.la
else
eina_bench_LDADD = @GLIB_LIBS@ $(top_builddir)/src/lib/libeina.la

endif



endif

EXTRA_DIST = eina_bench.h \
	eina_suite.h \
	Ecore_Data.h \
	Evas_Data.h \
	evas_mempool.h \
	strlog

clean-local:
	rm -rf *.gcno *.gcda
