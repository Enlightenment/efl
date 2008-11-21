
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/dlfcn \
@win32_cppflags@

AM_CFLAGS = @win32_cflags@

bin_PROGRAMS = evil_suite test_dlfcn test_pipe test_evil

evil_suite_SOURCES = \
evil_suite.c \
evil_test_environment.c \
evil_test_gettimeofday.c \
evil_test_link.c \
evil_test_memcpy.c

if EVIL_HAVE_WINCE

evil_suite_SOURCES +=  memcpy_glibc_arm.S

#else

#suite_SOURCES +=  memcpy_glibc_i686.S

endif

evil_suite_LDADD = $(top_builddir)/src/lib/libevil.la
evil_suite_LDFLAGS = -Wl,--enable-auto-import

test_dlfcn_SOURCES = test_dlfcn.c
test_dlfcn_LDADD = $(top_builddir)/src/lib/libevil.la $(top_builddir)/src/lib/dlfcn/libdl.la
test_dlfcn_LDFLAGS = -Wl,--enable-auto-import

test_pipe_SOURCES = test_pipe.c
test_pipe_LDADD = $(top_builddir)/src/lib/libevil.la
test_pipe_LDFLAGS = -Wl,--enable-auto-import

test_evil_SOURCES = test_evil.c
test_evil_LDADD = $(top_builddir)/src/lib/libevil.la
test_evil_LDFLAGS = -Wl,--enable-auto-import

EXTRA_DIST = \
evil_suite.h \
evil_test_environment.h \
evil_test_link.h \
evil_test_memcpy.h
