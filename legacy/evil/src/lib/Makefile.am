
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = . dlfcn

lib_LTLIBRARIES = libevil.la

include_HEADERS = \
Evil.h \
evil_fcntl.h \
evil_langinfo.h \
evil_stdlib.h \
evil_unistd.h \
evil_util.h

nobase_include_HEADERS = fnmatch.h pwd.h sys/mman.h

libevil_la_SOURCES = \
evil.c \
evil_fcntl.c \
evil_fnmatch.c \
evil_fnmatch_list_of_states.c \
evil_langinfo.c \
evil_mman.c \
evil_pwd.c \
evil_stdlib.c \
evil_unistd.c \
evil_util.c

libevil_la_CPPFLAGS = -DEFL_EVIL_BUILD -D_WIN32_WCE=0x0420
libevil_la_CFLAGS = @win32_cflags@
libevil_la_LIBADD = @win32_libs@ $(EFL_MPATROL_LIBS)
libevil_la_LDFLAGS = -no-undefined -Wl,--enable-auto-import -version-info @version_info@
