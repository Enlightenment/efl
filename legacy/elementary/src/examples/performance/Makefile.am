MAINTAINERCLEANFILES = Makefile.in

include ../../../Makefile_Elm_Helpers.am

examplesdir = $(pkgdatadir)/examples/performance
filesdir = $(pkgdatadir)/examples/performance
files_DATA =

AM_CPPFLAGS = \
-Wno-unused-parameter \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR="\"$(pkgdatadir)\"" \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@ELEMENTARY_CFLAGS@

LDADD = \
@ELEMENTARY_LIBS@ \
$(top_builddir)/src/lib/libelementary.la

SRCS = performance.c camera_light.c graphical.c

.edc.edj:
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

if EFL_BUILD_EXAMPLES
files_DATA += $(SRCS) layout.edc layout.edj

performance_SOURCES = \
	performance.c \
	camera_light.c \
	graphical.c

clean-local:
	rm -f *.edj

examples_PROGRAMS = \
	performance

endif

EXTRA_DIST = layout.edc backgroud.png target_texture.png
