### Library

lib_LTLIBRARIES = libeet.la

installed_eetmainheadersdir = $(includedir)/eet-@VMAJ@
dist_installed_eetmainheaders_DATA = Eet.h

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
../../static_libs/rg_etc/rg_etc1.c \
../../static_libs/rg_etc/rg_etc2.c \
../../static_libs/rg_etc/etc2_encoder.c \
../../static_libs/rg_etc/rg_etc1.h

libeet_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/static_libs/rg_etc \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/eet\" \
@EET_CFLAGS@ \
@NEON_CFLAGS@
libeet_la_LIBADD = @EET_SUBBUILD_LIBS@ @EVIL_SUBBUILD_LIBS@
libeet_la_DEPENDENCIES = @EET_SUBBUILD_INTERNAL_LIBS@
libeet_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@
