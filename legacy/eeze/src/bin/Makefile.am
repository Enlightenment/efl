MAINTAINERCLEANFILES = Makefile.in

EEZE_CFLAGS = \
-I$(top_srcdir)/src/lib \
@EEZE_CFLAGS@

noinst_PROGRAMS = @EEZE_UDEV_TEST_PRG@
EXTRA_PROGRAMS = eeze_udev_test eeze_mount eeze_umount eeze_disk_ls eeze_scanner

if HAVE_EEZE_MOUNT
  DISK_PROGS = eeze_mount eeze_umount eeze_disk_ls eeze_scanner
else
  DISK_PROGS = 
endif

bin_PROGRAMS = $(DISK_PROGS)

eeze_udev_test_SOURCES = eeze_udev_test.c
eeze_udev_test_CPPFLAGS = -I$(top_srcdir)/src/lib @EEZE_CFLAGS@
eeze_udev_test_LDADD = $(top_builddir)/src/lib/libeeze.la @EEZE_LIBS@

if HAVE_EEZE_MOUNT
  eeze_mount_SOURCES = eeze_mount.c
  eeze_mount_CFLAGS = -I$(top_srcdir)/src/lib $(EEZE_CFLAGS) @LIBMOUNT_CFLAGS@ @ECORE_FILE_CFLAGS@
  eeze_mount_LDADD = $(top_builddir)/src/lib/libeeze.la @LIBMOUNT_LIBS@ @ECORE_FILE_LIBS@

  eeze_umount_SOURCES = eeze_umount.c
  eeze_umount_CFLAGS = -I$(top_srcdir)/src/lib $(EEZE_CFLAGS) @LIBMOUNT_CFLAGS@ @ECORE_FILE_CFLAGS@
  eeze_umount_LDADD = $(top_builddir)/src/lib/libeeze.la @LIBMOUNT_LIBS@ @ECORE_FILE_LIBS@

  eeze_disk_ls_SOURCES = eeze_disk_ls.c
  eeze_disk_ls_CFLAGS = -I$(top_srcdir)/src/lib $(EEZE_CFLAGS) @LIBMOUNT_CFLAGS@ @ECORE_FILE_CFLAGS@
  eeze_disk_ls_LDADD = $(top_builddir)/src/lib/libeeze.la @LIBMOUNT_LIBS@ @ECORE_FILE_LIBS@

  eeze_scanner_SOURCES = eeze_scanner.c
  eeze_scanner_CFLAGS = -I$(top_srcdir)/src/lib $(EEZE_CFLAGS) @LIBMOUNT_CFLAGS@ @ECORE_FILE_CFLAGS@ @ECORE_CON_CFLAGS@ @EET_CFLAGS@
  eeze_scanner_LDADD = $(top_builddir)/src/lib/libeeze.la @LIBMOUNT_LIBS@ @ECORE_FILE_LIBS@ @ECORE_CON_LIBS@ @EET_LIBS@
  includesdir = $(includedir)/eeze-@VMAJ@
  includes_HEADERS = eeze_scanner.h
endif
