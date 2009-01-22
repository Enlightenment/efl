AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/bin \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_FB_CFLAGS@ \
@ELEMENTARY_WIN32_CFLAGS@ \
@ELEMENTARY_WINCE_CFLAGS@ \
@ELEMENTARY_EDBUS_CFLAGS@

if ELEMENTARY_WINDOWS_BUILD
AM_CPPFLAGS += -DELEMENTARY_BUILD
endif

bin_PROGRAMS = elementary_test

elementary_test_SOURCES = test.c
elementary_test_LDADD = $(top_builddir)/src/lib/libelementary.la
elementary_test_LDFLAGS = 
