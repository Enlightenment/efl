
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
@EVIL_CPPFLAGS@

AM_CFLAGS = @EVIL_CFLAGS@

bin_PROGRAMS = evil_suite test_evil

evil_suite_SOURCES = \
evil_suite.c \
evil_test_dlfcn.c \
evil_test_environment.c \
evil_test_gettimeofday.c \
evil_test_link.c \
evil_test_memcpy.c \
evil_test_mkstemp.c \
evil_test_pipe.c \
evil_test_print.c \
evil_test_realpath.c \
evil_test_util.c \
evil_suite.h \
evil_test_dlfcn.h \
evil_test_environment.h \
evil_test_gettimeofday.h \
evil_test_link.h \
evil_test_memcpy.h \
evil_test_mkstemp.h \
evil_test_pipe.h \
evil_test_print.h \
evil_test_realpath.h \
evil_test_util.h

if HAVE_WINCE

evil_suite_SOURCES +=  memcpy_glibc_arm.S

#else

#suite_SOURCES +=  memcpy_glibc_i686.S

endif

evil_suite_LDADD = $(top_builddir)/src/lib/libdl.la $(top_builddir)/src/lib/libevil.la -lm
evil_suite_LDFLAGS = -Wl,--enable-auto-import

test_evil_SOURCES = test_evil.c
test_evil_LDADD = $(top_builddir)/src/lib/libevil.la
test_evil_LDFLAGS = -Wl,--enable-auto-import
