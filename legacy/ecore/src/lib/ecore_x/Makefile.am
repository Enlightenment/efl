MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_X

SUBDIRS = xlib xcb

if BUILD_ECORE_X_XCB
DEP = xcb/libecore_x_xcb.la
else
DEP = xlib/libecore_x_xlib.la
endif

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
@EINA_CFLAGS@

lib_LTLIBRARIES = libecore_x.la

libecore_x_la_SOURCES =

libecore_x_la_LIBADD = $(DEP) @EINA_LIBS@
libecore_x_la_LDFLAGS = -version-info @version_info@ @release_info@
libecore_x_la_DEPENDENCIES = $(DEP)

includes_HEADERS = \
Ecore_X.h \
Ecore_X_Atoms.h \
Ecore_X_Cursor.h
includesdir = $(includedir)/ecore-@VMAJ@

endif

EXTRA_DIST = ecore_x_atoms_decl.h
