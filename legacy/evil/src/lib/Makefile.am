
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = . dlfcn

lib_LTLIBRARIES = libevil.la

install_evilheadersdir = $(includedir)/evil-@VMAJ@
dist_install_evilheaders_DATA = \
Evil.h \
evil_fcntl.h \
evil_inet.h \
evil_langinfo.h \
evil_libgen.h \
evil_main.h \
evil_stdlib.h \
evil_stdio.h \
evil_string.h \
evil_time.h \
evil_unistd.h \
evil_util.h

nobase_stdheadersdir = $(includedir)/evil-@VMAJ@
dist_nobase_stdheaders_DATA = pwd.h sys/mman.h fnmatch.h dirent.h

if EVIL_HAVE_WINCE

dist_nobase_stdheaders_DATA += errno.h

endif

libevil_la_SOURCES = \
evil_dirent.c \
evil_fcntl.c \
evil_fnmatch.c \
evil_fnmatch_list_of_states.c \
evil_inet.c \
evil_langinfo.c \
evil_libgen.c \
evil_main.c \
evil_mman.c \
evil_pwd.c \
evil_stdlib.c \
evil_stdio.c \
evil_string.c \
evil_time.c \
evil_unistd.c \
evil_util.c \
evil_uuid.c \
evil_private.h \
evil_fnmatch_private.h

if EVIL_HAVE_WINCE

libevil_la_SOURCES += evil_errno.c evil_link_ce.c

else

libevil_la_SOURCES += evil_link_xp.cpp

endif

libevil_la_CPPFLAGS = @win32_cppflags@
libevil_la_CFLAGS = @win32_cflags@
libevil_la_CXXFLAGS = @win32_cxxflags@
libevil_la_LIBADD = @win32_libs@ $(EFL_MPATROL_LIBS)
libevil_la_LDFLAGS = -no-undefined -Wl,--enable-auto-import -version-info @version_info@ 

if EVIL_HAVE_WINCE

libevil_la_LINK = $(LINK) $(libevil_la_LDFLAGS)

else

libevil_la_LINK = $(CXXLINK) $(libevil_la_LDFLAGS)

endif
