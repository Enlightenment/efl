MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @EEZE_CFLAGS@

if HAVE_EEZE_MOUNT
  AM_CFLAGS = @EEZE_CFLAGS@ @LIBMOUNT_CFLAGS@ @ECORE_FILE_CFLAGS@
  MOUNT_FILES = eeze_disk.c eeze_disk_udev.c eeze_disk_mount.c
if OLD_LIBMOUNT
  MOUNT_FILES += eeze_disk_libmount_old.c
else
  MOUNT_FILES += eeze_disk_libmount.c
endif
  MOUNT_INCLUDES = Eeze_Disk.h
else
  AM_CFLAGS = @EEZE_CFLAGS@
  MOUNT_FILES = 
  MOUNT_INCLUDES =
endif

lib_LTLIBRARIES = libeeze.la
includes_HEADERS = Eeze.h $(MOUNT_INCLUDES)
includesdir = $(includedir)/eeze-@VMAJ@

libeeze_la_SOURCES = \
eeze_main.c \
eeze_udev_find.c \
eeze_udev_private.h \
eeze_udev_private.c \
eeze_udev_syspath.c \
eeze_udev_walk.c \
eeze_udev_watch.c \
$(MOUNT_FILES)

if HAVE_EEZE_MOUNT
  libeeze_la_LIBADD = @EEZE_LIBS@ @LIBMOUNT_LIBS@ @ECORE_FILE_LIBS@
else
  libeeze_la_LIBADD = @EEZE_LIBS@
endif
libeeze_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

EXTRA_DIST = eeze_disk.c eeze_disk_udev.c eeze_disk_mount.c \
	eeze_disk_libmount_old.c \
	eeze_disk_libmount.c \
	Eeze_Disk.h \
	eeze_disk_private.h
