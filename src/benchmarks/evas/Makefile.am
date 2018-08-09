MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/modules/evas/engines/buffer \
-DPACKAGE_BUILD_DIR=\"$(abs_top_builddir)\" \
-DTESTS_SRC_DIR=\"$(abs_top_srcdir)/src/tests/evas\" \
@EVAS_CFLAGS@

EXTRA_PROGRAMS = evas_bench

benchmark: evas_bench

evas_bench_SOURCES = \
evas_bench.c \
evas_bench_loader.c \
evas_bench_saver.c \
evas_bench.h

nodist_EXTRA_evas_bench_SOURCES = dummy.cc
evas_bench_LDADD = \
$(top_builddir)/src/lib/evas/libevas.la \
$(top_builddir)/src/lib/eina/libeina.la \
@EVAS_LDFLAGS@

EXTRA_DIST =

clean-local:
	rm -rf *.gcno ..\#..\#src\#*.gcov *.gcda

