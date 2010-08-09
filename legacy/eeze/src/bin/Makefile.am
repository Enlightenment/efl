MAINTAINERCLEANFILES = Makefile.in

EEZE_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
@ECORE_CFLAGS@ \
@LIBUDEV_CFLAGS@

EEZE_UDEV_PROG = eeze_udev_test

bin_PROGRAMS = eeze_udev_test

eeze_udev_test_SOURCES = eeze_udev_test.c
eeze_udev_test_CPPFLAGS = $(EEZE_CPPFLAGS)
eeze_udev_test_LDADD = $(top_builddir)/src/lib/libeeze.la
