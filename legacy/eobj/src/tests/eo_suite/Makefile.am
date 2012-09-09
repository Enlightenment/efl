
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@ \
@CHECK_CFLAGS@

if EO_BUILD_UNIT_TEST

check_PROGRAMS = eo_suite

eo_suite_SOURCES = \
class_simple.c \
class_simple.h \
eo_suite.c \
eo_suite.h \
eo_test_class_errors.c \
eo_test_general.c \
eo_test_init.c

eo_suite_LDADD = $(top_builddir)/src/lib/libeo.la @EO_LIBS@ @CHECK_LIBS@

endif
