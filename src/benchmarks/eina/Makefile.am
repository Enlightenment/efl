MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-DPACKAGE_BUILD_DIR=\"$(abs_top_builddir)\" \
-DCITYHASH_BENCH \
-DEINA_ENABLE_BENCH_E17 \
@EINA_CFLAGS@ \
@GLIB_CFLAGS@

EXTRA_PROGRAMS = eina_bench

benchmark: eina_bench libcity.la

eina_bench_SOURCES = \
eina_bench.c \
eina_bench_sort.c \
eina_bench_hash.c \
eina_bench_stringshare.c \
eina_bench_convert.c \
eina_bench_mempool.c \
eina_bench_stringshare_e17.c \
eina_bench_array.c \
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
eina_bench_quad.c \
eina_bench.h \
Ecore_Data.h \
Evas_Data.h \
evas_mempool.h

EXTRA_LTLIBRARIES = libcity.la
libcity_la_SOURCES = city.cc city.h

nodist_EXTRA_eina_bench_SOURCES = dummy.cc
eina_bench_LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
libcity.la \
@GLIB_LIBS@ \
@EINA_LDFLAGS@

EXTRA_DIST = strlog

clean-local:
	rm -rf *.gcno ..\#..\#src\#*.gcov *.gcda

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
