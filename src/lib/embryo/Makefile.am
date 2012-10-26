
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DEFL_EMBRYO_BUILD \
@EFL_CFLAGS@

if HAVE_WINDOWS
AM_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil
endif

includes_HEADERS = Embryo.h
includesdir = $(includedir)/embryo-@VMAJ@

lib_LTLIBRARIES = libembryo.la

libembryo_la_SOURCES  = \
embryo_amx.c \
embryo_args.c \
embryo_float.c \
embryo_main.c \
embryo_rand.c \
embryo_str.c \
embryo_time.c \
embryo_private.h

libembryo_la_LIBADD = $(top_builddir)/src/lib/eina/libeina.la -lm
if HAVE_WINDOWS
libembryo_la_LIBADD += $(top_builddir)/src/lib/evil/libevil.la
endif
libembryo_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@
