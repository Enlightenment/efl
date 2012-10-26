MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eet \
-I$(top_srcdir)/src/lib/eet/lz4 \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eet \
-I$(top_builddir)/src/lib/eet/lz4 \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" \
-DEFL_EET_BUILD \
@EFL_CFLAGS@ \
@EET_CFLAGS@

if HAVE_WINDOWS
AM_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil
endif

includes_HEADERS = Eet.h
includesdir = $(includedir)/eet-@VMAJ@

lib_LTLIBRARIES = libeet.la

libeet_la_SOURCES = \
Eet_private.h \
eet_alloc.c \
eet_cipher.c \
eet_connection.c \
eet_data.c \
eet_dictionary.c \
eet_image.c \
eet_lib.c \
eet_node.c \
eet_utils.c \
lz4/lz4.c \
lz4/lz4.h \
lz4/lz4hc.c \
lz4/lz4hc.h

libeet_la_LIBADD = $(top_builddir)/src/lib/eina/libeina.la

if HAVE_WINDOWS
libeet_la_LIBADD += $(top_builddir)/src/lib/evil/libevil.la
endif

libeet_la_LIBADD += @EET_LIBS@ -lm

libeet_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

EXTRA_DIST = lz4/README

clean-local:
	@rm -rf *.gcno
