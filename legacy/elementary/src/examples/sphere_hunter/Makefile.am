MAINTAINERCLEANFILES = Makefile.in

include ../../../Makefile_Elm_Helpers.am

examplesdir = $(pkgdatadir)/examples/sphere_hunter
filesdir = $(pkgdatadir)/examples/sphere_hunter
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

SRCS = evas_3d_sphere_hunter.c

.edc.edj:
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) $< $@

if EFL_BUILD_EXAMPLES
files_DATA += $(SRCS) sphere_hunter.edc sphere_hunter.edj

clean-local:
	rm -f *.edj

sphere_hunter_SOURCES = \
	evas_3d_sphere_hunter.c

examples_PROGRAMS = \
	sphere_hunter

endif

EXTRA_DIST = sphere_hunter.edc score.jpg EFL_victory.png EFL_defeat.png
