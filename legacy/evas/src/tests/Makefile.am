MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include \
-I$(top_builddir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@ \
@CHECK_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = evas_suite

evas_suite_SOURCES = 	\
evas_suite.c \
evas_test_init.c \
evas_suite.h

evas_suite_LDADD = @CHECK_LIBS@ $(top_builddir)/src/lib/libevas.la @EINA_LIBS@

endif
