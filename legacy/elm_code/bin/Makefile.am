MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = elm_code_test

AM_CPPFLAGS = -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-I$(top_builddir)/elm_code/bin/ \
-I$(top_srcdir)/elm_code/bin/ \
-I$(top_builddir)/elm_code/lib/ \
-I$(top_srcdir)/elm_code/lib/ \
@EFL_CFLAGS@

elm_code_test_SOURCES = elm_code_test_main.c
elm_code_test_LDADD = @EFL_LIBS@ $(top_builddir)/elm_code/lib/libelm_code.la

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

EXTRA_DIST = elm_code_test_private.h

