EOLIAN_FLAGS = -I$(srcdir)

include ../../Makefile_Eolian_Subbuild_Helper.am

### Library

EIO_EOS = eio_model.eo \
          eio_job.eo \
          eio_sentry.eo

EIO_EOS_H =  $(EIO_EOS:%.eo=%.eo.h)
EIO_EOS_C =  $(EIO_EOS:%.eo=%.eo.c)

BUILT_SOURCES = $(EIO_EOS_C) $(EIO_EOS_H)

eioeolianfilesdir = $(datadir)/eolian/include/eio-@VMAJ@
eioeolianfiles_DATA = $(EIO_EOS)

lib_LTLIBRARIES = libeio.la

installed_eiomainheadersdir = $(includedir)/eio-@VMAJ@

dist_installed_eiomainheaders_DATA = Eio.h eio_inline_helper.x
nodist_installed_eiomainheaders_DATA = $(EIO_EOS_H)

libeio_la_SOURCES = \
eio_dir.c \
eio_eet.c \
eio_file.c \
eio_main.c \
eio_map.c \
eio_monitor.c \
eio_monitor_poll.c \
eio_single.c \
eio_xattr.c \
eio_job.c \
eio_sentry.c \
eio_model.c \
eio_model_private.h \
eio_private.h

if HAVE_INOTIFY
libeio_la_SOURCES += eio_monitor_inotify.c
else
if HAVE_NOTIFY_WIN32
libeio_la_SOURCES += eio_monitor_win32.c
else
if HAVE_NOTIFY_COCOA
libeio_la_SOURCES += eio_monitor_cocoa.c
else
if HAVE_NOTIFY_KEVENT
libeio_la_SOURCES += eio_monitor_kevent.c
endif
endif
endif
endif

libeio_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl @EIO_CFLAGS@
libeio_la_LIBADD = @EIO_SUBBUILD_LIBS@
libeio_la_DEPENDENCIES = @EIO_SUBBUILD_INTERNAL_LIBS@
libeio_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@
if HAVE_NOTIFY_COCOA
libeio_la_LDFLAGS += -framework CoreServices
endif
