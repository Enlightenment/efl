MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

if BUILD_EXAMPLES

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@pthread_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

pkglib_PROGRAMS =

if BUILD_ENGINE_BUFFER
AM_CPPFLAGS += -I$(top_srcdir)/src/modules/engines/buffer

pkglib_PROGRAMS += evas_buffer_simple
evas_buffer_simple_SOURCES = evas-buffer-simple.c
evas_buffer_simple_LDADD = $(top_builddir)/src/lib/libevas.la
endif

endif



filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

if INSTALL_EXAMPLES
files_DATA += \
	evas-buffer-simple.c
endif
