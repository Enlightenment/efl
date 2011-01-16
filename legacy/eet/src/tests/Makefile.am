MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
@CHECK_CFLAGS@ \
@EINA_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = eet_suite

eet_suite_SOURCES = eet_suite.c eet_data_suite.c
eet_suite_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@ @CHECK_LIBS@

endif

EXTRA_DIST = eet_suite.h cert.pem key_enc_none.pem key_enc.pem key.pem
