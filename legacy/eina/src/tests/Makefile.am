MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/include \
@CHECK_CFLAGS@


if EINA_ENABLE_TESTS

check_PROGRAMS = eina_suite

eina_suite_SOURCES = \
eina_suite.c \
eina_test_stringshare.c \
eina_test_array.c \
eina_test_error.c \
eina_test_magic.c
eina_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/libeina.la

EXTRA_DIST = eina_suite.h

endif


