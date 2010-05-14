MAINTAINERCLEANFILES = Makefile.in

EEZE_CPPFLAGS = \
-I$(top_srcdir)/src/lib/udev \
@ECORE_CFLAGS@

LIBS = \
@ECORE_LIBS@

if BUILD_EEZE_UDEV
EEZE_UDEV_PROG = eeze_udev_test
endif

bin_PROGRAMS = \
$(EEZE_UDEV_PROG)

if BUILD_EEZE_UDEV
eeze_udev_test_SOURCES = eeze_udev_test.c
eeze_udev_test_CPPFLAGS = $(EEZE_CPPFLAGS)
eeze_udev_test_LDADD = $(top_builddir)/src/lib/udev/libeeze_udev.la
endif
