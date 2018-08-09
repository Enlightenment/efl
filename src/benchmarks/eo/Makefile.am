
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
@EO_CFLAGS@

EXTRA_PROGRAMS = eo_bench

benchmark: eo_bench

eo_bench_SOURCES = \
class_simple.c \
class_simple.h \
eo_bench.c \
eo_bench.h \
eo_bench_callbacks.c \
eo_bench_eo_do.c \
eo_bench_eo_add.c

eo_bench_LDADD = \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eina/libeina.la \
@EO_LDFLAGS@ \
-lpthread

clean-local:
	rm -rf *.gcno ..\#..\#src\#*.gcov *.gcda

