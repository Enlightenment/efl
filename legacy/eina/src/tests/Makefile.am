MAINTAINERCLEANFILES = Makefile.in

benchdir = $(bindir)

AM_CPPFLAGS = 					\
-I$(top_srcdir)/src/lib 			\
-I$(top_srcdir)/src/include 			\
-DPACKAGE_BIN_DIR=\"$(bindir)\" 		\
-DPACKAGE_LIB_DIR=\"$(libdir)\" 		\
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" 	\
-DPACKAGE_BUILD_DIR=\"`pwd`/$(top_builddir)\" 	\
@CHECK_CFLAGS@					\
@GLIB_CFLAGS@

if EINA_HAVE_GLIB

AM_CPPFLAGS += -DEINA_BENCH_HAVE_GLIB

endif

if EINA_ENABLE_TESTS

check_PROGRAMS = eina_suite

eina_suite_SOURCES = 	\
eina_suite.c 		\
eina_test_stringshare.c \
eina_test_array.c 	\
eina_test_error.c 	\
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
eina_test_list.c

eina_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/libeina.la

endif

if EINA_ENABLE_BENCH

bench_PROGRAMS = eina_bench

eina_bench_SOURCES = 	\
eina_bench.c 		\
eina_bench_hash.c 	\
eina_bench_stringshare.c \
eina_bench_convert.c	\
eina_bench_array.c

eina_bench_LDADD = @GLIB_LIBS@ $(top_builddir)/src/lib/libeina.la

endif

EXTRA_DIST = eina_bench.h eina_suite.h

