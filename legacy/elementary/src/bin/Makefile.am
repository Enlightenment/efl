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

bin_PROGRAMS = elementary_test elementary_quicklaunch elementary_run \
elementary_testql

elementary_test_SOURCES = test.c
elementary_test_LDADD = $(top_builddir)/src/lib/libelementary.la
elementary_test_LDFLAGS =

elementary_quicklaunch_SOURCES = quicklaunch.c
elementary_quicklaunch_LDADD = $(top_builddir)/src/lib/libelementary.la
elementary_quicklaunch_LDFLAGS = 

elementary_run_SOURCES = run.c
elementary_run_LDADD = 
elementary_run_LDFLAGS =

## This is how to build a quicklanch capable app
# build the shared lib version - libtool produces a .a and .la file as well
# as a .so - these get put in libdir ($PREIFX/lib) as elementary_testql.so
# etc. - this is where elementary will expect to find the .so's for
# quicklaunch apps.
elementary_testqldir = $(libdir)
elementary_testql_LTLIBRARIES = elementary_testql.la
elementary_testql_la_SOURCES = test.c
elementary_testql_la_LIBADD = $(top_builddir)/src/lib/libelementary.la
elementary_testql_la_CFLAGS = 
elementary_testql_la_LDFLAGS = -module -avoid-version -no-undefined
elementary_testql_SOURCES = test.c
elementary_testql_LDADD = $(top_builddir)/src/lib/libelementary.la
elementary_testql_CFLAGS = -DELM_LIB_QUICKLAUNCH=1
elementary_testql_LDFLAGS =
