MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @GLIB_CFLAGS@ @EVIL_CFLAGS@ @EINA_CFLAGS@ @WIN32_CPPFLAGS@ @EFL_ECORE_BUILD@
AM_CFLAGS = @WIN32_CFLAGS@ @EFL_PTHREAD_CFLAGS@

lib_LTLIBRARIES = libecore.la
includes_HEADERS = \
Ecore.h \
Ecore_Getopt.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_la_SOURCES = \
ecore.c \
ecore_anim.c \
ecore_app.c \
ecore_events.c \
ecore_getopt.c \
ecore_idle_enterer.c \
ecore_idle_exiter.c \
ecore_idler.c \
ecore_job.c \
ecore_main.c \
ecore_pipe.c \
ecore_poll.c \
ecore_time.c \
ecore_timer.c \
ecore_thread.c \
ecore_glib.c \
ecore_throttle.c

if ECORE_HAVE_WIN32

libecore_la_SOURCES += ecore_exe_win32.c

else

if ECORE_HAVE_WINCE

libecore_la_SOURCES += ecore_exe_wince.c

else

libecore_la_SOURCES += ecore_signal.c ecore_exe.c

endif

endif

libecore_la_LIBADD = @dlopen_libs@ @EINA_LIBS@ @EVIL_LIBS@ @GLIB_LIBS@ @WIN32_LIBS@ @LTLIBINTL@ @EFL_PTHREAD_LIBS@ @rt_libs@ -lm
libecore_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@ @EFL_PTHREAD_LIBS@

EXTRA_DIST = ecore_private.h
