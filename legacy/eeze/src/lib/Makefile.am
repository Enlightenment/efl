MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @EEZE_CFLAGS@ \
-DPACKAGE_LIB_DIR=\"$(libdir)\"

includes_HEADERS = Eeze.h Eeze_Net.h Eeze_Sensor.h

libeeze_la_SOURCES = \
eeze_main.c \
eeze_net.c \
eeze_net_private.h \
eeze_udev_find.c \
eeze_udev_private.h \
eeze_udev_private.c \
eeze_udev_syspath.c \
eeze_udev_walk.c \
eeze_udev_watch.c \
eeze_sensor.c

if HAVE_EEZE_MOUNT
  AM_CFLAGS = @EEZE_CFLAGS@ @LIBMOUNT_CFLAGS@ @ECORE_FILE_CFLAGS@
  libeeze_la_SOURCES += eeze_disk.c eeze_disk_udev.c eeze_disk_mount.c eeze_disk_private.h
if OLD_LIBMOUNT
  libeeze_la_SOURCES += eeze_disk_libmount_old.c
else
if NEW_LIBMOUNT
  libeeze_la_SOURCES += eeze_disk_libmount_new.c
else
  libeeze_la_SOURCES += eeze_disk_libmount.c
endif
endif
  includes_HEADERS += Eeze_Disk.h
else
  AM_CFLAGS = @EEZE_CFLAGS@
endif

lib_LTLIBRARIES = libeeze.la
includesdir = $(includedir)/eeze-@VMAJ@

libeeze_la_LIBADD = @EEZE_LIBS@

if HAVE_EEZE_MOUNT
  libeeze_la_LIBADD += @LIBMOUNT_LIBS@ @ECORE_FILE_LIBS@
endif

libeeze_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

