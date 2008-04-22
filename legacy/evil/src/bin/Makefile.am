
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/dlfcn

AM_CFLAGS = @win32_cflags@

bin_PROGRAMS = test_dlfcn test_pipe test_evil

test_dlfcn_SOURCES = test_dlfcn.c
test_dlfcn_LDADD = $(top_builddir)/src/lib/libevil.la $(top_builddir)/src/lib/dlfcn/libdl.la
test_dlfcn_LDFLAGS = @lt_enable_auto_import@
test_dlfcn_DEPENDENCIES = $(top_builddir)/src/lib/libevil.la $(top_builddir)/src/lib/dlfcn/libdl.la

test_pipe_SOURCES = test_pipe.c
test_pipe_LDADD = $(top_builddir)/src/lib/libevil.la
test_pipe_LDFLAGS = @lt_enable_auto_import@
test_pipe_DEPENDENCIES = $(top_builddir)/src/lib/libevil.la

test_evil_SOURCES = test_evil.c
test_evil_LDADD = $(top_builddir)/src/lib/libevil.la
test_evil_LDFLAGS = @lt_enable_auto_import@
test_evil_DEPENDENCIES = $(top_builddir)/src/lib/libevil.la
