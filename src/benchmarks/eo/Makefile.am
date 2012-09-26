
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/include/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/include/eina \
-I$(top_builddir)/src/lib/eo \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

EXTRA_PROGRAMS = eo_bench

benchmark: eo_bench

eo_bench_SOURCES = \
class_simple.c \
class_simple.h \
eo_bench.c \
eo_bench.h \
eo_bench_eo_do.c

eo_bench_LDADD = \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eina/libeina.la \
@EO_LIBS@

clean-local:
	rm -rf *.gcno ..\#..\#src\#*.gcov *.gcda
