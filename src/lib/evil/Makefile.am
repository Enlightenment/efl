if HAVE_WINDOWS

### Library

lib_LTLIBRARIES = libevil.la

install_evilheadersdir = $(includedir)/evil-@VMAJ@
dist_install_evilheaders_DATA = \
Evil.h \
evil_dlfcn.h \
evil_fcntl.h \
evil_inet.h \
evil_langinfo.h \
evil_locale.h \
evil_macro.h \
evil_macro_pop.h \
evil_macro_wrapper.h \
evil_main.h \
evil_stdio.h \
evil_stdlib.h \
evil_string.h \
evil_time.h \
evil_unistd.h \
evil_util.h \
dirent.h \
fnmatch.h \
pwd.h

evilmmanheadersdir = $(includedir)/evil-@VMAJ@/sys
dist_evilmmanheaders_DATA = \
sys/mman.h

libevil_la_SOURCES = \
evil_dirent.c \
evil_dlfcn.c \
evil_fcntl.c \
evil_fnmatch.c \
evil_fnmatch_list_of_states.c \
evil_inet.c \
evil_langinfo.c \
evil_locale.c \
evil_link_xp.cpp \
evil_main.c \
evil_mman.c \
evil_pwd.c \
evil_stdio.c \
evil_stdlib.c \
evil_string.c \
evil_time.c \
evil_unistd.c \
evil_util.c \
evil_private.h \
evil_fnmatch_private.h

libevil_la_CPPFLAGS = @EVIL_CPPFLAGS@
libevil_la_CFLAGS = @EVIL_CFLAGS@ @EVIL_CFLAGS_WRN@ -D__USE_MINGW_ANSI_STDIO
libevil_la_CXXFLAGS = @EVIL_CXXFLAGS@ @EVIL_CFLAGS@
libevil_la_LIBADD = @EVIL_SUBBUILD_LIBS@
libevil_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@

# regex

dist_install_evilheaders_DATA += \
regex/regex.h

libevil_la_SOURCES += \
regex/regcomp.c \
regex/regerror.c \
regex/regexec.c \
regex/regfree.c \
regex/cclass.h \
regex/cname.h \
regex/regex2.h \
regex/utils.h

libevil_la_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_srcdir)/src/lib/evil/regex \
-DPOSIX_MISTAKE

endif
