MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = elm_code_test

AM_CPPFLAGS = \
-I$(top_srcdir)/elm_code/lib/ \
-DLOCALEDIR=\"$(datadir)/locale\" \
-DEFL_BETA_API_SUPPORT \
@EFL_CFLAGS@

elm_code_test_SOURCES = \
elm_code_test_main.c \
elm_code_test_private.h

elm_code_test_LDADD = @EFL_LIBS@ $(top_builddir)/elm_code/lib/libelm_code.la

elm_code_test_LDFLAGS = @EFL_LTLIBRARY_FLAGS@

