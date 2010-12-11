MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = @EEZE_UDEV_TEST_PRG@
EXTRA_PROGRAMS = eeze_udev_test

eeze_udev_test_SOURCES = eeze_udev_test.c
eeze_udev_test_CPPFLAGS = -I$(top_srcdir)/src/lib @EEZE_CFLAGS@
eeze_udev_test_LDADD = $(top_builddir)/src/lib/libeeze.la @EEZE_LIBS@
