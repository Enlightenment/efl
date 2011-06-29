MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

#put here all EDCs one needs to the tests
EDCS = aspect.edc

filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

if BUILD_EXAMPLES

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DPACKAGE_EXAMPLES_DIR=\"$(datadir)/$(PACKAGE)/examples\" \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
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

pkglib_PROGRAMS += evas_init_shutdown
evas_init_shutdown_SOURCES = evas-init-shutdown.c
evas_init_shutdown_LDADD = $(top_builddir)/src/lib/libevas.la

#the ones using ecore_evas follow
AM_CPPFLAGS += @ECORE_EVAS_CFLAGS@

pkglib_PROGRAMS += evas_load_error_str
evas_load_error_str_SOURCES = evas-load-error-str.c
evas_load_error_str_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_events
evas_events_SOURCES = evas-events.c
evas_events_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_object_manipulation
evas_object_manipulation_SOURCES = evas-object-manipulation.c
evas_object_manipulation_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_hints
evas_hints_SOURCES = evas-hints.c
evas_hints_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_stacking
evas_stacking_SOURCES = evas-stacking.c
evas_stacking_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

#the ones using ecore_evas and edje follow
AM_CPPFLAGS += @EDJE_CFLAGS@

pkglib_PROGRAMS += evas_aspect_hints
evas_aspect_hints_DEPS = $(srcdir)/aspect.edc
evas_aspect_hints_SOURCES = evas-aspect-hints.c
evas_aspect_hints_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@ @EDJE_LIBS@

aspect.edj: ${evas_aspect_hints_DEPS}

.edc.edj:
	$(edje_cc) -v -id $(srcdir) $< $(builddir)/$(@F)

EDJS = $(EDCS:%.edc=%.edj)

files_DATA += $(EDJS)

endif # if BUILD_EXAMPLES

if INSTALL_EXAMPLES

#put here additional data when installing examples
files_DATA += $(srcdir)/enlightenment.png \
	$(srcdir)/red.png

files_DATA += \
	$(EDCS) \
	$(srcdir)/evas-buffer-simple.c \
	$(srcdir)/evas-init-shutdown.c \
	$(srcdir)/evas-load-error-str.c \
	$(srcdir)/evas-object-manipulation.c \
	$(srcdir)/evas-events.c \
	$(srcdir)/evas-aspect-hints.c \
	$(srcdir)/evas-hints.c \
	$(srcdir)/evas-stacking.c
endif

EXTRA_DIST = $(EDCS) \
	$(srcdir)/evas-buffer-simple.c \
	$(srcdir)/evas-init-shutdown.c \
	$(srcdir)/evas-load-error-str.c \
	$(srcdir)/evas-object-manipulation.c \
	$(srcdir)/evas-events.c \
	$(srcdir)/evas-aspect-hints.c \
	$(srcdir)/evas-hints.c \
	$(srcdir)/evas-stacking.c \
	$(srcdir)/enlightenment.png \
	$(srcdir)/red.png
