MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eet \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eet \
-DTESTS_SRC_DIR=\"$(top_srcdir)/src/tests/eet\" \
@CHECK_CFLAGS@ \
@EET_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = eet_suite

eet_suite_SOURCES = eet_suite.c eet_data_suite.c eet_suite.h
eet_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/eet/libeet.la $(top_builddir)/src/lib/eina/libeina.la @EET_LIBS@

endif

EXTRA_DIST = cert.pem key_enc_none.pem key_enc.pem key.pem
