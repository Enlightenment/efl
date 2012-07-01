MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-DTESTS_SRC_DIR=\"$(top_srcdir)/src/tests\" \
@CHECK_CFLAGS@ \
@EET_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = eet_suite

eet_suite_SOURCES = eet_suite.c eet_data_suite.c eet_suite.h
eet_suite_LDADD = $(top_builddir)/src/lib/libeet.la @EET_LIBS@ @CHECK_LIBS@

endif

EXTRA_DIST = cert.pem key_enc_none.pem key_enc.pem key.pem
