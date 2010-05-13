MAINTAINERCLEANFILES = Makefile.in

EEZE_CPPFLAGS = \
-I$(top_srcdir)/src/lib/udev \
@ECORE_CFLAGS@

LIBS = \
@ECORE_LIBS@

if BUILD_EUDEV
EUDEV_PROG = eeze_udev_test
endif

bin_PROGRAMS = \
$(EUDEV_PROG)

if BUILD_EUDEV
eeze_udev_test_SOURCES = eeze_udev_test.c
eeze_udev_test_CPPFLAGS = $(EEZE_CPPFLAGS)
eeze_udev_test_LDADD = $(top_builddir)/src/lib/udev/libeudev.la
endif
