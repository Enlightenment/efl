
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = . dlfcn

lib_LTLIBRARIES = libevil.la

include_HEADERS = \
Evil.h \
evil_fcntl.h \
evil_langinfo.h \
evil_stdlib.h \
evil_stdio.h \
evil_string.h \
evil_unistd.h \
evil_util.h

nobase_include_HEADERS = pwd.h sys/mman.h

if EVIL_HAVE_MINGW32CE

nobase_include_HEADERS += fnmatch.h errno.h

endif

libevil_la_SOURCES = \
evil_errno.c \
evil_fcntl.c \
evil_fnmatch.c \
evil_fnmatch_list_of_states.c \
evil_langinfo.c \
evil_mman.c \
evil_pwd.c \
evil_stdlib.c \
evil_stdio.c \
evil_string.c \
evil_time.c \
evil_unistd.c \
evil_util.c

libevil_la_CPPFLAGS = -DEFL_EVIL_BUILD

if EVIL_HAVE_MINGW32CE

libevil_la_CPPFLAGS += -D_WIN32_WCE=0x0420

else

libevil_la_CPPFLAGS += -D_WIN32_WINNT=0x0500

endif

libevil_la_CFLAGS = @win32_cflags@
libevil_la_LIBADD = @win32_libs@ $(EFL_MPATROL_LIBS)
libevil_la_LDFLAGS = -no-undefined -Wl,--enable-auto-import -version-info @version_info@

EXTRA_DIST = evil_fnmatch_private.h
