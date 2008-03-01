
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\"

lib_LTLIBRARIES      = libeet.la
include_HEADERS      = Eet.h
libeet_la_SOURCES  = \
eet_lib.c \
eet_data.c \
eet_image.c \
eet_memfile.c \
eet_dictionary.c \
eet_utils.c \
Eet_private.h

libeet_la_LIBADD       = -lz -ljpeg @fnmatch_libs@ @winsock_libs@ -lm
libeet_la_DEPENDENCIES = $(top_builddir)/config.h
libeet_la_LDFLAGS      = @create_shared_lib@ -version-info @version_info@
