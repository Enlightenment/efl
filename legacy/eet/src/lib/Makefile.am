
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@ \
@EET_CPPFLAGS@ \
@EFL_EET_BUILD@ \
@EFL_COVERAGE_CFLAGS@ \
@OPENSSL_CFLAGS@ \
@GNUTLS_CFLAGS@

include_HEADERS = Eet.h

lib_LTLIBRARIES = libeet.la

libeet_la_SOURCES  = \
eet_lib.c \
eet_data.c \
eet_image.c \
eet_cipher.c \
eet_dictionary.c \
eet_utils.c

libeet_la_CFLAGS = @EET_CFLAGS@ @DEBUG_CFLAGS@
libeet_la_LIBADD = @GNUTLS_LIBS@ @OPENSSL_LIBS@ @EFL_COVERAGE_LIBS@ @EET_LIBS@ @EINA_LIBS@ @EVIL_LIBS@ -lz -ljpeg -lm
libeet_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@

EXTRA_DIST = Eet_private.h

clean-local:
	@rm -rf *.gcno
